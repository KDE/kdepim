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
# pragma implementation "EmpathMailboxIMAP4.h"
#endif

// Local includes
#include "EmpathMailboxIMAP4.h"

EmpathMailboxIMAP4::EmpathMailboxIMAP4(const QString & name)
    :    EmpathMailbox    (name),
        serverAddress_    (QString::null),
        serverPort_        (110),
        username_        (QString::null),
        password_        (QString::null)
{
    empathDebug("ctor");
    type_    = IMAP4;
    setName(name);
}


    bool    
EmpathMailboxIMAP4::getMail()
{
    // STUB
    return false;
}

    void
EmpathMailboxIMAP4::s_getNewMail()
{
    // STUB
}
    void
EmpathMailboxIMAP4::s_checkNewMail()
{
    // STUB
}

    void
EmpathMailboxIMAP4::saveConfig()
{
    // STUB
}

    void
EmpathMailboxIMAP4::readConfig()
{
    // STUB
}

// Set methods
        
    void
EmpathMailboxIMAP4::setServerAddress(const QString & serverAddress)
{
    serverAddress_    = serverAddress;
}

    void
EmpathMailboxIMAP4::setServerPort(Q_UINT32 serverPort)
{
    serverPort_ = serverPort;
}

    void
EmpathMailboxIMAP4::setUsername(const QString & username)
{
    username_ = username;
}

    void
EmpathMailboxIMAP4::setPassword(const QString & password)
{
    // STUB
    password_ = password;
}

// Get methods
        
    QString
EmpathMailboxIMAP4::serverAddress()
{
    return serverAddress_;
}

    Q_UINT32
EmpathMailboxIMAP4::serverPort()
{
    return serverPort_;
}

    QString
EmpathMailboxIMAP4::username()
{
    return username_;
}

    QString
EmpathMailboxIMAP4::password()
{
    // STUB
    return password_;
}

    EmpathMailbox::SavePolicy
EmpathMailboxIMAP4::passwordSavePolicy()
{
    return passwordSavePolicy_;
}

    QString
EmpathMailboxIMAP4::_write(
    const EmpathURL & url, RMM::RMessage &, QString xinfo)
{
    // STUB
    emit (writeComplete(false, url, xinfo));
    return QString::null;
}

    bool
EmpathMailboxIMAP4::newMail() const
{
    // STUB
    return false;
}
    void
EmpathMailboxIMAP4::sync(const EmpathURL &)
{
    // STUB
}

    void
EmpathMailboxIMAP4::init()
{
    // STUB
    empathDebug("init() called");
}

    void
EmpathMailboxIMAP4::_retrieve(const EmpathURL &, const EmpathURL &, QString, QString)
{
    // STUB
}

    void
EmpathMailboxIMAP4::_retrieve(const EmpathURL & url, QString xinfo)
{
    // STUB
    emit (retrieveComplete(false, url, xinfo));
}

    void
EmpathMailboxIMAP4::_removeMessage(const EmpathURL & url, QString xinfo)
{
    // STUB
    emit (removeComplete(false, url, xinfo));
}

    void
EmpathMailboxIMAP4::_createFolder(const EmpathURL & url, QString xinfo)
{
    // STUB
    emit (createFolderComplete(false, url, xinfo));
}

    void
EmpathMailboxIMAP4::_removeFolder(const EmpathURL & url, QString xinfo)
{
    // STUB
    emit (removeFolderComplete(false, url, xinfo));
}

    void
EmpathMailboxIMAP4::_mark(
    const EmpathURL & url, RMM::MessageStatus, QString xinfo)
{
    // STUB
    emit (markComplete(false, url, xinfo));
}

    void
EmpathMailboxIMAP4::_mark(
    const EmpathURL & url,
    const QStringList & l,
    RMM::MessageStatus,
    QString xinfo)
{
    // STUB
    EmpathURL u(url);
    
    QStringList::ConstIterator it;
    
    for (it = l.begin(); it != l.end(); ++it) {
        u.setMessageID(*it);
        emit (markComplete(false, u, xinfo));
    }
}

    void
EmpathMailboxIMAP4::_removeMessage(
    const EmpathURL & url, const QStringList & l, QString xinfo)
{
    // STUB
    EmpathURL u(url);
    
    QStringList::ConstIterator it;
    
    for (it = l.begin(); it != l.end(); ++it) {
        u.setMessageID(*it);
        emit (removeComplete(false, u, xinfo));
    }

}

// vim:ts=4:sw=4:tw=78
