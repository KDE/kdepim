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

// Qt includes
#include <qdatastream.h>
#include <qregexp.h>
#include <qfileinfo.h>

// KDE includes
#include <kglobal.h>
#include <kstddirs.h>

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathIndex.h"
#include "EmpathFolder.h"
#include "Empath.h"

// Implementation note:
// The actual data (the value) is composed of an 8-bit byte, used for
// marking status, plus a serialised version of an EmpathIndexRecord.
// This saves us the time it takes to [de]serialise records.

EmpathIndex::EmpathIndex()
    :   blockSize_(1024),
        touched_(true),
        count_(0),
        unreadCount_(0)
{
    empathDebug("ctor");
    // Empty.
}

EmpathIndex::EmpathIndex(const EmpathURL & folder)
	:	blockSize_(1024), folder_(folder)
{
    empathDebug("ctor");
    QString resDir =
        KGlobal::dirs()->getSaveLocation("indices", folder.mailboxName(), true);

    empathDebug("saveLocation: " + resDir);

    if (resDir.isEmpty()) {
        empathDebug("Serious problem with local indices dir");
    }

    QString legalName = folder.folderPath().replace(QRegExp("/"), "_");

    // filename_ = resDir + "/" + legalName;  
    filename_ = "/tmp/" + legalName;
	
    empathDebug("Index filename: " + filename_);

    _open();
}

EmpathIndex::~EmpathIndex()
{
    empathDebug("");
    _close();
}

    void
EmpathIndex::setFilename(const QString & filename)
{
    empathDebug("");
    filename_ = filename;
    _open();
}

    void
EmpathIndex::setFolder(const EmpathURL & folder)
{
    empathDebug("");
    folder_ = folder;
}

    EmpathIndexRecord *
EmpathIndex::record(const QCString & key)
{
    empathDebug(QString(key));

    if (!dbf_) {
        empathDebug("dbf is not open");
        return 0;
    }
    
    datum k;
    k.dptr  = key.data();
    k.dsize = key.length();
    
    datum out = gdbm_fetch(dbf_, k);

    if (!out.dptr) {
        empathDebug("does not exist");
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
    if (!touched_)
        return unreadCount_;
    
    if (!dbf_) {
        empathDebug("dbf is not open");
        return 0;
    }

    unreadCount_ = 0;
    
    datum key = gdbm_firstkey(dbf_);
    
    while (key.dptr) {

        if (!((RMM::MessageStatus)(key.dptr[0]) & RMM::Read))
            ++unreadCount_;

        key = gdbm_nextkey(dbf_, key);
    }
    
    empathDebug("done");
    touched_ = false;
    return unreadCount_;
}

    Q_UINT32
EmpathIndex::count()
{
    if (!touched_)
        return count_;
    
    if (!dbf_) {
        empathDebug("dbf is not open");
        return 0;
    }

    count_ = 0;
    
    datum key = gdbm_firstkey(dbf_);
    
    while (key.dptr) {

        ++count_;
        key = gdbm_nextkey(dbf_, key);
    }
    
    touched_ = false;
    return count_;
}


    void
EmpathIndex::sync()
{
    empathDebug("");
    
    if (!dbf_) {
        empathDebug("dbf is not open");
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
    empathDebug("");
    
    if (!dbf_) {
        empathDebug("dbf is not open");
        return;
    }
    
    gdbm_close(dbf_);
}

    void
EmpathIndex::_open()
{
    empathDebug(filename_);
    
    dbf_ = gdbm_open(
        filename_.local8Bit().data(), blockSize_, GDBM_WRCREAT, 0600, NULL);
    
    if (!dbf_) {
        empathDebug(gdbm_strerror(gdbm_errno));
        touched_ = false;
    } else {
        touched_ = true;
    }
}

    QStrList
EmpathIndex::allKeys()
{
    empathDebug("");
    
    QStrList l;   

    if (!dbf_) {
        empathDebug("dbf is not open");
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
    empathDebug(key);

    if (!dbf_) {
        empathDebug("dbf is not open");
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
    
    touched_ = true;
    
    return (retval != -1);
}

    bool
EmpathIndex::remove(const QCString & key)
{  
    empathDebug(QString(key));

    if (!dbf_) {
        empathDebug("dbf is not open");
        return false;
    }


    datum k;
    
    k.dptr  = const_cast<char *>(key.data());
    k.dsize = key.length();
    
    touched_ = true;
    return (gdbm_delete(dbf_, k) == 0);
}

    void
EmpathIndex::clear()
{
    empathDebug("");

    if (!dbf_) {
        empathDebug("dbf is not open");
        return;
    }
    
    datum key = gdbm_firstkey(dbf_);

    while (key.dptr) {
        
        gdbm_delete(dbf_, key);
        key = gdbm_nextkey(dbf_, key);
    }

    touched_ = true;
}

    void
EmpathIndex::setStatus(const QString & id, RMM::MessageStatus status)
{
    if (!dbf_) {
        empathDebug("dbf is not open");
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
    
    changer.dptr[0] = (unsigned char)(status);

    int retval = gdbm_store(dbf_, k, changer, GDBM_REPLACE);

    touched_ = true;

    if (retval == -1) {
        empathDebug("Couldn't replace record");
    }
}
    
    QDateTime
EmpathIndex::lastModified() const
{
    QFileInfo fi(filename_);
    return fi.lastModified();
}	 
	

// vim:ts=4:sw=4:tw=78
