#ifndef RECORDBASE_H
#define RECORDBASE_H
/* recordbase.h			KPilot
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

#include <QtCore/QMap>

class KPILOT_EXPORT RecordBase : virtual Record {

private:
	QString fId;
	QStringList fFields;
	bool fModified;
	bool fDeleted;
	QMap<QString, QVariant> fValues;

public:
	RecordBase( const QStringList& fields, const QString &id );
	
	virtual ~RecordBase() {};
	
	/** METHODS FOR TESTPURPOSES **/
	
	virtual void setModified();
	
	virtual void setDeleted();
	
	/** IMPLEMTED VIRTUAL FUNCTIONS FROM BASECLASS **/
	
	virtual const QString id() const;
	
	virtual void setId( const QString &id );

	virtual QVariant value( const QString &field ) const;

	virtual bool setValue( const QString &field, const QVariant &value );

	virtual bool isModified() const;

	virtual bool isDeleted() const;

	virtual void synced();

	virtual QString toString() const;

	virtual const QStringList fields() const;

	//virtual bool operator==( const Record &other ) const;

	//virtual bool operator!=( const Record &other ) const;
};
#endif
