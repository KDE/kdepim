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
# pragma interface "EmpathMailSenderQmail.h"
#endif

#ifndef EMPATHMAILSENDERQMAIL_H
#define EMPATHMAILSENDERQMAIL_H

// Qt includes
#include <qobject.h>
#include <qcstring.h>

// KDE includes
#include <kprocess.h>

// Local includes
#include "EmpathMailSender.h"

/**
 * QMail sender
 * @author Rikkus
 */
class EmpathMailSenderQmail : public EmpathMailSender
{
    Q_OBJECT

    public:

        EmpathMailSenderQmail();
        ~EmpathMailSenderQmail();

        /**
         * Send one message.
         */
        void sendOne(RMM::RMessage & message, const QString & id);

        /**
         * Set the location of the qmail-inject binary.
         * On my system, it's /var/qmail/bin/qmail-inject
         */
        void setQmailLocation(const QString & qmailLocation);
        
        virtual void saveConfig();
        virtual void loadConfig();

    protected slots:

        /**
         * @internal
         */
        void wroteStdin(KProcess *);
        /**
         * @internal
         */
        void qmailExited(KProcess *);
        /**
         * @internal
         */
        void qmailReceivedStderr(KProcess *, char * buf, int buflen);

    private:

        QString     qmailLocation_;
        KProcess    qmailProcess_;
        QCString    messageAsString_;
        bool        error_;
        QString     errorStr_;
        Q_UINT32    messagePos_;
        bool        written_;
        
        QString     currentID_;
};

#endif

// vim:ts=4:sw=4:tw=78
