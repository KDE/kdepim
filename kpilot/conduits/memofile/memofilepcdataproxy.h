#ifndef MEMOFILEPCDATAPROXY_H
#define MEMOFILEPCDATAPROXY_H
/* MemofilePcDataproxy.h			KPilot
**
** Copyright (C) 2008 by Jason 'vanRijn' Kasper <vR@movingparts.net>
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

#include "dataproxy.h"



class MemofilePcDataProxy : public DataProxy
{
public:
	MemofilePcDataProxy( const IDMapping& mapping, QString directory );

	/**
	 * Adds the given category to the record and might do some internal things
	 * needed for category handling in the datastore.
	 *
	 * All other categories that might have been set to this record should be
	 * unchanged.
	 */
	/* virtual */ void addCategory( Record* rec, const QString& category );

	/**
	 * Sets the given category as the only category to the record and might do
	 * some internal things needed for category handling in the datastore.
	 *
	 * All other categories that might have been set to this record should be
	 * removed.
	 */
	/* virtual */ void setCategory( Record* rec, const QString& category );

};

#endif
