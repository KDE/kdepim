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

#ifndef FETCHSMSJOB_H
#define FETCHSMSJOB_H

#include "smslist.h"
#include "gammujob.h"

/**
 * This class triggers the fetch of the phone's sms
 *
 * @author Matthias Lechner
 */
class FetchSMSJob : public GammuJob
{
    public:
        explicit FetchSMSJob( Device *device, kmobiletoolsGammu_engine* parent = 0,
                     const char* name = 0 );
        kmobiletoolsJob::JobType type() { return kmobiletoolsJob::fetchSMS; }

        SMSList* smsList() { return m_smsList; }

    protected:
        void run();

    private:
        SMSList* m_smsList;
};

#endif
