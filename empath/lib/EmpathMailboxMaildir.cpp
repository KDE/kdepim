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
# pragma implementation "EmpathMailboxMaildir.h"
#endif

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

// Local includes
#include "EmpathMailboxMaildir.h"
#include "EmpathFolderList.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathMailboxMaildir::EmpathMailboxMaildir(const QString & name)
    :   EmpathMailbox(name),
        initialised_(false)
{
    type_ = Maildir;
    typeString_ = "Maildir";
    seq_ = 0;
    boxList_.setAutoDelete(true);
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
EmpathMailboxMaildir::_mark(
    const EmpathURL & url, RMM::MessageStatus s, QString xxinfo, QString xinfo)
{
    // Mark one message with status flag[s].

    EmpathMaildir * m = _box(url);

    if (m == 0) {
        emit (removeComplete(false, url, xxinfo, xinfo));
        return;
    }
    
    bool retval = m->mark(url.messageID(), s);
    emit (markComplete(retval, url, xxinfo, xinfo));
}

    void
EmpathMailboxMaildir::_mark(
    const EmpathURL & url,
    const QStringList & l,
    RMM::MessageStatus s,
    QString xxinfo,
    QString xinfo)
{
    // Mark a list of messages with status flag[s].

    EmpathMaildir * m = _box(url);
    
    if ((m == 0) || (l.count() == 0)) {
        
        EmpathURL u(url);
        
        QStringList::ConstIterator it;
        
        for (it = l.begin(); it != l.end(); ++it) {
        
            u.setMessageID(*it);
            emit (markComplete(false, u, xxinfo, xinfo));
        }
        
        return;
    }
    
    bool retval = m->mark(l, s);

    emit (markComplete(retval, url, xxinfo, xinfo));
}

    void
EmpathMailboxMaildir::sync(const EmpathURL & url)
{
    EmpathMaildirListIterator it(boxList_);
    
    for (; it.current(); ++it)
        if (it.current()->url().folderPath() == url.folderPath())
            it.current()->sync();
}
    
    void
EmpathMailboxMaildir::_setupDefaultFolders()
{
    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;
    
    c->setGroup(GROUP_FOLDERS);
    
    QStringList folders;

    folders
        << c->readEntry(FOLDER_INBOX,   i18n("Inbox"))
        << c->readEntry(FOLDER_OUTBOX,  i18n("Outbox"))
        << c->readEntry(FOLDER_TRASH,   i18n("Trash"))
        << c->readEntry(FOLDER_SENT,    i18n("Sent"))
        << c->readEntry(FOLDER_DRAFTS,  i18n("Drafts"));

    QStringList::ConstIterator it;

    for (it = folders.begin(); it != folders.end(); ++it) {
 
        QString folderPath = path_ + *it;

        QDir d(folderPath);
        
        if (d.exists())
            continue;

        if (!d.exists() && !d.mkdir(folderPath)) {
            empathDebug("Couldn't make " + path_ + " !!!!!");
            continue;
        }

        EmpathURL u(url_.mailboxName(), *it, QString::null);

        EmpathFolder * f = new EmpathFolder(u);
    
        if (c->readEntry(FOLDER_INBOX) == *it)
            f->setPixmap("folder-inbox");
        
        else if (c->readEntry(FOLDER_OUTBOX) == *it)
            f->setPixmap("folder-outbox");
        
        else if (c->readEntry(FOLDER_SENT) == *it)
            f->setPixmap("folder-sent");
        
        else if (c->readEntry(FOLDER_DRAFTS) == *it)
            f->setPixmap("folder-drafts");
        
        else if (c->readEntry(FOLDER_TRASH) == *it)
            f->setPixmap("folder-trash");
        
        folderList_.insert(*it, f);

        boxList_.append(new EmpathMaildir(path_, u));
    }
    
    saveConfig();
}
    
    QString
EmpathMailboxMaildir::_write(
    const EmpathURL & url, RMM::RMessage & m, QString xxinfo, QString xinfo)
{
    EmpathMaildir * box = _box(url);

    if (box == 0) {
        emit(writeComplete(false, url, xxinfo, xinfo));
        return QString::null;
    }

    QString s = box->writeMessage(m);

    emit(writeComplete(!(s.isEmpty()), url, xxinfo, xinfo));
    
    return s;
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
    
    using namespace EmpathConfig;
    c->setGroup(GROUP_MAILBOX + url_.mailboxName());
    
    c->writeEntry(M_TYPE, (unsigned int)type_);
    c->writeEntry(M_PATH, path_);
    
    c->writeEntry(M_CHECK,      autoCheck_);
    c->writeEntry(M_CHECK_INT,  autoCheckInterval_);
}

    void
EmpathMailboxMaildir::loadConfig()
{
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_MAILBOX + url_.mailboxName());
    
    autoCheck_          = c->readUnsignedNumEntry(M_CHECK);
    autoCheckInterval_  = c->readUnsignedNumEntry(M_CHECK_INT);
    
    folderList_.clear();
    boxList_.clear();
    
    path_ = c->readEntry(M_PATH);
    
    if (path_.at(path_.length()) != '/')
        path_ += '/';
 
    if (path_.isEmpty()) {
        empathDebug("My path is empty :(");
        return;
    }

    QDir d(path_);
    
    if (!d.exists())
        if (!d.mkdir(path_)) {
            empathDebug("Couldn't make " + path_ + " !!!!!");
        }

    _recursiveReadFolders(path_);
    _setupDefaultFolders();
    
    // Initialise all maildir objects.
    
    EmpathMaildirListIterator it(boxList_);
    
    for (; it.current(); ++it)
        it.current()->init();
    
    emit(updateFolderLists());
}

    void
