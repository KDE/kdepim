/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Tobias Koenig <tokoe@kdab.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef CALENDARSUPPORT_BLOCKALARMSATTRIBUTE_H
#define CALENDARSUPPORT_BLOCKALARMSATTRIBUTE_H

#include "calendarsupport_export.h"

#include <akonadi/attribute.h>

namespace CalendarSupport {

/**
 * @short An Attribute that marks that alarms from an calendar collection are blocked.
 *
 * A calendar collection which has this attribute set won't be evaluated by korgac and
 * therefor its alarms won't be used.
 *
 * @author Tobias Koenig <tokoe@kdab.com>
 * @see Akonadi::Attribute
 */
class CALENDARSUPPORT_EXPORT BlockAlarmsAttribute : public Akonadi::Attribute
{
  public:
    /**
     * Creates a new block alarms attribute.
     */
    BlockAlarmsAttribute();

    /**
     * Destroys the block alarms attribute.
     */
    ~BlockAlarmsAttribute();

    /**
     * Reimplemented from Attribute
     */
    QByteArray type() const;

    /**
     * Reimplemented from Attribute
     */
    BlockAlarmsAttribute *clone() const;

    /**
     * Reimplemented from Attribute
     */
    QByteArray serialized() const;

    /**
     * Reimplemented from Attribute
     */
    void deserialize( const QByteArray &data );
};

}

#endif
