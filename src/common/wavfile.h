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
    constexpr unsigned int blockAlign() const noexcept;
    constexpr unsigned int bitsPerSample() const noexcept;
    constexpr unsigned int sampleType() const noexcept;
    constexpr unsigned int dataSize() const noexcept;

    float nextSample(bool loop, bool *finished = nullptr) noexcept;

    bool save(const QString &fileName, int sampleRate, const QByteArray &data);
    void prepareHeader(int sampleRate);

protected:
    QFile m_file;

private:
    struct WaveHeader {
        struct Chunk {
            char id[4];
            qint32 size;
        };

        struct Wave {
            Chunk chunk     = {{'R', 'I', 'F', 'F'}, 4};
            char format[4]  =  {'W', 'A', 'V', 'E'};
        };

        struct AudioFormat {
            const static qint16 PCM     = 1;
            const static qint16 FLOAT   = 3;

            Chunk chunk             = {{'f', 'm', 't', ' '}, 16};
            qint16 audioFormat  = PCM;    //! PCM = 1
            qint16 channels     = 1;      //! Number of channels
            qint32 sampleRate   = 48000;
            qint32 byteRate     = 48000;  //! SampleRate * NumChannels * BitsPerSample/8
            qint16 blockAlign   = 1;      //! NumChannels * BitsPerSample/8
            qint16 bitsPerSample = 8;   //! 8 bits = 8, 16 bits = 16, etc.
        };

        Wave wave;
        AudioFormat format;
        Chunk data = {{'d', 'a', 't', 'a'}, 0};

        bool valid() const noexcept;
    } m_header;

    qint64 m_dataPosition;

    friend QDebug operator << (QDebug dbg, const WavFile::WaveHeader &header);
};

QDebug operator << (QDebug dbg, const WavFile::WaveHeader &header);

#endif // WAVFILE_H
