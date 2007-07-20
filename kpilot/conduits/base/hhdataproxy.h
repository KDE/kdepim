#ifndef HHDATAPROXY_H
#define HHDATAPROXY_H
/* hhdataproxy.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

class PilotDatabase;
class PilotRecord;
class HHRecord;

class KPILOT_EXPORT HHDataProxy : public DataProxy {
public:
	HHDataProxy( PilotDatabase *db );

	void resetSyncFlags();
	
	/**
	 * Returns wheter or not the pilot database is opened.
	 */
	virtual bool isOpen() const;

protected:
	void loadAllRecords();
	
	//virtual void readAppBlock() = 0;

	//virtual void writeAppBlock() = 0;
	
	/** THese functions must be implemented by the subclassing conduit **/
	
	/**
	 * This function creates a (subclass of) HHRecord for @p rec.
	 */
	virtual HHRecord* createHHRecord( PilotRecord *rec ) = 0;
	
	/**
	 * Commits created record @p rec to the datastore.
	 */
	virtual void commitCreate( Record *rec );
	
	/**
	 * Commits updated record @p rec to the datastore.
	 */
	virtual void commitUpdate( Record *rec );
	
	/**
	 * Undo the commit of created record @p rec to the datastore.
	 */
	virtual void commitDelete( Record *rec );

protected:
	PilotDatabase *fDatabase;
};
#endif
