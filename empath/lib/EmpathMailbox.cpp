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
# pragma implementation "EmpathMailbox.h"
#endif

// Local includes
#include "EmpathUtilities.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathIndex.h"
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
EmpathMailbox::setAutoCheckInterval(unsigned int i)
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

    unsigned int
EmpathMailbox::messageCount() const
{
    unsigned int c = 0;
    
    EmpathFolderListIterator it(folderList_);
    
    for (; it.current(); ++it)
        c += it.current()->index()->count();

    return c;
}

    unsigned int
EmpathMailbox::unreadMessageCount() const
{
    unsigned int c = 0;
    
    EmpathFolderListIterator it(folderList_);
    
    for (; it.current(); ++it)
        c += it.current()->index()->countUnread();

    return c;
}

    void
EmpathMailbox::s_countUpdated(unsigned int, unsigned int)
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
}


// vim:ts=4:sw=4:tw=78
