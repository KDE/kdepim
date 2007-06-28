#ifndef IDMAPPING_H
#define IDMAPPING_H

/* IDMapping.h			KPilot
**
** Copyright (C) 2004-2007 by Bertjan Broeksema
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


class IDMapping {
  public:
    IDMapping();

    /**
     * Validates the mapping file with given dataproxy. The mapping is considered valid if:
     * 1. The number of mappings matches the number of records in the dataproxy.
     * 2. Every record that is in the backup database has a mapping.
     */
    bool isValid();

    /**
     * Returns the pc record ID for given handheld record. Returns QString::Null if no mapping is found.
     */
    QString pcRecordId();

    /**
     * Returns the id for the HH record which is mapped to the given pc record or 0 if there is no mapping.
     */
    recordid_t hhRecordId();

    void setLastSyncedDate();

    void setLastSyncedPC();

    void save();

     setPCId();

     setHHId();

    /**
     * Creates a mapping for given records with id's.
     */
    void map();

    bool contains();

};
#endif
