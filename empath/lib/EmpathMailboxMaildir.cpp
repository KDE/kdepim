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

    emit (removeComplete(retval, url, xxinfo, xinfo));
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
    EmpathURL urlInbox  (url_.mailboxName(), i18n("Inbox"),     QString::null);
    EmpathURL urlOutbox (url_.mailboxName(), i18n("Outbox"),    QString::null);
    EmpathURL urlTrash  (url_.mailboxName(), i18n("Trash"),     QString::null);
    EmpathURL urlSent   (url_.mailboxName(), i18n("Sent"),      QString::null);
    EmpathURL urlDrafts (url_.mailboxName(), i18n("Drafts"),    QString::null);
    
    EmpathFolder    * folder_inbox  = new EmpathFolder  (urlInbox);
    EmpathMaildir   * box_inbox     = new EmpathMaildir (path_, urlInbox);
    
    EmpathFolder    * folder_outbox = new EmpathFolder  (urlOutbox);
    EmpathMaildir   * box_outbox    = new EmpathMaildir (path_, urlOutbox);
    
    EmpathFolder    * folder_drafts = new EmpathFolder  (urlDrafts);
    EmpathMaildir   * box_drafts    = new EmpathMaildir (path_, urlDrafts);

    EmpathFolder    * folder_sent   = new EmpathFolder  (urlSent);
    EmpathMaildir   * box_sent      = new EmpathMaildir (path_, urlSent);

    EmpathFolder    * folder_trash  = new EmpathFolder  (urlTrash);
    EmpathMaildir   * box_trash     = new EmpathMaildir (path_, urlTrash);
    
    folderList_.insert(folder_trash->name(), folder_trash);
    folderList_.insert(folder_sent->name(), folder_sent);
    folderList_.insert(folder_drafts->name(), folder_drafts);
    folderList_.insert(folder_outbox->name(), folder_outbox);
    folderList_.insert(folder_inbox->name(), folder_inbox);
    
    boxList_.append(box_trash);
    boxList_.append(box_sent);
    boxList_.append(box_drafts);
    boxList_.append(box_outbox);
    boxList_.append(box_inbox);
    
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
    empathDebug("Path == `" + path_ + "'");
    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_MAILBOX + url_.mailboxName());
    
    c->writeEntry(EmpathConfig::KEY_MAILBOX_TYPE, (unsigned int)type_);
    c->writeEntry(EmpathConfig::KEY_LOCAL_MAILBOX_PATH, path_);
    
    c->writeEntry(EmpathConfig::KEY_CHECK_MAIL, checkMail_);
    c->writeEntry(EmpathConfig::KEY_CHECK_MAIL_INTERVAL, checkMailInterval_);
}

    void
EmpathMailboxMaildir::loadConfig()
{
    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_MAILBOX + url_.mailboxName());
    
    checkMail_ = c->readUnsignedNumEntry(EmpathConfig::KEY_CHECK_MAIL);
    checkMailInterval_ =
        c->readUnsignedNumEntry(EmpathConfig::KEY_CHECK_MAIL_INTERVAL);
    
    folderList_.clear();
    boxList_.clear();
    
    path_ = c->readEntry(EmpathConfig::KEY_LOCAL_MAILBOX_PATH);
    
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
    
    // If this dir is not itself a maildir folder, drop out.
    if (!(hasCur && hasNew && hasTmp))
        return;
        
    
    QString s(d.absPath());

    s.remove(0, path_.length());
    s.remove(0, 1);

    EmpathURL url(url_.mailboxName(), s, QString::null);
    EmpathFolder * f = new EmpathFolder(url);
    
    folderList_.insert(url.folderPath(), f);
    
    EmpathMaildir * m = new EmpathMaildir(path_, url);
    
    boxList_.append(m);
}

    bool
EmpathMailboxMaildir::getMail()
{
    // STUB
    return false;
}

    void
EmpathMailboxMaildir::s_checkNewMail()
{
    sync(url_);    
}
    
    void
EmpathMailboxMaildir::s_getNewMail()
{
    // STUB
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
        empath->cacheMessage(url, message);
    
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
        empath->cacheMessage(from, message);
    
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
    saveConfig();
    loadConfig();
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
    EmpathFolder    * f = new EmpathFolder  (url);
    EmpathMaildir   * m = new EmpathMaildir (path_, url);

    m->init();
    
    folderList_.insert(f->name(), f);
    boxList_.append(m);
    
    emit(createFolderComplete(true, url, xxinfo, xinfo));
    emit(updateFolderLists());
}

    void
EmpathMailboxMaildir::_removeFolder(
    const EmpathURL & url, QString xxinfo, QString xinfo)
{
    bool removedFromFolderList  = false;
    bool removedFromMaildirList = false;
    
    EmpathFolderListIterator fit(folderList_);
    
    for (; fit.current(); ++fit)
        if (fit.current()->url().folderPath() == url.folderPath()) {
            folderList_.remove(fit.current()->name());
            removedFromFolderList = true;
            break;
        }
    
    if (!removedFromFolderList) {
        emit (removeFolderComplete(false, url, xxinfo, xinfo));
        return;
    }
    
    EmpathMaildirListIterator mit(boxList_);
    
    for (; mit.current(); ++mit)
        if (mit.current()->url() == url) {
            boxList_.remove(mit.current());
            removedFromMaildirList = true;
            break;
        }
    
    if (!removedFromMaildirList) {
        emit (removeFolderComplete(false, url, xxinfo, xinfo));
        return;
    }
    
    bool retval = _recursiveRemove(path_ + "/" + url.folderPath());

    emit (removeFolderComplete(retval, url, xxinfo, xinfo));
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
