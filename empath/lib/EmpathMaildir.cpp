/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathMaildir.h"
#endif

// System includes
#include <sys/file.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// Qt includes
#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qstringlist.h>

// KDE includes
#include <kapp.h>
#include <klocale.h>

// Local includes
#include "RMM_Envelope.h"
#include "EmpathMaildir.h"
#include "EmpathFolder.h"
#include "EmpathFolderList.h"
#include "EmpathIndex.h"
#include "Empath.h"
#include "EmpathTask.h"
#include "EmpathMailbox.h"

EmpathMaildir::EmpathMaildir(const QString & basePath, const EmpathURL & url)
    :   QObject(),
        url_(url),
        basePath_(basePath)
{
    path_ = basePath + "/" + url.folderPath();
    
    QObject::connect(&timer_, SIGNAL(timeout()),
        this, SLOT(s_timerBeeped()));
    
//    timer_.start(30000, true); // 30 seconds ok ? Hard coded for now.
}

EmpathMaildir::~EmpathMaildir()
{
    // Empty.
}

    void
EmpathMaildir::init()
{
    _checkDirs();           kapp->processEvents();
    _clearTmp();            kapp->processEvents();
    
    sync();
}
    void
EmpathMaildir::sync(bool force)
{
    EmpathFolder * f(empath->folder(url_));

    if (f == 0) {
        empathDebug("Cannot access my folder !");
        return;
    }

    if (!force && !_touched(f))
        return;
    
    kapp->processEvents();

    _markNewMailAsSeen();
    
    kapp->processEvents();
    
    _tagOrAdd(f);
    
    kapp->processEvents();

    _removeUntagged(f);
    
    f->index()->setInitialised(true);
}

    bool
EmpathMaildir::mark(const QString & id, RMM::MessageStatus msgStat)
{
    QRegExp re_flags(":2,[A-Za-z]*$");
    QDir d(path_ + "/cur/", id + "*");
 
    QString statusFlags;

    statusFlags += msgStat & RMM::Read    ? "S" : "";
    statusFlags += msgStat & RMM::Marked  ? "F" : "";
    statusFlags += msgStat & RMM::Trashed ? "T" : "";
    statusFlags += msgStat & RMM::Replied ? "R" : "";
    
    if (d.count() != 1) {
        empath->s_infoMessage(i18n("Couldn't mark message") +
            " [" + id + "] with flags " + statusFlags);
        return false;
    }
    
    QString filename(d[0]);
    
    QString newFilename(filename);
    
    if (!newFilename.contains(":2,"))
        newFilename += ":2," + statusFlags;
    else
        newFilename.replace(QRegExp(":2,.*"), ":2," + statusFlags);
    
    bool retval =
        d.rename(path_ + "/cur/" + filename, path_ + "/cur/" + newFilename);
    
    if (!retval)
        empath->s_infoMessage(i18n("Couldn't mark message") +
           " [" + id + "] with flags " + statusFlags);
 
    EmpathFolder * f(empath->folder(url_));

    if (f == 0) {
        empathDebug("Cannot access my folder !");
        return retval;
    }

    f->update();
   
    return retval;
}

    bool
EmpathMaildir::mark(const QStringList & l, RMM::MessageStatus msgStat)
{
    bool retval = true;
    
    EmpathTask * t(empath->addTask(i18n("Marking messages")));
    t->setMax(l.count());
    
    QStringList::ConstIterator it(l.begin());
    
    for (; it != l.end(); ++it) {
        if (!mark(*it, msgStat))
            retval = false;
        t->doneOne();
    }

    t->done();
    return retval;
}

    QString
EmpathMaildir::writeMessage(RMM::RMessage & m)
{
    return _write(m);
}

    RMM::RMessage *
EmpathMaildir::message(const QString & id)
{
    QCString s = _messageData(id);
    
    if (s.isEmpty()) {
        empathDebug("Couldn't load data for \"" + id + "\"");
        return 0;
    }
    
    RMM::RMessage * m = new RMM::RMessage(s);
    CHECK_PTR(m);
    return m;
}

    bool
EmpathMaildir::removeMessage(const QString & id)
{
    QDir d(path_ + "/cur/", id + "*");

    if (d.count() != 1) return false;
    
    EmpathFolder * folder(empath->folder(url_));
    
    QFile f(path_ + "/cur/" + d[0]);    
    
    if (!f.remove())
        return false;
    
    folder->index()->remove(id.latin1());

    return true;
}

    bool
