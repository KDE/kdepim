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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Qt includes
#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qapplication.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathMailboxMaildir.h"
#include "EmpathFolderList.h"
#include "EmpathTask.h"
#include "Empath.h"

EmpathMailboxMaildir::EmpathMailboxMaildir(const QString & name)
    :   EmpathMailbox(name),
        initialised_(false)
{
    type_ = Maildir;
    typeString_ = "Maildir";
    seq_ = 0;
    boxList_.setAutoDelete(true);
    empathDebug("ctor");
}

EmpathMailboxMaildir::~EmpathMailboxMaildir()
{
    // Empty.
}

    void
EmpathMailboxMaildir::init()
{
    if (initialised_)
        return;

    initialised_ = true;

    loadConfig();
}

    void
EmpathMailboxMaildir::sync(const EmpathURL & url)
{
    EmpathMaildirListIterator it(boxList_);

    for (; it.current(); ++it)
        if (it.current()->url().folderPath() == url.folderPath()) {
            it.current()->sync();
            break;
        }
}

    void
EmpathMailboxMaildir::_setupDefaultFolders()
{
    KConfig * c(KGlobal::config());

    c->setGroup("Folders");

    QStringList folders;

    folders
        << empath->inbox()  .folderPath()
        << empath->outbox() .folderPath()
        << empath->sent()   .folderPath()
        << empath->drafts() .folderPath()
        << empath->trash()  .folderPath();

    QStringList::ConstIterator it;

    for (it = folders.begin(); it != folders.end(); ++it) {

        QString folderPath = path_ + "/" + *it;

        QDir d(folderPath);

        if (d.exists())
            continue;

        if (!d.mkdir(folderPath)) {
            empathDebug("Couldn't make " + path_ + " !!!!!");
            continue;
        }

        d.mkdir(folderPath + "/cur");
        d.mkdir(folderPath + "/new");
        d.mkdir(folderPath + "/tmp");

        EmpathURL u(url_.mailboxName(), *it, QString::null);

        folderList_.insert(*it, new EmpathFolder(u));

        boxList_.append(new EmpathMaildir(path_, u));
    }

    saveConfig();
}

    bool
EmpathMailboxMaildir::newMail() const
{
    // STUB
    return false;
}

    void
EmpathMailboxMaildir::saveConfig()
{
    KConfig * c = KGlobal::config();

    c->setGroup("Mailbox_" + url_.mailboxName());

    c->writeEntry("Type", (unsigned int)type_);
    c->writeEntry("Path", path_);

    c->writeEntry("Check",     autoCheck_);
    c->writeEntry("CheckInterval",  autoCheckInterval_);
}

    void
EmpathMailboxMaildir::loadConfig()
{
    KConfig * c = KGlobal::config();

    c->setGroup("Mailbox_" + url_.mailboxName());

    autoCheck_          = c->readUnsignedNumEntry("Check");
    autoCheckInterval_  = c->readUnsignedNumEntry("CheckInterval");

    folderList_.clear();
    boxList_.clear();

    path_ = c->readEntry("Path");

    while (path_.at(path_.length() - 1) == '/')
        path_.truncate(path_.length() - 1);

    if (path_.isEmpty()) // FIXME Shouldn't work like this
        path_ = QDir::homeDirPath() + "/.empath";

    QDir d(path_);

    if (!d.exists())
        if (!d.mkdir(path_)) {
            empathDebug("Couldn't make directory `" + path_ + "' !");
            return;
        }

    c->setGroup("Mailbox_" + url_.mailboxName());
    QStringList folderList = c->readListEntry("FolderList");

    QStringList::ConstIterator it(folderList.begin());

    for (; it != folderList.end(); ++it) {

        EmpathURL url(*it);

        QString path = path_ + "/" + url.folderPath();

        if (QDir(path).exists()) {

            EmpathFolder * f = new EmpathFolder(url);

            folderList_.insert(url.folderPath(), f);

            if (QDir(path + "/cur").exists())
                boxList_.append(new EmpathMaildir(path_, url));
            else
                f->setContainer(true);
        }
    }

    emit(updateFolderLists());

    _recursiveReadFolders(path_);
    _setupDefaultFolders();

    emit(updateFolderLists());

    folderList.clear();

    for (EmpathFolderListIterator it(folderList_); it.current(); ++it)
        folderList << it.current()->url().asString();

    c->setGroup("Mailbox_" + url_.mailboxName());
    c->writeEntry("FolderList", folderList);

    // Initialise all maildir objects.

    EmpathTask * t = new EmpathTask(i18n("Reading folders"));
    t->setMax(boxList_.count());

    for (EmpathMaildirListIterator it(boxList_); it.current(); ++it) {
        it.current()->init();
        t->doneOne();
    }

    t->done();

    // Is this now needed ?
//    emit(syncFolderLists());
}

    void
