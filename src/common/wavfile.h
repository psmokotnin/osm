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
#ifndef WAVFILE_H
#define WAVFILE_H

#include <QFile>

class WavFile
{
public:
    WavFile();
    ~WavFile();

    bool load(const QString &fileName);
    int sampleRate() const noexcept;
    unsigned int blockAlign() const noexcept;
    unsigned int bitsPerSample() const noexcept;
    unsigned int sampleType() const noexcept;

    float nextSample(bool loop, bool *finished = nullptr) noexcept;

protected:
    QFile m_file;

private:
    struct WaveHeader {
        struct Chunk {
            char id[4];
            qint32 size;
        };

        struct Wave {
            Chunk chunk;
            char format[4];
        };

        struct AudioFormat {
            const static qint16 PCM     = 1;
            const static qint16 FLOAT   = 3;

            Chunk chunk;
            qint16 audioFormat;     //! PCM = 1
            qint16 channels;        //! Number of channels
            qint32 sampleRate;
            qint32 byteRate;        //! SampleRate * NumChannels * BitsPerSample/8
            qint16 blockAlign;      //! NumChannels * BitsPerSample/8
            qint16 bitsPerSample;   //! 8 bits = 8, 16 bits = 16, etc.
        };

        Wave wave;
        AudioFormat format;
        Chunk data;

        bool valid() const noexcept;
    } m_header;

    qint64 m_dataPosition;

    friend QDebug operator << (QDebug dbg, const WavFile::WaveHeader &header);
};

QDebug operator << (QDebug dbg, const WavFile::WaveHeader &header);

#endif // WAVFILE_H
