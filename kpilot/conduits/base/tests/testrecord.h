#ifndef TESTRECORD_H
#define TESTRECORD_H
/* record.h			KPilot
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

#include <QVariant>
#include <QStringList>

#include "record.h"

class KPILOT_EXPORT TestRecord : public Record {
public:
	TestRecord();

	TestRecord( const QString & id );
	
	virtual ~TestRecord();
	
	virtual Record* duplicate();

	virtual bool setValue( const QString &field, const QVariant &value );

	virtual bool isModified() const;
	
	virtual const QStringList fields() const;
	
	virtual QString toString() const;

	// For test purposes.
	void setUnmodified();
	
private:
	bool fModified;
	QStringList fFields;
};
#endif
