#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    ui->statusBar->addPermanentWidget(progressBar);
    progressBar->setValue(0);

    ui->pushButton_startStitch->setEnabled(false);

    connect(&watcher, SIGNAL(finished()), this, SLOT(returnResult()));
    connect(&stitch, SIGNAL(sendProgress(QString,int)), this, SLOT(progressBarStatus(QString,int)));
}

MainWindow::~MainWindow()
{
    disconnect(&watcher, SIGNAL(finished()), this, SLOT(returnResult()));
    disconnect(&stitch, SIGNAL(sendProgress(QString,int)), this, SLOT(progressBarStatus(QString,int)));
    delete ui;
}

void MainWindow::returnResult()
{
    ui->pushButton_startStitch->setEnabled(true);
    ui->graphicsView_result->initialize(1, result.cols, result.rows,1);
    ui->graphicsView_result->setImage(result);
}

void MainWindow::progressBarStatus(const QString &content, int num)
{
    ui->statusBar->showMessage(content);
    ui->textBrowser_log->append(content);
    progressBar->setValue(num);
    ui->statusBar->update();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionOpen_triggered()
{
    filenames.clear();
    filenames = QFileDialog::getOpenFileNames(this, "Open Images!");
    if(filenames.empty())
    {
        QMessageBox::critical(this, "Error", "Cannot get file names.");
        return;
    }
    images.clear();
    result.release();
    ui->pushButton_startStitch->setEnabled(false);

    cv::Mat tempimg;
    for(int i = 0; i < filenames.size(); i++)
    {
        tempimg = cv::imread(filenames.at(i).toStdString());
        images.push_back(tempimg.clone());
    }

    if(images.size() < 2)
    {
        QMessageBox::critical(this, "Error", "Not enough images.");
        return;
    }
    for(int i = 0; i < images.size(); i++)
    {
        if(images[i].empty())
        {
            QFileInfo fi(filenames.at(i));
            QMessageBox::critical(this, "Error", "Fail to read " + fi.fileName() + "!");
            return;
        }
    }

    ui->graphicsView->initialize(images.size(), images[0].cols, images[0].rows, images.size());
    ui->graphicsView->setImage(images);
    ui->pushButton_startStitch->setEnabled(true);
}

void MainWindow::on_actionSave_triggered()
{
    if(result.empty())
        return;

    QString name = QFileDialog::getSaveFileName(this, "Save Panorama");
    if(name.isEmpty())
        return;

    cv::imwrite(name.toStdString(), result);
}

void MainWindow::on_pushButton_startStitch_clicked()
{
    ui->pushButton_startStitch->setEnabled(false);
    //QtConcurrent copies the content of the parameters (pass-by-value). Use std::ref() to pass-by-reference (C++ 11 up only)
#ifdef _DEBUG
    //stitch.process(filenames, images, result);
#else
    QFuture<void> future = QtConcurrent::run(&this->stitch, &myStitch::process, filenames, images, std::ref(result));
    watcher.setFuture(future);
#endif
}
