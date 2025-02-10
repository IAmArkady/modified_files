#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::startModifiedFiles);

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

    ui->LE_Xor->setText("0000000000000000");
    ui->LE_Xor->setInputMask(">HH.HH.HH.HH.HH.HH.HH.HH");
}

QWidget* MainWindow::loadUI(QString file_path){
    QFile ui_file(file_path);
    QWidget* widget = ui_load.load(&ui_file, this);
    return widget;
}

void MainWindow::stopModifiedFiles(){
    timer->stop();
    ui->PB_Start->setVisible(true);
    ui->PB_Stop->setVisible(false);
    setInteractiveWidgets(ui->W_Settings, true);
}

QFileInfoList MainWindow::getFilesFromMask(QString dir_path, QString mask){
    QDir dir(dir_path);
    QStringList list_mask = ui->LE_Mask->text().split(',');
    for (QString &mask: list_mask){
        mask = mask.trimmed();
        if (mask.startsWith("."))
            mask = '*' + mask;
    }
    dir.setNameFilters(list_mask);
    return dir.entryInfoList(QDir::Files);
}

void MainWindow::startModifiedFiles(){
    if (ui->LE_SaveFolder->text().isEmpty()){
        QMessageBox::warning(this, "Ошибка", "Путь для сохранения файлов не выбран");
        return;
    }

    QString xor_key = ui->LE_Xor->text().replace(".", "");
    size_t count_point = ui->LE_Xor->maxLength()-ui->LE_Xor->text().count(".");
    if (xor_key.size() != count_point){
        QMessageBox::warning(this, "Ошибка", "Значение XOR должно иметь " + QString::number(count_point) + " символов");
        return;
    }

    QFileInfoList files = getFilesFromMask(ui->LE_WorkFolder->text(), ui->LE_Mask->text());
    if (files.isEmpty())
        addToList("Файлы по заданной маске не найдены");
    for (auto file_info: files)
        modifiedFile(file_info.filePath());


    /* Запуск таймера для автоматической работы */
    if (!ui->CB_PeriodMode->isChecked() && !timer->isActive()){
        timer->start(ui->SB_Time->value());
        ui->PB_Start->setVisible(false);
        ui->PB_Stop->setVisible(true);
        setInteractiveWidgets(ui->W_Settings, false);
    }
}

void MainWindow::modifiedFile(QString filepath){
    if (file_widgets.contains(filepath)) {
        addToList(filepath + ": Уже обрабатывается!", QColor(255, 165, 0, 40));
        return;
    }
    QWidget* widget = loadUI(":/fileProgress.ui");
    if (!widget)
        return;
    addToList(widget);

    ModifiedFileXor* modified_file = new ModifiedFileXor(filepath);
    modified_file->setSaveFilePath(ui->LE_SaveFolder->text());
    modified_file->setOverwrite(ui->CB_Overwrite->isChecked());
    modified_file->setXorKey(ui->LE_Xor->text());

    file_widgets[filepath] = widget;
    widget->findChild<QLabel*>("L_Filename")->setText(filepath);
    widget->findChild<QLabel*>("L_DateTime")->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss"));
    widget->findChild<QLabel*>("L_Size")->setText(QString::number(modified_file->getSizeFile()));
    widget->findChild<QLabel*>("L_Result")->setText("В обработке...");


    QThread* thread = new QThread(this);
    modified_file->moveToThread(thread);
    connect(thread, &QThread::started, modified_file, &ModifiedFileXor::startModified);
    connect(modified_file, &ModifiedFileXor::progressByte, this, &MainWindow::updateProgress);
    connect(modified_file, &ModifiedFileXor::finished, this, &MainWindow::modifiedFinished);
    connect(modified_file, &ModifiedFileXor::finished, thread, &QThread::quit);
    connect(modified_file, &ModifiedFileXor::finished, modified_file, &ModifiedFileXor::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();

    /* Удаление файла после обработки */
    if (ui->CB_Delete->isChecked()){
        QFile file(filepath);
        bool flag_remove = file.remove();
        if (!flag_remove)
            addToList(filepath + ": « Файл не может быть удален »", QColor(255, 0, 0, 40));
        addToList(filepath + ": « Файл успешно удален »", QColor(0, 255, 0, 40));
    }

}

void MainWindow::updateProgress(QString file_path, quint64 byte){
    if (file_widgets.contains(file_path)) {
        bool ok;
        QWidget *widget = file_widgets[file_path];
        int size = widget->findChild<QLabel*>("L_Size")->text().toInt(),
            cur_size = widget->findChild<QLabel*>("L_ProcessSize")->text().toInt(),
            value = (cur_size + byte) * 100 / size;
        widget->findChild<QProgressBar*>("PB_Progress")->setValue(value);
        widget->findChild<QLabel*>("L_ProcessSize")->setText(QString::number(cur_size + byte));
    }
}

void MainWindow::modifiedFinished(QString file_path, QString new_file_path, QString error){
    if (file_widgets.contains(file_path)) {
        QWidget *widget = file_widgets[file_path];
        if (error.isEmpty())
            widget->findChild<QLabel*>("L_Result")->setText("Успешно сохранено \"" + new_file_path + "\"");
        else
            widget->findChild<QLabel*>("L_Result")->setText(error);
        file_widgets.remove(file_path);
    }
}


void MainWindow::setInteractiveWidgets(QWidget* widget, bool interactive){
    if (widget){
        QList<QWidget*> children = ui->W_Settings->findChildren<QWidget*>();
        for (QWidget* child : children)
            child->setAttribute(Qt::WA_TransparentForMouseEvents, !interactive);
    }
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

void MainWindow::addToList(QWidget *widget){
    QListWidgetItem* item = new QListWidgetItem(ui->LW_Result);
    item->setSizeHint(widget->sizeHint());
    ui->LW_Result->setItemWidget(item, widget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

