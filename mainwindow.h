#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "controller.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void openFile();


public slots:

   void on_action_Open_triggered();

   void on_action_Save_triggered();

protected:
    Controller *controller;

private:
    Ui::MainWindow *ui;




};

#endif // MAINWINDOW_H
