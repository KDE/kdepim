#ifndef TESTHHRECORD_H
#define TESTHHRECORD_H
/* testhhrecord.h			KPilot
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

#include "kpilot_export.h"

#include "recordbase.h"
#include "hhrecord.h"

#include <QString>

class PilotRecordBase;
class TestRecord;

class KPILOT_EXPORT TestHHRecord : public HHRecord {
private:
	QString fId;
	QStringList fFields;
	bool fModified;
	bool fDeleted;
	bool fArchived;
	QMap<QString, QVariant> fValues;

public:
	TestHHRecord( const QStringList& fields, const QString &id );
		
	TestHHRecord( const TestHHRecord *other );
	
	TestHHRecord( const TestRecord *other );

	virtual ~TestHHRecord() {};
	
	/** METHODS FOR TESTPURPOSES **/
	
	virtual void setModified();
	
	virtual void setDeleted();
	
	void setArchived();
	
	/** IMPLEMTED VIRTUAL FUNCTIONS FROM BASECLASS **/
	
	virtual bool isArchived() const { return fArchived; }
	
	virtual const QString id() const;
	
	virtual void setId( const QString &id );

	virtual QVariant value( const QString &field ) const;

	virtual bool setValue( const QString &field, const QVariant &value );

	virtual bool isModified() const;

	virtual bool isDeleted() const;

	virtual void synced();

	virtual QString toString() const;

	virtual const QStringList fields() const;
	
	virtual Record* duplicate() const;
	
	virtual bool operator==( const Record &other ) const;

	virtual bool operator!=( const Record &other ) const;
};
#endif
