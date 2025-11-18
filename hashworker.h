#ifndef HASHWORKER_H
#define HASHWORKER_H

#include <QDateTime>
#include <QStringList>
#include <QThread>
#include "mainwindow.h" // если нужно использовать HashMethod

class HashWorker : public QThread
{
    Q_OBJECT
public:
    HashWorker(const QStringList &files, HashMethod method, QObject *parent = nullptr);

signals:
    void fileHashed(const QString &filename,
                    const QString &path,
                    const QString &hash,
                    qint64 size,
                    QDateTime modified);
    void progress(int current, int total);

protected:
    void run() override;

private:
    QString calculateHash(const QString &filePath);

    QStringList m_files;
    HashMethod m_method;
};

#endif // HASHWORKER_H
