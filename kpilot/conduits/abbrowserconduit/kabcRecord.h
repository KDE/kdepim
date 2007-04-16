#ifndef _KPILOT_KABCRECORD_H
#define _KPILOT_KABCRECORD_H
/* KPilot
**
** Copyright (C) 2000,2001 by Dan Pilone
** Copyright (C) 2002-2003 by Reinhold Kainhofer
** Copyright (C) 2007 by Adriaan de Groot <groot@kde.org>
**
** The abbrowser conduit copies addresses from the Pilot's address book to
** the KDE addressbook maintained via the kabc library. This file
** deals with the actual copying of HH addresses to KABC addresses
** and back again.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include <qvaluevector.h>

#include <kabc/phonenumber.h>

#include "pilotAddress.h"

#define SYNCNONE 0
#define SYNCMOD 1
#define SYNCDEL 3

namespace KABCSync
{
	// This duplicates values from the config settings,
	// and must be kept in sync if new settings are added
	// -- there are compile time checks for this in the
	// abbrowser conduit code.
	enum MappingForOtherPhone {
		eOtherPhone=0,
		eAssistant,
		eBusinessFax,
		eCarPhone,
		eEmail2,
		eHomeFax,
		eTelex,
		eTTYTTDPhone
	} ;

	enum MappingForCustomField {
		eCustomField=0,
		eCustomBirthdate,
		eCustomURL,
		eCustomIM
	} ;

	class Settings
	{
	public:
		Settings();
		QString dateFormat() const
		{
			return fDateFormat;
		}
		void setDateFormat(const QString& s)
		{
			fDateFormat = s;
		}

		const QValueVector<int> &customMapping() const
		{
			return fCustomMapping;
		}
		void setCustomMapping(const QValueVector<int> &v)
		{
			if (v.count()==4)
			{
				fCustomMapping = v;
			}
		}
		int custom(int index) const
		{
			if ( (index<0) || (index>3) )
			{
				return 0;
			}
			else
			{
				return fCustomMapping[index];
			}
		}

		int fieldForOtherPhone() const
		{
			return fOtherPhone;
		}
		void setFieldForOtherPhone(int v)
		{
			fOtherPhone = v;
		}

		bool preferHome() const
		{
			return fPreferHome;
		}
		void setPreferHome(bool v)
		{
			fPreferHome = v;
		}

		int faxTypeOnPC() const
		{
			return fFaxTypeOnPC;
		}
		void setFaxTypeOnPC(int v)
		{
			fFaxTypeOnPC = v;
		}
	private:
		QString fDateFormat;
		QValueVector<int> fCustomMapping;
		int fOtherPhone;
		bool fPreferHome;
		int fFaxTypeOnPC;
	} ;


	/** Return a list of all the phone numbers (max. 8) set in this
	*   handheld entry @p a . Email entries are ignored.
	*/
	KABC::PhoneNumber::List getPhoneNumbers(const PilotAddress &a);

	/** Set the phone numbers from @p list in the handheld entry
	*   @p a (with info block @p info providing the mapping of category
	*   names and some other fiddly stuff) as far as possible.
	*   @em No overflow handling is done at all. If the desktop has
	*   more than 5 phone entries, the remainder are dropped.
	*/
	void setPhoneNumbers(const PilotAddressInfo &info,
		PilotAddress &a,
		const KABC::PhoneNumber::List &list);

	/** Given a list of category names from the KDE side (e.g. attached
	*   to a PC-based Addressee) @p categorynames , look for the
	*   category @em best matching the category @p category
	*   in the appinfo block @p info . Here, best is defined as follows:
	*     - if the name of category @p category is in the list, use it
	*     - otherwise use the first category from the list that is a valid
	*       category on the handheld.
	*     - use Pilot::Unfiled if none match.
	*
	*   @return Category index that best matches.
	*   @return Pilot::Unfiled if no best match.
	*/
	unsigned int bestMatchedCategory(const QStringList &categorynames,
		const PilotAddressInfo &info,
		unsigned int category);

	/** As above, but return the name of the category. */
	inline QString bestMatchedCategoryName(const QStringList &categorynames,
		const PilotAddressInfo &info,
		unsigned int category)
	{
		return info.categoryName(
			bestMatchedCategory(categorynames, info, category));
	}

	/** Give the addressee @p abEntry the category @p cat (leaving
	*   existing category assignments intact).
	*/
	void setCategory(KABC::Addressee &abEntry, const QString &cat);

	/* These are string identifiers used for custom properties in the addressees,
	*  used to store KPilot-specific settings.
	*/
	const QString appString=CSL1("KPILOT"); ///< Identifier for the application
	const QString flagString=CSL1("Flag"); ///< Flags: synced or not
	const QString idString=CSL1("RecordID"); ///< Record ID on HH for this addressee


	/** Get the string value for HH custom field @p index (0..3) from the addressee
	*   @p abEntry . Which @em actual field this is depends on the mapping
	*   of custom HH fields to PC fields. This mapping is given by the @p customMapping
	*   which may be created from the conduit settings or by hand. Since one of the
	*   possible actual fields is "birthday," which needs formatting, use the date format
	*   string @p dateFormat. If this is empty, use the locale setting.
	*
	*   @return String value for HH custom field @p index
	*   @return Null QString on error (is also a valid return value)
	*/
	QString getFieldForHHCustom(
		unsigned int index,
		const KABC::Addressee &abEntry,
		const Settings &settings);

	/** Set a field of the PC @p abEntry address from the custom HH field.
	*   Use value @p value . The value comes from custom field @p index
	*   using the interpretation of custom fields @p customMapping . Because
	*   one of the interpretations includes the birthday, use the date format
	*   @p dateFormat ; if empty, use the local format when setting dates from the HH.
	*/
	void setFieldFromHHCustom(
		const unsigned int index,
		KABC::Addressee &abEntry,
		const QString &value,
		const Settings &settings);

	/** The HH has a phone type "other" which may be mapped to any one of
	*   several PC side phone numbers. Return the right one depending in the mapping.
	*
	* @note @p mappingForOther should come from AbbrowserSettings::pilotOther()
	*/
	QString getFieldForHHOtherPhone(const KABC::Addressee &abEntry, const Settings &s);

	/** The HH has a phone type "other" which may be mapped to any one
	*   of several PC side phone numbers. Store the number @p nr in the
	*   PC side phone field indicated by @p mappingForOther .
	*
	* @note @p mappingForOther should come from AbbrowserSettings::pilotOther()
	*/
	void setFieldFromHHOtherPhone(KABC::Addressee &abEntry, const QString &nr, const Settings &s);

	/** Returns the address portion of an addressee. Since the HH can only store
	*   one address, we return the preferred address (if the Addressee @p abEntry
	*   has one) and then either home or business depending on @p preferHome
	*   and if that doesn't exist, the other one and if @em that doesn't exist,
	*   return the preferred home or work address if it exists.
	*/
	KABC::Address getAddress(const KABC::Addressee &abEntry, const Settings &);

	/** Set the address fields of the HH record from the @p abAddress . */
	void setAddress(PilotAddress &toPilotAddr, const KABC::Address &abAddress);

	bool isArchived(const KABC::Addressee &);
	void makeArchived(KABC::Addressee &);

	void copy(PilotAddress &toPilotAddr,
		const KABC::Addressee &fromAbEntry,
		const PilotAddressInfo &appInfo,
		const Settings &syncSettings);
	void copy(KABC::Addressee &toAbEntry,
		const PilotAddress &fromPiAddr,
		const PilotAddressInfo &appInfo,
		const Settings &syncSettings);

	void showAddressee(const KABC::Addressee &);
}

#endif

