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
# pragma interface "EmpathMailboxPOP3.h"
#endif

#ifndef EMPATHMAILBOXPOP3_H
#define EMPATHMAILBOXPOP3_H

// Qt includes
#include <qstring.h>
#include <qcstring.h>
#include <qfile.h>
#include <qintdict.h>
#include <qstrlist.h>
#include <qlist.h>
#include <qqueue.h>

// Local includes
#include "EmpathMailbox.h"

class KIOJob;

/**
 * @internal
 * @author Rikkus
 */
class EmpathPOPCommand
{
    public:
        
        enum Type { Stat, Index, Get, Remove };

        EmpathPOPCommand(Type, int);
        ~EmpathPOPCommand(); 
        
        QString command();
        Type    type();
        int messageNumber();
        QCString & data();
        
        const char * className() const { return "EmpathPOPCommand"; }

    private:
    
        QCString    data_;
        QString     command_;
        Type        type_;
        int         msgNo_;
};

/**
 * @internal
 */
class EmpathPOPIndexEntry
{
    public:
        
        EmpathPOPIndexEntry(int, const QString &);
        ~EmpathPOPIndexEntry();
        
        int        number();
        QString    id();
        
        const char * className() const { return "EmpathPOPIndexEntry"; }
        
    private:
        
        int        number_;
        QString    id_;
};

/**
 * @internal
 */
class EmpathPOPIndex : public QList<EmpathPOPIndexEntry>
{
    public:
        
        EmpathPOPIndex();
        ~EmpathPOPIndex();
        int compareItems(EmpathPOPIndexEntry * i1, EmpathPOPIndexEntry * i2);
        
        const char * className() const { return "EmpathPOPIndex"; }
};

typedef QListIterator<EmpathPOPIndexEntry>    EmpathPOPIndexIterator;

typedef QQueue<EmpathPOPCommand>            EmpathPOPQueue;

/**
 * @short POP3 mailbox
 * @author Rikkus
 */
class EmpathMailboxPOP3 : public EmpathMailbox
{
    Q_OBJECT

    public:

        EmpathMailboxPOP3(const QString & name);
        
        ~EmpathMailboxPOP3();

        bool logging();
        void setLogging(bool policy);
        bool alreadyHave();
        EmpathURL path();

#include "EmpathMailboxAbstract.h"

    protected slots:

        void s_jobError(int, int, const char *);
        void s_jobData(int, const char *, int);
        void s_jobFinished(int);
        void s_jobCancelled(int);
    
    public:

        // Set methods
        
        void setServerAddress           (const QString &);
        void setServerPort              (Q_UINT32);
        void setUsername                (const QString &);
        void setPassword                (const QString &);
        void setUseAPOP                 (bool);
        void setPasswordSavePolicy      (SavePolicy);
        void setLoggingPolicy           (bool);
        void setLogFilePath             (const QString &);
        void setLogFileDisposalPolicy   (bool);
        void setMaxLogFileSize          (Q_UINT32);
        void setMessageSizeThreshold    (Q_UINT32);
        void setSaveAllAddresses        (bool);
        void setRetrieveIfHave          (bool);

        // Get methods
        
        QString     serverAddress();
        Q_UINT32    serverPort();
        QString     username();
        QString     password();
        bool        useAPOP();
        bool        loggingPolicy();
        QString     logFilePath();
        bool        logFileDisposalPolicy();
        Q_UINT32    maxLogFileSize();
        Q_UINT32    messageSizeThreshold();
        bool        autoGetNewMail();
        bool        saveAllAddresses();
        bool        retrieveIfHave();

    private:

        // The pop3 server is a state machine, sort of.
        // It can either be in the Transaction or Authorisation state.
        // We, on the other hand, have another state -
        // Not connected to the server.
    
        void _enqueue(EmpathPOPCommand::Type, int i);
        void _nextCommand();
    
        // Order dependency
        QString     serverAddress_;
        Q_UINT32    serverPort_;
        QString     username_;
        QString     password_;
        bool        logging_;
        Q_UINT32    numMessages_;
        Q_UINT32    mailboxSize_;
        bool        logFileOpen_;
        Q_UINT32    authenticationTries_;
        // End order dependency
        
        bool        useAPOP_;
        bool        loggingPolicy_;
        QString     logFilePath_;
        bool        logFileDisposalPolicy_;
        Q_UINT32    maxLogFileSize_;
        Q_UINT32    messageSizeThreshold_;
        bool        saveAllAddresses_;
        bool        retrieveIfHave_;
        int         sock_fd; // socket fd
        QCString    errorStr;
        bool        connected_;
        bool        loggedIn_;
        QString     timeStamp_;
        QFile       logFile_;
        QCString    greeting_;
        
        KIOJob * job;
        
        EmpathPOPIndex  index_;
        EmpathPOPQueue  commandQueue_;
        
        unsigned int msgsInSpool_;
        unsigned int octetsInSpool_;
};


#endif
// vim:ts=4:sw=4:tw=78
