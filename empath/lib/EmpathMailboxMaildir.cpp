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

// FIXME: The writeNewMail method needs to find a way of telling the
// caller whether message write was successful.

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
#include "EmpathMessageList.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathMailboxMaildir::EmpathMailboxMaildir(const QString & name)
    :    EmpathMailbox(name)
{
    empathDebug("ctor");
    type_ = Maildir;
    seq_ = 0;
    boxList_.setAutoDelete(true);
}

EmpathMailboxMaildir::~EmpathMailboxMaildir()
{
    empathDebug("dtor");
}

    void
EmpathMailboxMaildir::_mark(
    const EmpathURL & url, RMM::MessageStatus s, QString xinfo)
{
    EmpathMaildir * m = _box(url);

    if (m == 0) {
        emit (removeComplete(false, url, xinfo));
        return;
    }
    
    bool retval = m->mark(url.messageID(), s);
    emit (markComplete(retval, url, xinfo));
}

    void
EmpathMailboxMaildir::_mark(
    const EmpathURL & url,
    const QStringList & l,
    RMM::MessageStatus s,
    QString xinfo)
{
    EmpathMaildir * m = _box(url);
    
    if ((m == 0) || (l.count() == 0)) {
        
        EmpathURL u(url);
        
        QStringList::ConstIterator it;
        
        for (it = l.begin(); it != l.end(); ++it) {
        
            u.setMessageID(*it);
            emit (markComplete(false, u, xinfo));
        }
        
        return;
    }
    
    bool retval = m->mark(l, s);
    emit (removeComplete(retval, url, xinfo));
}

    void
EmpathMailboxMaildir::sync(const EmpathURL & url)
{
    empathDebug("");
    EmpathMaildirListIterator it(boxList_);
    
    for (; it.current(); ++it) {
        if (it.current()->url() == url)
            it.current()->sync(url);
    }
}
    
    void
EmpathMailboxMaildir::_setupDefaultFolders()
{
    empathDebug("_setupDefaultFolders() called");
    EmpathURL urlInbox    (url_.mailboxName(), i18n("Inbox"),        QString::null);
    EmpathURL urlOutbox    (url_.mailboxName(), i18n("Outbox"),    QString::null);
    EmpathURL urlTrash    (url_.mailboxName(), i18n("Trash"),        QString::null);
    EmpathURL urlSent    (url_.mailboxName(), i18n("Sent"),        QString::null);
    EmpathURL urlDrafts    (url_.mailboxName(), i18n("Drafts"),    QString::null);
    
    EmpathFolder * folder_inbox = new EmpathFolder(urlInbox);
    CHECK_PTR(folder_inbox);
        
    EmpathMaildir * box_inbox =
        new EmpathMaildir(path_, urlInbox);
    CHECK_PTR(box_inbox);
    
    EmpathFolder * folder_outbox = new EmpathFolder(urlOutbox);
    CHECK_PTR(folder_outbox);
    
    EmpathMaildir * box_outbox =
        new EmpathMaildir(path_, urlOutbox);
    CHECK_PTR(box_outbox);
    
    EmpathFolder * folder_drafts = new EmpathFolder(urlDrafts);
    CHECK_PTR(folder_drafts);
    
    EmpathMaildir * box_drafts =
        new EmpathMaildir(path_, urlDrafts);
    CHECK_PTR(box_drafts);
    
    EmpathFolder * folder_sent = new EmpathFolder(urlSent);
    CHECK_PTR(folder_sent);
    
    EmpathMaildir * box_sent =
        new EmpathMaildir(path_, urlSent);
    CHECK_PTR(box_sent);
    
    EmpathFolder * folder_trash = new EmpathFolder(urlTrash);
    CHECK_PTR(folder_trash);
    
    EmpathMaildir * box_trash =
        new EmpathMaildir(path_, urlTrash);
    CHECK_PTR(box_trash);
    
    folderList_.append(folder_trash);
    folderList_.append(folder_sent);
    folderList_.append(folder_drafts);
    folderList_.append(folder_outbox);
    folderList_.append(folder_inbox);
    
    boxList_.append(box_trash);
    boxList_.append(box_sent);
    boxList_.append(box_drafts);
    boxList_.append(box_outbox);
    boxList_.append(box_inbox);
    
    saveConfig();
}
    
    QString
