#ifndef KPILOT_PILOTADDRESS_H
#define KPILOT_PILOTADDRESS_H
/* pilotAddress.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2007 by Adriaan de Groot <groot@kde.org>
**
** This is a wrapper for pilot-link's address structures.
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

// KPilot headers
#include "kpilot_export.h"
#include "pilotAppInfo.h"
#include "pilotRecord.h"

// pilot-link headers
#include <pi-macros.h>
#include <pi-address.h>

inline int _upAAI(struct AddressAppInfo *m, const unsigned char *b, size_t s)
{
	return unpack_AddressAppInfo(m,b,s);
}

inline int _pAAI(const struct AddressAppInfo *m, unsigned char *b, size_t s)
{
	return pack_AddressAppInfo(m,b,s);
}

/** Interpreted form of the AppInfo block in the address database. */
typedef PilotAppInfo<
	AddressAppInfo,
	_upAAI,
	_pAAI> PilotAddressInfo_;

/** This class exists @em only to clear up the type mess that
*   is the field-numbers-and-indexes for phone numbers in the
*   handheld records. The standard address record has 19 fields,
*   five of which are phone fields. Those are fields 3..7 and they
*   are referred to as fields Phone1 .. Phone5. Sometimes we
*   need to act as if the phone field numbers are indeed the field
*   numbers (3..7) and sometimes we need to use those same field
*   numbers to index into a C array (0 based!) so then we map
*   field number 3 (Phone1) to a 0 index.
*
*   Also handles iteration nicely.
*
*   A phone slot value may be invalid. If so, operations on it will
*   fail (yielding invalid again) and isValid() will return @c false.
*/
class KPILOT_EXPORT PhoneSlot
{
friend class PilotAddress;
protected:
	/** Constructor. Use the specified value for the phone slot.
	*   @p v is a field number (3..8).
	*/
	explicit PhoneSlot( const int v );

	/** Assignment operator. Set the value of the slot to
	*   the specified value @p v . This may yield an invalid
	*   phone slot.
	*/
	const PhoneSlot &operator=(const int &v);

	/** Map the slot to an offset (for use in finding the phone type
	*   for a given slot).
	* @return Offset of this slot within the phone fields.
	*/
	unsigned int toOffset() const;

	/** Map the slot to a field number. */
	unsigned int toField() const;

public:
	static const int invalid = -1; ///< Value for invalid slots. */

	/** Constructor. The slot is invalid. */
	PhoneSlot()
	{
		i = invalid;
	}

	/** Comparison operator. */
	bool operator ==( const PhoneSlot &v ) const
	{
		return v.i == i;
	}

	/** Iterator operation. Go to the next slot (or invalid when
	*   the range runs out).
	*/
	const PhoneSlot &operator++();

	/** Begin value of an iteration through the phone slots. */
	static const PhoneSlot begin();

	/** When the slot range runs out (past entryPhone5) it
	*   is invalid, so the end compares with that.
	*/
	static const PhoneSlot end();

	/** Valid slots are entryPhone1 (3) through entryPhone5 (7).
	*   @return @c true if the slot is valid.
	*/
	bool isValid() const
	{
		return (entryPhone1 <= i) && (i <= entryPhone5);
	}

	operator QString() const;
private:
	int i;
} ;


class KPILOT_EXPORT PilotAddressInfo : public PilotAddressInfo_
{
public:
	PilotAddressInfo(PilotDatabase *d) : PilotAddressInfo_(d)
	{
	}

	/** This resets the entire AppInfo block to one as it would be
	*   in an English-language handheld, with 3 categories and
	*   default field labels for everything.
	*/
	void resetToDefault();

	enum EPhoneType {
		eWork=0,
		eHome,
		eFax,
		eOther,
		eEmail,
		eMain,
		ePager,
		eMobile,
		eNone=-1
	} ;

	QString phoneLabel(EPhoneType i) const;
} ;

/** @brief A wrapper class around the Address struct provided by pi-address.h
 *
 * This class allows the user to set and get address field values.
 * For everything but phone fields, the user can simply pass the
 * the pi-address enum for the index for setField() and getField() such
 * as entryLastname.
 *
 * Phone fields are a bit trickier.  The structure allows for 8 possible
 * phone fields with 5 possible slots.  That means there could be three
 * fields that don't have available storage. The setPhoneField() method
 * will attempt to store the extra fields in a custom field if there
 * is an overflow.
 *
 * There are eight possible fields for 5 view slots:
 * - fields: Work, Home, Fax, Other, Pager, Mobile, E-mail, Main
 * - slots: entryPhone1, entryPhone2, entryPhone3, entryPhone4, entryPhone5
 *
 * Internally in the pilot-link library, the AddressAppInfo phone
 * array stores the strings for the eight possible phone values.
 * Their English string values are :
 * - phone[0] = Work
 * - phone[1] = Home
 * - phone[2] = Fax
 * - phone[3] = Other
 * - phone[4] = E-mail
 * - phone[5] = Main
 * - phone[6] = Pager
 * - phone[7] = Mobile
 *
 * Apparently, this order is kept for all languages, just with localized
 * strings.  The implementation of the internal methods will assume
 * this order is kept. In other languages, main can replaced with
 * Corporation.
 */
