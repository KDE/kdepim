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
# pragma interface "EmpathMailSenderSMTP.h"
#endif

#ifndef EMPATHMAILSENDERSMTP_H
#define EMPATHMAILSENDERSMTP_H

// Qt includes
#include <qobject.h>

// KDE includes
#include <kio_job.h>

// Local includes
#include "RMM_Message.h"
#include "EmpathDefines.h"
#include "EmpathMessageList.h"
#include "EmpathMailSender.h"

/**
 * @short SMTP sender
 * The SMTP sender is pretty cool in that we don't need to be running
 * on your typical server-like host so we can run on 'embedded' devices
 * that don't have the capability to run a local mail server.
 * 
 * @author Rikkus
 */
class EmpathMailSenderSMTP : public EmpathMailSender
{
    Q_OBJECT

    public:

        EmpathMailSenderSMTP();
        ~EmpathMailSenderSMTP();

        /**
         * Set the server name and port to connect to.
         */
        void setServer(const QString & name, const Q_UINT32 port);
        
        /**
         * Send one message.
         */
        void sendOne(RMM::RMessage & message);
        
        virtual void saveConfig();
        virtual void readConfig();

    private:

        QString     serverName_;
        Q_UINT32    serverPort_;
        
        KIOJob  * job_;
};

#endif

// vim:ts=4:sw=4:tw=78