EmpathMailboxMaildir::_write(
    const EmpathURL & url, RMM::RMessage & m, QString xinfo)
{
    empathDebug("");
    
    EmpathMaildir * box = _box(url);

    if (box == 0) {
        emit(writeComplete(false, url, xinfo));
        return QString::null;
    }

    QString s = box->writeMessage(m);

    emit(writeComplete(!(s.isEmpty()), url, xinfo));
    
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
    empathDebug("saveConfig() called - my name is " + url_.asString());
    
    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_MAILBOX + url_.mailboxName());
    
    c->writeEntry(EmpathConfig::KEY_MAILBOX_TYPE, (unsigned int)type_);
    c->writeEntry(EmpathConfig::KEY_LOCAL_MAILBOX_PATH, path_);
    
    c->writeEntry(EmpathConfig::KEY_CHECK_MAIL, checkMail_);
    c->writeEntry(EmpathConfig::KEY_CHECK_MAIL_INTERVAL, checkMailInterval_);
    c->sync();
}

    void
EmpathMailboxMaildir::readConfig()
{
    empathDebug("readConfig() called");

    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_MAILBOX + url_.mailboxName());
    
    checkMail_ = c->readUnsignedNumEntry(EmpathConfig::KEY_CHECK_MAIL);
    checkMailInterval_ =
        c->readUnsignedNumEntry(EmpathConfig::KEY_CHECK_MAIL_INTERVAL);
    
    folderList_.clear();
    boxList_.clear();
    
    path_ = c->readEntry(EmpathConfig::KEY_LOCAL_MAILBOX_PATH);
    if (path_.at(path_.length()) != '/') path_ += '/';
    _recursiveReadFolders(path_);
    
    EmpathMaildirListIterator it(boxList_);
    
    for (; it.current(); ++it) {
        it.current()->init();
    }
}

    void
