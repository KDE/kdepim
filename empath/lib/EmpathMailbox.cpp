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
    :    url_(name, QString::null, QString::null)
{
    empathDebug("ctor - url == \"" + url_.asString() + "\"");
    pixmapName_ = "mailbox.png";
    folderList_.setAutoDelete(true);
    QObject::connect(this, SIGNAL(updateFolderLists()),
        empath, SLOT(s_updateFolderLists()));
}

EmpathMailbox::~EmpathMailbox()
{
    empathDebug("dtor");
}

    void
EmpathMailbox::setCheckMail(bool yn)
{
    empathDebug(QString("Setting check mail to ") + (yn ? "true" : "false"));
    checkMail_ = yn;
    if (checkMail_) {
        empathDebug("Switching on timer");
        timer_.stop();
        timer_.start(checkMailInterval_ * 60000);
    }
    else {
        empathDebug("Switching off timer");
        timer_.stop();
    }
}

    void
EmpathMailbox::setCheckMailInterval(Q_UINT32 checkMailInterval)
{
    empathDebug("Setting timer interval to  " +
            QString().setNum(checkMailInterval));
    checkMailInterval_ = checkMailInterval;
    if (checkMail_) {
        timer_.stop();
        timer_.start(checkMailInterval_ * 60000);
    }
}

    void
EmpathMailbox::setName(const QString & name) 
{
    url_.setMailboxName(name);
}

    Q_UINT32
EmpathMailbox::messageCount() const
{
    empathDebug("messageCount() called");

    Q_UINT32 c = 0;
    
    EmpathFolderListIterator it(folderList_);
    
    for (; it.current(); ++it)
        c += it.current()->messageCount();

    return c;
}

    Q_UINT32
EmpathMailbox::unreadMessageCount() const
{
    empathDebug("unreadMessageCount() called");

    Q_UINT32 c = 0;
    
    EmpathFolderListIterator it(folderList_);
    empathDebug("There are " + QString().setNum(folderList_.count()) +
        "folders to count messages in");
    
    for (; it.current(); ++it)
        c += it.current()->unreadMessageCount();

    return c;
}

    void
EmpathMailbox::s_countUpdated(int, int)
{
    empathDebug("s_countUpdated() called");
    empathDebug("emitting(" + QString().setNum(unreadMessageCount()) +
       ", " + QString().setNum(messageCount()) + ")");
    emit(countUpdated((int)unreadMessageCount(), (int)messageCount()));
}

    EmpathFolder *
EmpathMailbox::folder(const EmpathURL & url)
{
    empathDebug("folder(" + url.asString() + ") called");
    QString fp(url.folderPath());

    if (fp.at(0) == '/') fp.remove(0, 1);
    if (fp.at(fp.length() - 1) == '/') fp.remove(fp.length() - 1, 1);
    
    EmpathFolderListIterator it(folderList_);
    for (; it.current(); ++it) {
        if (it.current()->url().folderPath() == fp) {
            return it.current();
        }
    }
    
    empathDebug("nothing found!");
    return 0;
}

    void
EmpathMailbox::retrieve(const EmpathURL & url, QString xinfo)
{
    _enqueue(RetrieveMessage, url, xinfo);
}

    void
EmpathMailbox::retrieve(
    const EmpathURL &, const EmpathURL &, QString, QString)
{
    _enqueue(RetrieveMessage, from, to, xxinfo, xinfo);
}

    EmpathURL
EmpathMailbox::write(
    const EmpathURL & folder, RMM::RMessage & msg, QString xinfo)
{
    QString id = empath->generateUnique();
    EmpathURL u(folder);
    u.setMessageID(id);
    _enqueue(u, msg, xinfo);
    return u;
}
    
    void
EmpathMailbox::remove(const EmpathURL & url, QString xinfo)
{
    if (url.hasMessageID())
        _enqueue(RemoveMessage, url, xinfo);
    else
        _enqueue(RemoveFolder, url, xinfo);
}
    
    void
EmpathMailbox::remove(
    const EmpathURL & url, const QStringList & l, QString xinfo)
{
    EmpathURL u(url);
    
    QStringList::ConstIterator it;

    for (it = l.begin(); it != l.end(); ++it) {
        u.setMessageID(*it);
        _enqueue(RemoveMessage, u, xinfo);
    }
}
    
    void
EmpathMailbox::createFolder(const EmpathURL & url, QString xinfo)
{
    _enqueue(CreateFolder, url, xinfo);
}
    
    void
EmpathMailbox::mark(const EmpathURL & url, RMM::MessageStatus s, QString xinfo)
{
    _enqueue(url, s, xinfo);
}
    
    void
EmpathMailbox::mark(
    const EmpathURL & url,
    const QStringList & l,
    RMM::MessageStatus s,
    QString xinfo)
{
    EmpathURL u(url);
    
    QStringList::ConstIterator it;

    for (it = l.begin(); it != l.end(); ++it) {
        u.setMessageID(*it);
        _enqueue(u, s, xinfo);
    }
}

    void
EmpathMailbox::_enqueue(
    const EmpathURL & url, RMM::MessageStatus s, QString xinfo)
{
    _enqueue(new MarkAction(url, s, xinfo));
}

    void
EmpathMailbox::_enqueue(ActionType t, const EmpathURL & url, QString xinfo)
{
    _enqueue(new Action(t, url, xinfo));
}

    void
EmpathMailbox::_enqueue(
    const EmpathURL & url, RMM::RMessage & msg, QString xinfo)
{
    _enqueue(new WriteAction(url, msg, xinfo));
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
    empathDebug("");
    
    while (queue_.count() != 0) {

        Action * a = queue_.dequeue();

        EmpathURL u = a->url();

        switch (a->actionType())
        {
            case RetrieveMessage:
                _retrieve(u, a->xinfo());
                break;
        
            case MarkMessage:
                _mark(u, ((MarkAction *)a)->status(), a->xinfo());
                break;
        
            case WriteMessage:
                _write(u, ((WriteAction *)a)->msg(), a->xinfo());
                break;
        
            case RemoveMessage:
                _removeMessage(u, a->xinfo());
                break;
        
            case CreateFolder:
                _createFolder(u, a->xinfo());
                break;
        
            case RemoveFolder:
                _removeFolder(u, a->xinfo());
                break;
                
            default:
                break;
        }
    }
}

// vim:ts=4:sw=4:tw=78
