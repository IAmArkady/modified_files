#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDataStream>
#include <QMessageBox>
#include <QByteArray>
#include <QBuffer>
#include <QScrollBar>
#include <QListView>
#include <QTimer>
#include <QFile>
#include <QRegularExpression>
#include <QDir>
#include <Windows.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectSaveFolder();
    void selectWorkFolder();
    void startModifiedFiles();
    void stopModifiedFiles();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    void addToList(QString text, QColor color_row = Qt::white, QColor color_text = Qt::black, bool time = true);
    QByteArray xorEncryptDecrypt(QByteArray xor_key, QByteArray data);
    void modifiedFiles();
    void setInteractiveWidgets(QWidget* wiget, bool interactive);
};
#endif // MAINWINDOW_H
