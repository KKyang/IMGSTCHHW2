#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QProgressBar>
#include <QtConcurrent/QtConcurrent>

#include <functional>

#include "opencv2/opencv.hpp"
#include "mystitch.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void returnResult();
    void progressBarStatus(const QString &content, int num);
    void on_actionExit_triggered();

    void on_actionOpen_triggered();


    void on_actionSave_triggered();

    void on_pushButton_startStitch_clicked();

private:

    Ui::MainWindow *ui;
    QProgressBar *progressBar;

    //Stitching function
    myStitch stitch;
    //Three dataset
    QStringList filenames;
    std::vector<cv::Mat> images;
    cv::Mat result;
    //Future watcher for monitoring myStitch (QtConcurrent)
    QFutureWatcher<void> watcher;
};

#endif // MAINWINDOW_H
