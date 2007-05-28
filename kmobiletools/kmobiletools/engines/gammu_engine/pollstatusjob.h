/***************************************************************************
   Copyright (C) 2005 by Marco Gulino <marco.gulino@gmail.com>
   Copyright (C) 2005 by Stefan Bogner <bochi@kmobiletools.org>
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

#ifndef POLLSTATUSJOB_H
#define POLLSTATUSJOB_H

#include "gammujob.h"

/**
 * This class triggers the fetch of phone status information
 *
 * @author Matthias Lechner
 */
class PollStatusJob : public GammuJob
{
    public:
        explicit PollStatusJob( Device *device, kmobiletoolsGammu_engine* parent = 0,
                       const char* name = 0 );
        kmobiletoolsJob::JobType type() { return kmobiletoolsJob::pollStatus; }

        int phoneCharge() { return m_battery; }
        int phoneSignal() { return m_signal; }
        QString networkName() { return m_network; }
        int unreadSMS() { return m_unreadSMS; }
        int totalSMS() { return m_totalSMS; }
        bool ringing() { return m_ringing; }
    protected:
        void run();
    private:
        int m_battery, m_signal, m_unreadSMS, m_totalSMS;
        QString m_network;
        bool m_ringing;
};

#endif
