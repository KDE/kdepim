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

#include "editaddressbookjob.h"
#include <kdebug.h>

EditAddressBookJob::EditAddressBookJob( kmobiletoolsJob::JobType jobType,
                                        KABC::Addressee::List *addresseeList,
                                        Device *device,
                                        kmobiletoolsGammu_engine *parent,
                                        const char *name )
                                      : GammuJob( device, parent, name ) {
    kDebug() <<"Gammu engine: edit addressbook job created.";
    m_jobType = jobType;
    m_device = device;
    m_addresseeList = addresseeList;
}

EditAddressBookJob::EditAddressBookJob( KABC::Addressee* oldAddressee,
                            KABC::Addressee* newAddressee,
                            Device *device, kmobiletoolsGammu_engine *parent,
                            const char *name)
                            : GammuJob( device, parent, name ) {
    m_jobType = kmobiletoolsJob::editAddressee;
    m_device = device;
    m_oldAddressee = oldAddressee;
    m_newAddressee = newAddressee;
}

void EditAddressBookJob::run() {
    switch( m_jobType ) {
        case addAddressee:
            if( m_addresseeList == 0 )
                break;

            m_device->addAddressee( m_addresseeList );
            break;

        case editAddressee:
            if( m_newAddressee == 0 || m_oldAddressee == 0 )
                break;

            m_device->editAddressee( m_oldAddressee, m_newAddressee );
            break;

        case delAddressee:
            if( m_addresseeList == 0 )
                break;

            m_device->removeAddressee( m_addresseeList );
            break;

        default:
            kDebug() <<"Gammu engine: EditAddressBookJob::run():"
                     << "unsupported job type requested" << endl;
            break;
    }
}
