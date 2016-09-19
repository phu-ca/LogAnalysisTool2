#ifndef WORKINGSPACE_H
#define WORKINGSPACE_H

#include <QString>
#include <QObject>
#include <QStringList>
#include <QJsonObject>

class WorkingSpace : public QObject
{
    Q_OBJECT
private:
    QString workingspace;
    QString tmpdir;

    WorkingSpace(QString ws, QObject *parent=0);
    WorkingSpace(const WorkingSpace&);
    void operator= (const WorkingSpace&);

public:
    static WorkingSpace& GetInstance (QString ws)
    {
        static WorkingSpace instance(ws);
        return instance;
    }

    bool ExecuteCommand (const QJsonObject in, QJsonObject &out);

public:
    bool Open (const QStringList filenames);
};

#endif // WORKINGSPACE_H
