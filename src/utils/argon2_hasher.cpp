#include "argon2_hasher.h"

#include <argon2.h>

#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <random>

namespace smart_diet {

// Argon2id 参数
static constexpr uint32_t ARGON2_MEMORY   = 65536;   // 64 MB
static constexpr uint32_t ARGON2_ITER     = 3;
static constexpr uint32_t ARGON2_PARALLEL = 4;
static constexpr uint32_t ARGON2_SALT_LEN = 16;
static constexpr uint32_t ARGON2_HASH_LEN = 32;

static QByteArray generateSalt(int length) {
    QByteArray salt(length, Qt::Uninitialized);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < length; ++i)
        salt[i] = static_cast<char>(dis(gen));
    return salt;
}

QString Argon2Hasher::hash(const QString &plainText) {
    const QByteArray password = plainText.toUtf8();
    const QByteArray salt = generateSalt(ARGON2_SALT_LEN);
    QByteArray hashOut(ARGON2_HASH_LEN, Qt::Uninitialized);

    const int ret = argon2id_hash_raw(
        ARGON2_ITER, ARGON2_MEMORY, ARGON2_PARALLEL,
        password.constData(), password.size(),
        salt.constData(), salt.size(),
        hashOut.data(), hashOut.size());

    if (ret != ARGON2_OK) {
        qWarning() << "[Argon2Hasher] hash failed:" << argon2_error_message(ret);
        return {};
    }

    // 编码为标准格式：$argon2id$v=19$m=...,t=...,p=...$<salt>$<hash>
    const QString encoded = QStringLiteral(
        "$argon2id$v=19$m=%1,t=%2,p=%3$%4$%5")
        .arg(ARGON2_MEMORY)
        .arg(ARGON2_ITER)
        .arg(ARGON2_PARALLEL)
        .arg(QString::fromLatin1(salt.toBase64(QByteArray::OmitTrailingEquals)))
        .arg(QString::fromLatin1(hashOut.toBase64(QByteArray::OmitTrailingEquals)));

    return encoded;
}

bool Argon2Hasher::verify(const QString &plainText, const QString &encodedHash) {
    if (encodedHash.isEmpty()) return false;
    const QByteArray password = plainText.toUtf8();
    const QByteArray encoded  = encodedHash.toUtf8();

    const int ret = argon2id_verify(
        encoded.constData(), password.constData(), password.size());

    return ret == ARGON2_OK;
}

} // namespace smart_diet
