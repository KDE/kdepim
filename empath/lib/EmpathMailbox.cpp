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
# pragma implementation "EmpathMailbox.h"
#endif

// Local includes
#include "EmpathUtilities.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathURL.h"
#include "Empath.h"

EmpathMailbox::EmpathMailbox(const QString & name)
    :   url_(name, QString::null, QString::null),
        autoCheck_(false),
        autoCheckInterval_(0)
{
    pixmapName_ = "menu-mailbox";
    
    folderList_.setAutoDelete(true);
    
    _connectUp();
}

EmpathMailbox::~EmpathMailbox()
{
    // Empty.
}

    void
EmpathMailbox::setAutoCheck(bool yn)
{
    autoCheck_ = yn;
    
    timer_.stop();
    
    if (autoCheck_)
        timer_.start(autoCheckInterval_ * 60000);
}

    void
EmpathMailbox::setAutoCheckInterval(Q_UINT32 i)
{
    autoCheckInterval_ = i;

    if (autoCheck_) {
        timer_.stop();
        timer_.start(autoCheckInterval_ * 60000);
    }
}

    void
EmpathMailbox::setName(const QString & s) 
{
    QString oldName = url_.mailboxName();
    url_.setMailboxName(s);
    emit(rename(this, oldName));
}

    Q_UINT32
EmpathMailbox::messageCount() const
{
    Q_UINT32 c = 0;
    
    EmpathFolderListIterator it(folderList_);
    
    for (; it.current(); ++it)
        c += it.current()->messageCount();

    return c;
}

    Q_UINT32
EmpathMailbox::unreadMessageCount() const
{
    Q_UINT32 c = 0;
    
    EmpathFolderListIterator it(folderList_);
    
    for (; it.current(); ++it)
        c += it.current()->unreadMessageCount();

    return c;
}

    void
EmpathMailbox::s_countUpdated(Q_UINT32, Q_UINT32)
{
    emit(countUpdated(unreadMessageCount(), messageCount()));
}

    EmpathFolder *
EmpathMailbox::folder(const EmpathURL & url)
{
    QString fp(url.folderPath());

    // If the first char is '/', remove it.
    if (fp.at(0) == '/')
        fp.remove(0, 1);
    
    // If the last char is '/', remove it.
    if (fp.at(fp.length() - 1) == '/')
        fp.remove(fp.length() - 1, 1);
    
    return folderList_[fp];
}

    void
EmpathMailbox::retrieve(const EmpathURL & url, QString xxinfo, QString xinfo)
{
    _enqueue(RetrieveMessage, url, xxinfo, xinfo);
}

    EmpathURL
EmpathMailbox::write(
   const EmpathURL & folder, RMM::RMessage & msg, QString xxinfo, QString xinfo)
{
    QString id = empath->generateUnique();
    
    EmpathURL u(folder);
    u.setMessageID(id);
    
    _enqueue(u, msg, xxinfo, xinfo);
    
    return u;
}
    
    void
EmpathMailbox::remove(const EmpathURL & url, QString xxinfo, QString xinfo)
{
    _enqueue(
        url.isMessage() ? RemoveMessage : RemoveFolder,
        url, xxinfo, xinfo);
}
    
    void
EmpathMailbox::remove(
    const EmpathURL & url, const QStringList & l, QString xxinfo, QString xinfo)
{
    EmpathURL u(url);
    
    QStringList::ConstIterator it;

    for (it = l.begin(); it != l.end(); ++it) {
        u.setMessageID(*it);
        _enqueue(RemoveMessage, u, xxinfo, xinfo);
    }
}
    
    void
EmpathMailbox::createFolder(
    const EmpathURL & url, QString xxinfo, QString xinfo)
{
    _enqueue(CreateFolder, url, xxinfo, xinfo);
}
    
    void
EmpathMailbox::mark(
    const EmpathURL & url, RMM::MessageStatus s, QString xxinfo, QString xinfo)
{
    _enqueue(url, s, xxinfo, xinfo);
}
    
    void
