#ifndef HHRECORD_H
#define HHRECORD_H
/* hhrecord.h			KPilot
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

class PilotRecord;

class KPILOT_EXPORT HHRecord : public Record {

protected:
	PilotRecord *fRecord;
	QString fCategory;
	QString fTempId;

public:
	HHRecord( PilotRecord *record, const QString &category );

	virtual ~HHRecord();
	
	/** HHRecord methods */
	
	PilotRecord* pilotRecord() const;
	
	/**
	 * Compares @p pcRecord with @p hhRec and returns true if they are considered
	 * equal.
	 */
	virtual bool equal( const HHRecord* other ) const  = 0;
	
	/**
	 * Return whether or not this record is marked for deletion and archiving.
	 */
	virtual bool isArchived() const;
	
	/**
	 * Marks the record for deletion and for archiving.
	 */
	virtual void setArchived();
	
	/** Record methods */
	
	virtual const QString id() const;
	
	/**
	 * Sets the id of this record to @p id;
	 */
	virtual void setId( const QString &id );
	
	/**
	 * Returns the number of categories that is set for this record.
	 */
	virtual int categoryCount() const;
	
	/**
	 * Returns whether or not the given category is set for this record.
	 */
	virtual bool containsCategory( const QString& category ) const;

	/**
	 * Returns the list of categories set for this record.
	 */
	virtual QStringList categories() const;

	/**
	 * Returns the category of this record.
	 */
	virtual QString category() const;
	
	/**
	 * Sets the category and the id as stored in the appinfo block of the database
	 * for this record.
	 */
	virtual void setCategory( int id, const QString& category );

	/**
	 * Returns true if the record is modified and/or if it's marked as deleted.
	 */
	virtual bool isModified() const;
	
	/**
	 * Returns true when this record is marked for deletion.
	 */
	virtual bool isDeleted() const;
	
	/**
	 * Notify the record that syncing is finished so that it can reset flags.
	 * After calling this function Record::isModified() should return false.
	 */
	virtual void synced();
	
	/**
	 * Returns a string representation of the record.
	 */
	virtual QString toString() const;

};
#endif
