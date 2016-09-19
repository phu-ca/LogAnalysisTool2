#include <QMessageBox>
#include <QFileDialog>
#include <QStringList>

#include "mainwindow.h"
#include "ui_mainwindow.h"





MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/loganalysistool.ico"));
    //if(NULL == controller)
        controller = new Controller();

}

MainWindow::~MainWindow()
{
    delete ui;

    //if(NULL != controller)
        delete controller;

}

void MainWindow::on_action_Open_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if(dialog.exec())
    {
        QStringList files = dialog.selectedFiles();


        //open file to extract log data
        controller->openFiles(files);
    }

}

void MainWindow::on_action_Save_triggered()
{

}