EmpathMailboxMaildir::_recursiveReadFolders(const QString & currentDir)
{
    // We need to look at the maildir base directory, and go recursively
    // through subdirs. Any subdir that has cur, tmp and new is a Maildir
    // folder.

    while (path_.at(path_.length() - 1) == '/')
        path_.truncate(path_.length() - 1);
    
    QDir d(currentDir);
    d.setFilter(QDir::Dirs);
    
    if (d.count() == 0) {
        empathDebug("No folders in maildir");
        return;
    }
    
    QStringList l(d.entryList());
    
    QStringList::ConstIterator it(l.begin());
    
    bool hasCur, hasNew, hasTmp;
    hasCur = hasNew = hasTmp = false;
    
    for (; it != l.end(); ++it) {
        
        if ((*it).left(1) == ".")
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
 
    bool isMaildir = hasCur && hasNew && hasTmp;

    EmpathURL url(
        url_.mailboxName(),
        currentDir.right(currentDir.length() - path_.length()),
        QString::null);
    
    EmpathFolder * f = new EmpathFolder(url);

    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;

    c->setGroup(GROUP_FOLDERS);

    if (currentDir == c->readEntry(FOLDER_INBOX))
        f->setPixmap("folder-inbox");
    
    else if (currentDir == c->readEntry(FOLDER_OUTBOX))
        f->setPixmap("folder-outbox");
    
    else if (currentDir == c->readEntry(FOLDER_SENT))
        f->setPixmap("folder-sent");
    
    else if (currentDir == c->readEntry(FOLDER_DRAFTS))
        f->setPixmap("folder-drafts");
    
    else if (currentDir == c->readEntry(FOLDER_TRASH))
        f->setPixmap("folder-trash");

    folderList_.insert(url.folderPath(), f);
 
    // If this dir is not itself a maildir folder, we add it as a containing
    // folder.
    if (!isMaildir) {
        f->setContainer(true);
        return;
    }
   
    boxList_.append(new EmpathMaildir(path_, url));
}

    bool
EmpathMailboxMaildir::getMail()
{
    // STUB
    return false;
}

    void
EmpathMailboxMaildir::s_checkMail()
{
    sync(url_);    
}
    
    void
EmpathMailboxMaildir::_retrieve(
    const EmpathURL & url, QString xxinfo, QString xinfo)
{
    EmpathMaildir * m = _box(url);
    
    if (m == 0) {
        emit(retrieveComplete(false, url, xxinfo, xinfo));
        return;
    }
    
    RMM::RMessage * message = m->message(url.messageID());

    if (message != 0)
        empath->cacheMessage(url, message, xinfo);
    
    emit(retrieveComplete((message != 0), url, xxinfo, xinfo));
}

    void
EmpathMailboxMaildir::_retrieve(
    const EmpathURL & from, const EmpathURL & to, QString xxinfo, QString xinfo)
{
    EmpathMaildir * m = _box(from);
    
    if (m == 0) {
        emit(retrieveComplete(false, from, to, xxinfo, xinfo));
        return;
    }
    
    RMM::RMessage * message = m->message(from.messageID());

    if (message != 0)
        empath->cacheMessage(from, message, xinfo);
    
    emit(retrieveComplete((message != 0), from, to, xxinfo, xinfo));
}

    void
EmpathMailboxMaildir::_removeMessage(
    const EmpathURL & url, QString xxinfo, QString xinfo)
{
    EmpathMaildir * m = _box(url);
    
    if (m == 0) {
        emit(removeComplete(false, url, xxinfo, xinfo));
        return;
    }
    
    bool retval = m->removeMessage(url.messageID());
    
    emit(removeComplete(retval, url, xxinfo, xinfo));
}

    void
EmpathMailboxMaildir::_removeMessage(
    const EmpathURL & url, const QStringList & l, QString xxinfo, QString xinfo)
{
    EmpathMaildir * m = _box(url);
    
    if (m == 0) {
        
        // If we can't find the maildir object, signal removeComplete(false)
        // for each message that should have been removed.

        EmpathURL u(url);
        
        QStringList::ConstIterator it;
        
        for (it = l.begin(); it != l.end(); ++it) {
            
            u.setMessageID(*it);
            emit(removeComplete(false, u, xxinfo, xinfo));
        }
        
        return;
    }
    
    bool retval = m->removeMessage(l);

    emit(removeComplete(retval, url, xxinfo, xinfo));
}

    void
EmpathMailboxMaildir::setPath(const QString & path)
{
    path_ = path;
    empathDebug("path_ == " + path_);
    saveConfig();
    loadConfig();
    empathDebug("path_ == " + path_);
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

    void
EmpathMailboxMaildir::_createFolder(
    const EmpathURL & url, QString xxinfo, QString xinfo)
{
    folderList_.insert(url.folderPath(), new EmpathFolder(url));
    
    EmpathMaildir * m = new EmpathMaildir(path_, url);
    boxList_.append(m);
    m->init();
    
    emit(createFolderComplete(true, url, xxinfo, xinfo));
    emit(updateFolderLists());
}

    void
EmpathMailboxMaildir::_removeFolder(
    const EmpathURL & url, QString xxinfo, QString xinfo)
{
    bool ok = _recursiveRemove(path_ + "/" + url.folderPath());

    if (!ok) {
        empathDebug("Could not remove dir");
        emit (removeFolderComplete(false, url, xxinfo, xinfo));
        return;
    }

    folderList_.clear();
    boxList_.clear();
    
    _recursiveReadFolders(path_);
    _setupDefaultFolders();
    
    // Initialise all maildir objects.
    
    EmpathMaildirListIterator it(boxList_);
    
    for (; it.current(); ++it)
        it.current()->init();
    
    emit (removeFolderComplete(true, url, xxinfo, xinfo));
    emit (updateFolderLists());
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
        if (!QFile::remove(dir + "/" + *it))
            return false;
    
    return d.rmdir(dir);
}

// vim:ts=4:sw=4:tw=78
