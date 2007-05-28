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

#ifndef INITPHONEJOB_H
#define INITPHONEJOB_H

#include "gammujob.h"

/**
 * This class triggers the initialization of the phone
 *
 * @author Matthias Lechner
 */
class InitPhoneJob : public GammuJob
{
    public:
        explicit InitPhoneJob( Device *device, kmobiletoolsGammu_engine* parent = 0,
                      const char* name = 0);

        JobType type() { return kmobiletoolsJob::initPhone; }
    protected:
        void run ();
};

#endif
