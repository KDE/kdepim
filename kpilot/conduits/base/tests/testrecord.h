#ifndef TESTRECORD_H
#define TESTRECORD_H
/* testrecord.h			KPilot
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

#include "kpilot_export.h"

#include "record.h"
#include "recordbase.h"

class TestHHRecord;

class KPILOT_EXPORT TestRecord : public Record {
private:
	QString fId;
	QStringList fFields;
	QStringList fCategories;
	bool fModified;
	bool fDeleted;
	QMap<QString, QVariant> fValues;
	
public:
	TestRecord( const TestHHRecord *other );
	
	TestRecord( const TestRecord *other );

	TestRecord( const QStringList& fields );
	
	TestRecord( const QStringList& fields, const QString &id );
	
	virtual ~TestRecord() {}

	virtual QString description() const;
	
	/** METHODS FOR TESTPURPOSES **/
	
	void setModified();
	
	void setDeleted();
	
	const QStringList fields() const;
	
	QVariant value( const QString &field ) const;

	bool setValue( const QString &field, const QVariant &value );
	
	TestRecord* duplicate() const;
	
	void setCategory( const QString& c );
	
	void addCategory( const QString& c );
	
	/** IMPLEMTED VIRTUAL FUNCTIONS FROM BASECLASS **/
	
	virtual const QString id() const;
	
	virtual void setId( const QString &id );
	
	virtual int categoryCount() const;
	
	virtual bool containsCategory( const QString& c ) const;

	virtual QStringList categories() const;

	virtual bool isModified() const;

	virtual bool isDeleted() const;

	virtual void synced();

	virtual QString toString() const;
};
#endif
