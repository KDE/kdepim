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
class PilotAppInfoBase;

class KPILOT_EXPORT HHRecord : public Record {

protected:
	PilotRecord *fRecord;
	
public:
	HHRecord( PilotRecord *record, PilotAppInfoBase *appInfo );

	virtual ~HHRecord();
	
	/**
	 * Sets the (a handheld record only has one) category to this record. It
	 * should be mentioned that only the label is set. This gets associated with
	 * the category id set in fPilotRecord. The caller should make sure that it is
	 * the label that can be found in the appinfo block of the database which
	 * contains this record. If you want to change the category use 
	 * setCategory( int id, const QString &label).
	 */
	virtual void setCategoryNames( const QStringList &categories );
	
	/**
	 * Returns the catogories of this record.
	 */
	virtual QStringList categoryNames() const;
	
	/** HHRecord methods */
	
	PilotRecord* pilotRecord() const;
	
	PilotAppInfoBase *appInfo() const;
	
	/**
	 * Changes the label and also the category id in fPilotRecord. The caller
	 * should make sure that the label matches the label in the appinfo block of
	 * the database which contains this record.
	 */
	void setCategory( int id, const QString label );
	
	/**
	 * Added for convenience. Returns the same as categories().first().
	 */
	QString categoryName() const;
	
	/**
	 * Returns the id of the category which is set in fRecord.
	 */
	int categoryId() const;
	
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

private:
	QString fCategory;
	PilotAppInfoBase *fAppInfo;
};
#endif
