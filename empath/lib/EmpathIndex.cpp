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

EmpathIndex::EmpathIndex(const EmpathURL & folder)
    :   folder_(folder),
        initialised_(false),
        dirty_(false)
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
EmpathIndex::record(const QString & key) const
{
    EmpathIndexRecord rec;
   
    EmpathIndexRecord * found = dict_[key];

    if (0 != found)
        rec = *found;

    return rec;
}

    unsigned int
EmpathIndex::countUnread() const
{
    unsigned int unreadCount = 0;

    QDictIterator<EmpathIndexRecord> it(dict_);

    for (; it.current(); ++it)
        unreadCount +=
            ((it.current()->status() & EmpathIndexRecord::Read) ? 0 : 1);

    return unreadCount;
}

    void
EmpathIndex::sync()
{
    EmpathFolder * f = empath->folder(folder_);
    
    if (!f) {
        empathDebug("Can't access my folder :(");
        return;
    }
    
    f->update();
    _close(); // == write(), but needs renaming.
}

    void
EmpathIndex::_close()
{
    if (!dirty_ && QFile(filename_).exists())
        return;

    QString tempFilename = filename_ + "." + QString::number(getpid());

    QFile f(tempFilename);

    if (!f.open(IO_WriteOnly))
        return;

    QDataStream d(&f);

    QDictIterator<EmpathIndexRecord> it(dict_);

    for (; it.current(); ++it)
        d << *(it.current());

    f.flush();
    f.close();

    if (f.status() != IO_Ok) {

        empathDebug("Couldn't close() file");
        return;

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

        empathDebug("Index written successfully (` " + filename_ + "')");
        dirty_ = false;

    }
    
    f.remove();
}

    bool
EmpathIndex::_open()
{
    QFile f(filename_);

    if (!f.open(IO_ReadOnly))
        return false;

    QDataStream d(&f);

    EmpathIndexRecord rec;

    while (!d.eof()) {
        d >> rec;
        dict_.insert(rec.id(), new EmpathIndexRecord(rec));
    }
    
    f.close();

    return true;
}

    QStringList
EmpathIndex::allKeys() const
{
    QStringList l;
 
    QDictIterator<EmpathIndexRecord> it(dict_);

    for (; it.current(); ++it)
        l << it.currentKey();

    return l;
}

    bool
EmpathIndex::insert(const QString & key, EmpathIndexRecord & rec)
{
    if (key.isEmpty()) {
        empathDebug("Key is empty !");
        return false;
    }

    dict_.insert(key, new EmpathIndexRecord(rec));
    
    _setDirty();
    return true;
}

    bool
EmpathIndex::replace(const QString & key, EmpathIndexRecord & rec)
{
    if (key.isEmpty()) {
        empathDebug("Key is empty !");
        return false;
    }

    // Save old record first.
    EmpathIndexRecord oldrec = record(key);

    dict_.replace(key, new EmpathIndexRecord(rec));

    _setDirty();
    return true;
}

    bool
EmpathIndex::remove(const QString & key)
{  
    EmpathIndexRecord r = record(key);

    bool ok = dict_.remove(key);

    if (ok)
        _setDirty();

    return ok;
}

    void
EmpathIndex::clear()
{
    dict_.clear();
    _setDirty();
}

    void
EmpathIndex::setStatus(const QString & key, EmpathIndexRecord::Status status)
{
    EmpathIndexRecord * rec = dict_.find(key);
    
    if (0 == rec) {
        empathDebug("Couldn't find record");
        return;
    }
    
    unsigned int oldStatus = rec->status();

    if (oldStatus != status) {
 
        rec->setStatus(status);
        _setDirty();
    }
}

    void
EmpathIndex::_setDirty()
{
    dirty_ = true;
}
    
// vim:ts=4:sw=4:tw=78
