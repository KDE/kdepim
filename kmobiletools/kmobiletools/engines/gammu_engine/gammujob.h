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

#ifndef GAMMUJOB_H
#define GAMMUJOB_H

#include "kmobiletoolsengine.h"
#include "kmobiletoolsgammu_engine.h"

class kmobiletoolsGammu_engine;
class Device;

/**
 * This class extends the default job class by providing
 * a device to the jobs
 *
 * @author Matthias Lechner
 */
class GammuJob : public kmobiletoolsJob
{
    public:
        explicit GammuJob(Device *device, kmobiletoolsGammu_engine* parent = 0 , const char* name = 0);

    protected:
        Device* device();
        kmobiletoolsGammu_engine* engine();

    private:
        Device *m_device;
        kmobiletoolsGammu_engine *m_engine;
};

#endif
