/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
#include "EmpathTask.h"
#include "Empath.h"
#include "EmpathMailbox.h"

EmpathMaildir::EmpathMaildir(const QString & basePath, const EmpathURL & url)
    :   QObject(),
        url_(url),
        basePath_(basePath)
{
    path_ = basePath + "/" + url.folderPath();
    
    QObject::connect(&timer_, SIGNAL(timeout()),
        this, SLOT(s_timerBeeped()));
    
    timer_.start(10000, true); // 10 seconds. Hard coded for now.
}

EmpathMaildir::~EmpathMaildir()
{
    // Empty.
}

    void
EmpathMaildir::init()
{
    if (_checkDirs()) {
        _clearTmp();
        sync();
    }
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
        
    tagList_.clear();
    
    _markNewMailAsSeen();
    _tagOrAdd(f);
    _removeUntagged(f);
    _recalculateCounters(f);
    
    f->setIndexInitialised();
}

    QMap<QString, bool>
EmpathMaildir::mark(const QStringList & l, RMM::MessageStatus msgStat)
{
    empathDebug("Number to mark: " + QString::number(l.count()));
    QMap<QString, bool> successMap;
    
    EmpathTask * t = new EmpathTask (i18n("Marking messages"));
    t->setMax(l.count());
    
    QStringList::ConstIterator it(l.begin());
    
    for (; it != l.end(); ++it) {
        successMap[*it] = _mark(*it, msgStat);
        t->doneOne();
    }

    t->done();
    return successMap;
}

    QString
EmpathMaildir::writeMessage(RMM::RMessage m)
{
    return _write(m);
}

    RMM::RMessage
EmpathMaildir::message(const QString & id)
{
    QCString s = _messageData(id);
    
    if (s.isEmpty()) {
        empathDebug("Couldn't load data for \"" + id + "\"");
        return RMM::RMessage();
    }
    
    return RMM::RMessage(s);
}

    QMap<QString, bool>
EmpathMaildir::removeMessage(const QStringList & l)
{
    QMap<QString, bool> successMap;

    EmpathTask * t = new EmpathTask(i18n("Removing messages"));

    t->setMax(l.count());

    QStringList::ConstIterator it(l.begin());
    
    for (; it != l.end(); ++it) {
        successMap[*it] = _removeMessage(*it);
        t->doneOne();
    }
    
    t->done();

    return successMap;
}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    bool
EmpathMaildir::_removeMessage(const QString & id)
{
    QDir d(path_ + "/cur/", id + "*");

    if (d.count() != 1) return false;
    
    EmpathFolder * folder(empath->folder(url_));
    
    QFile f(path_ + "/cur/" + d[0]);    
    
    if (!f.remove())
        return false;
    
    folder->removeFromIndex(id);

    return true;
}

    bool