EmpathMailbox::mark(
    const EmpathURL & url,
    const QStringList & l,
    RMM::MessageStatus s,
    QString xxinfo,
    QString xinfo)
{
    EmpathURL u(url);
    
    QStringList::ConstIterator it;

    for (it = l.begin(); it != l.end(); ++it) {
        u.setMessageID(*it);
        _enqueue(u, s, xxinfo, xinfo);
    }
}

    void
EmpathMailbox::_enqueue(
    const EmpathURL & url, RMM::MessageStatus s, QString xxinfo, QString xinfo)
{
    _enqueue(new MarkAction(url, s, xxinfo, xinfo));
}

    void
EmpathMailbox::_enqueue(
    ActionType t, const EmpathURL & url, QString xxinfo, QString xinfo)
{
    _enqueue(new Action(t, url, xxinfo, xinfo));
}

    void
EmpathMailbox::_enqueue(
    const EmpathURL & url, RMM::RMessage & msg, QString xxinfo, QString xinfo)
{
    _enqueue(new WriteAction(url, msg, xxinfo, xinfo));
}

    void
EmpathMailbox::_enqueue(Action * a)
{
    queue_.enqueue(a);
    _runQueue();
}

    void
EmpathMailbox::_runQueue()
{
    while (queue_.count() != 0) {

        Action * a = queue_.dequeue();

        EmpathURL u = a->url();

        switch (a->actionType())
        {
            case RetrieveMessage:
                _retrieve(u, a->xxinfo(), a->xinfo());
                break;
        
            case MarkMessage:
                _mark(u, ((MarkAction *)a)->status(), a->xxinfo(), a->xinfo());
                break;
        
            case WriteMessage:
                _write(u, ((WriteAction *)a)->msg(), a->xxinfo(), a->xinfo());
                break;
        
            case RemoveMessage:
                _removeMessage(u, a->xxinfo(), a->xinfo());
                break;
        
            case CreateFolder:
                _createFolder(u, a->xxinfo(), a->xinfo());
                break;
        
            case RemoveFolder:
                _removeFolder(u, a->xxinfo(), a->xinfo());
                break;
                
            default:
                break;
        }
    }
}

    void
EmpathMailbox::_connectUp()
{
    QObject::connect(
        empath, SIGNAL(checkMail()),
        this,   SLOT(s_checkMail()));

    QObject::connect(
        this, SIGNAL(newMailArrived()),
        empath, SLOT(s_newMailArrived()));

    QObject::connect(
        this,   SIGNAL(updateFolderLists()),
        empath, SLOT(s_updateFolderLists()));

    QObject::connect(
            this,
            SIGNAL(retrieveComplete(bool, const EmpathURL &, const EmpathURL &,
                    QString, QString)),
            empath,
            SLOT(s_retrieveComplete(bool, const EmpathURL &, const EmpathURL &,
                    QString, QString)));

    QObject::connect(
        this,
        SIGNAL(retrieveComplete(bool, const EmpathURL &,
                QString, QString)),
        empath,
        SLOT(s_retrieveComplete(bool, const EmpathURL &,
                QString, QString)));

    QObject::connect(
        this,
        SIGNAL(removeComplete(bool, const EmpathURL &, QString, QString)),
        empath, SLOT(
        s_removeComplete(bool, const EmpathURL &, QString, QString)));

    QObject::connect(
        this,
        SIGNAL(writeComplete(bool, const EmpathURL &, QString, QString)),
        empath,
        SLOT(s_writeComplete(bool, const EmpathURL &, QString, QString)));

    QObject::connect(
        this,
        SIGNAL(markComplete(bool, const EmpathURL &, QString, QString)),
        empath,
        SLOT(s_markComplete(bool, const EmpathURL &, QString, QString)));
}


// vim:ts=4:sw=4:tw=78
