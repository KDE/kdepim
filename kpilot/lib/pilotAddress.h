#ifndef _KPILOT_PILOTADDRESS_H
#define _KPILOT_PILOTADDRESS_H
/* pilotAddress.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <time.h>
#include <string.h>

#ifndef _PILOT_MACROS_H_
#include <pi-macros.h>
#endif

#ifndef _PILOT_ADDRESS_H_
#include <pi-address.h>
#endif

#ifndef _KPILOT_PILOTAPPCATEGORY_H
#include "pilotAppCategory.h"
#endif

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
 * <li> fields: Work, Home, Fax, Other, Pager, Mobile, E-mail, Main
 * <li> slots: entryPhone1, entryPhone2, entryPhone3, entryPhone4,
 * entryPhone5
 *
 * Internally in the pilot-link library, the AddressAppInfo phone
 * array stores the strings for the eight possible phone values.
 * Their English string values are :
 * <li> phone[0] = Work
 * <li> phone[1] = Home
 * <li> phone[2] = Fax
 * <li> phone[3] = Other
 * <li> phone[4] = E-mail
 * <li> phone[5] = Main
 * <li> phone[6] = Pager
 * <li> phone[7] = Mobile
 *
 * Apparently, this order is kept for all languages, just with localized
 * strings.  The implementation of the internal methods will assume
 * this order is kept. In other languages, main can replaced with
 * Corporation.
 */
class PilotAddress : public PilotAppCategory
{
public:
	enum EPhoneType {
		eWork=0, eHome, eFax, eOther, eEmail, eMain,
		ePager, eMobile
		};

	PilotAddress(struct AddressAppInfo &appInfo);
	PilotAddress(struct AddressAppInfo &appInfo, PilotRecord* rec);
	PilotAddress(const PilotAddress &copyFrom);
	PilotAddress& operator=( const PilotAddress &r );
	bool operator==(const PilotAddress &r);

	~PilotAddress();

	/** Returns a text representation of the address. If richText is true, the
	 *  text is allowed to contain Qt-HTML tags.
	 */
	virtual QString getTextRepresentation(bool richText=false);

	/** Zeros the internal address info structure, in effect clearing
	*  out all prior set values
	*/
	void reset() { memset(&fAddressInfo, 0, sizeof(struct Address)); }

	/** @param field int values associated with the enum defined in
	*  pi-address.h.
	*  The copied possible enum's are: (copied from pi-address.h on 1/12/01)
	*  enum { entryLastname, entryFirstname, entryCompany,
	*  entryPhone1, entryPhone2, entryPhone3, entryPhone4, entryPhone5,
	*  entryAddress, entryCity, entryState, entryZip, entryCountry,
	*  entryTitle, entryCustom1, entryCustom2, entryCustom3, entryCustom4,
	*  entryNote };
	*/
	void setField(int field, const QString &text);
	QString getField(int field) const;

	QString getCategoryLabel() const;

	/** If the label already exists, uses the id; if not, adds the label
	*  to the category list
	*  @return false if category labels are full
	*/
	bool setCategory(const QString &label);


	/** @param checkCustom4 flag if true, checks the entryCustom4 field
	*  for extra phone fields
	*  @return the field associated with the type
	*/
	QString getPhoneField(EPhoneType type, bool checkCustom4=true) const;

	/** @param overflowCustom is true, and entryPhone1 to entryPhone5 is full
	*  it will use entryCustom4 field to store the field
	*/
	void setPhoneField(EPhoneType type, const QString &field,
		bool overflowCustom=true);

	/**
	* Returns the (adjusted) index of the phone number
	* selected by the user to be shown in the
	* overview of addresses. Adjusted here means
	* that it's actually an index into 3..8, the fields
	* that store phone numbers, so 0 means field 3 is selected.
	* @return # between 0 and 3, where 0 is entryPhone1 and 3 is entryPhone4
	*/
	int getShownPhone() const { return fAddressInfo.showPhone; }
	void setShownPhone(EPhoneType phoneType);
	int  getPhoneLabelIndex(int index) { return fAddressInfo.phoneLabel[index]; }
	PilotRecord* pack() { return PilotAppCategory::pack(); }


	void *pack(void *, int *);
	void unpack(const void *, int = 0) { }

	static const int APP_BUFFER_SIZE;

protected:
	// Get the pointers in cases where no conversion to
	// unicode is desired.
	//
	const char *getFieldP(int field) const
		{ return fAddressInfo.entry[field]; }

private:
	void _copyAddressInfo(const struct Address &copyFrom);
	int _getNextEmptyPhoneSlot() const;

	/** @return the phone label number (0 through 8) that corresponds
	*  to the phoneType; do O(n) search though the phoneLabels array
	*/
	int _getAppPhoneLabelNum(const QString &phoneType) const;

	/** @return entryPhone1 to entryPhone5 if the appTypeNum number is
	*  found in the phoneLabel array; return -1 if not found
	*/
	int _findPhoneFieldSlot(int appTypeNum) const;

	struct AddressAppInfo &fAppInfo;
	struct Address fAddressInfo;
};




#endif
