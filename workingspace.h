#ifndef WORKINGSPACE_H
#define WORKINGSPACE_H

#include <QString>
#include <QObject>
#include <QStringList>
#include <QTextEdit>
#include <QDate>
#include <QList>
#include <QHash>
#include <QJsonArray>

class WorkingSpace : public QObject
{
    Q_OBJECT
private:
    QString workingspace;
    QString tmpdir;
    QJsonArray events;

    WorkingSpace(QString ws, QObject *parent=0);
    WorkingSpace(const WorkingSpace&);
    void operator= (const WorkingSpace&);

    QList<QHash<QString, QString>> getEvents (QString filter);

public:
    static WorkingSpace& GetInstance (QString ws)
    {
        static WorkingSpace instance(ws);
        return instance;
    }

    enum Action
    {
        KEYWORDS_SEARCH,
        EVENTS_SEARCH
    };

    bool Untar (const QStringList filenames);
    void Analyze (Action act, const QStringList params, const QStringList objects, const QDate sdate, const QDate edate, const QStringList filters, QTextEdit *viewer);

/* working space is moved to controller
    bool Open (const QStringList filenames);
    QStringList GetFolderList ();
    QStringList GetFolderList (QString tid);
    QStringList GetFileList (QString tid, QString dir);

private:
    QStringList getFolderList (QString rpath);
*/
};

#endif // WORKINGSPACE_H
