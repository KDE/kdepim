#ifndef KEYRINGHHRECORD_H
#define KEYRINGHHRECORD_H
/* keyringhhrecord.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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
class PilotAppInfoBase;

class KeyringHHRecordBase
{
	public:
		QString account;
		QString password;
		QString notes;
		QDateTime lastChanged;
};

class KeyringHHRecord : public HHRecord
{
public:
	KeyringHHRecord( PilotRecord *rec, const QString& category, const QString &key );
	
	KeyringHHRecord( const QString &name = QString()
                 , const QString &account = QString()
                 , const QString &password = QString()
                 , const QString &notes = QString()
                 , const QString &key = QString() );
	
	QString name() const;
	
	QString account() const;
	
	QString password() const;
	
	QString notes() const;
	
	QDateTime lastChangedDate() const;
	
	void setName( const QString &name );
	
	void setAccount( const QString &account  );
	
	void setPassword( const QString &password  );
	
	void setNotes( const QString &notes  );
	
	void setLastChangedDate( const QDateTime &lastChangedDate );
	
	/** Implemented virtual methods */
	
	virtual QString description() const;

	/**
	 * Returns whether or not the current record is equal to @p other. Implementing 
	 * conduits should add support for both implementing records for this. This
	 * means that if pcRec->equal( hhRec ) is true, then also hhRec->equal( pcRec )
	 * should be true.
	 */
	virtual bool equal( const HHRecord* other ) const;

	/**
	 * Returns a string representation of the record.
	 */
	virtual QString toString() const;
	
	/** Methods added for the viewer/editor **/
	
	/**
	 * Marks the record as modified.
	 */
	void setModified();

private: // functions
	/**
	 * Returns a KeyringHHRecordBase with the unencrypted values in it.
	 */
	KeyringHHRecordBase unpack() const;
	
	/**
	 * Puts the unencrypted data in the pilotRecord.
	 */
	void pack( const KeyringHHRecordBase &data );
	
private: // members
	QString fKey;
	QString fName;
};

#endif
