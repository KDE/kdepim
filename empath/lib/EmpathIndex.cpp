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
# pragma implementation "EmpathIndex.h"
#endif

// System includes
#include <sys/file.h>
#include <unistd.h>

// Qt includes
#include <qdatastream.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathIndex.h"
#include "EmpathFolder.h"
#include "EmpathConfig.h"
#include "Empath.h"

// Increment this if you change the index format.
Q_UINT8 EmpathIndexVersion = 0;

/**
 * Provides access to a dict of EmpathIndexRecord.
 *
 * The index is mirrored on disk, automatically loaded when needed
 * and flagged as dirty when changed, such that it will be written
 * on destruction of the object.
 *
 * If the dict is not accessed within a time limit, it is flushed out.
 */

EmpathIndex::EmpathIndex(const EmpathURL & folder)
    :   folder_             (folder),
        initialised_        (false),
        dirty_              (false),
        read_               (false),
        count_              (0),
        unreadCount_        (0)
{
    dict_.setAutoDelete(true);

    QString resDir =
        KGlobal::dirs()->saveLocation("indices", folder.mailboxName(), true);

    QString path = resDir + "/" + folder.folderPath();

    bool ok = KGlobal::dirs()->makeDir(path);

    if (!ok) {
        empathDebug("KStdDirs wouldn't make dir `" + path + "'");
        return;
    }

    filename_ = path + "/index";
}

EmpathIndex::~EmpathIndex()
{
    _write();
}

    void
EmpathIndex::setFilename(const QString & filename)
{
    filename_ = filename;
    _flush();
}

    void
EmpathIndex::setFolder(const EmpathURL & folder)
{
    folder_ = folder;
    _flush();
}

    bool
EmpathIndex::contains(const QString & key)
{
    _read();
    _resetIdleTimer();
    return (0 != dict_.find(key));
}

    QDict<EmpathIndexRecord>
EmpathIndex::dict()
{
    _read();
    _resetIdleTimer();
    return dict_;
}

    EmpathIndexRecord
EmpathIndex::record(const QString & key)
{
    EmpathIndexRecord rec;
   
    if (!_read())
        return rec;

    EmpathIndexRecord * found = dict_[key];

    if (0 != found)
        rec = *found;

    _resetIdleTimer();
    return rec;
}

    unsigned int
EmpathIndex::count()
{
    _read();
    return count_;
    _resetIdleTimer();
}

    unsigned int
EmpathIndex::countUnread()
{
    _read();
    return unreadCount_;
}

    bool
EmpathIndex::_write()
{
    if (!dirty_ && QFile::exists(filename_))
        return false;
    
    QString tempFilename = filename_ + "." + QString::number(getpid());

    QFile f(tempFilename);

    if (!f.open(IO_WriteOnly)) {

        empathDebug("Could not open file for writing");
        return false;
    }


    QDataStream d(&f);

    d << EmpathIndexVersion;

    QDictIterator<EmpathIndexRecord> it(dict_);

    for (; it.current(); ++it)
        d << *(it.current());

    f.flush();
    f.close();

    if (f.status() != IO_Ok) {

        empathDebug("Couldn't close() file");
        return false;

    } else {

        QFile oldIndex(filename_);

        if (oldIndex.exists())
            oldIndex.remove();
    }

    if (0 !=
        ::link(QFile::encodeName(tempFilename), QFile::encodeName(filename_)))
    {

        empathDebug("Couldn't write index (`" + filename_ + "')");

    } else {

        dirty_ = false;
        QFile(tempFilename).remove();
    }
    
    f.remove();

    return true;
}

    bool
EmpathIndex::_read()
{
    if (read_)
        return true;

    killTimers();
    
    count_ = unreadCount_ = 0;

    QFile f(filename_);

    if (!f.open(IO_ReadOnly | IO_Raw)) {

        read_ = true;
        return true;
    }

    QByteArray a = f.readAll();

    QDataStream d(a, IO_ReadOnly);
    
    Q_UINT8 savedVersion;
    d >> savedVersion;

    if (savedVersion != EmpathIndexVersion) {
        read_ = false;
        return false;
    }

    EmpathIndexRecord rec;

    while (!d.eof()) {
        d >> rec;
        dict_.insert(rec.id(), new EmpathIndexRecord(rec));
        unreadCount_ += (rec.status() & EmpathIndexRecord::Read) ? 0 : 1;
        ++count_;
    }
    
    f.close();

    read_ = true;
    _resetIdleTimer();
    emit(countUpdated(count_, unreadCount_));
    return true;
}

    QStringList
EmpathIndex::allKeys()
{
    QStringList l;
    
    if (!_read())
        return l;
 
    QDictIterator<EmpathIndexRecord> it(dict_);

    for (; it.current(); ++it)
        l << it.currentKey();

    _resetIdleTimer();
    return l;
}

    bool
EmpathIndex::insert(const QString & key, EmpathIndexRecord & rec)
{
    if (!_read())
        return false;

    if (key.isEmpty()) {
        empathDebug("Key is empty !");
        return false;
    }

    dict_.insert(key, new EmpathIndexRecord(rec));
    
    _setDirty();
    _resetIdleTimer();
    return true;
}

    bool
EmpathIndex::replace(const QString & key, EmpathIndexRecord & rec)
{
    if (!_read())
        return false;

    if (key.isEmpty()) {
        empathDebug("Key is empty !");
        return false;
    }

    // Save old record first.
    EmpathIndexRecord oldrec = record(key);

    dict_.replace(key, new EmpathIndexRecord(rec));

    _setDirty();
    _resetIdleTimer();
    return true;
}

    bool
EmpathIndex::remove(const QString & key)
{  
    if (!_read())
        return false;

    EmpathIndexRecord r = record(key);

    bool ok = dict_.remove(key);

    if (ok) {
        _setDirty();
        emit(itemGone(key));
    }

    _resetIdleTimer();
    return ok;
}

    void
EmpathIndex::clear()
{
    if (_read())
        return;

    dict_.clear();
    _setDirty();
    _resetIdleTimer();
}

    bool
EmpathIndex::setStatus(const QString & key, EmpathIndexRecord::Status status)
{
    if (!_read())
        return false;

    EmpathIndexRecord * rec = dict_.find(key);
    
    if (0 == rec) {
        empathDebug("Couldn't find record");
        return false;
    }
    
    unsigned int oldStatus = rec->status();

    if (oldStatus != status) {

        rec->setStatus(status);
        _setDirty();
        emit(statusChange(key, status));
    }

    _resetIdleTimer();
    return true;
}

    void
EmpathIndex::_setDirty()
{
    dirty_ = true;
    emit(countUpdated(count_, unreadCount_));
}

    void
EmpathIndex::_resetIdleTimer()
{
    killTimers();
    startTimer(1000); // 1 sec, hardcoded for now.
}

    void
EmpathIndex::timerEvent(QTimerEvent *)
{
    killTimers();
    _flush();
}

    void
EmpathIndex::_flush()
{
    _write();
    dict_.clear();
    read_ = false;
}

    QDateTime
EmpathIndex::lastSync() const
{
    QFileInfo fi(filename_);
    if (fi.exists())
        return fi.lastModified();
    else
        return QDateTime(QDate(0, 1, 1));
}

// vim:ts=4:sw=4:tw=78