EmpathMaildir::_mark(const QString & id, RMM::MessageStatus msgStat)
{
    empathDebug("");
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

    QString oldName = path_ + "/cur/" + filename;
    QString newName = path_ + "/cur/" + newFilename;
    
    bool renameOK = QDir().rename(oldName, newName);
    
    if (!renameOK)
        empath->s_infoMessage(i18n("Couldn't mark message") +
           " [" + id + "] with flags " + statusFlags);
 
    EmpathFolder * f(empath->folder(url_));

    if (f == 0) {
        empathDebug("Cannot access my folder !");
        return renameOK;
    }

    f->update();
   
    return renameOK;
}

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
        
        if (cur.count() == 0) {
            empathDebug("Can't match the filename, giving up.");
            return "";
        }
        
        if (cur.count() > 1) {
            empathDebug("Duplicate messages with the name `" + filename + "'");
            empathDebug("Using the first one I found.");
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
EmpathMaildir::_recalculateCounters(EmpathFolder * f)
{
    QDir d(
        path_ + "/cur",
        QString::null,
        QDir::Unsorted,
        QDir::Files | QDir::NoSymLinks);
    
    QRegExp re_flags(":2,[A-Z]*$");
    QString s;
    unsigned int unread(0);

    QStringList l(d.entryList());
    
    for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {

        s = *it;

        if (s[0] != '.') { // Ignore dotfiles.
            
            int i = s.find(re_flags);
            
            // If no flags or 'S' not in flags, then unread.
            if ((i == -1) || !(s.right(s.length() - i - 3).contains('S')))
                ++unread;
        }
    }

    f->setIndexUnread(unread);
}

    void
EmpathMaildir::_markNewMailAsSeen()
{
    QDir dn(
        path_ + "/new",
        QString::null,
        QDir::Unsorted,
        QDir::Files | QDir::NoSymLinks | QDir::Writable);

    QStringList l(dn.entryList());
    
    for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {

        if ((*it)[0] != '.') {

            _markAsSeen(*it);
            kapp->processEvents();
        }
    }
}

    void
EmpathMaildir::_markAsSeen(const QString & name)
{
    QString oldName = path_ + "/new/" + name;
    QString newName = path_ + "/cur/" + name;

    if (!QDir().rename(oldName, newName)) {
        empathDebug("Couldn't rename `" + oldName + "' to `" + newName + "'");
    }
}

    void
EmpathMaildir::_clearTmp()
{
    QDate now = QDate::currentDate();
    
    QDir tmpDir(
        path_ + "/tmp/",
        QString::null,
        QDir::Unsorted,
        QDir::Files | QDir::NoSymLinks | QDir::Writable);

    const QFileInfoList * infoList = tmpDir.entryInfoList();

    if (infoList) {
        
        QFileInfoListIterator it(*infoList);
        
        for (; it.current(); ++it) {
            
            if (it.current()->lastRead().daysTo(now) > 2) {
                
                if (!tmpDir.remove(it.current()->filePath(), true)) {
                    empathDebug("Couldn't delete " + it.current()->filePath());
                }
            }
            
            kapp->processEvents();
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
    
    kapp->processEvents();
    return true;
}

    QString
EmpathMaildir::_write(RMM::RMessage msg)
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

    QString linkName(canonName + ":2," + flags);

    QString linkPath(path_ + "/new/" + linkName);
    
    if (::link(QFile::encodeName(path), QFile::encodeName(linkPath)) != 0) {
        empathDebug("Couldn't successfully link `" + path + "' to `" +
            linkPath + "' - giving up");
        perror("link");
        f.close();
        f.remove();
        return QString::null;
    }
    
    _markAsSeen(linkName);
    
    return canonName;
}

    QString
EmpathMaildir::_generateFlagsString(RMM::MessageStatus s)
{
    QString flags;
    
    if (s & RMM::Read)      flags += 'S';
    if (s & RMM::Marked)    flags += 'F';
    if (s & RMM::Trashed)   flags += 'T';
    if (s & RMM::Replied)   flags += 'R';
    
    return flags;
}
    
    void
EmpathMaildir::s_timerBeeped()
{
    timer_.stop();
    
    EmpathFolder * f(empath->folder(url_));

    if (f == 0) {
        empathDebug("Cannot access my folder !");
        return;
    }

    f->update();

    timer_.start(10000, true);
}

    bool
EmpathMaildir::_touched(EmpathFolder * f)
{
    if (!(f->indexInitialised()))
        return true;
    
    QFileInfo fiDir(path_ + "/cur/");
    
    if (fiDir.lastModified() > f->indexModified()) {
        empathDebug("Index file is older than Maildir/cur");
        return true;
    }
    
    fiDir = (path_ + "/new/");
    
    if (fiDir.lastModified() > f->indexModified()) {
        empathDebug("Index file is older than Maildir/new");
        return true;
    }
 
    return false;
}

    void
EmpathMaildir::_tagOrAdd(EmpathFolder * f)
{
    QDir d(
        path_ + "/cur/",
        QString::null,
        QDir::Name | QDir::IgnoreCase,
        QDir::NoSymLinks | QDir::Files
        );

    QStringList fileList(d.entryList());
    
    // Finished reading dir. Get the UI going again quick ! :)
    kapp->processEvents();

    QStringList::ConstIterator it(fileList.begin());
    
    QString s;
    QRegExp re_flags(":2,[A-Z]*$");

    for (; it != fileList.end(); ++it) {

        s = *it;

        RMM::MessageStatus status(RMM::MessageStatus(0));
        
        int i = s.find(re_flags);
        QString flags;
        
        if (i != -1) {
            
            flags = s.right(s.length() - i - 3);
            
            status = (RMM::MessageStatus)
                (   (flags.contains('S') ? RMM::Read    : 0)    |
                    (flags.contains('R') ? RMM::Replied : 0)    |
                    (flags.contains('F') ? RMM::Marked  : 0));
        }
        
        s.replace(re_flags, QString::null);
        
        tagList_.append(QString(s));

        bool exists = f->indexContains(s);

        if (exists) {
        
            EmpathIndexRecord rec = f->indexRecord(s);

            if (rec.isNull())
                continue;
            
            if (rec.status() != status) {
                rec.setStatus(status);
                f->replaceInIndex(rec.id(), rec);
            }
        
        } else {
 
            QCString messageData = _messageData(*it, true);

            if (messageData.isEmpty()) {
                empathDebug("Message data not retrieved !");
                continue;
            }

            RMM::RMessage m(messageData);
            EmpathIndexRecord ir(s, m);
            
            ir.setStatus(status);
            
            f->insertInIndex(s, ir);
            f->itemCome(s);
        }

        kapp->processEvents();
    }
}

    void
EmpathMaildir::_removeUntagged(EmpathFolder * f)
{
    QStringList l(f->allIndexKeys());

    QStringList::ConstIterator it;
    
    for (it = l.begin(); it != l.end(); ++it) {

        if (!tagList_.contains(*it)) {

            f->removeFromIndex(*it);
            f->itemGone(*it);
        }
    }
}

// vim:ts=4:sw=4:tw=78
