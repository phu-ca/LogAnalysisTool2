#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include "logfile.h"
#include "workingspace.h"

WorkingSpace::WorkingSpace(QString ws, QObject *parent) : QObject(parent)
{
    if (!ws.isEmpty())
    {
        workingspace = ws;
        if (workingspace.at(workingspace.size()-1) == '/')
            workingspace.chop(1);
        tmpdir = ws + "/tmp";

        QDir d1(workingspace), d2(tmpdir);
        if (!d1.exists())
            d1.mkpath(workingspace);
        if (!d2.exists())
            d2.mkpath(tmpdir);
    }

    QFile fd("templates.json");
    if (fd.open(QIODevice::ReadOnly))
    {
        QByteArray data = fd.readAll();
        events = QJsonDocument(QJsonDocument::fromJson(data)).object()["events"].toArray();
    }
}

bool WorkingSpace::Untar(const QStringList filenames)
{
    bool ret = false;

    for (QStringList::const_iterator fn = filenames.begin(); fn != filenames.end(); fn ++)
    {
        if (fn->contains(".tar"))
        {
            int p1 = fn->lastIndexOf ("/"), p2 = fn->indexOf(".tar");
            QString filename = (p1 == -1)?fn->mid(0,p2):fn->mid(p1+1, p2-p1-1);

            if (!filename.isEmpty())
            {
                QDir d(tmpdir + "/" + filename);
                if (!d.exists()) d.mkpath(tmpdir + "/" + filename); else { ret = true; continue; }

                QStringList params;
                QProcess untar;
                QString tarfile = *fn;
                if (fn->contains(".tar.gz"))
                {
                    params << "e" << *fn << "-o" + tmpdir;
                    untar.start("7z", params);
                    if (!untar.waitForFinished(60*1000))
                        continue;

                    params.clear();
                    tarfile = tmpdir + "/" + filename + ".tar";
                }

                if (tarfile.contains(".tar"))
                {
                    params << "x" << tarfile << "-o" + tmpdir + "/" + filename;
                    untar.start("7z", params);
                    qApp->processEvents();
                    if (untar.waitForFinished(60*1000))
                        ret = true;
                }
                QFile::remove(tmpdir + "/" + filename + ".tar");
            }
        }
    }

    return ret;
}

