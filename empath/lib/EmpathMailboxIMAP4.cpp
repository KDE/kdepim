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


// Local includes
#include "EmpathMailboxIMAP4.h"

EmpathMailboxIMAP4::EmpathMailboxIMAP4(const QString & name)
    :   EmpathMailbox   (name),
        serverAddress_  (QString::null),
        serverPort_     (110),
        username_       (QString::null),
        password_       (QString::null)
{
    type_ = IMAP4;
    typeString_ = "IMAP4";
}

EmpathMailboxIMAP4::~EmpathMailboxIMAP4()
{
    // Empty.
}

    void
EmpathMailboxIMAP4::s_checkMail()
{
    // STUB
}

    void
EmpathMailboxIMAP4::saveConfig()
{
    // STUB
}

    void
EmpathMailboxIMAP4::loadConfig()
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

    bool
EmpathMailboxIMAP4::removeMessage(const EmpathURL & /* url */)
{
    // STUB
    return false;
}
 
    bool
EmpathMailboxIMAP4::createFolder(const EmpathURL &)
{
    // STUB
    return false;
}

    bool
EmpathMailboxIMAP4::removeFolder(const EmpathURL &)
{
    // STUB
    return false;
}
 
    RMM::Message
EmpathMailboxIMAP4::retrieveMessage(const EmpathURL & /* url */)
{
    // STUB
    RMM::Message retval;
    return retval;
}

    QString
EmpathMailboxIMAP4::writeMessage(
    RMM::Message &,
    const EmpathURL &
)
{
    // STUB
    return QString::null;
}

    bool
EmpathMailboxIMAP4::markMessage(const EmpathURL &, EmpathIndexRecord::Status)
{
    // STUB
    return false;
}

    EmpathSuccessMap
EmpathMailboxIMAP4::markMessage(
    const EmpathURL &,
    const QStringList &,
    EmpathIndexRecord::Status)
{
    // STUB
    EmpathSuccessMap retval;
    return retval;
}

    EmpathSuccessMap
EmpathMailboxIMAP4::removeMessage(
    const EmpathURL & /* folder */,
    const QStringList & /* messageIDList */
)
{
    // STUB
    EmpathSuccessMap retval;
    return retval;
}


// vim:ts=4:sw=4:tw=78
