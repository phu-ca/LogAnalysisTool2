#include "controller.h"

Controller::Controller(QObject *parent) : QObject(parent), ws(WorkingSpace::GetInstance("c:/working/Downloads/WorkingSpace"))
{

}
void Controller::openFiles(QStringList files)
{
    ws.Open(files);
}
