#ifndef LOGFILE_H
#define LOGFILE_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QList>
#include <QHash>
#include <QRegExp>

class LogFile {
private:
    int searchBegin (const int m, const int n, const QDate t);
    int searchEnd (const int m, const int n, const QDate t);

protected:
    QString path;
    QString filename;
    QString year;
    QStringList parsed_contents;
    int len;

    virtual QDate getDate(const int idx) = 0;

public:
    LogFile (const QString p, const QString names)
    {
        path = p;
        filename = names;

        QRegExp d("([0-9]{4})-([0-9]{2})-([0-9]{2}).");
        if (d.indexIn(path) != -1)
            year = d.cap(1);
    }
    virtual ~LogFile () {}

    virtual void Parsed() = 0;
    int SearchBegin (const QDate sdate);
    int SearchEnd (const QDate edate);
    void KeywordsSearch (const int sidx, const int eidx, const QStringList keywords, QStringList &searched);
    void EventsSearch(const int sidx, const int eidx, const QList<QHash<QString, QString>> &events, QHash<QString, int> &searched);
};

class TraceLog : public LogFile
{
public:
    TraceLog (const QString p, const QString n) : LogFile (p, n) {}

    QDate getDate (const int idx);
    void Parsed ();
};

class MessagesLog : public LogFile
{
public:
    MessagesLog (const QString p, const QString n) : LogFile (p, n) {}

    QDate getDate (const int idx);
    void Parsed();
};

class SysLog : public LogFile
{
public:
    SysLog (const QString p, const QString n) : LogFile (p, n) {}

    QDate getDate(const int idx);
    void Parsed();
};

class CrashLog : public LogFile
{
public:
    CrashLog (const QString p, const QString n) : LogFile (p, n) {}

    QDate getDate(const int idx);
    void Parsed();
};

#endif // LOGFILE_H
