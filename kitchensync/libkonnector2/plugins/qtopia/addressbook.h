/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef OpieHelperAddressBookShit_H
#define OpieHelperAddressBookShit_H

#include <qdatetime.h>
#include <qstring.h>

#include <addressbooksyncee.h>

#include "helper.h"

namespace OpieHelper {

    class AddressBook : public Base {
    public:
        AddressBook( CategoryEdit* edit = 0,
                     KSync::KonnectorUIDHelper* helper = 0,
                     const QString &tz = QString::null,
                     Device *dev = 0);
        ~AddressBook();
        KSync::AddressBookSyncee * toKDE( const QString &fileName, ExtraMap& );
        KTempFile* fromKDE(KSync::AddressBookSyncee* syncee, ExtraMap& );
    private:
        static QStringList supportedAttributes();
        static QDate fromString( const QString& );
        // from OConversion
        static QDate dateFromString( const QString& );
        static QString dateToString( const QDate& );
    };
}


#endif
