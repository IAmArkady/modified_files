#ifndef MODIFIEDFILEXOR_H
#define MODIFIEDFILEXOR_H

#include <QObject>
#include <QDataStream>
#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QRegularExpression>
#include <QDir>

class ModifiedFileXor : public QObject
{
    Q_OBJECT
    QString file_path;
    QString xor_key;
    QString save_file_path;
    bool overwrite;
    QString error_buf;
    QFile file;

    bool openFile();

public:
    ModifiedFileXor(QObject *parent = nullptr);
    ModifiedFileXor(QString file_path, QObject *parent = nullptr);

    void setFilepath(QString file_path);
    void setXorKey(QString xor_key);
    void setSaveFilePath(QString path);
    void setOverwrite(bool mode);

    QString getFilename();
    QString getFilepath();
    QString getXorKey();
    QString getSaveFilePath();
    quint64 getSizeFile();
    bool getOverwrite();


    bool startModified();
    QString getLastError();
    QByteArray xorEncryptDecrypt(QByteArray data);

signals:
    void progressByte(QString file_path, quint64 byte);
    void finished(QString file_path, QString new_file_path, QString error);

};

#endif // MODIFIEDFILEXOR_H
