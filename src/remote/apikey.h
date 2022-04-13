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
#ifndef REMOTE_APIKEY_H
#define REMOTE_APIKEY_H

#include <QString>
#include <QByteArray>
#include <optional>

namespace remote {
class ApiKey
{
public:
    ApiKey();
    ApiKey(QString key);
    ApiKey(QString owner, QString type, QString sign);

    QString owner() const;
    QString type() const;
    QString sign() const;

    QString title() const;

    bool isEmpty() const;
    bool valid() const;

private:
    void updateHash();
    bool verifySignature(const unsigned char *sign, size_t signSize, const unsigned char *data, size_t dataSize) const;
    const static std::string PUBLIC_KEY;

    QString m_owner, m_type;
    QByteArray m_hash;
    QByteArray m_sign;
    mutable std::optional<bool> m_valid;
};
}

QDebug operator << (QDebug dbg, const remote::ApiKey &key);

#endif // REMOTE_APIKEY_H
