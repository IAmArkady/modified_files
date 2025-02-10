#include "modifiedfilexor.h"

ModifiedFileXor::ModifiedFileXor(QObject *parent)
    :overwrite(false),  QObject{parent}{
}


ModifiedFileXor::ModifiedFileXor(QString file_path, QObject *parent)
    : file_path(file_path), overwrite(false),  QObject{parent}{
}

bool ModifiedFileXor::openFile(){
    file.setFileName(file_path);
    if (file.isOpen()){
        error_buf = "Ошибка, файл уже открыт";
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)){
        error_buf = "Ошибка открытия файла";
        return false;
    }
    return true;
}

bool ModifiedFileXor::startModified(){
    if (!openFile()){
        emit finished(file_path, "", error_buf);
        return false;
    }

    QFile modified_file;
    QFileInfo file_info(file_path);
    QString file_basename = file_info.completeBaseName(),
        file_suffix = file_info.suffix(),
        file_name = file_info.fileName();

    if (overwrite){
        /* Перезапись модифицированного файла */
        modified_file.setFileName(save_file_path + '/' + file_name);
    }
    else{
        /* Создание нового модифицированного файла */
        QDir dir_save(save_file_path);
        QStringList list_filename = dir_save.entryList({file_basename + "*"}, QDir::Files);
        QRegularExpression regex(QString("^%1_(\\d+)$").arg(QRegularExpression::escape(file_basename)));
        qulonglong max_counter = 0;
        bool find_filename = false;
        for (const QString &dir_filename : list_filename){
            if (!find_filename && file_name == dir_filename)
                find_filename = true;
            QRegularExpressionMatch match = regex.match(QFileInfo(dir_filename).completeBaseName());
            if (match.hasMatch()) {
                int counter = match.captured(1).toInt();
                if (counter > max_counter) {
                    max_counter = counter;
                }
            }
        }

        /* Если модифицированные файлы отсутствуют - задать имя исходного файла, иначе - задать имя файла со счетчиком*/
        QString add_text;
        if (find_filename || max_counter)
            add_text = "_" + QString::number(++max_counter);
        modified_file.setFileName(save_file_path + '/' + file_basename + add_text + '.' + file_suffix);
    }

    if (!modified_file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        error_buf = "Ошибка открытия модифицированного файла";
        emit finished(file_path, modified_file.fileName(), error_buf);
        return false;
    }

    while(!file.atEnd()){
        QByteArray buffer = file.read(4960),
            result_xor = xorEncryptDecrypt(buffer);
        modified_file.write(result_xor);
        emit progressByte(file_path, result_xor.size());
    }

    file.close();
    modified_file.close();
    emit finished(file_path, modified_file.fileName(), error_buf);
}

QByteArray ModifiedFileXor::xorEncryptDecrypt(QByteArray data){
    QByteArray xor_key_byte = QByteArray::fromHex(xor_key.toUtf8()), result;
    for (int i = 0; i < data.size(); ++i)
        result.append(data[i] ^ xor_key_byte[i % xor_key_byte.size()]);
    return result;
}

void ModifiedFileXor::setFilepath(QString file_path){
    this->file_path = file_path;
}

void ModifiedFileXor::setXorKey(QString xor_key){
    this->xor_key = xor_key;
}

void ModifiedFileXor::setSaveFilePath(QString path){
    save_file_path = path;
}

void ModifiedFileXor::setOverwrite(bool mode){
    overwrite = mode;
}

QString ModifiedFileXor::getFilepath(){
    return file_path;
}

QString ModifiedFileXor::getFilename(){
    return QFileInfo(file_path).completeBaseName();
}

QString ModifiedFileXor::getXorKey(){
    return xor_key;
}

QString ModifiedFileXor::getSaveFilePath(){
    return save_file_path;
}

bool ModifiedFileXor::getOverwrite(){
    return overwrite;
}

QString ModifiedFileXor::getLastError(){
    return error_buf;
}

quint64 ModifiedFileXor::getSizeFile(){
    return QFileInfo(file_path).size();
}
