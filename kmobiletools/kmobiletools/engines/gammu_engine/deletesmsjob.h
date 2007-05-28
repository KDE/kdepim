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

#ifndef DELETESMSJOB_H
#define DELETESMSJOB_H

#include "gammujob.h"
#include "gammusms.h"

/**
 * This class triggers the deletion of a sms
 *
 * @author Matthias Lechner
 */
class DeleteSMSJob : public GammuJob
{
    public:
        DeleteSMSJob ( GammuSMS *sms,
                       Device *device, kmobiletoolsGammu_engine* parent = 0 , const char* name = 0 );
        kmobiletoolsJob::JobType type() { return kmobiletoolsJob::delSMS; }

    protected:
        void run();

    private:
        GammuSMS *m_sms;
};


#endif
