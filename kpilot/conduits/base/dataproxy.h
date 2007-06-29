#ifndef DATAPROXY_H
#define DATAPROXY_H
/* dataproxy.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
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

#include "record.h"

class CUDCounter;

class DataProxy {
protected:
	CUDCounter *fCounter;

public:
	DataProxy();
	
	virtual ~DataProxy();

	enum Mode {
		ALL=1,
		MODIFIED=2
	};

	/**
		* Adds the record to the database and returns the internal id for the added
		* record.
		*/
	QVariant addRecord();

	void deleteRecord();

	void editRecord();

	void syncFinished();

	/**
	 * Sets the mode which is used to iterate over the loaded records. It defaults
	 * to iterate over all records.
	 */
	void setIterateMode( const Mode m = ALL );

	virtual void commitChanges() = 0;

	/**
		* Looks for a matching record. Should return 0 if there is no match.
		*/
	virtual Record* lookForMatch() = 0;

	virtual void loadAllRecords() = 0;

	/**
		* Dependend on the iterateMode it should give the next record, the next
		* modified record or 0 if there are no more records to iterate over.
		*/
	virtual Record* nextRecord() = 0;

	virtual Record* readRecordById() = 0;

	/**
		* Returns QString::Null if everything is ok, or a message explaining the
		* cause (invalid count, or volatility exceeds configured threshold).
		*/
	virtual QString volatilityMessage() = 0;


protected:
	Mode fMode;
	Record *fRecords;
};
#endif
