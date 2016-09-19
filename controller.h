#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QStringList>
#include "workingspace.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = 0);
    void openFiles(QStringList files);

signals:

public slots:

private:
    WorkingSpace &ws;
};

#endif // CONTROLLER_H
