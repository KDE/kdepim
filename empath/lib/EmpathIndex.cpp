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
    :   blockSize_(1024),
        dbf_(0),
        count_(0),
        unreadCount_(0),
        initialised_(false)
{
    // Empty.
    empathDebug("");
}

EmpathIndex::EmpathIndex(const EmpathURL & folder)
	:	blockSize_(1024),
        folder_(folder),
        dbf_(0),
        count_(0),
        unreadCount_(0),
        initialised_(false)
{
    empathDebug(folder.asString());
    QString resDir =
        KGlobal::dirs()->saveLocation("indices", folder.mailboxName(), true);

    QDir d(resDir);
    
    if (!d.exists()) {
        if (!d.mkdir(resDir)) {
            empathDebug("Cannot make index dir " + resDir);
            return;
        }
    }
    
    QString legalName = folder.folderPath().replace(QRegExp("/"), "_");
    
    // filename_ = resDir + "/" + legalName;
    filename_ = "/tmp/" + legalName;

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
    if ((dbf_ == 0) && !_open()) {
        empathDebug("Index not open!");
        return 0;
    }
    
    datum k;
    k.dptr  = key.data();
    k.dsize = key.length();
    
    datum out = gdbm_fetch(dbf_, k);

    if (!out.dptr) {
        return 0;
    }
    
    QByteArray a;
    a.setRawData(out.dptr + 1, out.dsize - 1);
    
    EmpathIndexRecord * rec = new EmpathIndexRecord;

    QDataStream s(a, IO_ReadOnly);
    s >> *rec;
    
    a.resetRawData(out.dptr + 1, out.dsize - 1);
    
    rec->setStatus((RMM::MessageStatus)(out.dptr[0]));
    
    return rec;
}

    Q_UINT32
EmpathIndex::countUnread()
{
    return unreadCount_;
}

     Q_UINT32
EmpathIndex::count()
{
    return count_;
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
    
    gdbm_close(dbf_);
    
    count_ = unreadCount_ = 0;
}

    bool
EmpathIndex::_open()
{
    if (dbf_ != 0) {
        empathDebug("Already open");
        return true;
    }

    dbf_ = gdbm_open(
        filename_.local8Bit().data(), blockSize_, GDBM_WRCREAT, 0600, NULL);
    
    if (dbf_ == 0) {
        empathDebug(gdbm_strerror(gdbm_errno));
        return false;
    }

    count_ = unreadCount_ = 0;

    return true;
}

    QStrList
EmpathIndex::allKeys()
{
    QStrList l;   

    if ((dbf_ == 0) && !_open()) {
        empathDebug("Index not open!");
        return l;
    }
 
    datum key = gdbm_firstkey(dbf_);

    while (key.dptr) {
        
        QCString s(key.dptr, key.dsize + 1);
        
        l.append(s);
        
        key = gdbm_nextkey(dbf_, key);
    }
  
    return l;
}

    bool
EmpathIndex::insert(const QCString & key, EmpathIndexRecord & rec)
{
    if ((dbf_ == 0) && !_open()) {
        empathDebug("Index not open!");
        return false;
    }

    datum k;
    k.dptr  = key.data();
    k.dsize = key.length();
    
    QByteArray a;
    QDataStream s(a, IO_WriteOnly);
    s << rec;
    
    unsigned int dataSize = 1 + a.size();
    char * data = new char[dataSize];
    memcpy(data + 1, a.data(), a.size());
    data[0] = (unsigned char)(rec.status());
    
    datum d;
    d.dptr  = data;
    d.dsize = dataSize;
    
    int retval = gdbm_store(dbf_, k, d, GDBM_REPLACE);
    
    if (retval == -1) {
        empathDebug("Could not insert record !");
        return false;
    }

    ++count_;
    
    if (!(rec.status() & RMM::Read))
        ++unreadCount_;
    
    return true;
}

    bool
EmpathIndex::remove(const QCString & key)
{  
    if ((dbf_ == 0) && !_open()) {
        empathDebug("Index not open!");
        return false;
    }

    datum k;
    
    k.dptr  = const_cast<char *>(key.data());
    k.dsize = key.length();
 
    datum die = gdbm_fetch(dbf_, k);

    if (!die.dptr) {
        empathDebug("Record does not exist");
        return false;
    }
    
    RMM::MessageStatus status = (RMM::MessageStatus)(die.dptr[0]);

    bool ok = (gdbm_delete(dbf_, k) == 0);
    
    if (ok) {
        --count_;
        if (!(status & RMM::Read))
            --unreadCount_;
    
    } else {
        empathDebug("Could not delete record");
    }
    
    return ok;
}

    void
EmpathIndex::clear()
{
    if ((dbf_ == 0) && !_open()) {
        empathDebug("Index not open!");
        return;
    }
    
    datum key = gdbm_firstkey(dbf_);

    while (key.dptr) {
        
        gdbm_delete(dbf_, key);
        key = gdbm_nextkey(dbf_, key);
    }

    count_ = unreadCount_ = 0;
}

    void
EmpathIndex::setStatus(const QString & id, RMM::MessageStatus status)
{
    if ((dbf_ == 0) && !_open()) {
        empathDebug("Index not open!");
        return;
    }
    
    datum k;
    k.dptr  = const_cast<char *>(id.data());
    k.dsize = id.length();
    
    datum changer = gdbm_fetch(dbf_, k);

    if (!changer.dptr) {
        empathDebug("does not exist");
        return;
    }
    
    bool wasRead = ((RMM::MessageStatus)(changer.dptr[0])) & RMM::Read;
    bool isRead = status & RMM::Read;
    
    changer.dptr[0] = (unsigned char)(status);

    int retval = gdbm_store(dbf_, k, changer, GDBM_REPLACE);

    if (retval == -1) {
        empathDebug("Couldn't replace record");
        return;
    }
    
    if (wasRead && !isRead)
        unreadCount_++;
    
    if (!wasRead && isRead)
        unreadCount_--;
}
    
    QDateTime
EmpathIndex::lastModified() const
{
    QFileInfo fi(filename_);
    return fi.lastModified();
}	 
	

// vim:ts=4:sw=4:tw=78
