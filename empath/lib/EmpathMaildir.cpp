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
#include <qstringlist.h>

// KDE includes
#include <kapp.h>
#include <klocale.h>

// Local includes
#include "EmpathMaildir.h"
#include "EmpathUtilities.h"
#include "EmpathIndex.h"
#include "EmpathIndexRecord.h"
#include "EmpathTask.h"
#include "Empath.h"

EmpathMaildir::EmpathMaildir(const QString & basePath, const EmpathURL & url)
    :   QObject(),
        url_(url),
        basePath_(basePath)
{
    path_ = basePath + "/" + url.folderPath();
    disappeared_.setAutoDelete(true);

    index_ = new EmpathIndex(url);

    QObject::connect(
        &timer_,    SIGNAL(timeout()),
        this,       SLOT(s_timerBeeped()));

    timer_.start(10000, true); // 10 seconds. Hard coded for now.
}

EmpathMaildir::~EmpathMaildir()
{
    // Empty.
}

    void
EmpathMaildir::init()
{
    // FIXME don't read this when we're not in a thread.
    QDir d(_cur(), QString::null, QDir::Unsorted, QDir::Files);

    cachedEntryList_ = d.entryList();

    mtime_ = QFileInfo(_cur()).lastModified();

    createdOK_ = _checkDirs();
}

    void
EmpathMaildir::sync()
{
    if (!_touched())
        return;

    disappeared_.clear();

    _markNewMailAsSeen();
    _tagAsDisappearedOrAddToIndex();
    _removeDisappeared();
}

    QString
EmpathMaildir::writeMessage(RMM::Message m)
{
    return _write(m);
}

    RMM::Message
EmpathMaildir::message(const QString & id)
{
    empathDebug(id);
    RMM::Message retval;

    QCString s = _messageData(id);

    if (s.isEmpty()) {
        empathDebug("Cannot load data for \"" + id + "\"");
        return retval;
    }

    retval = RMM::Message(s);
    return retval;
}

    EmpathSuccessMap