class KPILOT_EXPORT PilotAddress : public PilotRecordBase
{
public:
	PilotAddress(PilotRecord *rec = 0L);
	PilotAddress(const PilotAddress &copyFrom);
	PilotAddress& operator=( const PilotAddress &r );
	bool operator==(const PilotAddress &r);

	virtual ~PilotAddress();

	/** Returns a text representation of the address. If @p richText is true, the
	 *  text will be formatted with Qt-HTML tags. The AppInfo structure @p info
	 *  is used to figure out the phone labels; if it is NULL then bogus labels are
	 *  used to identify phone types.
	 */
	QString getTextRepresentation(const PilotAddressInfo *info, Qt::TextFormat richText) const;

	/**
	*   @param text set the field value
	*   @param field int values associated with the enum defined in
	*  pi-address.h.
	*  The copied possible enum's are: (copied from pi-address.h on 1/12/01)
	*  enum { entryLastname, entryFirstname, entryCompany,
	*  entryPhone1, entryPhone2, entryPhone3, entryPhone4, entryPhone5,
	*  entryAddress, entryCity, entryState, entryZip, entryCountry,
	*  entryTitle, entryCustom1, entryCustom2, entryCustom3, entryCustom4,
	*  entryNote };
	*/
	void setField(int field, const QString &text);
	/** Set a field @p i to a given text value. Uses the phone slots only. */
	void setField(const PhoneSlot &i, const QString &t)
	{
		if (i.isValid())
		{
			setField(i.toField(),t);
		}
	}

	/** Returns the text value of a given field @p field (or QString()
	*   if there is no such field).
	*/
	QString getField(int field) const;
	/** Returns the value of the phone field @p i . */
	QString getField(const PhoneSlot &i) const
	{
		return i.isValid() ? getField(i.toField()) : QString();
	}

	/**
	*   Return list of all email addresses.  This will search through our "phone"
	*   fields and will return only those which are e-mail addresses.
	*/
	QStringList getEmails() const;
	void setEmails(const QStringList &emails);

	enum PhoneHandlingFlags
	{
		NoFlags=0, ///< No special handling
		Replace    ///< Replace existing entries of same type
	} ;

	/**
	*  @param type is the type of phone
	*  @param checkCustom4 flag if true, checks the entryCustom4 field
	*  for extra phone fields
	*  @return the field associated with the type
	*/
	QString getPhoneField(PilotAddressInfo::EPhoneType type) const;

	/**
	*  @param type is the type of phone
	*  @param field is value to store
	*  @param overflowCustom is true, and entryPhone1 to entryPhone5 is full
	*  it will use entryCustom4 field to store the field
	*  @param overwriteExisting is true, it will overwrite an existing record-type
	*  with the field, else it will always search for the first available slot
	 * @return index of the field that this information was set to
	*/
	PhoneSlot setPhoneField(PilotAddressInfo::EPhoneType type, const QString &value, PhoneHandlingFlags flags);

	/**
	* Returns the slot of the phone number
	* selected by the user to be shown in the
	* overview of addresses.
	*
	* @return Slot of phone entry (between entryPhone1 and entryPhone5)
	*/
	PhoneSlot getShownPhone() const;

	/**
	* Set the shown phone (the one preferred by the user for display
	* on the handheld's overview page) to the @em type (not index)
	* indicated. Looks through the phone entries of this record to
	* find the first one one of this type.
	*
	* @return Slot of phone entry.
	*
	* @note Sets the shown phone to the first entry if no field of
	*       type @p phoneType can be found @em and no Home phone
	*       field (the fallback) can be found either.
	*/
	PhoneSlot setShownPhone(PilotAddressInfo::EPhoneType phoneType);

	/**
	* Set the shown phone (the one preferred by the user for display
	* on the handheld's overview page) to the given @p slot .
	*
	* @return @p v
	*/
	const PhoneSlot &setShownPhone(const PhoneSlot &v);

	/** Get the phone type (label) for a given field @p field
	*   in the record. The @p field must be within the
	*   phone range (entryPhone1 .. entryPhone5).
	*
	* @return Phone type for phone field @p field .
	* @return @c eNone (fake phone type) if @p field is invalid.
	*/
	PilotAddressInfo::EPhoneType getPhoneType(const PhoneSlot &field) const;

	PilotRecord *pack() const;

	const struct Address *address() const { return &fAddressInfo; } ;


protected:
	// Get the pointers in cases where no conversion to
	// unicode is desired.
	//
	const char *getFieldP(int field) const
	{
		return fAddressInfo.entry[field];
	}

private:
	void _copyAddressInfo(const struct Address &copyFrom);
	PhoneSlot _getNextEmptyPhoneSlot() const;

	/** @return entryPhone1 to entryPhone5 if the appTypeNum number is
	*  found in the phoneLabel array; return -1 if not found
	*/
	PhoneSlot _findPhoneFieldSlot(PilotAddressInfo::EPhoneType t) const;

	struct Address fAddressInfo;
};




#endif
