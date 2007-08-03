#ifndef KEYRINGHHRECORD_H
#define KEYRINGHHRECORD_H
/* keyringhhdataproxy.h			KPilot
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

#include "hhrecord.h"

#include <QtCrypto>

class PilotRecord;

class KeyringHHRecord : public HHRecord
{
public:
	KeyringHHRecord( PilotRecord *rec, const QString &key );
	
	virtual QString name() const;
	
	virtual QString account() const;
	
	/** Implemented virtual methods */
	
	/**
	 * Returns wheter or not the current record is equal to @p other. Implementing 
	 * conduits should add support for both implementing records for this. This
	 * means that if pcRec->equal( hhRec ) is true, then also hhRec->equal( pcRec )
	 * should be true.
	 */
	virtual bool equal( const Record* other ) const;

	/**
	 * Returns a string representation of the record.
	 */
	virtual QString toString() const;

private: // functions
	enum Field {
		eAccount = 0,
		ePassword,
		eNotes,
		eLastChangeTime
	};
	
	QVariant getEncryptedField( const Field f ) const;
	
private: // members
	QString fKey;
	QString fName;
};

#endif
