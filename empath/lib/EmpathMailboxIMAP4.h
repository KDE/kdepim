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
# pragma interface "EmpathMailboxIMAP4.h"
#endif

#ifndef EMPATHMAILBOXIMAP4_H
#define EMPATHMAILBOXIMAP4_H

// Qt includes
#include <qstring.h>

// Local includes
#include "EmpathMailbox.h"

/**
 * @short IMAP4 mailbox
 * @author Rikkus
 */
class EmpathMailboxIMAP4 : public EmpathMailbox
{
    Q_OBJECT

    public:

        EmpathMailboxIMAP4(const QString & name);
        
        EmpathMailboxIMAP4(
                const QString &    name,
                const QString &    serverAddress,
                Q_UINT32        serverPort,
                const QString &    username,
                const QString &    password);

        EmpathMailboxIMAP4(const EmpathMailboxIMAP4 &);

        EmpathMailboxIMAP4 & operator = (const EmpathMailboxIMAP4 &);


        ~EmpathMailboxIMAP4 () { empathDebug ("dtor"); } // blank
    
#include "EmpathMailboxAbstract.h"
        
        // Set methods
        
        /**
         * Set the server address to connect to.
         */
        void setServerAddress            (const QString &);
        /**
         * Set the server port to connect to.
         */
        void setServerPort               (Q_UINT32);
        /**
         * Set the username to use when logging in to the server.
         */
        void setUsername                (const QString &);
        /**
         * Set the password to use when logging in to the server.
         */
        void setPassword                (const QString &);
        /**
         * Set how to store (or not) the password.
         */
        void setPasswordSavePolicy      (SavePolicy);
        /**
         * Set whether to check for new mail at intervals.
         */
        void setCheckForNewMail         (bool);
        /**
         * Set the interval to check for new mail.
         */
        void setMailCheckInterval       (Q_UINT32);

        // Get methods
        
        QString        serverAddress();
        Q_UINT32       serverPort();
        QString        username();
        QString        password();
    
    private:

        QString             serverAddress_;
        Q_UINT32            serverPort_;
        QString             username_;
        QString             password_;
        bool                loggingPolicy_;
        QString             logFilePath_;
        bool                logFileDisposalPolicy_;
        unsigned            int maxLogFileSize_;
};

#endif
// vim:ts=4:sw=4:tw=78
