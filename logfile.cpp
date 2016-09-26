#include <QDir>
#include "logfile.h"

int LogFile::SearchBegin(const QDate sdate)
{
    if (len <= 0)
        return -1;
    else if (sdate <= getDate(0))
        return 0;

    return searchBegin (0, len - 1, sdate);
}

int LogFile::SearchEnd(const QDate edate)
{
    if (len <= 0)
        return -1;
    else if (edate >= getDate(len - 1))
        return len - 1;

    return searchEnd (0, len - 1, edate);
}

int LogFile::searchBegin(const int m, const int n, const QDate t)
{
    if (m >= n)
    {
        if (getDate(n) >= t)
            return n;
        else if ( getDate(m) >= t)
            return m;
        else
            return -1;
    }

    int i = (m + n) / 2;
    QDate d = getDate(i);
    if (d >= t)
    {
        if (i > 0)
        {
            if (getDate(i - 1) < t)
                return i;
            else
                return searchBegin(m, i - 1, t);
        }
        else
            return 0;
    }
    else
    {
        if (i < len - 1)
            return searchBegin(i + 1, n, t);
        else
            return -1;
    }
}

int LogFile::searchEnd(const int m, const int n, const QDate t)
{
    if (m >= n)
    {
        if (getDate(m) <= t)
            return m;
        else if (getDate(n) <= t)
            return n;
        else
            return -1;
    }

    int i = (m + n) / 2;
    QDate d = getDate(i);
    if (d <= t)
    {
        if (i < len - 1)
        {
            if (getDate(i + 1) > t)
                return i;
            else
                return searchEnd(i + 1, n, t);
        }
        else
            return len - 1;
    }
    else
    {
        if (i > 0)
            return searchEnd(m, i - 1, t);
        else
            return -1;
    }
}

void LogFile::KeywordsSearch(const int sidx, const int eidx, const QStringList keywords, QStringList &searched)
{
    for (int i = sidx; i <= eidx; i ++)
    {
        for (QStringList::const_iterator j = keywords.begin(); j != keywords.end(); j ++)
        {
            if (parsed_contents.at(i).contains(*j))
                searched << parsed_contents.at(i);
        }
    }
}

void LogFile::EventsSearch(const int sidx, const int eidx, const QList<QHash<QString, QString> > &events, QHash<QString, int> &searched)
{
    QHash<QString, int> tracing;
    for (QList<QHash<QString, QString>>::const_iterator m = events.begin(); m != events.end(); m ++)
    {
        if (m->contains("StartToken") && m->contains("EndToken"))
            tracing[(*m)["Name"]] = 0;
    }

    for (int i = sidx; i <= eidx; i ++)
    {
        for (QList<QHash<QString, QString>>::const_iterator j = events.begin(); j != events.end(); j ++)
        {
            if (j->contains("Token"))
            {
                if (parsed_contents.at(i).contains((*j)["Token"]))
                {
                    if (searched.contains((*j)["Name"]))
                        searched[(*j)["Name"]] += 1;
                    else
                        searched[(*j)["Name"]] = 1;
                }
            }
            else if (j->contains("StartToken") && j->contains("EndToken"))
            {
                if (parsed_contents.at(i).contains((*j)["StartToken"]) || parsed_contents.at(i).contains((*j)["EndToken"]))
                    tracing[(*j)["Name"]] += 1;

                if (tracing[(*j)["Name"]] == 2)
                {
                    if (searched.contains((*j)["Name"]))
                        searched[(*j)["Name"]] += 1;
                    else
                        searched[(*j)["Name"]] = 1;

                    tracing[(*j)["Name"]] = 0;
                }
            }
        }
    }
}

void TraceLog::Parsed()
{
    QDir d(path);
    QFileInfoList flist = d.entryInfoList(QStringList() << filename, QDir::Files, QDir::Time|QDir::Reversed);
    for (QFileInfoList::const_iterator i = flist.begin(); i != flist.end(); i ++)
    {
        QFile file (i->absoluteFilePath());
        if (file.open(QIODevice::ReadOnly))
            parsed_contents += QString(file.readAll()).split("\n");
    }

    len = parsed_contents.size();
}

QDate TraceLog::getDate(const int idx)
{
    QRegExp datestr ("^(.)*\\|([0-9]{2}-[0-9]{2}) ");
    if (datestr.indexIn(parsed_contents.at(idx)) != -1)
        return QDate().fromString(datestr.cap(2) + "-" + year, "MM-dd-yyyy");
    else
        return QDate();
}

void MessagesLog::Parsed()
{
    QDir d(path);
    QFileInfoList flist = d.entryInfoList(QStringList() << filename, QDir::Files, QDir::Time|QDir::Reversed);
    for (QFileInfoList::const_iterator i = flist.begin(); i != flist.end(); i ++)
    {
        QFile file (i->absoluteFilePath());
        if (file.open(QIODevice::ReadOnly))
            parsed_contents += QString(file.readAll()).split("\n");
    }

    len = parsed_contents.size();
}

QDate MessagesLog::getDate(const int idx)
{
    QRegExp datestr ("^([a-zA-Z]{3}) +([0-9]{1,2}) ");
    if (datestr.indexIn(parsed_contents.at(idx)) != -1)
        return QDate().fromString(datestr.cap(1) + " " + datestr.cap(2) + " " + year, "MMM d yyyy");
    else
        return QDate();
}

void SysLog::Parsed()
{
    QDir d(path);
    QFileInfoList flist = d.entryInfoList(QStringList() << filename, QDir::Files, QDir::Name);
    for (QFileInfoList::const_iterator i = flist.begin(); i != flist.end(); i ++)
    {
        QFile file (i->absoluteFilePath());
        if (file.open(QIODevice::ReadOnly))
            parsed_contents += QString(file.readAll()).split("\n");
    }

    len = parsed_contents.size();
}

QDate SysLog::getDate(const int idx)
{
    QRegExp datestr ("^([a-zA-Z]{3}) +([0-9]{1,2}) ");
    if (datestr.indexIn(parsed_contents.at(idx)) != -1)
        return QDate().fromString(datestr.cap(1) + " " + datestr.cap(2) + " " + year, "MMM d yyyy");
    else
        return QDate();
}

void CrashLog::Parsed()
{
    QFile file(path + "/crash.report");
    if (file.open(QIODevice::ReadOnly))
    {
        QStringList data = QString(file.readAll()).split("\n");
        QString item;
        bool found = false;
        for (QStringList::const_iterator i = data.begin(); i != data.end(); i ++ )
        {
            QRegExp r("Crashed: [0-9]{14}");
            if (r.indexIn(*i) != -1)
            {
                found = true;
                if (item.size() > 0)
                {
                    parsed_contents << item;
                    item.clear();
                }
                else
                    item = *i;
            }
            else if (found)
                item += "\n" + *i;
        }

        if (item.size() > 0)
            parsed_contents << item;
    }

    len = parsed_contents.size();
}

QDate CrashLog::getDate(const int idx)
{
    QRegExp r("Crashed: ([0-9]{14})");
    if (r.indexIn(parsed_contents.at(idx)) != -1)
        return QDate().fromString(r.cap(1).left(8), "yyyyMMdd");
    else
        return QDate();
}
