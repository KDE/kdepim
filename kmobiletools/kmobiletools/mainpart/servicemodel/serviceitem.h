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

#ifndef SERVICEITEM_H
#define SERVICEITEM_H

#include <libkmobiletools/coreservice.h>

#include "treeitem.h"

/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class ServiceItem : public TreeItem {
    Q_OBJECT
public:
    ServiceItem( const QString& name, TreeItem* parent );
    ~ServiceItem();

    /**
     * Sets the service that the item provides
     *
     * @param service the service that is provided
     */
    void setService( QObject* service );

    /**
     * Returns the service that the item provides
     *
     * @return the service
     */
    QObject* service() const;

private:
    QObject* m_service;
};

#endif
