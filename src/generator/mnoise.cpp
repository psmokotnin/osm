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

/**
    M‑NOISE™ TEST SIGNAL
    END USER LICENSE AGREEMENT
    The Meyer Sound M‑Noise test signal is being provided to you subject to the following license agreement. Please read. IF YOU DO NOT AGREE TO THE TERMS OF THIS LICENSE, DO NOT DOWNLOAD OR USE THE M‑NOISE TEST SIGNAL.

    BACKGROUND
    The M‑Noise test signal was created by Meyer Sound Laboratories, Incorporated (“Meyer Sound”) for the use and benefit of the professional audio community. Subject to the provisions of the license, Meyer Sound is making this test signal freely available to all audio professionals who wish to use this signal. TO WORK PROPERLY THE M‑NOISE TEST SIGNAL MUST BE USED AS INSTRUCTED. DETAILED INSTRUCTIONS ON THE PROPER USE OF THE M‑NOISE TEST SIGNAL ARE PROVIDED ON THE MEYER SOUND DEDICATED WEBSITE FOR M‑NOISE UNDER “M‑NOISE PROCEDURE.” See https://m‑noise.org/procedure/.

    FILE DELIVERY FORMAT AND LICENSE
    The M‑Noise test signal is provided to you in Waveform Audio File Format (.wav) for use in testing loudspeaker systems (“the Purpose”). You are granted free of charge the non-exclusive, transferable, worldwide and perpetual license to copy, use and analyze the M‑Noise .wav file and the test signal provided by this file for the Purpose.

    USE OF M‑NOISE TRADEMARK
    M‑Noise is a trademark of Meyer Sound Laboratories, Incorporated and signifies that the audio file used in the tests is based on an original unaltered .wav audio file supplied by Meyer Sound (“genuine M‑Noise test signal”). You may use the M‑NOISE trademark to identify a test signal as a genuine M‑Noise test signal and in connection with test results produced by a genuine M‑Noise test signal used in accordance with Meyer Sound’s instructions. Your use of the M‑Noise trademark shall inure to the benefit of Meyer Sound.

    You may not use the M‑Noise trademark:

    To identify a loudspeaker system test signal that is not based on a genuine M‑Noise test signal; or
    To identify a loudspeaker system test signal based on a genuine M‑Noise test signal that has been altered in any way; or
    To identity or describe test results that were not produced by a genuine M‑Noise test signal; or
    To identity or describe test results that were produced by a genuine M‑Noise test signal without following Meyer Sound’s instructions on the proper use of the M‑Noise test signal.
    WARRANTY DISCLAIMER
    The M‑Noise test signal is provided to you “as is.” Meyer Sound disclaims all warranties with respect to M‑Noise, including, without limitation, any implied warranties of merchantability or fitness for a particular purpose. The entire risk as to the performance of M‑Noise resides with you, the user. Meyer Sound does not warrant that the functionality of M‑Noise will meet your requirements or that your use of M‑Noise will be error free.

    LIMITATION OF LIABILITY
    In no event shall Meyer Sound be liable to you or to any third party for any consequential, incidental or special damages (including lost profits) arising out of or in any way connected with this license or your use of the M‑Noise test signal, regardless of legal theory, even if Meyer Sound has been advised of the possibility of such damages.

    GENERAL
    If a court of competent jurisdiction holds any provision of this agreement invalid, such provision shall to that extent be deemed omitted and shall not affect the remaining provisions of this agreement, which shall remain in full force and effect. This agreement constitutes the entire agreement between you and Meyer Sound with respect to the M‑Noise test signal and your use thereof. Any changes to this agreement must be in writing and signed by an authorized representative of Meyer Sound. A waiver of a term of this agreement shall not be deemed a continuing waiver of such term or a waiver of any other term of this agreement.

    M‑Noise is a trademark of Meyer Sound Laboratories, Incorporated.

    M‑Noise Test Signal and Test Method is Patent Pending.
*/
#include "mnoise.h"
#include <QtMath>

MNoise::MNoise(QObject *parent) : Wav(parent)
{
    m_name = "M-Noise™";
    if (!load(":/audio/mnoise.wav")) {
        qCritical() << "can't load M-Noise";
    }
}

Sample MNoise::sample()
{
    if (m_sampleRate != sampleRate()) {
        return {NAN};
    }

    return Wav::sample();
}