EmpathMaildir::removeMessage(const QStringList & l)
{
    bool retval = true;

    EmpathTask * t(empath->addTask(i18n("Removing messages")));

    t->setMax(l.count());

    QStringList::ConstIterator it(l.begin());
    
    for (; it != l.end(); ++it) {
        if (!removeMessage(*it))
            retval = false;
        t->doneOne();
    }
    
    t->done();

    return retval;
}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    QCString
EmpathMaildir::_messageData(const QString & filename, bool isFullName)
{
    QString filename_(filename);

    if (filename.length() == 0) {
        empathDebug("Must supply filename !");
        return "";
    }

    if (!isFullName) {

        // We need to locate the actual file, by looking for the basename
        // with the flags section appended.
        
        QDir cur(path_ + "/cur/", filename + "*");
        
        if (cur.count() != 1) {
            empathDebug("Can't match the filename, giving up.");
            return "";
        }

        filename_ = cur[0];
    }
    
    QFile f(path_ + "/cur/" + filename_);

    if (!f.open(IO_ReadOnly)) {
        empathDebug("Couldn't open mail file " + filename_ + " for reading.");
        return "";
    }

    Q_UINT32 buflen = 32768;
    char * buf = new char[buflen];
    QCString messageBuffer;
    
    while (!f.atEnd()) {
        
        int bytesRead = f.readBlock(buf, buflen);

        if (bytesRead == -1) {
            empathDebug("A serious error occurred while reading the file.");
            delete [] buf;
            buf = 0;
            f.close();
            return "";
        }
        
        messageBuffer += QCString(buf).left(bytesRead);
    }

    delete [] buf;
    buf = 0;
    f.close();
    return messageBuffer;
}


    void
EmpathMaildir::_markNewMailAsSeen()
{
    QDir dn(path_ + "/new");

    dn.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Writable);
    
    QStringList l = dn.entryList();
    
    for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
        _markAsSeen(*it);
}

    void
EmpathMaildir::_markAsSeen(const QString & name)
{
    QString oldName = path_ + "/new/" + name;

    QString newName = path_ + "/cur/" + name + ":2,";
            
    if (::rename(oldName.latin1(), newName.latin1()) != 0)
        perror("rename");
}

    void
EmpathMaildir::_clearTmp()
{
    QDate now = QDate::currentDate();
    QDateTime aTime;
    
    QDir d(path_ + "/tmp/");
    
    if (d.count() == 0) {
        empathDebug("Can't clear tmp !");
        return;
    }
    
    d.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Writable);
    
    QFileInfoListIterator it(*(d.entryInfoList()));
    
    for (; it.current(); ++it) {
        
        aTime = it.current()->lastRead();
        
        if (aTime.daysTo(now) > 2) {
            
            empathDebug("Deleting \"" +
                QString(it.current()->filePath()) + "\"");

            d.remove(it.current()->filePath(), false);
        }
    }
}

    bool
EmpathMaildir::_checkDirs()
{
    QDir d(path_);
    
    if (!d.exists() && !d.mkdir(path_)) {
        empathDebug("Couldn't create " + path_);
        return false;
    }
    
    if (!d.exists(path_ + "/cur") && !d.mkdir(path_ + "/cur")) {
        empathDebug("Couldn't create " + path_ + "/cur");
        return false;
    }
    
    if (!d.exists(path_ + "/new") && !d.mkdir(path_ + "/new")) {
        empathDebug("Couldn't create " + path_ + "/new");
        return false;
    }
    
    if (!d.exists(path_ + "/tmp") && !d.mkdir(path_ + "/tmp")) {
        empathDebug("Couldn't create " + path_ + "/tmp");
        return false;
    }
    
    return true;
}

    QString
