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

// Implementation note:
//
// The value part is composed of an 8-bit byte, used for marking status,
// plus a serialised version of an EmpathIndexRecord. This saves us
// [de]serialising records when we only want to access the status.

EmpathIndex::EmpathIndex()
    :   dbf_(0),
        count_(0),
        unreadCount_(0),
        initialised_(false)
{
    // Empty.
    empathDebug("");
}

EmpathIndex::EmpathIndex(const EmpathURL & folder)
    :   folder_(folder),
        dbf_(0),
        count_(0),
        unreadCount_(0),
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

    EmpathIndexRecord *
EmpathIndex::record(const QCString & key)
{
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return 0;
    }

    if (key.isEmpty()) {
        empathDebug("I'm not retrieving using an empty key");
        return 0;
    }

    QByteArray a = dbf_->retrieve(key);

    if (a.isNull())
        return 0;
    
    EmpathIndexRecord * rec = new EmpathIndexRecord;

    QDataStream s(a, IO_ReadOnly);
    s >> *rec;
    
    return rec;
}

    Q_UINT32
EmpathIndex::countUnread() const
{
    // STUB
    return 0;
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
    
    count_ = unreadCount_ = 0;
}

    bool
EmpathIndex::_open()
{
    if (dbf_ != 0) {
        empathDebug("Already open");
        return true;
    }

    dbf_ = new RDB::Database(filename_);
    
    count_ = unreadCount_ = 0;

    return true;
}

    QStrList
EmpathIndex::allKeys() const
{
    QStrList l;   

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
EmpathIndex::insert(const QCString & key, EmpathIndexRecord & rec)
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

    ++count_;
    
    if (!(rec.status() & RMM::Read))
        ++unreadCount_;
    
    return true;
}

    bool
EmpathIndex::replace(const QCString & key, EmpathIndexRecord & rec)
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
    
    bool dummy;
    bool ok = dbf_->replace(key, a, dummy);
    
    if (!ok) {
        empathDebug("Could not replace record !");
        return false;
    }

    return true;
}

    bool
EmpathIndex::remove(const QCString & key)
{  
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return false;
    }

    bool ok = dbf_->remove(key);
    
    if (ok) {
        --count_;
        // FIXME
     //   if (!(status & RMM::Read))
     //       --unreadCount_;
    
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

    count_ = unreadCount_ = 0;
}

    void
EmpathIndex::setStatus(const QString & id, RMM::MessageStatus status)
{
    if (dbf_ == 0) {
        empathDebug("Index not open!");
        return;
    }

    // TODO
    
}
    
    QDateTime
EmpathIndex::lastModified() const
{
    return dbf_->lastModified();
}	 

    bool
EmpathIndex::contains(const QCString & s) const
{
    return dbf_->exists(s);
}
	

// vim:ts=4:sw=4:tw=78
