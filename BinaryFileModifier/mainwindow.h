#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDataStream>
#include <QMessageBox>
#include <QScrollBar>
#include <QListView>
#include <QTimer>
#include <QFile>
#include <QUiLoader>
#include <QDir>
#include <QThread>
#include <QProgressBar>
#include "modifiedfilexor.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Ui::MainWindow *ui;
    QUiLoader ui_load;
    QTimer *timer;
    QMap<QString, QWidget*> file_widgets;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectSaveFolder();
    void selectWorkFolder();
    void startModifiedFiles();
    void stopModifiedFiles();
    void updateProgress(QString file_path, quint64 byte);
    void modifiedFinished(QString file_path, QString new_file_path, QString error);

private:
    void addToList(QString text, QColor color_row = Qt::white, QColor color_text = Qt::black, bool time = true);
    void addToList(QWidget* widget);
    void modifiedFile(QString filepath);
    void setInteractiveWidgets(QWidget* wiget, bool interactive);
    QWidget* loadUI(QString file_path);
    QFileInfoList getFilesFromMask(QString dir_path, QString mask);
};
#endif // MAINWINDOW_H
