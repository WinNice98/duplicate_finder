#include "hashworker.h"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

HashWorker::HashWorker(const QStringList &files, HashMethod method, QObject *parent)
    : QThread(parent)
    , m_files(files)
    , m_method(method)
{}

void HashWorker::run()
{
    int total = m_files.size();
    for (int i = 0; i < total; ++i) {
        QString filePath = m_files[i];
        QString hash = calculateHash(filePath);

        QFileInfo info(filePath);
        emit fileHashed(info.fileName(), filePath, hash, info.size(), info.lastModified());
        emit progress(i + 1, total);
    }
}

QString HashWorker::calculateHash(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QCryptographicHash::Algorithm algo;
    switch (m_method) {
    case HashMethod::MD5:
        algo = QCryptographicHash::Md5;
        break;
    case HashMethod::SHA1:
        algo = QCryptographicHash::Sha1;
        break;
    case HashMethod::SHA256:
        algo = QCryptographicHash::Sha256;
        break;
    }

    QCryptographicHash crypt(algo);

    const qint64 bufferSize = 512 * 1024;
    QByteArray buffer;
    buffer.resize(bufferSize);

    while (!file.atEnd()) {
        qint64 bytesRead = file.read(buffer.data(), bufferSize);
        if (bytesRead > 0) {
            crypt.addData(buffer.constData(), bytesRead);
        } else {
            break; // ошибка чтения
        }
    }

    file.close();
    return crypt.result().toHex();
}

