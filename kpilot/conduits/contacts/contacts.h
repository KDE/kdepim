#ifndef CONTACTS_H
#define CONTACTS_H
/* contacts.h			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2008 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "recordconduit.h"

#include <QtCore/QVector>

#include <kabc/addressee.h>
#include <kabc/phonenumber.h>

class PilotAddress;
class PilotAddressInfo;
class Settings;
namespace KABC
{
	class Addressee;
}

class Contacts : public RecordConduit
{
private:
	Settings* fSettings;
	PilotAddressInfo* fAddressInfo;

public:
	Contacts( KPilotLink *o, const QVariantList &a = QVariantList() );
	
	~Contacts();

	virtual void loadSettings();
	
	virtual bool initDataProxies();
	
	/**
	 * Compares @p pcRecord with @p hhRec and returns true if they are considered
	 * equal.
	 */
	virtual bool equal( const Record *pcRec, const HHRecord *hhRec ) const;
	
	/**
	 * Creates a new Record object with the same data as @p hhRec.
	 */
	virtual Record* createPCRecord( const HHRecord *hhRec );
	
	/**
	 * Creates a new HHRecord object with the same data as @p pcRec.
	 */
	virtual HHRecord* createHHRecord( const Record *pcRec );
	
	/**
	 * Copies the field values of @p from to @p to. The method should only touch
	 * data that can be synced between the two records and leave the rest of the
	 * records data unchanged. After calling this method
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	virtual void _copy( const Record *from, HHRecord *to );
	
	/**
	 * Copies the field values of @p from to @p to. The method should only touch
	 * data that can be synced between the two records and leave the rest of the
	 * records data unchanged. After calling this method
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	virtual void _copy( const HHRecord *from, Record *to  );
	
	/**
	 * This method is called when the conduit is run in Test Mode. The 
	 * implementing class can do whatever it wants to do for test purposes.
	 */
	virtual void test() {}

protected:
	/**
	 * Given a list of category names from the KDE side (e.g. attached
	 * to a PC-based Addressee) @p categorynames , look for the
	 * category @em best matching the category @p category
	 * in the appinfo block @p info . Here, best is defined as follows:
	 * - if the name of category @p category is in the list, use it
	 * - otherwise use the first category from the list that is a valid
	 *   category on the handheld.
	 * - use Pilot::Unfiled if none match.
	 *
	 * @return Category index that best matches.
	 * @return Pilot::Unfiled if no best match.
	 */
	unsigned int bestMatchedCategory(const QStringList &categorynames,
		unsigned int category ) const;
	
	/**
	 * As above, but return the name of the category.
	 */
	QString bestMatchedCategoryName( const QStringList &categorynames
		, unsigned int category ) const;
	
	/**
	 * Returns the address portion of an addressee. Since the HH can only store
	 * one address, we return the preferred address (if the Addressee @p abEntry
	 * has one) and then either home or business depending on @p preferHome
	 * and if that doesn't exist, the other one and if @em that doesn't exist,
	 * return the preferred home or work address if it exists.
	 */
	KABC::Address getAddress( const KABC::Addressee &abEntry ) const;

	/**
	 * Get the string value for HH custom field @p index (0..3) from the addressee
	 * @p abEntry . Which @em actual field this is depends on the mapping
	 * of custom HH fields to PC fields. This mapping is given by the @p customMapping
	 * which may be created from the conduit settings or by hand. Since one of the
	 * possible actual fields is "birthday," which needs formatting, use the date format
	 * string @p dateFormat. If this is empty, use the locale setting.
	 *
	 * @return String value for HH custom field @p index
	 * @return Null QString on error (is also a valid return value)
	 */
	QString getFieldForHHCustom( unsigned int index, const KABC::Addressee &abEntry ) const;

	/**
	 * The HH has a phone type "other" which may be mapped to any one of
	 * several PC side phone numbers. Return the right one depending in the mapping.
	 *
	 * @note @p mappingForOther should come from AbbrowserSettings::pilotOther()
	 */
	QString getFieldForHHOtherPhone( const KABC::Addressee &abEntry ) const;

	/**
	 * Return a list of all the phone numbers (max. 8) set in this
	 * handheld entry @p a . Email entries are ignored.
	 */
	KABC::PhoneNumber::List getPhoneNumbers( const PilotAddress &a ) const;
	
	/**
	 * Set the address fields of the HH record from the @p abAddress .
	 */
	void setAddress(PilotAddress &toPilotAddr, const KABC::Address &abAddress) const;
	
	/**
	 * Set a field of the PC @p abEntry address from the custom HH field.
	 * Use value @p value . The value comes from custom field @p index
	 * using the interpretation of custom fields @p customMapping . Because
	 * one of the interpretations includes the birthday, use the date format
	 * @p dateFormat ; if empty, use the local format when setting dates from the HH.
	 */
	void setFieldFromHHCustom(
		const unsigned int index,
		KABC::Addressee &abEntry,
		const QString &value );

	/**
	 * The HH has a phone type "other" which may be mapped to any one
	 * of several PC side phone numbers. Store the number @p nr in the
	 * PC side phone field indicated by @p mappingForOther .
	 *
	 * @note @p mappingForOther should come from ContactsSettings::pilotOther()
	 */
	void setFieldFromHHOtherPhone( KABC::Addressee &abEntry, const QString &nr );

	/**
	 * Set the phone numbers from @p list in the handheld entry
	 * @p a (with info block @p info providing the mapping of category
	 * names and some other fiddly stuff) as far as possible.
	 * @em No overflow handling is done at all. If the desktop has
	 * more than 5 phone entries, the remainder are dropped.
	 */
	void setPhoneNumbers( PilotAddress &a, const KABC::PhoneNumber::List &list );
};

#endif
