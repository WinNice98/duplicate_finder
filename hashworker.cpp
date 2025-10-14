#include "hashworker.h"
#include <QFile>
#include <QCryptographicHash>
#include <QFileInfo>

HashWorker::HashWorker(const QStringList &files, HashMethod method, QObject *parent)
    : QThread(parent), m_files(files), m_method(method) {}

void HashWorker::run() {
    int total = m_files.size();
    for (int i = 0; i < total; ++i) {
        QString filePath = m_files[i];
        QString hash = calculateHash(filePath);

        QFileInfo info(filePath);
        emit fileHashed(info.fileName(), filePath, hash, info.size(), info.lastModified());
        emit progress(i + 1, total);
    }
}

QString HashWorker::calculateHash(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QCryptographicHash::Algorithm algo;
    switch (m_method) {
    case HashMethod::MD5:    algo = QCryptographicHash::Md5; break;
    case HashMethod::SHA1:   algo = QCryptographicHash::Sha1; break;
    case HashMethod::SHA256: algo = QCryptographicHash::Sha256; break;
    }

    QCryptographicHash crypt(algo); // создаём объект сразу с алгоритмом
    crypt.addData(&file);
    file.close();
    return crypt.result().toHex();
}