void WorkingSpace::Analyze(Action act, const QStringList params, const QStringList objects, const QDate sdate, const QDate edate, const QStringList filters, QTextEdit *viewer)
{
    QHash<QString, int> events_summary;
    QStringList keywords_summary;

    if (sdate > edate)
        return;

    for (QStringList::const_iterator i = objects.begin(); i != objects.end(); i ++)
    {
        QString tid = i->split(":")[0];
        QStringList dirs = i->split(":")[1].split(",");

        if (!tid.isEmpty() && dirs.size() > 0)
        {
            for (QStringList::const_iterator j = dirs.begin(); j != dirs.end(); j ++)
            {
                QDir d(workingspace + "/" + tid + "/" + *j);
                if (!d.exists())
                {
                    d.setPath(workingspace + "/" + tid + "/portal/" + *j);
                    if (!d.exists())
                    {
                        d.setPath(workingspace + "/" + tid + "/sped/" + *j);
                        if (!d.exists())
                            continue;
                    }
                }

                if (QDate().fromString(j->split(".")[0], "yyyy-MM-dd") < sdate)
                    continue;

                for (QStringList::const_iterator m = filters.begin(); m != filters.end(); m ++)
                {
                    QStringList filter = m->split(",");
                    for (QStringList::const_iterator n = filter.begin(); n != filter.end(); n++)
                    {
                        QString f = *n;
                        if (!f.contains("*"))
                            f += "*";

                        d.setPath(workingspace + "/" + tid + "/" + *j + "/var/log");
                        QFileInfoList flist = d.entryInfoList(QStringList() << f, QDir::Files);
                        if (flist.size() <= 0)
                        {
                            d.setPath(workingspace + "/" + tid + "/" + *j + "/var/log/mainapp");
                            flist = d.entryInfoList(QStringList() << f, QDir::Files);
                            if (flist.size() <= 0)
                                continue;
                        }

                        QList<QHash<QString, QString>> e;
                        if (act == EVENTS_SEARCH)
                        {
                            e = getEvents(f);
                            if (e.size() <= 0)
                                continue;
                        }

                        QSharedPointer<LogFile> lf;
                        if (f.contains("Data.Tracing") || f.contains("commerr") || f.contains("carderr") || f.contains("card") || f.contains("commtest") || f.contains("dialtest") )
                            lf = QSharedPointer<LogFile>(new TraceLog(d.path(), f));
                        else if (f.contains("messages"))
                            lf = QSharedPointer<LogFile>(new MessagesLog(d.path(), f));
                        else if (f.contains("syslog"))
                            lf = QSharedPointer<LogFile>(new SysLog(d.path(), f));
                        else if (f.contains("crash"))
                            lf = QSharedPointer<LogFile>(new CrashLog(d.path(), f));

                        lf->Parsed();
                        int sidx = lf->SearchBegin(sdate);
                        int eidx = lf->SearchEnd(edate);
                        if (sidx > eidx || sidx == -1 || eidx == -1)
                            continue;

                        switch (act)
                        {
                            case KEYWORDS_SEARCH:
                                {
                                    QStringList s;
                                    lf->KeywordsSearch(sidx, eidx, params, s);
                                    keywords_summary += s;
                                    break;
                                }
                            case EVENTS_SEARCH:
                                {
                                    QHash<QString,int> r;
                                    lf->EventsSearch(sidx, eidx, e, r);
                                    for (QHash<QString, int>::const_iterator k = r.begin(); k != r.end(); k ++)
                                    {
                                        if (events_summary.contains(k.key()))
                                            events_summary[k.key()] += k.value();
                                        else
                                            events_summary[k.key()] = k.value();
                                    }
                                    break;
                                }
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }

    switch (act)
    {
        case KEYWORDS_SEARCH:
            viewer->clear();
            viewer->append(keywords_summary.join("\n"));
            break;
        case EVENTS_SEARCH:
            viewer->clear();
            for (QHash<QString,int>::const_iterator l = events_summary.begin(); l != events_summary.end(); l ++)
                viewer->append(QString("%s: %d\n").arg(l.key()).arg(l.value()));
            break;
        default:
            break;

    }
}

QList<QHash<QString, QString>> WorkingSpace::getEvents(QString filter)
{
    QList<QHash<QString, QString>> ret;
    for (int i = 0; i < events.size(); i++)
    {
        QHash<QString, QString> event;
        QJsonObject json = events[i].toObject();

        if (json["File"].toString().contains(filter.left(5)))
        {
            if (!json["Name"].toString().isEmpty())
                event["Name"] = json["Name"].toString();

            if (!json["Token"].toString().isEmpty())
                event["Token"] = json["Token"].toString();

            if (!json["StartToken"].toString().isEmpty())
                event["StartToken"] = json["StartToken"].toString();

            if (!json["EndToken"].toString().isEmpty())
                event["EndToken"] = json["EndToken"].toString();

            if (event.size() > 0)
                ret << event;
        }
    }

    return ret;
}

/* working space is moved to controller
bool WorkingSpace::Open(const QStringList filenames)
{
    bool ret = true;

    for (QStringList::const_iterator fn = filenames.begin(); fn != filenames.end(); fn++)
    {
        if (!fn->contains(".tar"))
        {
            ret = false;
            continue;
        }

        int p1 = fn->lastIndexOf("_"), p2 = fn->indexOf(".tar"), p3=fn->lastIndexOf("/");
        QString filename = (p3 == -1)?fn->mid(0, p2):fn->mid(p3+1, p2-p3-1);
        QString dir = fn->mid(p1+1, p2-p1-1 );
        QString tmp = (p3 == -1)?fn->mid(0, p1):fn->mid(p3+1, p1-p3-1);
        QString tid = tmp.contains("_")?tmp.mid(0,tmp.indexOf("_")):tmp;
        QString device = tmp.contains("_")?tmp.mid(tmp.indexOf("_")+1):"";

        if (!filename.isEmpty())
        {
            QString dd = (tid.isEmpty())?workingspace:((device.isEmpty())?workingspace + "/" + tid:workingspace + "/" + tid + "/" + device);
            if (dd == workingspace) dir = filename;
            QDir d(dd + "/" + dir);
            if (d.exists()) continue; else d.mkpath (dd + "/" + dir);

            QStringList params;
            QProcess untar;
            QString tarfile = *fn;
            if (fn->contains(".tar.gz"))
            {
                params << "e" << *fn << "-o" + tmpdir;
                untar.start("7z", params);
                qApp->processEvents();
                if (!untar.waitForFinished(60*1000))
                {
                  ret = false;
                  continue;
                }
                tarfile = tmpdir + "/" + filename + ".tar";
                params.clear();
            }

            if (tarfile.contains(".tar"))
            {
                params << "x" << tarfile << "-o" + dd + "/" + dir;
                untar.start("7z", params);
                qApp->processEvents();
                if (!untar.waitForFinished(60*1000))
                  ret = false;
            }
            QFile::remove(tmpdir + "/" + filename + ".tar");
        }
        else
            ret = false;
    }

    return ret;
}

QStringList WorkingSpace::getFolderList(QString rpath)
{
    QDir ws(rpath);
    QFileInfoList dlist = ws.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    QStringList dirs;
    for (QFileInfoList::const_iterator i = dlist.begin(); i != dlist.end(); i ++)
    {
        if (i->fileName() == "portal" || i->fileName() == "sped")
        {
            QDir subd(rpath + "/" + i->fileName());
            QFileInfoList sublist = subd.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

            for (QFileInfoList::const_iterator i = sublist.begin(); i != sublist.end(); i ++)
            {
                if (i->fileName() != "tmp")
                    dirs << i->fileName();
            }
        }
        else if (i->fileName() != "tmp")
            dirs << i->fileName();
    }

    return dirs;
}

QStringList WorkingSpace::GetFolderList()
{
    return getFolderList(workingspace);
}

QStringList WorkingSpace::GetFolderList(QString tid)
{
    return getFolderList(workingspace + "/" + tid);
}

QStringList WorkingSpace::GetFileList(QString tid, QString dir)
{
    QDir d(workingspace + "/" + tid + "/" + dir);
    if (!d.exists())
    {
        d.setPath(workingspace + "/" + tid + "/portal/" + dir);
        if (!d.exists())
        {
            d.setPath(workingspace + "/" + tid + "/sped/" + dir);
            if (!d.exists())
                return QStringList();
        }
    }

    QStringList flist;
    QDirIterator it(d.path(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        flist << it.fileName();
    }
    return flist;
}
*/