EmpathMaildir::mark(const QStringList & l, EmpathIndexRecord::Status msgStat)
{
    empathDebug("Number to mark: " + QString::number(l.count()));
    EmpathSuccessMap successMap;

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

    EmpathSuccessMap
EmpathMaildir::removeMessage(const QStringList & l)
{
    EmpathSuccessMap successMap;

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
    QDir d(_cur(), id + "*", QDir::Unsorted);

    if (d.count() != 1) return false;

    QFile f(_cur() + d[0]);

    return f.remove();
}

    bool
EmpathMaildir::_mark(const QString & id, EmpathIndexRecord::Status msgStat)
{
    QStringList matchingEntries = _entryList().grep(id);

    if (matchingEntries.count() != 1) {
        empathDebug("Couldn't match exactly one entry with id `" + id + "'");
        return false;
    }

    QString statusFlags = _generateFlagsString(msgStat);

    QString filename(matchingEntries[0]);

    QString newFilename(filename);

    if (!newFilename.contains(":2,"))
        newFilename += ":2," + statusFlags;
    else
        newFilename.replace(QRegExp(":2,.*"), ":2," + statusFlags);

    QString oldName = _cur() + filename;
    QString newName = _cur() + newFilename;

    bool renameOK = QDir().rename(oldName, newName);

    if (!renameOK) {
        empathDebug("Failed");
        return false;
    }

    return renameOK;
}

    QCString
EmpathMaildir::_messageData(const QString & filename, bool isFullName)
{
    if (filename.isEmpty()) {

        empathDebug("No filename supplied !");
        return "";
    }

    QString filename_(filename);

    if (!isFullName) {

        // We need to locate the actual file, by looking for the basename
        // with the flags section appended.


        QStringList matchingEntries = _entryList().grep(filename);

        if (matchingEntries.count() != 1) {
            empathDebug("Cannot match `" + filename + "'");
            return "";
        }

        filename_ = matchingEntries[0];
    }

    QFile f(_cur() + filename_);

    if (!f.open(IO_ReadOnly)) {

        empathDebug("Cannot open file " + filename_ + " for reading");
        return "";
    }

    QByteArray a(f.readAll());
    QCString messageBuffer(a.data(), a.size());

    f.close();

    return messageBuffer;
}

    void
EmpathMaildir::_markNewMailAsSeen()
{
    empathDebug("");
    QDir dn(
        _new(),
        QString::null,
        QDir::Unsorted,
        QDir::Files | QDir::Writable);

    QStringList l(dn.entryList());

    for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
        if ((*it)[0] != '.')
            _markAsSeen(*it);
}

    void
EmpathMaildir::_markAsSeen(const QString & name)
{
    QString oldName = _new() + name;
    QString newName = _cur() + name;

    if (!QDir().rename(oldName, newName)) {
        empathDebug("Couldn't rename `" + oldName + "' to `" + newName + "'");
    }
}

    void
EmpathMaildir::_clearTmp()
{
    QDate now = QDate::currentDate();

    QDir tmpDir(
        _tmp(),
        QString::null,
        QDir::Unsorted,
        QDir::Files | QDir::Writable);

    const QFileInfoList * infoList = tmpDir.entryInfoList();

    if (infoList)
        for (QFileInfoListIterator it(*infoList); it.current(); ++it)
            if (it.current()->lastRead().daysTo(now) > 2)
                tmpDir.remove(it.current()->filePath(), true);
}

    bool
EmpathMaildir::_checkDirs()
{
    QDir d(path_);

    if (!d.exists() && !d.mkdir(path_)) {
        empathDebug("Couldn't create " + path_);
        return false;
    }

    if (!d.exists(_cur()) && !d.mkdir(_cur())) {
        empathDebug("Couldn't create " + _cur());
        return false;
    }

    if (!d.exists(_new()) && !d.mkdir(_new())) {
        empathDebug("Couldn't create " + _new());
        return false;
    }

    if (!d.exists(_tmp()) && !d.mkdir(_tmp())) {
        empathDebug("Couldn't create " + _tmp());
        return false;
    }

    return true;
}

    QString
EmpathMaildir::_write(RMM::Message msg)
{
    // See docs for how this shit works.
    // I can't be bothered to maintain the comments.

    // FIXME not thread safe
    QString canonName   = empath->generateUnique();

    QString flags =
        _generateFlagsString(EmpathIndexRecord::Status(msg.status()));

    QString path   = _tmp() + canonName;

    QFile f(path);

    if (f.exists()) {

        for (int i = 0 ; i < 20 ; i++) {
            usleep(100000);
        }

        if (f.exists()) {
            empathDebug("File exists");
            return QString::null;
        }
    }

    if (!f.open(IO_WriteOnly)) {
        empathDebug("Couldn't open file for writing");
        return QString::null;
    }

    QDataStream outputStream(&f);

    outputStream << msg.asString();

    f.flush();
    f.close();

    if (f.status() != IO_Ok) {
        empathDebug("Couldn't flush/close file");
        f.close();
        f.remove();
        return QString::null;
    }

    QString linkName(canonName + ":2," + flags);

    QString linkPath(_new() + linkName);

    if (::link(QFile::encodeName(path), QFile::encodeName(linkPath)) != 0) {

        empathDebug("Couldn't link `" + path + "' to `" + linkPath + "'");
        perror("link");
        f.close();
        f.remove();
        return QString::null;
    }

    _markAsSeen(linkName);

    return canonName;
}

    QString
EmpathMaildir::_generateFlagsString(EmpathIndexRecord::Status s)
{
    QString flags;

    if (s & EmpathIndexRecord::Read)      flags += 'S';
    if (s & EmpathIndexRecord::Marked)    flags += 'F';
    if (s & EmpathIndexRecord::Trashed)   flags += 'T';
    if (s & EmpathIndexRecord::Replied)   flags += 'R';

    return flags;
}

    void
EmpathMaildir::s_timerBeeped()
{
    timer_.stop();
    sync();
    timer_.start(10000, true);
}

    bool
EmpathMaildir::_touched()
{
    QFileInfo fiDir(_cur());

    if (fiDir.lastModified() > index_->lastSync()) {
        empathDebug("Index is older than " + _cur());
        return true;
    }

    fiDir = _new();

    if (fiDir.lastModified() > index_->lastSync()) {
        empathDebug("Index is older than " + _new());
        return true;
    }

    return false;
}

    void
EmpathMaildir::_tagAsDisappearedOrAddToIndex()
{
    empathDebug("");

    QStringList & fileList(_entryList());

    QString readingFolder = i18n("Reading folder %1");
    QString taskMessage = readingFolder.arg(url_.folderPath());
    EmpathTask * t = new EmpathTask(taskMessage);
    t->setMax(fileList.count());

    QStringList::ConstIterator it(fileList.begin());

    QString s;
    QRegExp re_flags(":2,[A-Z]*$");

    empathDebug("There are " + QString::number(fileList.count()) + " files to look at");

    for (; it != fileList.end(); ++it) {

        s = *it;

        EmpathIndexRecord::Status status(EmpathIndexRecord::Status(0));

        int i = s.find(re_flags);
        QString flags;

        if (i != -1) {

            flags = s.right(s.length() - i - 3);

            status = (EmpathIndexRecord::Status)
                (   (flags.contains('S') ? EmpathIndexRecord::Read    : 0)  |
                    (flags.contains('R') ? EmpathIndexRecord::Replied : 0)  |
                    (flags.contains('F') ? EmpathIndexRecord::Marked  : 0));
        }

        s.replace(re_flags, QString::null);

        disappeared_.insert(s, new bool(true));

        if (index_->contains(s)) {

            EmpathIndexRecord rec = index_->record(s);

            if (rec.isNull()) {
                t->doneOne();
                continue;
            }

            if (rec.status() != status) {
                rec.setStatus(status);
                index_->replace(rec.id(), rec);
            }

        } else {

            QCString messageData = _messageData(*it, true);

            if (messageData.isEmpty()) {
                empathDebug("Message data not retrieved !");
                t->doneOne();
                continue;
            }

            RMM::Message m(messageData);
            EmpathIndexRecord ir = indexRecordFromMessage(s, m);

            ir.setStatus(status);

            index_->insert(s, ir);
        }

        t->doneOne();
    }

    t->done();
}

    void
EmpathMaildir::_removeDisappeared()
{
    empathDebug("");

    QStringList l(index_->allKeys());
    empathDebug("index has " + QString::number(l.count()) + " keys");

    for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
        if (0 == disappeared_.find(*it))
            index_->remove(*it);
}

    QStringList &
EmpathMaildir::_entryList()
{
    QDateTime currentMtime = QFileInfo(_cur()).lastModified();

    if (currentMtime != mtime_) {

        QDir d(_cur(), QString::null, QDir::Unsorted, QDir::Files);

        empathDebug("Looking at path '" + d.absPath() + "'");

        cachedEntryList_ = d.entryList();

        mtime_ = currentMtime;
    }

    return cachedEntryList_;
}

    EmpathIndex *
EmpathMaildir::index()
{
    empathDebug("");
    sync();
    return index_;
}

// vim:ts=4:sw=4:tw=78