EmpathMaildir::_write(RMM::RMessage & msg)
{
    // See docs for how this shit works.
    // I can't be bothered to maintain the comments.

    QString canonName   = empath->generateUnique();
    QString flags       = _generateFlagsString(msg.status());
    QString path        = path_ + "/tmp/" + canonName;

    QFile f(path);
    
    if (f.exists()) {
        
        for (int i = 0 ; i < 20 ; i++) {
            usleep(100);
            kapp->processEvents();
        }
    
        if (f.exists())
            return QString::null;
    }

    if (!f.open(IO_WriteOnly)) {
        empathDebug("Couldn't open file for writing.");
        return QString::null;
    }

    QDataStream outputStream(&f);

    outputStream.writeRawBytes(msg.asString(), msg.asString().length());

    f.flush();

    if (f.status() != IO_Ok) {
        empathDebug("Couldn't flush() file");
        f.close();
        f.remove();
        return QString::null;
    }

    f.close();
   
    if (f.status() != IO_Ok) {
        empathDebug("Couldn't close() file");
        f.close();
        f.remove();
        return QString::null;
    }

    QString linkTarget(path_ + "/new/" + canonName + ":2," + flags);
    
    if (::link(path.latin1(), linkTarget.latin1()) != 0) {
        empathDebug("Couldn't successfully link file - giving up");
        perror("link");
        f.close();
        f.remove();
        return QString::null;
    }
    
    _markAsSeen(canonName);
    
    return canonName;
}

    QString
EmpathMaildir::_generateFlagsString(RMM::MessageStatus s)
{
    QString flags;
    
    flags += s & RMM::Read      ? "S" : "";
    flags += s & RMM::Marked    ? "F" : "";
    flags += s & RMM::Trashed   ? "T" : "";
    flags += s & RMM::Replied   ? "R" : "";
    
    return flags;
}
    
    void
EmpathMaildir::s_timerBeeped()
{
    timer_.stop();
    sync();
    timer_.start(30000, true);
}

    bool
EmpathMaildir::_touched(EmpathFolder * f)
{
    if (!(f->index()->initialised()))
        return true;
    
    QFileInfo fiDir(path_ + "/cur/");
    
    QFileInfo fiIndex(f->index()->indexFileName());
    
    if (!fiIndex.exists()) {
        empathDebug("Index file does not yet exist");
        return true;
    }
    
    if (fiDir.lastModified() > f->index()->lastModified()) {
        empathDebug("Index file is older than Maildir");
        return true;
    }

    return false;
}

    void
EmpathMaildir::_tagOrAdd(EmpathFolder * f)
{
    EmpathTask * t(empath->addTask(i18n("Scanning")));

    QDir d(
        path_ + "/cur/",
        QString::null,
        QDir::Name | QDir::IgnoreCase,
        QDir::NoSymLinks | QDir::Files
        );

    QStringList fileList(d.entryList());

    QStringList::ConstIterator it(fileList.begin());
    
    t->setMax(fileList.count());
    
    QCString s;
    QRegExp re_flags(":2,[A-Za-z]*$");

    for (; it != fileList.end(); ++it) {
        
        s = (*it).utf8();

        RMM::MessageStatus status(RMM::MessageStatus(0));
        
        int i = s.find(re_flags);
        QCString flags;
        
        if (i != -1) {
            
            flags = s.right(s.length() - i - 3);
            
            status = (RMM::MessageStatus)
                (   (flags.contains('S') ? RMM::Read    : 0)    |
                    (flags.contains('R') ? RMM::Replied : 0)    |
                    (flags.contains('F') ? RMM::Marked  : 0));
        }
        
        s.replace(re_flags, QString::null);

        empathDebug("s == `" + s + "'");
        
        EmpathIndexRecord * rec = f->index()->record(s);
        
        if (rec != 0) {
        
            rec->tag(true);
            rec->setStatus(status);
        
        } else {
 
            QCString messageData = _messageData(*it, true);
            RMM::RMessage m(messageData);
            EmpathIndexRecord ir(s, m);
            
            ir.tag(true);
            ir.setStatus(status);
            
            f->index()->insert(s, ir);
            f->itemCome(s);
        }

        t->doneOne();
    }

    t->done();
}

    void
EmpathMaildir::_removeUntagged(EmpathFolder * f)
{
    EmpathTask * t(empath->addTask(i18n("Clearing")));

    QStrList l(f->index()->allKeys());
    
    t->setMax(l.count());

    QStrListIterator iit(l);
    
    for (; iit.current(); ++iit) {
        
        t->doneOne();
        
        EmpathIndexRecord * i = f->index()->record(iit.current());
        
        if (i == 0) {

            empathDebug("Can't find record, but I'd called allKeys !");
            continue;
        }

        if (!i->isTagged()) {
            
            f->itemGone(iit.current());
            f->index()->remove(iit.current());
        }
    }
    
    t->done();
}

// vim:ts=4:sw=4:tw=78
