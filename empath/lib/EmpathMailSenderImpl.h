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

#ifndef EMPATH_MAIL_SENDER_IMPL_H
#define EMPATH_MAIL_SENDER_IMPL_H

// Qt includes
#include <qobject.h>
#include <qstring.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathEnum.h"
#include "EmpathURL.h"
#include <RMM_Message.h>

/**
 * @short Sender Implementation base class
 * 
 * @author Rikkus
 */

class EmpathMailSenderImpl : public QObject
{
    Q_OBJECT

    public:

        EmpathMailSenderImpl() { }

        virtual ~EmpathMailSenderImpl() { }
        
        /**
         * Send one message.
         */
        virtual void sendOne(RMM::RMessage message, const QString & id) = 0L;

        /**
         * Save your config now !
         * Called by Empath when settings have changed.
         */
        virtual void saveConfig() = 0;

        /**
         * Load your config now !
         * Called by Empath on startup.
         */
        virtual void loadConfig() = 0;
};

#endif

// vim:ts=4:sw=4:tw=78
