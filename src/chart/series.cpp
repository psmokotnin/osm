#include "series.h"

#include <cmath>
#include <complex>
#include <QPen>
#include <QPainter>
#include <gl.h>

using namespace Fftchart;

Series::Series(Source *source, Type *type, Axis *axisX, Axis *axisY, QQuickItem *parent)
    : PaintedItem(parent)
{
    setRenderTarget(FramebufferObject);

    _source = source;
    _type   = type;
    _axisX  = axisX;
    _axisY  = axisY;
    prepareConvert();
}
void Series::prepareConvert()
{
    if (_axisX->type() == AxisType::linear) {
        _xadd = _mm_set_ps1(-1 * _axisX->min());
        _xmul = _mm_set_ps1(width() / (_axisX->max() - _axisX->min()));
    } else {
        _xadd = _mm_set_ps1(-1 * std::log(_axisX->min()));
        _xmul = _mm_set_ps1(width() / std::log(_axisX->max() / _axisX->min()));
    }

    if (_axisY->type() == AxisType::linear) {
        _yadd = _mm_set_ps1(-1 * _axisY->min());
        _ymul = _mm_set_ps1(height() / (_axisY->max() - _axisY->min()));
    } else {
        throw std::invalid_argument("_axisY logarithmic scale.");
    }
}
void Series::paint(QPainter *painter)
{
    //TODO: call if only width or height were changed!
    prepareConvert();

    painter->beginNativePainting();

    //TODO: add multisampling, antialiasing and/or smooth shader

    QColor color = _source->color();
    glColor3f(color.redF(), color.greenF(), color.blueF());
    glLineWidth(2.0);

    switch (*_type) {
        //Type {RTA, Magnitude, Phase, Scope, Impulse};
        case RTA:
            if (pointsPerOctave() > 0) {
                bandBars();
            } else {
                paintLine(_source->size(), &Source::frequency, &Source::module);
            }
        break;

        case Magnitude:
            smoothLine(&Source::magnitude);
        break;

        case Phase:
            smoothLine(&Source::phase);
        break;

        //case Scope:
        case Impulse:
            paintLine(_source->impulseSize(), &Source::impulseTime, &Source::impulseValue);
        break;

        default:
        break;
    }

    painter->endNativePainting();
}
void Series::paintLine(int size, float (Source::*xFunc)(int) const, float (Source::*yFunc)(int) const)
{
    glBegin(GL_LINE_STRIP);
    for (int i = 1; i < size; i += 4) {
        x[0] = (_source->*xFunc)(i + 0);
        x[1] = (_source->*xFunc)(i + 1);
        x[2] = (_source->*xFunc)(i + 2);
        x[3] = (_source->*xFunc)(i + 3);

        y[0] = (_source->*yFunc)(i + 0);
        y[1] = (_source->*yFunc)(i + 1);
        y[2] = (_source->*yFunc)(i + 2);
        y[3] = (_source->*yFunc)(i + 3);

        convert4Vertexes(&x, &y);
        line4Vertexes(&x, &y);
    }
    glEnd();
}
void Series::convert4Vertexes(v4sf *x, v4sf *y) const
{
    if (_axisX->type() == AxisType::logarithmic) {
        *x = log_ps(*x);
    }
    *x = _mm_add_ps(*x, _xadd);
    *x = _mm_mul_ps(*x, _xmul);

    *y = _mm_add_ps(*y, _yadd);       // y -= min
    *y = _mm_mul_ps(*y, _ymul);       // y *= height() / (max - min)
}
void Series::line4Vertexes(v4sf *x, v4sf *y, unsigned int count) const
{
    for (unsigned int j = 0; j < count; j++) {
        if (j > 1 && (int)(*x)[j] == (int)(*x)[j-1])
            return;

        glVertex3f((*x)[j], (*y)[j], 0);
    }
}
void Series::draw4Bands(v4sf *x, v4sf *y, float *lastX, float width, unsigned int count) const noexcept
{
    for (unsigned int j = 0; j < count; j++) {
        if (j > 1 && (int)(*x)[j] == (int)(*x)[j-1])
            return;

        *lastX = std::min((*x)[j] - width, *lastX);
        glRectf(*lastX, (*y)[j], (*x)[j] + width, 0);
        *lastX = (*x)[j] + width;
    }
}
void Series::needUpdate()
{
    update();
}
void Series::smoothLine(float (Source::*valueFunc)(int) const)
{
    if (!pointsPerOctave()) {
        return paintLine(_source->size(), &Source::frequency, valueFunc);
    }
    constexpr float startFrequency = 24000 / 2048;//pow(2, 11);

    float frequencyFactor = pow(2, 1.0 / pointsPerOctave()), //TODO: cache this value
        bandStart = startFrequency,
        bandEnd   = bandStart * frequencyFactor,
        frequency,
        value = 0,
        lValue = 0, tvalue = 0, ltvalue = 0
        ;

    int c = _source->size(),
        count = 0;

    float a[4], f[4];
    int bCount = 0;
    bool bCollected = false;
    float splinePoint[4], currentPoint;
    bool phaseType;
    phaseType = (*_type == Phase);
    constexpr float D_PI = 2.0 * M_PI;

    glBegin(GL_LINE_STRIP);
    for (int i = 1; i < c; i++) {
        frequency = _source->frequency(i);
        if (frequency < bandStart) continue;
        while (frequency > bandEnd) {
            bandStart = bandEnd;
            bandEnd   = bandStart * frequencyFactor;
            if (count) {

                value /= count;

                if (bCollected) {
                    splinePoint[0]  = splinePoint[1];f[0] = f[1];
                    splinePoint[1]  = splinePoint[2];f[1] = f[2];
                    splinePoint[2]  = splinePoint[3];f[2] = f[3];
                }

                if (phaseType && bCount > 0) {
                    if (std::abs(splinePoint[bCount - 1] - value) > M_PI) {
                        value  -= std::copysign(D_PI, value);
                        lValue -= std::copysign(D_PI, value);
                    }
                }
                splinePoint[bCount] =  value;

                f[bCount] = frequency;
                value = 0;
                count = 0;

                if (bCount == 3) {
                    //make Spline function
                    a[0] = (     splinePoint[0] + 4 * splinePoint[1] +     splinePoint[2]) / 6;
                    a[1] = (-1 * splinePoint[0] +                          splinePoint[2]) / 2;
                    a[2] = (     splinePoint[0] - 2 * splinePoint[1] +     splinePoint[2]) / 2;
                    a[3] = (-1 * splinePoint[0] + 3 * splinePoint[1] - 3 * splinePoint[2] + splinePoint[3]) / 6;

                    //draw line from splinePoint[1] to splinePoint[2]
                    float xfs, xfe, yc, lyc, d;
                    xfs = (log(f[1]) + _xadd[0]) * _xmul[0];
                    xfe = (log(f[2]) + _xadd[0]) * _xmul[0];
                    d = 1 / (xfe - xfs);

                    for (float xc = xfs, t = 0; xc < xfe; ++xc, t += d) {
                        currentPoint = 0;
                        currentPoint = a[0] + a[1] * t + a[2] * pow(t, 2) + a[3] * pow(t, 3);

                        if (phaseType) {
                            yc = currentPoint;
                            while (yc >  M_PI) yc -= D_PI;
                            while (yc < -M_PI) yc += D_PI;
                            if (std::abs(yc - lyc) > M_PI) {
                                glEnd();
                                glBegin(GL_LINE_STRIP);
                            }
                            lyc = yc;

                            yc = (yc + _yadd[0]) * _ymul[0];
                        } else {
                            yc = (currentPoint + _yadd[0]) * _ymul[0];
                        }
                        glVertex3f(xc, yc, 0);
                    }

                    bCollected = true;
                } else {
                    ++bCount;
                }
            }
        }
        ++ count;

        if (phaseType) {

            tvalue = (_source->*valueFunc)(i);
            //make phase linear (non periodic) function
            if (std::abs(tvalue - ltvalue) > M_PI) {
                tvalue += std::copysign(D_PI, ltvalue);                //move to next/prev circle
            }

            tvalue += lValue - ltvalue;   //circles count
            lValue = tvalue;
            value += tvalue;
            ltvalue = (_source->*valueFunc)(i);

        } else {
            value += (_source->*valueFunc)(i);
        }
    }
    glEnd();
}
void Series::bandBars()
{
    static float startFrequency = 24000 / pow(2, 11);

    float frequencyFactor = pow(2, 1.0 / pointsPerOctave()), //TODO: cache this value
        bandStart = startFrequency,
        bandEnd   = bandStart * frequencyFactor,
        frequency,
        value = 0,
        bandWidth = 0,
        lastX = 0;

    int c = _source->size(),
        j = 0,
        count = 0;
    x[0] = 24000.0;
    x[1] = x[0] * frequencyFactor;
    convert4Vertexes(&x, &y);
    bandWidth = std::abs(x[1] - x[0]) / 2.0;

    for (int i = 1; i < c; i++) {
        frequency = _source->frequency(i);
        if (frequency < bandStart) continue;
        while (frequency > bandEnd) {
            bandStart = bandEnd;
            bandEnd   = bandStart * frequencyFactor;
            if (count) {
                x[j] = frequency;
                y[j] = 10 * log10f(value);
                value = 0;
                count = 0;
                j++;
                if (j == 4) {
                    convert4Vertexes(&x, &y);
                    draw4Bands(&x, &y, &lastX, bandWidth);
                    j = 0;
                }
            }
        }
        count ++;
        value += pow(_source->dataAbs(i) / _source->fftSize(), 2);
    }
    if (j) {
        convert4Vertexes(&x, &y);
        draw4Bands(&x, &y, &lastX, bandWidth, j);
    }
}
