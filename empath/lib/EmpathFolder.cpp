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
# pragma implementation "EmpathFolder.h"
#endif

// Local includes
#include "Empath.h"
#include "EmpathFolderList.h"
#include "EmpathFolder.h"
#include "EmpathDefines.h"
#include "EmpathMailbox.h"
#include "EmpathIndex.h"
#include "EmpathUtilities.h"
#include "EmpathIndexAllocator.h"

EmpathFolder::EmpathFolder()
    :    QObject(),
        messageCount_(0),
        unreadMessageCount_(0)
{
    empathDebug("default ctor !");
    indexAllocator_ = new EmpathIndexAllocator;
    pixmapName_ = "mini-folder-grey.png";
}

EmpathFolder::EmpathFolder(const EmpathURL & url)
    :    QObject(),
        messageCount_(0),
        unreadMessageCount_(0),
        url_(url)
{
    indexAllocator_ = new EmpathIndexAllocator;
    empathDebug("ctor with url == \"" + url_.asString() + "\"");
    messageList_.setFolder(this);
    QObject::connect(this, SIGNAL(countUpdated(int, int)),
        empath->mailbox(url_), SLOT(s_countUpdated(int, int)));
    pixmapName_ = "mini-folder-grey.png";
}

    bool
EmpathFolder::operator == (const EmpathFolder &) const
{
    empathDebug("operator == () called");
    return false;
}

EmpathFolder::~EmpathFolder()
{
    empathDebug("dtor");
}

    void
EmpathFolder::setPixmap(const QString & p)
{
    pixmapName_ = p;
}

    const EmpathIndexRecord *
EmpathFolder::messageDescription(RMM::RMessageID & id) const
{
    empathDebug("messageWithID(" + id.asString() + ") called");
    return messageList_.messageDescription(id);
}

    void
EmpathFolder::dropIndex()
{
    messageList_.clear();
    
    delete indexAllocator_;
    indexAllocator_ = new EmpathIndexAllocator;
}

    void
EmpathFolder::update()
{
    empathDebug("update() called");
    EmpathMailbox * m = empath->mailbox(url_);
    if (m == 0) return;
    empathDebug("mailbox name = " + m->name());
    m->sync(url_);
    empathDebug("emitting(" + QString().setNum(messageList_.countUnread()) +
       ", " + QString().setNum(messageList_.count()) + ")");
    emit(countUpdated(messageList_.countUnread(), messageList_.count()));
}

    EmpathFolder *
EmpathFolder::parent() const
{
    empathDebug("parent() called");
    QString f = url_.folderPath();
    QString m = url_.mailboxName();
    empathDebug("My folder path is \"" + f + "\"");
    if (f.right(1) == "/")
        f.remove(f.length() - 1, 1);
    if (!f.contains("/")) return 0;
    f = f.left(f.findRev("/"));
    f += "/";
    empathDebug("Parent folder path is \"" + f + "\"");
    EmpathURL u(url_.mailboxName(), f, QString::null);
    return empath->folder(u);
}

// vim:ts=4:sw=4:tw=78
