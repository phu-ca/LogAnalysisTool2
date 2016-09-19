#include <QFile>
#include <QDir>
#include <QProcess>
#include <QApplication>
#include <QDebug>
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
}

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
        qDebug() << filename << dir << " " << tid << " " << device << " " << *fn;

        if (!filename.isEmpty())
        {
            QString dd = (tid.isEmpty())?workingspace:((device.isEmpty())?workingspace + "/" + tid:workingspace + "/" + tid + "/" + device);
            if (dd == workingspace) dir = filename;
            QDir d(dd + "/" + dir);
            if (d.exists()) continue; else d.mkpath (dd + "/" + dir);
            qDebug () << dd << dd + "/" + dir;

            QStringList params;
            QProcess untar;
            QString tarfile = *fn;
            if (fn->contains(".tar.gz"))
            {
                params << "e" << *fn << "-o" + tmpdir;
                qDebug() << params;

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
                qDebug() << params;

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
