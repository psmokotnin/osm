/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "wavfile.h"
#include <QtEndian>
#include <QtMath>
#include <QDebug>
#include <cstring>

WavFile::WavFile() : m_header(), m_dataPosition(0)
{
}

WavFile::~WavFile()
{
    m_file.close();
}

bool WavFile::load(const QString &fileName)
{
    m_file.setFileName(fileName);

    if (!m_file.open(QFile::ReadOnly)) {
        qDebug() << m_file.error() << "str:" << m_file.errorString();
        return false;
    }

    auto read = m_file.read(reinterpret_cast<char *>(&m_header.wave), sizeof (m_header.wave));
    if (read != sizeof (m_header.wave)) {
        return false;
    }

    //read next chunk until data
    char chunkId[4];
    unsigned int chunkSize = 0;
    while (m_file.read(chunkId, 4) == 4) {

        m_file.read(reinterpret_cast<char *>(&chunkSize), 4);
        chunkSize = qFromLittleEndian(chunkSize);
        auto qChunkId = QString::fromLocal8Bit(chunkId, 4);

        if (qChunkId == "fmt ") {
            m_file.seek(m_file.pos() - 8);
            m_file.read(reinterpret_cast<char *>(&m_header.format), sizeof (m_header.format));
        } else if (qChunkId == "data") {
            memcpy(m_header.data.id, chunkId, 4);
            m_header.data.size = qToLittleEndian(chunkSize);
            break;
        } else {
            //skip meta data
            m_file.seek(m_file.pos() + chunkSize);
        }
    }
    m_dataPosition = m_file.pos();

    return m_header.valid();
}

bool WavFile::save(const QString &fileName, int sampleRate, const QByteArray &data)
{
    m_file.setFileName(fileName);
    if (!m_file.open(QFile::WriteOnly)) {
        return false;
    }
    prepareHeader(sampleRate);
    m_header.wave.chunk.size = qToLittleEndian(36 + data.count());;
    m_header.data.size = qToLittleEndian(data.count());
    m_file.write(reinterpret_cast<char *>(&m_header), sizeof (m_header));
    m_file.write(data.data(), data.count());
    m_file.close();

    return true;
}

void WavFile::prepareHeader(int sampleRate)
{
    std::memcpy(m_header.wave.chunk.id, "RIFF", 4);

    m_header.format.chunk.size      = qToLittleEndian(16);
    m_header.format.audioFormat     = qToLittleEndian(WaveHeader::AudioFormat::FLOAT);
    m_header.format.channels        = qToLittleEndian(1);
    m_header.format.bitsPerSample   = qToLittleEndian(32);
    m_header.format.sampleRate      = qToLittleEndian(sampleRate);
    m_header.format.blockAlign      = qToLittleEndian(m_header.format.channels * m_header.format.bitsPerSample / 8);
    m_header.format.byteRate        = qToLittleEndian(m_header.format.sampleRate * m_header.format.blockAlign);
}

int WavFile::sampleRate() const noexcept
{
    return qFromLittleEndian(m_header.format.sampleRate);
}

constexpr unsigned int WavFile::blockAlign() const noexcept
{
    return qFromLittleEndian(m_header.format.blockAlign);
}

constexpr unsigned int WavFile::bitsPerSample() const noexcept
{
    return qFromLittleEndian(m_header.format.bitsPerSample);
}

constexpr unsigned int WavFile::sampleType() const noexcept
{
    return qFromLittleEndian(m_header.format.audioFormat);
}

constexpr unsigned int WavFile::dataSize() const noexcept
{
    return qFromLittleEndian(m_header.data.size);
}

float WavFile::nextSample(bool loop, bool *finished) noexcept
{
    float s = NAN;

    if (!m_file.isReadable()) {
        return s;
    }

    std::vector<char> buffer;
    buffer.resize(blockAlign());
    if (m_file.pos() >= m_dataPosition + dataSize()) {
        if (loop) {
            m_file.seek(m_dataPosition);
            m_file.read(buffer.data(), blockAlign());
        } else {
            if (finished) {
                *finished = true;
            }
            return NAN;
        }
    }

    if (m_file.read(buffer.data(), blockAlign()) == 0) {
        return NAN;
    }

    switch (sampleType()) {
    case WaveHeader::AudioFormat::PCM:
        switch (bitsPerSample()) {
        case 16: {
            qint16_le value;
            memcpy(&value, buffer.data(), 2);
            s = qFromLittleEndian<float>(value) / std::numeric_limits<qint16>::max();
            break;
        }

        case 24: {
            qint32_le value(0);
            memcpy(reinterpret_cast<char *>(&value) + 1, buffer.data(), 3);
            s = qFromLittleEndian<double>(value) / std::numeric_limits<qint32>::max();
            break;
        }

        case 32: {
            qint32_le value;
            memcpy(&value, buffer.data(), 4);
            s = qFromLittleEndian<double>(value) / std::numeric_limits<qint32>::max();
            break;
        }

        default:
            Q_UNREACHABLE();
            qCritical() << "unknown bitsPerSample";
            return NAN;
        }
        break;

    case WaveHeader::AudioFormat::FLOAT: {
        float value;
        memcpy(&value, buffer.data(), 4);
        s = qFromLittleEndian<float>(value);
        break;
    }
    }

    return s;
}

QDebug operator << (QDebug dbg, const WavFile::WaveHeader &header)
{
    dbg.nospace() << "Wave file header: {:"
                  << header.wave.format             << " "
                  << header.format.channels << " "
                  << header.format.audioFormat      << " "
                  << header.format.bitsPerSample    << " "
                  << header.format.blockAlign       << " "
                  << header.format.byteRate         << " "
                  << header.format.sampleRate       << " "
                  << "}";

    return dbg.maybeSpace();
}

bool WavFile::WaveHeader::valid() const noexcept
{
    return
        memcmp(wave.chunk.id,   "RIFF", 4) == 0 &&
        memcmp(wave.format,     "WAVE", 4) == 0 &&

        memcmp(format.chunk.id, "fmt ", 4) == 0 &&
        (format.audioFormat == AudioFormat::PCM ||
         format.audioFormat == AudioFormat::FLOAT) &&
        format.channels >= 1 &&

        memcmp(data.id, "data", 4) == 0;
}
