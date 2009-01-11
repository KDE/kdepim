/*
  This file is part of libkabc.
  Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2008 Kevin Krammer <kevin.krammer@gmx.at>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KABC_CONTACTGROUPTOOL_H
#define KABC_CONTACTGROUPTOOL_H

#include "kdepim_export.h"

class QIODevice;
class QString;

template <class T> class QList;

namespace KPIM {

class ContactGroup;

/**
 * @since 4.2
 */
class KDEPIM_EXPORT ContactGroupTool
{
  public:
    static bool convertFromXml( QIODevice *device, ContactGroup &group,
                                QString *errorMessage = 0 );

    static bool convertToXml( const ContactGroup &group, QIODevice *device,
                              QString *errorMessage = 0 );

    static bool convertFromXml( QIODevice *device,
                                QList<ContactGroup> &groupList,
                                QString *errorMessage = 0 );

    static bool convertToXml( const QList<ContactGroup> &groupList,
                              QIODevice *device, QString *errorMessage = 0 );
};

}

#endif
