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

// Qt includes
#include <qdatastream.h>
#include <qregexp.h>
#include <qfileinfo.h>
#include <qdir.h>

// KDE includes
#include <kglobal.h>
#include <kstddirs.h>

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathIndex.h"
#include "EmpathFolder.h"
#include "Empath.h"

EmpathIndex::EmpathIndex()
    :   dbf_(0),
        initialised_(false)
{
    mtime_ = QDateTime::currentDateTime();
}

EmpathIndex::EmpathIndex(const EmpathURL & folder)
    :   folder_(folder),
        dbf_(0),
        initialised_(false)
{
    QString resDir =
        KGlobal::dirs()->saveLocation("indices", folder.mailboxName(), true);

    QString path = resDir + "/" + folder.folderPath();

    bool ok = KGlobal::dirs()->makeDir(path);

    if (!ok) {
        empathDebug("KStdDirs wouldn't make dir `" + path + "'");
        return;
    }

    filename_ = path + "/index";
    
    QFileInfo fi(filename_);

    if (!fi.exists())
        initialised_ = false;
    
    _open();
}

EmpathIndex::~EmpathIndex()
{
    _close();
}

    void
EmpathIndex::setFilename(const QString & filename)
{
    filename_ = filename;
    _close();
    _open();
}

    void
EmpathIndex::setFolder(const EmpathURL & folder)
{
    folder_ = folder;
}

    EmpathIndexRecord
EmpathIndex::record(const QString & key)
{
    EmpathIndexRecord rec;

    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return rec;
    }

    if (key.isEmpty()) {
        empathDebug("I'm not retrieving using an empty key");
        return rec;
    }

    QByteArray a = dbf_->retrieve(key);

    if (a.isNull())
        return rec;
    
    QDataStream s(a, IO_ReadOnly);
    s >> rec;
    
    return rec;
}

    Q_UINT32
EmpathIndex::countUnread() const
{
    return dbf_->unreadCount();
}

     Q_UINT32
EmpathIndex::count() const
{
    return dbf_->index().count();
}
    
    void
EmpathIndex::sync()
{
    if ((dbf_ == 0) && !_open()) {
        empathDebug("Index not open!");
        return;
    }

    EmpathFolder * f = empath->folder(folder_);
    
    if (!f) {
        empathDebug("Can't access my folder :(");
        return;
    }
    
    f->update();
}

    void
EmpathIndex::_close()
{
    if (dbf_ == 0) {
        empathDebug("dbf is not open");
        return;
    }
    
    delete dbf_;
    dbf_ = 0;
}

    bool
EmpathIndex::_open()
{
    if (dbf_ != 0) {
        empathDebug("Already open");
        return true;
    }

    dbf_ = new RDB::Database(filename_);
    
    mtime_ = dbf_->lastModified();
    return true;
}

    QStringList
EmpathIndex::allKeys() const
{
    QStringList l;

    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return l;
    }
 
    RDB::IndexIterator it(dbf_->index());

    for (; it.current(); ++it)
        l.append(it.currentKey());

    return l;
}

    bool
EmpathIndex::insert(const QString & key, EmpathIndexRecord & rec)
{
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return false;
    }

    if (key.isEmpty()) {
        empathDebug("Key is empty !");
        return false;
    }

    QByteArray a;
    QDataStream s(a, IO_WriteOnly);
    s << rec;
    
    bool ok = dbf_->insert(key, a);
    
    if (!ok) {
        empathDebug("Could not insert record !");
        return false;
    }

    if (!(rec.status() & RMM::Read))
        dbf_->increaseUnreadCount();
    
    mtime_ = QDateTime::currentDateTime();
    return true;
}

    bool
EmpathIndex::replace(const QString & key, EmpathIndexRecord & rec)
{
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return false;
    }

    if (key.isEmpty()) {
        empathDebug("Key is empty !");
        return false;
    }
    
    EmpathIndexRecord r = record(key);

    QByteArray a;
    QDataStream s(a, IO_WriteOnly);
    s << rec;
    
    bool dummy;
    bool ok = dbf_->replace(key, a, dummy);
    
    if (!ok) {
        empathDebug("Could not replace record !");
        return false;
    }

    if (!r.isNull() && !(r.status() & RMM::Read))
        dbf_->decreaseUnreadCount();

    mtime_ = QDateTime::currentDateTime();
    return true;
}

    bool
EmpathIndex::remove(const QString & key)
{  
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return false;
    }
    
    EmpathIndexRecord r = record(key);

    bool ok = dbf_->remove(key);
    
    if (ok) {

        if (!r.isNull() && !(r.status() & RMM::Read))
            dbf_->decreaseUnreadCount();
        
        mtime_ = QDateTime::currentDateTime();
    
    } else {
        empathDebug("Could not delete record");
    }
    
    return ok;
    
}

    void
EmpathIndex::clear()
{
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return;
    }
    
    dbf_->clear();

    mtime_ = QDateTime::currentDateTime();
}

    void
EmpathIndex::setStatus(const QString & key, RMM::MessageStatus status)
{
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return;
    }

    EmpathIndexRecord rec = record(key);
    
    if (rec.isNull()) {
        empathDebug("Couldn't find record");
        return;
    }
    
    rec.setStatus(status);

    replace(key, rec);

    mtime_ = QDateTime::currentDateTime();
}
    
    QDateTime
EmpathIndex::lastModified() const
{
    return mtime_;
}	 

    bool
EmpathIndex::contains(const QString & s) const
{
    return dbf_->exists(s);
}
	

// vim:ts=4:sw=4:tw=78
