#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::modifiedFiles);

    connect(ui->CB_PeriodMode, &QCheckBox::toggled, ui->SB_Time, [this](bool checked){
        ui->SB_Time->setEnabled(!checked);
    });
    connect(ui->PB_SaveFolder, &QPushButton::clicked, this, &MainWindow::selectSaveFolder);
    connect(ui->PB_WorkFolder, &QPushButton::clicked, this, &MainWindow::selectWorkFolder);
    connect(ui->PB_Start, &QPushButton::clicked, this, &MainWindow::startModifiedFiles);
    connect(ui->PB_Stop, &QPushButton::clicked, this, &MainWindow::stopModifiedFiles);
    connect(ui->PB_ClearList, &QPushButton::clicked, ui->LW_Result, &QListWidget::clear);
    connect(ui->LW_Result->model(), &QAbstractItemModel::rowsInserted, ui->LW_Result, &QListWidget::scrollToBottom);

    ui->PB_Stop->setVisible(false);
    ui->LE_WorkFolder->setText(QDir::currentPath());
}

void MainWindow::stopModifiedFiles(){
    timer->stop();
    ui->PB_Start->setVisible(true);
    ui->PB_Stop->setVisible(false);
    setInteractiveWidgets(ui->W_Settings, true);
}

void MainWindow::startModifiedFiles(){
    if (ui->LE_SaveFolder->text().isEmpty()){
        QMessageBox::warning(this, "Ошибка", "Путь для сохранения файлов не выбран");
        return;
    }

    if (ui->LE_Xor->text().size() != ui->LE_Xor->maxLength()){
        QMessageBox::warning(this, "Ошибка", "Значение XOR должно иметь 8 символов");
        return;
    }

    modifiedFiles();

    /* Запуск таймера для автоматической работы */
    if (!ui->CB_PeriodMode->isChecked() && !timer->isActive()){
        timer->start(ui->SB_Time->value());
        ui->PB_Start->setVisible(false);
        ui->PB_Stop->setVisible(true);
        setInteractiveWidgets(ui->W_Settings, false);
    }
}

void MainWindow::setInteractiveWidgets(QWidget* widget, bool interactive){
    if (widget){
        QList<QWidget*> children = ui->W_Settings->findChildren<QWidget*>();
        for (QWidget* child : children)
            child->setAttribute(Qt::WA_TransparentForMouseEvents, !interactive);
    }
}

void MainWindow::modifiedFiles(){
    addToList("Запуск работы программы");
    QDir dir(QDir::currentPath());
    QFileInfoList files;

    QStringList list_mask = ui->LE_Mask->text().split(',');
    for (QString &mask: list_mask){
        mask = mask.trimmed();
        if (mask.startsWith("."))
            mask = '*' + mask;
    }

    dir.setNameFilters(list_mask);
    files = dir.entryInfoList(QDir::Files);

    if (files.isEmpty())
        addToList("Файлы по заданной маске не найдены");

    for (auto file_info: files){
        QFile file(file_info.filePath());
        if (file.isOpen()){
            addToList(file_info.filePath() + ": « Ошибка, файл уже отрыт »", QColor(255, 0, 0, 40));
            continue;
        }

        if (!file.open(QIODevice::ReadOnly)){
            addToList(file_info.filePath() + ": « Ошибка открытия файла »", QColor(255, 0, 0, 40));
            continue;
        }

        QFile modified_file;
        QString path_saveFolder = ui->LE_SaveFolder->text();
        if (ui->CB_Overwrite->isChecked()){
            /* Перезапись модифицированного файла */
            modified_file.setFileName(path_saveFolder + '/' + file_info.fileName());
            if (!modified_file.open(QIODevice::ReadWrite)){
                addToList(modified_file.fileName() + ": « Ошибка открытия модифицированного файла »", QColor(255, 0, 0, 40));
                continue;
            }
        }
        else{
            /* Создание нового модифицированного файла */
            QDir dir_save(path_saveFolder);
            QStringList list_filename = dir_save.entryList({file_info.completeBaseName() + "*"}, QDir::Files);
            QRegularExpression regex(QString("^%1_(\\d+)$").arg(QRegularExpression::escape(file_info.completeBaseName())));
            qulonglong max_counter = 0;
            bool find_filename = false;
            for (const QString &filename : list_filename){
                if (!find_filename && file_info.fileName() == filename)
                    find_filename = true;
                QRegularExpressionMatch match = regex.match(QFileInfo(filename).completeBaseName());
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
            modified_file.setFileName(path_saveFolder + '/' + file_info.completeBaseName() + add_text + '.' + file_info.suffix());
            if (!modified_file.open(QIODevice::ReadWrite)){
                addToList(modified_file.fileName() + ": « Ошибка открытия модифицированного файла »", QColor(255, 0, 0, 40));
                continue;
            }
        }
        while(!file.atEnd()){
            QByteArray buffer = file.read(8),
                result_xor = xorEncryptDecrypt(QByteArray::fromHex(ui->LE_Xor->text().remove(0, 2).toUtf8()), buffer);
            modified_file.write(result_xor);
        }

        file.close();
        modified_file.close();
        addToList(modified_file.fileName() + ": « Успешное сохранение модифицированного файла »", QColor(0, 255, 0, 40));

        /* Удаление файла после обработки */
        if (ui->CB_Delete->isChecked()){
            bool flag_remove = file.remove();
            if (!flag_remove)
                addToList(file_info.filePath() + ": « Файл не может быть удален »", QColor(255, 0, 0, 40));
            addToList(file_info.filePath() + ": « Файл успешно удален »", QColor(0, 255, 0, 40));
        }
    }
    addToList("Остановка работы программы");
    addToList("",QColor(0, 0, 0, 10), Qt::black, false);
}

QByteArray MainWindow::xorEncryptDecrypt(QByteArray xor_key, QByteArray data){
    QByteArray result;
    for (int i = 0; i < data.size(); ++i)
        result.append(data[i] ^ xor_key[i % xor_key.size()]);
    return result;
}

void MainWindow::selectSaveFolder(){
    QString path = QFileDialog::getExistingDirectory(this);
    ui->LE_SaveFolder->setText(path);
}

void MainWindow::selectWorkFolder(){
    QString path = QFileDialog::getExistingDirectory(this);
    path.isEmpty() ? ui->LE_WorkFolder->setText(QDir::currentPath()) : ui->LE_WorkFolder->setText(path);
}

void MainWindow::addToList(QString text, QColor color_row, QColor color_text, bool time){
    if (time){
        QString current_time = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss");
        ui->LW_Result->addItem('(' + current_time + ") " + text);
    }
    else
        ui->LW_Result->addItem("");
    QListWidgetItem *item = ui->LW_Result->item(ui->LW_Result->count() - 1);
    item->setForeground(QBrush(color_text));
    item->setBackground(QBrush(color_row));
}

MainWindow::~MainWindow()
{
    delete ui;
}

