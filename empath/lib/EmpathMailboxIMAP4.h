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
        
        void setServerAddress            (const QString &);
        void setServerPort                (Q_UINT32);
        void setUsername                (const QString &);
        void setPassword                (const QString &);
        void setPasswordSavePolicy        (SavePolicy);
        void setLoggingPolicy            (bool);
        void setLogFilePath                (const QString &);
        void setLogFileDisposalPolicy    (bool);
        void setMaxLogFileSize            (Q_UINT32);
        void setMessageSizeThreshold    (Q_UINT32);
        void setCheckForNewMail            (bool);
        void setMailCheckInterval        (Q_UINT32);
        void setDeleteFromServer        (bool);
        void setAutoGetNewMail            (bool);
        void setSaveAllAddresses        (bool);
        void setNotify                    (bool);
        void setRetrieveIfHave            (bool);

        // Get methods
        
        QString        serverAddress();
        Q_UINT32    serverPort();
        QString        username();
        QString        password();
        bool        loggingPolicy();
        QString        logFilePath();
        bool        logFileDisposalPolicy();
        Q_UINT32    maxLogFileSize();
        Q_UINT32    messageSizeThreshold();
        SavePolicy    passwordSavePolicy();
    
    private:

        QString                serverAddress_;
        Q_UINT32            serverPort_;
        QString                username_;
        QString                password_;
        SavePolicy            passwordSavePolicy_;
        bool                loggingPolicy_;
        QString                logFilePath_;
        bool                logFileDisposalPolicy_;
        unsigned            int maxLogFileSize_;
};

#endif
// vim:ts=4:sw=4:tw=78