EmpathMailboxMaildir::_recursiveReadFolders(const QString & currentDir)
{
    // We need to look at the maildir base directory, and go recursively
    // through subdirs. Any subdir that has cur, tmp and new is a Maildir
    // folder.

    // Cheat like fuck.
    struct stat s;
    stat(QFile::encodeName(currentDir), &s);
    if (2 == s.st_nlink)
        return;

    QDir d(
        currentDir,
        QString::null,
        QDir::Unsorted,
        QDir::Dirs | QDir::Readable);

    if (d.count() == 0)
        return;

    QStringList l(d.entryList());

    QStringList::ConstIterator it(l.begin());

    bool hasCur, hasNew, hasTmp;
    hasCur = hasNew = hasTmp = false;

    for (; it != l.end(); ++it) {

        if ((*it).at(0) == '.')
            continue;

        if (*it == "cur") {
            hasCur = true;
            continue;
        }

        if (*it == "new") {
            hasNew = true;
            continue;
        }

        if (*it == "tmp") {
            hasTmp = true;
            continue;
        }

        _recursiveReadFolders(currentDir + "/" + *it);
    }

    if (currentDir == path_)
        return;

    bool isMaildir = hasCur && hasNew && hasTmp;

    EmpathURL url(
        url_.mailboxName(),
        currentDir.right(currentDir.length() - path_.length()),
        QString::null);

    if (0 == folderList_[url.folderPath()]) {

        EmpathFolder * f = new EmpathFolder(url);

        folderList_.insert(url.folderPath(), f);

        if (!isMaildir)
            f->setContainer(true);
        else
            boxList_.append(new EmpathMaildir(path_, url));
    }
}

    void
EmpathMailboxMaildir::s_checkMail()
{
    sync(url_);
}

    void
EmpathMailboxMaildir::setPath(const QString & path)
{
    path_ = path;
    saveConfig();
}

    EmpathMaildir *
EmpathMailboxMaildir::_box(const EmpathURL & id)
{
    EmpathMaildirListIterator it(boxList_);

    for (; it.current(); ++it)
        if (it.current()->url().folderPath() == id.folderPath())
            return it.current();

    return 0;
}

    bool
EmpathMailboxMaildir::_recursiveRemove(const QString & dir)
{
    QDir d(dir);

    // First remove all dirs.
    d.setFilter(QDir::Dirs | QDir::NoSymLinks);

    QStringList l(d.entryList());

    QStringList::Iterator it(l.begin());

    for (; it != l.end(); ++it) {

        if (*it == ".") continue;
        if (*it == "..") continue;

        if (!_recursiveRemove(dir + "/" + *it))
            return false;
    }

    // Now remove all files.
    d.setFilter(QDir::NoSymLinks | QDir::Files | QDir::Hidden);

    l = d.entryList();

    it = l.begin();

    for (; it != l.end(); ++it)
        if (!QFile::remove(dir + "/" + *it)) {
            empathDebug("Couldn't remove " + dir + "/" + *it);
            return false;
        }

    bool removedDirOK = d.rmdir(dir);

    if (!removedDirOK) {
        empathDebug("Couldn't remove " + dir);
    }

    return removedDirOK;
}

    RMM::Message
EmpathMailboxMaildir::retrieveMessage(const EmpathURL & url)
{
    empathDebug(url.asString());
    RMM::Message retval;

    EmpathMaildir * box = _box(url);

    if (box == 0) {
        empathDebug("!box !");
        return retval;
    }

    retval = box->message(url.messageID());

    return retval;
}

    QString
EmpathMailboxMaildir::writeMessage(
    RMM::Message & message,
    const EmpathURL & folder
)
{
    EmpathMaildir * box = _box(folder);

    if (box == 0)
        return QString::null;

    return box->writeMessage(message);
}

    bool
EmpathMailboxMaildir::removeMessage(const EmpathURL & url)
{
    return removeMessage(url, url.messageID())[url.messageID()];
}

    EmpathSuccessMap
EmpathMailboxMaildir::removeMessage(
    const EmpathURL & folder,
    const QStringList & messageIDList)
{
    EmpathSuccessMap retval;

    EmpathMaildir * box = _box(folder);

    if (box == 0)
        return retval;

    retval = box->removeMessage(messageIDList);

    return retval;
}

    bool
EmpathMailboxMaildir::markMessage(
    const EmpathURL & url,
    EmpathIndexRecord::Status s
)
{
    return (markMessage(url, url.messageID(), s))[url.messageID()];
}

    EmpathSuccessMap
EmpathMailboxMaildir::markMessage(
    const EmpathURL & folder,
    const QStringList & messageIDList,
    EmpathIndexRecord::Status status)
{
    EmpathSuccessMap retval;

    EmpathMaildir * box = _box(folder);

    if (box == 0)
        return retval;

    retval = box->mark(messageIDList, status);

    return retval;
}

    bool
EmpathMailboxMaildir::createFolder(const EmpathURL & url)
{
    EmpathMaildir * m = new EmpathMaildir(path_, url);

    if (!m->createdOK()) {

        delete m;
        m = 0;
        return false;
    }

    folderList_.insert(url.folderPath(), new EmpathFolder(url.folderPath()));

    boxList_.append(m);
    m->init();

    return true;
}

    bool
EmpathMailboxMaildir::removeFolder(const EmpathURL & url)
{
    bool ok = _recursiveRemove(path_ + "/" + url.folderPath());

    if (!ok)
        return false;

    loadConfig();

    return true;
}

    unsigned int
EmpathMailboxMaildir::messageCount() const
{
    empathDebug("STUB");
    return 0;
}

    unsigned int
EmpathMailboxMaildir::unreadMessageCount() const
{
    empathDebug("STUB");
    return 0;
}

    EmpathIndex *
EmpathMailboxMaildir::index(const EmpathURL & folder)
{
    EmpathMaildir * box = _box(folder);

    if (box == 0) {
        empathDebug("Couldn't find box " + folder.asString());
        return 0L;
    }

    return box->index();
}


// vim:ts=4:sw=4:tw=78
#include "EmpathMailboxMaildir.moc"
