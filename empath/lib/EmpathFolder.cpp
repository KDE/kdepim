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

    EmpathMailbox * m = empath->mailbox(url_);
	
    if (m != 0)
        QObject::connect(
            this,   SIGNAL(countUpdated(unsigned int, unsigned int)),
            m,      SLOT(s_countUpdated(unsigned int, unsigned int)));
 
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

    void
EmpathFolder::update()
{
    EmpathMailbox * m = empath->mailbox(url_);
    
    if (m == 0) {
        empathDebug("Can't find my mailbox !");
        return;
    }
    
    unsigned int oldUnread = index_->countUnread();
    unsigned int oldRead = index_->count();

    m->sync(url_);
    
    unsigned int unread = index_->countUnread();
    unsigned int read = index_->count();

    if ((oldUnread != unread) || (oldRead != read)) {
        empathDebug("Count changed");
        emit(countUpdated(index_->countUnread(), index_->count()));
    }
}

    void
EmpathFolder::syncIndex()
{
    index_->sync();
} 

    EmpathIndexRecord
EmpathFolder::indexRecord(const QString & key)
{
    return index_->record(key);
}

    QStringList
EmpathFolder::allIndexKeys()
{
    return index_->allKeys();
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
EmpathFolder::setStatus(const QString & key, RMM::MessageStatus status)
{
    index_->setStatus(key, status);
}

    bool
EmpathFolder::removeFromIndex(const QString & key)
{
    return index_->remove(key);
}

    bool
EmpathFolder::insertInIndex(const QString & key, EmpathIndexRecord & rec)
{
    return index_->insert(key, rec);
}

    bool
EmpathFolder::replaceInIndex(const QString & key, EmpathIndexRecord & rec)
{
    return index_->replace(key, rec);
}

    void
EmpathFolder::setIndexInitialised()
{
    index_->setInitialised(true);
}

    bool
EmpathFolder::indexInitialised()
{
    return index_->initialised();
}

    QDateTime
EmpathFolder::indexModified()
{
    return index_->lastModified();
}

    bool
EmpathFolder::indexContains(const QString & key)
{
    return index_->contains(key);
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
