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

// Qt includes
#include <qregexp.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>

// Local includes
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathFolderList.h"
#include "EmpathFolder.h"
#include "EmpathDefines.h"
#include "EmpathMailbox.h"
#include "EmpathIndex.h"
#include "EmpathUtilities.h"

EmpathFolder::EmpathFolder()
    :   QObject(),
        container_(false)
{
    pixmapName_ = "folder-normal";
}

EmpathFolder::EmpathFolder(const EmpathURL & url)
    :   QObject(),
        url_(url),
        container_(false)
{
    index_ = new EmpathIndex(url_);
	
    QObject::connect(this, SIGNAL(countUpdated(Q_UINT32, Q_UINT32)),
        empath->mailbox(url_), SLOT(s_countUpdated(Q_UINT32, Q_UINT32)));
 
    if      (url_ == empath->inbox())   pixmapName_ = "folder-inbox";
    else if (url_ == empath->outbox())  pixmapName_ = "folder-outbox";
    else if (url_ == empath->sent())    pixmapName_ = "folder-sent";
    else if (url_ == empath->drafts())  pixmapName_ = "folder-drafts";
    else if (url_ == empath->trash())   pixmapName_ = "folder-trash";
    else                                pixmapName_ = "folder-normal";
}

    bool
EmpathFolder::operator == (const EmpathFolder &) const
{
    return false;
}

EmpathFolder::~EmpathFolder()
{
    delete index_;
}

    void
EmpathFolder::setPixmap(const QString & p)
{
    pixmapName_ = p;
}

    const EmpathIndexRecord *
EmpathFolder::record(const QCString & key)
{
    return index_->record(key);
}

    void
EmpathFolder::update()
{
    EmpathMailbox * m = empath->mailbox(url_);
    
    if (m == 0) {
        empathDebug("Can't find my mailbox !");
        return;
    }
    
    m->sync(url_);
    
    emit(countUpdated(index_->countUnread(), index_->count()));
}

    EmpathFolder *
EmpathFolder::parent() const
{
    QString f = url_.folderPath();
    QString m = url_.mailboxName();
    
    if (f.right(1) == "/")
        f.remove(f.length() - 1, 1);
    
    if (!f.contains("/"))
        return 0;
    
    f = f.left(f.findRev("/"));
    f += "/";
    
    EmpathURL u(url_.mailboxName(), f, QString::null);
    return empath->folder(u);
}

    Q_UINT32
EmpathFolder::messageCount()
{
    return index_->count();
}

    Q_UINT32
EmpathFolder::unreadMessageCount()
{
    return index_->countUnread();
}

    void
EmpathFolder::setStatus(const QString & id, RMM::MessageStatus status)
{
    index_->setStatus(id, status);
}

    bool
EmpathFolder::isContainer() const
{
    return container_;
}

    void
EmpathFolder::setContainer(bool b)
{
    container_ = b;
}

// vim:ts=4:sw=4:tw=78
