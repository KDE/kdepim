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

#ifndef PHONEINFOSJOB_H
#define PHONEINFOSJOB_H

#include "gammujob.h"

/**
 * This class triggers the fetch of basic information about the phone
 *
 * @author Matthias Lechner
 */
class PhoneInfosJob : public GammuJob
{
    public:
        explicit PhoneInfosJob ( Device *device, kmobiletoolsGammu_engine* parent = 0 ,
                          const char* name = 0 );
        kmobiletoolsJob::JobType type() { return kmobiletoolsJob::fetchPhoneInfos; }

        QString model() const { return m_model; }
        QString manufacturer() const { return m_manufacturer; }
        QString revision() const { return m_firmware; }
        QString imei() const { return m_imei; }

    protected:
        void run();

    private:
        QString m_model, m_manufacturer, m_firmware, m_imei;
};

#endif
