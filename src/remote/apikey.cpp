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
#include "apikey.h"
#include <QtDebug>
#include <QCryptographicHash>

#define OPENSSL_NO_DEPRECATED_3_0
#include <openssl/rsa.h>
#include <openssl/pem.h>

namespace remote {
ApiKey::ApiKey() : m_owner(), m_sign()
{
}

ApiKey::ApiKey(QString owner, QString type, QString sign) : m_owner(owner), m_type(type)
{
    QString forHash = owner + type;
    m_hash = QCryptographicHash::hash(forHash.toLocal8Bit(), QCryptographicHash::Sha256);
    m_sign = QByteArray::fromBase64(sign.toLocal8Bit());
}

QString ApiKey::owner() const
{
    return m_owner;
}

QString ApiKey::type() const
{
    return m_type;
}

QString ApiKey::sign() const
{
    return m_sign.toBase64();
}

QString ApiKey::title() const
{
    return owner() + (type().isEmpty() ? "" : " (" + type() + ")");
}

bool ApiKey::isEmpty() const
{
    return m_owner.isEmpty() || m_sign.isEmpty();
}

bool ApiKey::valid() const
{
    if (isEmpty()) {
        return false;
    }

    if (!m_valid.has_value()) {
        m_valid = verifySignature(
                      reinterpret_cast<const unsigned char *>(m_sign.toStdString().c_str()),
                      m_sign.size(),
                      reinterpret_cast<const unsigned char *>(m_hash.data()),
                      m_hash.size()
                  );
    }
    return m_valid.value_or(false);
}

const std::string ApiKey::PUBLIC_KEY =
    "-----BEGIN PUBLIC KEY----- \n"\
    "MEwwDQYJKoZIhvcNAQEBBQADOwAwOAIxAMtZ2S5u3IVkJie4ftq0oq1hPOiitRg3\n"\
    "W+ajIm5ZaoXjZE1d3QXJvxFwbprU7vb6hQIDAQAB\n"\
    "-----END PUBLIC KEY----- \n\0"
    ;

bool ApiKey::verifySignature(const unsigned char *sign, size_t signSize,
                             const unsigned char *data, size_t dataSize) const
{
    BIO *keyBio = BIO_new_mem_buf(static_cast<const void *>(PUBLIC_KEY.c_str()), -1);
    if (keyBio == nullptr) {
        return false;
    }

    auto *publicKey  = EVP_PKEY_new();
    PEM_read_bio_PUBKEY(keyBio, &publicKey, nullptr, nullptr);
    auto *context = EVP_MD_CTX_create();

    if (EVP_DigestVerifyInit(context, nullptr, EVP_sha1(), nullptr, publicKey) <= 0) {
        return false;
    }

    if (EVP_DigestVerifyUpdate(context, data, dataSize) <= 0) {
        return false;
    }

    int checkCode = EVP_DigestVerifyFinal(context, sign, signSize);

    BIO_free(keyBio);
    EVP_PKEY_free(publicKey);
    EVP_MD_CTX_free(context);
    return (checkCode == 1);
}
}

QDebug operator<<(QDebug dbg, const remote::ApiKey &key)
{
    dbg.nospace() << "Api key:"
                  << key.owner()
                  << key.type()
                  << key.sign()
                  << key.valid();

    return dbg.maybeSpace();
}
