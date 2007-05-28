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

#ifndef EDITADDRESSBOOKJOB_H
#define EDITADDRESSBOOKJOB_H

#include "gammujob.h"

/**
 * This class triggers the edit of a address book entry
 *
 * @author Matthias Lechner
 */
class EditAddressBookJob : public GammuJob
{
    public:
        /**
         * Creates a new address book job for adding new or deleting
         * existing contacts
         */
        EditAddressBookJob( kmobiletoolsJob::JobType jobType,
                            KABC::Addressee::List *addresseeList,
                            Device *device, kmobiletoolsGammu_engine *parent = 0,
                            const char *name = 0
                          );

        /**
         * Creates a new address book job for editing existing contacts
         */
        EditAddressBookJob( KABC::Addressee *oldAddressee,
                            KABC::Addressee *newAddressee,
                            Device *device, kmobiletoolsGammu_engine *parent = 0,
                            const char *name = 0
                          );

        kmobiletoolsJob::JobType type() { return m_jobType; }

    protected:
        void run();

    private:
        kmobiletoolsJob::JobType m_jobType;
        Device* m_device;

        KABC::Addressee::List* m_addresseeList;
        KABC::Addressee* m_oldAddressee;
        KABC::Addressee* m_newAddressee;

};

#endif
