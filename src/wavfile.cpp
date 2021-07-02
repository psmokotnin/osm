/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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

WavFile::WavFile(QObject *parent) : OutputDevice(parent), m_dataPosition(0)
{
    m_name = "Wav";
}

bool WavFile::load()
{
    if (!m_file.open(QFile::ReadOnly)) {
        qDebug() << m_file.error() << "str:" << m_file.errorString();
        return false;
    }

    auto read = m_file.read(reinterpret_cast<char *>(&m_header), sizeof (m_header));
    if (read != sizeof (m_header)) {
        return false;
    }

    m_dataPosition = m_file.pos();

    return m_header.valid();
}

unsigned int WavFile::sampleRate() const noexcept
{
    return qFromLittleEndian(m_header.format.sampleRate);
}

unsigned int WavFile::blockAlign() const noexcept
{
    return qFromLittleEndian(m_header.format.blockAlign);
}

unsigned int WavFile::bitsPerSample() const noexcept
{
    return qFromLittleEndian(m_header.format.bitsPerSample);
}

Sample WavFile::sample()
{
    Sample s;
    s.f = NAN;

    if (!m_file.isReadable()) {
        return s;
    }
    if (m_sampleRate != m_header.format.sampleRate) {
        return s;
    }
    std::vector<char> buffer;
    buffer.resize(blockAlign());
    if (m_file.read(buffer.data(), blockAlign()) == 0) {
        m_file.seek(m_dataPosition);
        m_file.read(buffer.data(), blockAlign());
    }

    switch (bitsPerSample()) {
    case 16:
        qint16_le value;
        memcpy(&value, buffer.data(), 2);
        s.f = m_gain * qFromLittleEndian<float>(value) / std::numeric_limits<qint16>::max();
        break;

    default:
        Q_UNREACHABLE();
    }

    return s;
}

QDebug operator << (QDebug dbg, const WavFile::WaveHeader &header)
{
    dbg.nospace() << "Wave file header: {:"
                  << header.wave.format             << " "
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
        memcmp(wave.chunk.id,   "RIFF", 4) &&
        memcmp(wave.format,     "WAVE", 4) &&

        memcmp(format.chunk.id, "fmt ", 4) &&
        format.audioFormat == AudioFormat::PCM &&
        format.channels >= 1 &&

        memcmp(data.id, "data", 4);
}
