/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "sendsmsjob.h"

SendSMSJob::SendSMSJob( kmobiletoolsJob::JobType jobType, GammuSMS* sms, Device *device, 
                        kmobiletoolsGammu_engine* parent, const char* name )
                      : GammuJob( device, parent, name ) {
    m_jobType = jobType;
    m_sms = sms;
}

SendSMSJob::SendSMSJob( kmobiletoolsJob::JobType jobType, const QString& number, const QString &text,
                        Device *device, kmobiletoolsGammu_engine* parent, const char* name )
                      : GammuJob( device, parent, name ) {
    m_sms = new GammuSMS();
    m_sms->setText( text );
    m_sms->setNumbers( QStringList( number ) );

    m_jobType = jobType;
}

void SendSMSJob::run() {
    switch( m_jobType ) {
        case kmobiletoolsJob::sendStoredSMS:
            device()->sendStoredSMS( m_sms );
            break;

        case kmobiletoolsJob::sendSMS:
            device()->sendSMS( (SMS*) m_sms );
            break;

        default:
            kdDebug() << "Gammu engine: SendSMSJob called with unknown job type" << endl;
            kdDebug() << "Gammu engine: This should not happen and is a coding error ;-)" << endl;
    }
}
