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
# pragma interface "EmpathMailSenderSendmail.h"
#endif

#ifndef EMPATHMAILSENDERSENDMAIL_H
#define EMPATHMAILSENDERSENDMAIL_H

// Qt includes
#include <qobject.h>
#include <qcstring.h>

// KDE includes
#include <kprocess.h>

// Local includes
#include "EmpathMailSender.h"

/**
 * @short Sendmail sender
 *
 * Note: Options should probably be -oem -oi -t by default.
 * Option -t means 'Get recipient list from To:, Cc: and Bcc: fields'
 * 
 * We use -t so that sendmail will work out the recipients from the
 * envelope, rather than us having to produce a list for the command line.
 * It's also cleaner that way, I think.
 *
 * Option -oem means 'Mail back errors, don't tell me now'
 * Option -oi  means 'Ignore dots'.
 * 
 * @author Rikkus
 */

class EmpathMailSenderSendmail : public EmpathMailSender
{
    Q_OBJECT

    public:

        EmpathMailSenderSendmail();
        ~EmpathMailSenderSendmail();

        void sendOne(RMM::RMessage & message, const QString & id);

        void setSendmailLocation(const QString & location);
        
        virtual void saveConfig();
        virtual void loadConfig();

    protected slots:

        void wroteStdin(KProcess *);
        void sendmailExited(KProcess *);
        void sendmailReceivedStderr(KProcess *, char * buf, int buflen);

    private:

        QString     sendmailLocation_;
        KProcess    sendmailProcess_;
        QCString    messageAsString_;
        bool        error_;
        QString     errorStr_;
        Q_UINT32    messagePos_;
        bool        written_;
        
        QString     currentID_;
};

#endif

// vim:ts=4:sw=4:tw=78
