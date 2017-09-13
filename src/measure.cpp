#include "measure.h"
#include "math.h"

Measure::Measure(QObject *parent) : Chartable(parent)
{
    fftPower = 16;//65K 0.73Hz;
    _fftSize = pow(2, fftPower);

    workingData = (complex *)calloc(_fftSize, sizeof(complex));
    workingReferenceData = (complex *)calloc(_fftSize, sizeof(complex));
    workingImpulseData = (complex *)calloc(_fftSize, sizeof(complex));
    setAverage(1);
    alloc();

    dataComplex      = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * _fftSize);
    referenceComplex = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * _fftSize);
    impulseComplex   = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * _fftSize);

    dataPlan            = fftw_plan_dft_1d(_fftSize, dataComplex, dataComplex, FFTW_FORWARD, FFTW_ESTIMATE);
    referencePlan       = fftw_plan_dft_1d(_fftSize, referenceComplex, referenceComplex, FFTW_FORWARD, FFTW_ESTIMATE);
    impulsePlan         = fftw_plan_dft_1d(_fftSize, impulseComplex, impulseComplex, FFTW_BACKWARD, FFTW_ESTIMATE);

    QAudioDeviceInfo d = QAudioDeviceInfo::defaultInputDevice();
    foreach (int c, d.supportedChannelCounts()) {
        if (c > _chanelCount)
            _chanelCount = c;
    }

    format.setSampleRate(48000);
    format.setChannelCount(_chanelCount);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    audio = new QAudioInput(format, this);
    open(WriteOnly);
    audio->start(this);

    fft = new FFT(this);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(transform()));
    timer->start(80*2); //12.5 per sec
}
Measure::~Measure()
{
    if (timer->isActive())
        timer->stop();

    audio->stop();

    fftw_destroy_plan(dataPlan);
    fftw_free(dataComplex);
}
void Measure::setActive(bool active)
{
    Chartable::setActive(active);

    _level  = 0;
    _referenceLevel = 0;
    emit levelChanged();
    emit referenceLevelChanged();
}
void Measure::setDelay(int delay)
{
    _delay = delay;
    referenceStack->setSize(fftSize() + _delay);
}
void Measure::setAverage(int average)
{
    _setAverage = average;
}
void Measure::averageRealloc()
{
    if (_average == _setAverage)
        return;

    averageImpulseData  = new complex *[_setAverage];

    for (int i = 0; i < _setAverage; i ++) {
        averageImpulseData[i] = new complex[_fftSize];
    }

    averageData      = new fftw_complex*[_setAverage];
    averageReference = new fftw_complex*[_setAverage];

    for (int i = 0; i < _setAverage; i ++) {
        averageData[i]      = new fftw_complex[_fftSize];
        averageReference[i] = new fftw_complex[_fftSize];
    }

    //aply new value
    _average = _setAverage;
}
int Measure::sampleRate()
{
    return audio->format().sampleRate();
}
qint64 Measure::writeData(const char *data, qint64 len)
{
    Sample s;
    int currentChanel = 0;

    for (qint64 i = 0; i < len; i += 4, currentChanel++) {
        if (currentChanel == _chanelCount)
            currentChanel = 0;

        s.c[0] = data[i];
        s.c[1] = data[i + 1];
        s.c[2] = data[i + 2];
        s.c[3] = data[i + 3];

        if (currentChanel == _dataChanel) {
            dataStack->add((_polarity ? -1 * s.f : s.f));
        }

        if (currentChanel == _referenceChanel) {
            referenceStack->add(s.f);
        }
    }
    return len;
}
void Measure::transform()
{
    if (!_active)
        return;

    _level = _referenceLevel = 0.0;

    dataStack->reset();
    referenceStack->reset();

    for (int i = 0; i < _fftSize; i++) {

        workingData[i] = dataStack->current();
        workingReferenceData[i] = referenceStack->current();

        dataComplex[i][0] = dataStack->current();//real
        dataComplex[i][1] = 0.0;//imag

        referenceComplex[i][0] = referenceStack->current();//real
        referenceComplex[i][1] = 0.0;//imag

        if (workingData[i].real() > _level)
            _level = workingData[i].real();

        if (workingReferenceData[i].real() > _referenceLevel)
            _referenceLevel = workingReferenceData[i].real();

        dataStack->next();
        referenceStack->next();
    }

    fftw_execute(dataPlan);
    fftw_execute(referencePlan);

    for (int i = 0; i < _fftSize; i ++) {

        workingImpulseData[i] = workingData[i] / workingReferenceData[i];
        fft_divide(impulseComplex[i], dataComplex[i], referenceComplex[i]);

        if (i < _fftSize / 2) {
            data[i].frequency = (qreal)i * sampleRate() / (qreal)_fftSize;
            memcpy(data[i].data, dataComplex[i], sizeof(fftw_complex));
            memcpy(data[i].reference, referenceComplex[i], sizeof(fftw_complex));
        }
    }
    fftw_execute(impulsePlan);
    averaging();

    memcpy(referenceData, workingReferenceData, _fftSize *sizeof(complex));
    memcpy(impulseData, workingImpulseData, _fftSize *sizeof(complex));

    emit readyRead();
    emit levelChanged();
    emit referenceLevelChanged();
}
/**
 * @brief Measure::averaging
 * vector averaging
 * TODO: add polar averaging mode
 */
void Measure::averaging()
{
    averageRealloc();

    _avgcounter ++;
    if (_avgcounter >= _average) _avgcounter = 0;

    for (int i = 0; i < fftSize() ; i++) {

        averageData[_avgcounter][i][0]      = data[i].data[0];
        averageData[_avgcounter][i][1]      = data[i].data[1];
        averageReference[_avgcounter][i][0] = data[i].reference[0];
        averageReference[_avgcounter][i][1] = data[i].reference[1];

        averageImpulseData[_avgcounter][i] = impulseComplex[i][0];

        data[i].data[0] = 0.0;
        data[i].data[1] = 0.0;
        data[i].reference[0] = 0.0;
        data[i].reference[1] = 0.0;
        workingImpulseData[i] = 0.0;

        for (int j = 0; j < _average; j++) {
            data[i].data[0]      += averageData[j][i][0];
            data[i].data[1]      += averageData[j][i][1];
            data[i].reference[0] += averageReference[j][i][0];
            data[i].reference[1] += averageReference[j][i][1];

            workingImpulseData[i] += averageImpulseData[j][i];
        }

        data[i].module        = 20.0 * log10(fft_abs(data[i].data) / (_fftSize * _average));
        data[i].magnitude     = 20.0 * log10(fft_abs(data[i].data) / fft_abs(data[i].reference));
        data[i].phase         = fft_arg(data[i].data) - fft_arg(data[i].reference);
        while (std::abs(data[i].phase) > M_PI)
            data[i].phase -= 2 * (data[i].phase / std::abs(data[i].phase)) * M_PI;
        workingImpulseData[i] /= _average * _fftSize;
    }
}
QObject *Measure::store()
{
    Stored *store = new Stored(this);
    store->setSampleRate(sampleRate());
    store->build(this);

    return store;
}