EmpathMailboxMaildir::_recursiveReadFolders(const QString & currentDir)
{
    empathDebug("_recursiveReadFolders(" + currentDir + ") called");
    // We need to look at the maildir base directory, and go recursively
    // through subdirs. Any subdir that has cur, tmp and new is a Maildir
    // folder.

    while (path_.at(path_.length() - 1) == '/')
        path_.truncate(path_.length() - 1);
    empathDebug("path == " + path_);
    
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
    
    if (hasCur && hasNew && hasTmp) {
        
        // This dir is itself a maildir folder.
        
        QString s(d.absPath());
    
        s.remove(0, path_.length());
        s.remove(0, 1);
        empathDebug("s == " + s);
        empathDebug("Folder path is " + s);

        EmpathURL url(url_.mailboxName(), s, QString::null);
        EmpathFolder * f = new EmpathFolder(url);
        
        CHECK_PTR(f);

        folderList_.append(f);
        
        EmpathMaildir * m = new EmpathMaildir(path_, url);
        CHECK_PTR(m);
        
        boxList_.append(m);
        emit(updateFolderLists());
    }
    else
        empathDebug("dir " + currentDir + " doesn't look like a maildir!");
    
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
EmpathMailboxMaildir::init()
{
    empathDebug("init() called");
    readConfig();
    QDir d(path_);
    
    if (!d.exists())
        if (!d.mkdir(path_)) {
            empathDebug("Couldn't make " + path_ + " !!!!!");
        }
}

    void
EmpathMailboxMaildir::_retrieve(const EmpathURL & url, QString xinfo)
{
    empathDebug("");
    EmpathMaildir * m = _box(url);
    
    if (m == 0) {
        emit(retrieveComplete(false, url, xinfo));
        return;
    }
    
    RMM::RMessage * message = m->message(url.messageID());

    if (message != 0) {
    
        empathDebug("calling cacheMessage");
        empath->cacheMessage(url, message);
    }
    
    empathDebug("emitting retrieveComplete");
    emit(retrieveComplete((message != 0), url, xinfo));
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
EmpathMailboxMaildir::_removeMessage(const EmpathURL & url, QString xinfo)
{
    EmpathMaildir * m = _box(url);
    
    if (m == 0) {
        emit(removeComplete(false, url, xinfo));
        return;
    }
    
    bool retval = m->removeMessage(url.messageID());
    
    emit(removeComplete(retval, url, xinfo));
}

    void
EmpathMailboxMaildir::_removeMessage(
    const EmpathURL & url, const QStringList & l, QString xinfo)
{
    EmpathMaildir * m = _box(url);
    
    if ((m == 0) || (l.count() == 0)) {

        EmpathURL u(url);
        
        QStringList::ConstIterator it;
        
        for (it = l.begin(); it != l.end(); ++it) {
            
            u.setMessageID(*it);
            emit(removeComplete(false, u, xinfo));
        }
        
        return;
    }
    
    bool retval = m->removeMessage(l);

    emit (removeComplete(retval, url, xinfo));
}

    void
EmpathMailboxMaildir::setPath(const QString & path)
{
    path_ = path;
}

    EmpathMaildir *
EmpathMailboxMaildir::_box(const EmpathURL & id)
{
    empathDebug("_box(" + id.asString() + ") called");
    EmpathMaildirListIterator it(boxList_);
    
    for (; it.current(); ++it) {
        empathDebug("Looking at \"" + it.current()->url().folderPath() + "\"");
        if (it.current()->url().folderPath() == id.folderPath())
            return it.current();
    }
    
    empathDebug("Can't find box with id \"" + id.asString() + "\"");
    return 0;
}

    void
EmpathMailboxMaildir::_createFolder(const EmpathURL & url, QString xinfo)
{
    empathDebug("addFolder(" + url.asString() + ") called");
    EmpathFolder * f = new EmpathFolder(url);
    CHECK_PTR(f);
    
    EmpathMaildir * m = new EmpathMaildir(path_, url);
    CHECK_PTR(m);
    m->init();
    
    folderList_.append(f);
    boxList_.append(m);
    
    // XXX Always ok ?
    emit (createFolderComplete(true, url, xinfo));
    emit (updateFolderLists());
}

    void
EmpathMailboxMaildir::_removeFolder(const EmpathURL & url, QString xinfo)
{
    empathDebug("removeFolder(" + url.asString() + ") called");
    
    bool removedFromFolderList = false;
    bool removedFromMaildirList = false;
    
    EmpathFolderListIterator fit(folderList_);
    
    for (; fit.current(); ++fit)
        if (fit.current()->url().folderPath() == url.folderPath()) {
            folderList_.remove(fit.current());
            removedFromFolderList = true;
            break;
        }
    
    if (!removedFromFolderList) {
        empathDebug("Couldn't remove from folder list");
        emit (removeFolderComplete(false, url, xinfo));
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
        empathDebug("Couldn't remove from box list");
        emit (removeFolderComplete(false, url, xinfo));
        return;
    }
    
    bool retval = _recursiveRemove(path_ + url.folderPath());

    emit (removeFolderComplete(retval, url, xinfo));
}

    bool
EmpathMailboxMaildir::_recursiveRemove(const QString & dir)
{
    empathDebug("_recursiveRemove(" + dir + ") called");
    
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
    
    for (; it != l.end(); ++it) {

        empathDebug("REMOVE FILE \"" + dir + "/" + *it + "\"");
        if (!QFile::remove(dir + "/" + *it)) {
            empathDebug("Couldn't remove " + dir + "/" + *it);
            return false;
        }
    }
    
    // Finally, remove this dir.
    empathDebug("REMOVE DIR  \"" + dir + "\"");
    if (!d.rmdir(dir)) {
        empathDebug("Couldn't remove " + dir);
        return false;
    }
    
    return true;
}

// vim:ts=4:sw=4:tw=78
