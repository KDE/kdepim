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

#include "options.h"

#include <qregexp.h>

#include <kglobal.h>
#include <kabc/addressee.h>

#include "kabcRecord.h"

/**
 * Okay, this is so that we can map the Pilot phone types to Phone Number
 * types. Email addresses are NOT included in this map, and are handled
 * separately (not in PhoneNumber at all). The Pilot has 8 different kinds
 * of phone numbers (which may be *labeled* however you like). These
 * need to be mapped to the things that KABC::PhoneNumber handles.
 *
 * From KABC::PhoneNumber
 *		enum Types { Home = 1, Work = 2, Msg = 4, Pref = 8, Voice = 16, Fax = 32,
 *				Cell = 64, Video = 128, Bbs = 256, Modem = 512, Car = 1024,
 *				Isdn = 2048, Pcs = 4096, Pager = 8192 };
 *
 *
 * From PilotAddress:
 * enum EPhoneType {
 *		eWork=0, eHome, eFax, eOther, eEmail, eMain,
 *		ePager, eMobile
 *		};
 *
 * This array must have as many elements as PilotAddress::PhoneType
 * and its elements must be KABC::PhoneNumber::Types.
 */

static KABC::PhoneNumber::Types pilotToPhoneMap[8] = {
	KABC::PhoneNumber::Work,  // eWork
	KABC::PhoneNumber::Home,  // eHome,
	KABC::PhoneNumber::Fax,   // eFax,
	(KABC::PhoneNumber::Types)0, // eOther -> wasn't mapped properly,
	(KABC::PhoneNumber::Types)0, // eEmail -> shouldn't occur,
	KABC::PhoneNumber::Home,  // eMain
	KABC::PhoneNumber::Pager, // ePager,
	KABC::PhoneNumber::Cell   // eMobile
} ;

KABC::PhoneNumber::List KABCSync::getPhoneNumbers(const PilotAddress &a)
{
	FUNCTIONSETUP;

	KABC::PhoneNumber::List list;
	QString test;

	PhoneSlot shownPhone = a.getShownPhone();

	DEBUGKPILOT << fname << ": preferred pilot index is: ["
		<< shownPhone << "], preferred phone number is: ["
		<< a.getField(shownPhone) << "]" << endl;

	for (PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i)
	{
		// skip email entries
		if ( a.getPhoneType(i) == PilotAddressInfo::eEmail )
		{
			continue;
		}

		test = a.getField(i);
		// only look at this if the field is populated
		if (test.isEmpty())
		{
			continue;
		}

		int phoneType = pilotToPhoneMap[a.getPhoneType(i)];

		// only populate a PhoneNumber if we have a corresponding type
		if (phoneType >=0)
		{
			// if this is the preferred phone number, set it as such
			if (shownPhone == i)
			{
				phoneType |= KABC::PhoneNumber::Pref;
				DEBUGKPILOT << fname << ": found preferred pilot index: ["
					<< i << "], text: [" << test << "]" << endl;
			}
			KABC::PhoneNumber ph(test, phoneType);
			list.append(ph);
		}
		else
		{
			DEBUGKPILOT << fname << ": whoopsie.  pilot phone number: ["
				<< test << "], index: [" << i << "], type: ["
				<< phoneType << "], has no corresponding PhoneNumber type." << endl;
		}
	}

	DEBUGKPILOT << fname << ": returning: ["
		<< list.count() << "] phone numbers." << endl;

	return list;
}

void KABCSync::setPhoneNumbers(const PilotAddressInfo &info,
	PilotAddress &a,
	const KABC::PhoneNumber::List &list)
{
	FUNCTIONSETUP;
	QString test;

	// clear all phone numbers (not e-mails) first
	for ( PhoneSlot i = PhoneSlot::begin(); i.isValid() ; ++i )
	{
		PilotAddressInfo::EPhoneType ind = a.getPhoneType( i );
		if (ind != PilotAddressInfo::eEmail)
		{
			a.setField(i, QString());
		}
	}

	// now iterate through the list and for each PhoneNumber in the list,
	// iterate through our phone types using our map and set the first one
	// we find as the type of address for the Pilot
	for(KABC::PhoneNumber::List::ConstIterator listIter = list.begin();
		   listIter != list.end(); ++listIter)
	{
		KABC::PhoneNumber phone = *listIter;

		PilotAddressInfo::EPhoneType phoneType = PilotAddressInfo::eHome;

		for ( int pilotPhoneType = PilotAddressInfo::eWork;
			pilotPhoneType <= PilotAddressInfo::eMobile;
			++pilotPhoneType)
		{
			int phoneKey = pilotToPhoneMap[pilotPhoneType];
			if ( phone.type() & phoneKey)
			{
				DEBUGKPILOT << fname << ": found pilot type: ["
					<< pilotPhoneType << "] ("
					<< info.phoneLabel( (PilotAddressInfo::EPhoneType)pilotPhoneType)
					<< ") for PhoneNumber: ["
					<< phone.number() << "]" << endl;

				phoneType = (PilotAddressInfo::EPhoneType) pilotPhoneType;
				break;
			}
		}
		PhoneSlot fieldSlot =
			a.setPhoneField(phoneType, phone.number(), PilotAddress::NoFlags);

		// if this is the preferred phone number, then set it as such
		if (fieldSlot.isValid() && (phone.type() & KABC::PhoneNumber::Pref))
		{
			DEBUGKPILOT << fname << ": found preferred PhoneNumber. "
				<< "setting showPhone to index: ["
				<< fieldSlot << "], PhoneNumber: ["
				<< phone.number() << "]" << endl;
			a.setShownPhone( fieldSlot );
		}

		if (!fieldSlot.isValid())
		{
			DEBUGKPILOT << fname << ": Phone listing overflowed." << endl;
		}
	}

	DEBUGKPILOT << fname << ": Pilot's showPhone now: ["
		<< a.getShownPhone() << "]." << endl;

	// after setting the numbers, make sure that something sensible is set as the
	// shownPhone on the Pilot if nothing is yet...
	QString pref = a.getField(a.getShownPhone());
	if (!a.getShownPhone().isValid() || pref.isEmpty())
	{
		DEBUGKPILOT << fname << ": Pilot's showPhone: ["
			<< a.getShownPhone()
			<< "] not properly set to a default."
			<< endl;

		for (PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i)
		{
			pref = a.getField(i);
			if (!pref.isEmpty())
			{
				a.setShownPhone( i );
				DEBUGKPILOT << fname << ": Pilot's showPhone now: ["
					<< a.getShownPhone() 
					<< "], and that's final." << endl;
				break;
			}
		}
	}
}

unsigned int KABCSync::bestMatchedCategory(const QStringList &pccategories,
	const PilotAddressInfo &info,
	unsigned int hhcategory)
{
	FUNCTIONSETUP;
	// No categories in list, must be unfiled
	if (pccategories.size()<1)
	{
		return Pilot::Unfiled;
	}

	// See if the suggested hhcategory is in the list, and if
	// so that is the best match.
	if (Pilot::validCategory(hhcategory) &&
		pccategories.contains(info.categoryName(hhcategory)))
	{
		return hhcategory;
	}

	// Look for the first category from the list which is available on
	// the handheld as well.
	for(QStringList::ConstIterator it = pccategories.begin(); it != pccategories.end(); ++it)
	{
		// Do not map unknown to unfiled when looking for category
		int c = info.findCategory( *it, false );
		if ( c >= 0)
		{
			Q_ASSERT(Pilot::validCategory(c));
			return c;
		}
	}

	// didn't find anything. return null
	return Pilot::Unfiled;
}

void KABCSync::setCategory(KABC::Addressee & abEntry, const QString &cat)
{
	if ( (!cat.isEmpty()))
	{
		abEntry.insertCategory(cat);
	}
}


QString KABCSync::getFieldForHHCustom(
	const unsigned int index,
	const KABC::Addressee &abEntry,
	const KABCSync::Settings &settings)
{
	FUNCTIONSETUPL(4);

	QString retval;

	if (index>3)
	{
		WARNINGKPILOT << "Bad index number " << index << endl;
		retval = QString();
	}
	if (settings.customMapping().count() != 4)
	{
		WARNINGKPILOT << "Mapping does not have 4 elements." << index << endl;
		retval = QString();
	}

	switch (settings.custom(index))
	{
	case eCustomBirthdate:
		if (settings.dateFormat().isEmpty())
		{
			retval = KGlobal::locale()->formatDate(abEntry.birthday().date());
		}
		else
		{
			QString tmpfmt(KGlobal::locale()->dateFormat());
			KGlobal::locale()->setDateFormat(settings.dateFormat());
			QString ret(KGlobal::locale()->formatDate(abEntry.birthday().date()));
			KGlobal::locale()->setDateFormat(tmpfmt);
			retval = ret;
		}
		break;
	case eCustomURL:
		retval = abEntry.url().url();
		break;
	case eCustomIM:
		retval = abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"));
		break;
	case eCustomField:
	default:
		retval = abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));
		break;
	}

	return retval;
}

void KABCSync::setFieldFromHHCustom(
	const unsigned int index,
	KABC::Addressee &abEntry,
	const QString &value,
	const KABCSync::Settings &settings)
{
	FUNCTIONSETUPL(4);

	if (index>3)
	{
		WARNINGKPILOT << "Bad index number " << index << endl;
		return;
	}
	if (settings.customMapping().count() != 4)
	{
		WARNINGKPILOT << "Mapping does not have 4 elements." << index << endl;
		return;
	}

	switch (settings.custom(index))
	{
	case eCustomBirthdate:
	{
		QDate bdate;
		bool ok=false;
		if (settings.dateFormat().isEmpty())
		{
			// empty format means use locale setting
			bdate=KGlobal::locale()->readDate(value, &ok);
		}
		else
		{
			// use given format
			bdate=KGlobal::locale()->readDate(value, settings.dateFormat(), &ok);
		}

		if (!ok)
		{
			QString format = KGlobal::locale()->dateFormatShort();
			QRegExp re(CSL1("%[yY][^%]*"));
			format.remove(re); // Remove references to year and following punctuation
			bdate = KGlobal::locale()->readDate(value, format, &ok);
		}
		DEBUGKPILOT << "Birthdate from " << index << "-th custom field: "
			<< bdate.toString() << endl;
		DEBUGKPILOT << "Is Valid: " << bdate.isValid() << endl;
		if (bdate.isValid())
		{
			abEntry.setBirthday(bdate);
		}
		else
		{
			abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("X-Birthday"), value);
		}
		break;
	}
	case eCustomURL:
		abEntry.setUrl(value);
		break;
	case eCustomIM:
		abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"), value);
		break;
	case eCustomField:
	default:
		abEntry.insertCustom(appString, CSL1("CUSTOM")+QString::number(index), value);
		break;
	}
}


/** First search for a preferred  address. If we don't have one, search
 *  for home or work as specified in the config dialog. If we don't have
 *  such one, either, search for the other type. If we still have no luck,
 *  return an address with preferred + home/work flag (from config dlg). */
KABC::Address KABCSync::getAddress(const KABC::Addressee &abEntry, const KABCSync::Settings &s)
{
	// preferhome == (AbbrowserSettings::pilotStreet==0)

	// Check for preferred address first
	KABC::Address ad(abEntry.address(KABC::Address::Pref));
	if (!ad.isEmpty()) return ad;

	// Look for home or work, whichever is preferred
	int type = s.preferHome() ? KABC::Address::Home : KABC::Address::Work;
	ad=abEntry.address(type);
	if (!ad.isEmpty()) return ad;

	// Switch preference if still none found
	type = !s.preferHome() ? KABC::Address::Home : KABC::Address::Work;
	ad=abEntry.address(type);
	if (!ad.isEmpty()) return ad;

	// Last-ditch attempt; see if there is a preferred home or work address
	type = s.preferHome() ? KABC::Address::Home : KABC::Address::Work;
	return abEntry.address(type | KABC::Address::Pref);
}


QString KABCSync::getFieldForHHOtherPhone(const KABC::Addressee & abEntry, const KABCSync::Settings &s)
{
	switch(s.fieldForOtherPhone())
	{
		case eOtherPhone:
			return abEntry.phoneNumber(0).number();
		case eAssistant:
			return abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"));
		case eBusinessFax:
			return abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work).number();
		case eCarPhone:
			return abEntry.phoneNumber(KABC::PhoneNumber::Car).number();
		case eEmail2:
			return abEntry.emails().first();
		case eHomeFax:
			return abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home).number();
		case eTelex:
			return abEntry.phoneNumber(KABC::PhoneNumber::Bbs).number();
		case eTTYTTDPhone:
			return abEntry.phoneNumber(KABC::PhoneNumber::Pcs).number();
		default:
			return QString::null;
	}
}

void KABCSync::setFieldFromHHOtherPhone(KABC::Addressee & abEntry, const QString &nr, const KABCSync::Settings &s)
{
	int phoneType = 0;
	switch (s.fieldForOtherPhone())
	{
	// One very special case which doesn't even map to a real phone type in KABC
	case eAssistant:
		abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"), nr);
		return;
	// Special case: map phone to email, needs different handling.
	case eEmail2:
		abEntry.insertEmail(nr);
		return;
	// Remaining cases all map to various phone types
	case eOtherPhone:
		phoneType = 0;
		break;
	case eBusinessFax:
		phoneType = KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work;
		break;
	case eHomeFax:
		phoneType = KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home;
		break;
	case eCarPhone:
		phoneType = KABC::PhoneNumber::Car;
		break;
	case eTelex:
		phoneType = KABC::PhoneNumber::Bbs;
		break;
	case eTTYTTDPhone:
		phoneType = KABC::PhoneNumber::Pcs;
		break;
	default:
		WARNINGKPILOT << "Unknown phone mapping " << s.fieldForOtherPhone() << endl;
		phoneType = 0;
	}
	KABC::PhoneNumber phone = abEntry.phoneNumber(phoneType);
	phone.setNumber(nr);
	phone.setType(phoneType); // Double-check in case there was no phonenumber of given type
	abEntry.insertPhoneNumber(phone);
}

void KABCSync::setAddress(PilotAddress &toPilotAddr,
	const KABC::Address & abAddress)
{
	toPilotAddr.setField(entryAddress, abAddress.street());
	toPilotAddr.setField(entryCity, abAddress.locality());
	toPilotAddr.setField(entryState, abAddress.region());
	toPilotAddr.setField(entryZip, abAddress.postalCode());
	toPilotAddr.setField(entryCountry, abAddress.country());
}


bool KABCSync::isArchived(const KABC::Addressee &addr)
{
	return addr.custom(KABCSync::appString, KABCSync::flagString) == QString::number(SYNCDEL);
}

void KABCSync::makeArchived(KABC::Addressee &addr)
{
	FUNCTIONSETUP;
	addr.insertCustom(KABCSync::appString, KABCSync::flagString, QString::number(SYNCDEL));
	addr.removeCustom(KABCSync::appString, KABCSync::idString);
}




void KABCSync::copy(PilotAddress &toPilotAddr,
	const KABC::Addressee &fromAbEntry,
	const PilotAddressInfo &appInfo,
	const KABCSync::Settings &syncSettings)
{
	FUNCTIONSETUP;

	toPilotAddr.setDeleted(false);

	// don't do a reset since this could wipe out non copied info
	//toPilotAddr.reset();
	toPilotAddr.setField(entryLastname, fromAbEntry.familyName());
	toPilotAddr.setField(entryFirstname, fromAbEntry.givenName());
	toPilotAddr.setField(entryCompany, fromAbEntry.organization());
	toPilotAddr.setField(entryTitle, fromAbEntry.prefix());
	toPilotAddr.setField(entryNote, fromAbEntry.note());

	// do email first, to ensure they get stored
	toPilotAddr.setEmails(fromAbEntry.emails());

	// now in one fell swoop, set all phone numbers from the Addressee.  Note,
	// we don't need to differentiate between Fax numbers here--all Fax numbers
	// (Home Fax or Work Fax or just plain old Fax) will get synced to the Pilot
	KABCSync::setPhoneNumbers(appInfo,toPilotAddr,fromAbEntry.phoneNumbers());

	// Other field is an oddball and if the user has more than one field set
	// as "Other" then only one will be carried over.
	QString oth = KABCSync::getFieldForHHOtherPhone(fromAbEntry,syncSettings);
	DEBUGKPILOT << fname << ": putting: ["<<oth<<"] into Palm's other"<<endl;
	toPilotAddr.setPhoneField(PilotAddressInfo::eOther,
		oth, PilotAddress::Replace);

	KABC::Address homeAddress = KABCSync::getAddress(fromAbEntry, syncSettings);
	KABCSync::setAddress(toPilotAddr, homeAddress);

	// Process the additional entries from the Palm(the palm database app block tells us the name of the fields)
	unsigned int customIndex = 0;
	unsigned int hhField = entryCustom1;

	for ( ; customIndex<4; ++customIndex,++hhField )
	{
		toPilotAddr.setField(hhField,getFieldForHHCustom(customIndex,fromAbEntry,syncSettings));
	}

	int categoryForHH = KABCSync::bestMatchedCategory(fromAbEntry.categories(),
		appInfo,toPilotAddr.category());
	toPilotAddr.setCategory(categoryForHH);

	if (isArchived(fromAbEntry))
	{
		toPilotAddr.setArchived( true );
	}
	else
	{
		toPilotAddr.setArchived( false );
	}
}

void KABCSync::copy(KABC::Addressee &toAbEntry,
	const PilotAddress &fromPiAddr,
	const PilotAddressInfo &appInfo,
	const KABCSync::Settings &syncSettings)
{
	FUNCTIONSETUP;

	// copy straight forward values
	toAbEntry.setFamilyName(fromPiAddr.getField(entryLastname));
	toAbEntry.setGivenName(fromPiAddr.getField(entryFirstname));
	toAbEntry.setOrganization(fromPiAddr.getField(entryCompany));
	toAbEntry.setPrefix(fromPiAddr.getField(entryTitle));
	toAbEntry.setNote(fromPiAddr.getField(entryNote));

	// set the formatted name
	// TODO this is silly and should be removed soon.
	toAbEntry.setFormattedName(toAbEntry.realName());

	// copy the phone stuff
	// first off, handle the e-mail addresses as a group and separate from
	// the other phone number fields
	toAbEntry.setEmails(fromPiAddr.getEmails());

	// going from Pilot to kabc, we need to clear out all phone records in kabc
	// so that they can be set from the Pilot.  If we do not do this, then records
	// will be left in kabc when they are removed from the Pilot and we'll look
	// broken.
	KABC::PhoneNumber::List old = toAbEntry.phoneNumbers();
	for (KABC::PhoneNumber::List::Iterator it = old.begin(); it != old.end(); ++it) {
		KABC::PhoneNumber phone = *it;
		toAbEntry.removePhoneNumber(phone);
	}

	// now, get the phone numbers from the Pilot and set them one at a time in kabc
	KABC::PhoneNumber::List phones = KABCSync::getPhoneNumbers(fromPiAddr);
	for (KABC::PhoneNumber::List::Iterator it = phones.begin(); it != phones.end(); ++it) {
		KABC::PhoneNumber phone = *it;
		// check for fax number if it is one, set the type per the user's direction
		if (phone.type() & KABC::PhoneNumber::Fax)
		{
			phone.setType(syncSettings.faxTypeOnPC());
		}
		toAbEntry.insertPhoneNumber(phone);
	}

	// Note:  this is weird, and it may cause data to not be synced if there is
	// more than one "Other" field being used on the Pilot, since only one will
	// be synced in either direction.
	KABCSync::setFieldFromHHOtherPhone(toAbEntry,
		fromPiAddr.getPhoneField(PilotAddressInfo::eOther),syncSettings);

	// going from Pilot to kabc, we need to clear out all addresses in kabc
	// so that they can be set from the Pilot.  If we do not do this, then records
	// will be left in kabc when they are removed from the Pilot and we'll look
	// broken.
	KABC::Address::List oAddr = toAbEntry.addresses();
	for (KABC::Address::List::Iterator it = oAddr.begin(); it != oAddr.end(); ++it) {
		const KABC::Address addr = *it;
		toAbEntry.removeAddress(addr);
	}
	KABC::Address homeAddress = KABCSync::getAddress(toAbEntry,syncSettings);
	homeAddress.setStreet(fromPiAddr.getField(entryAddress));
	homeAddress.setLocality(fromPiAddr.getField(entryCity));
	homeAddress.setRegion(fromPiAddr.getField(entryState));
	homeAddress.setPostalCode(fromPiAddr.getField(entryZip));
	homeAddress.setCountry(fromPiAddr.getField(entryCountry));
	toAbEntry.insertAddress(homeAddress);

	unsigned int customIndex = 0;
	unsigned int hhField = entryCustom1;

	for ( ; customIndex<4; ++customIndex,++hhField )
	{
		KABCSync::setFieldFromHHCustom(customIndex,
			toAbEntry,
			fromPiAddr.getField(hhField),
			syncSettings);
	}

	// copy the fromPiAddr pilot id to the custom field KPilot_Id;
	// pilot id may be zero(since it could be new) but couldn't hurt
	// to even assign it to zero; let's us know what state the
	// toAbEntry is in
	toAbEntry.insertCustom(KABCSync::appString, KABCSync::idString, QString::number(fromPiAddr.id()));

	KABCSync::setCategory(toAbEntry, appInfo.categoryName(fromPiAddr.category()));

	showAddressee(toAbEntry);
}

void KABCSync::showAddressee(const KABC::Addressee & abAddress)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << "\tAbbrowser Contact Entry" << endl;
	if (abAddress.isEmpty())
	{
		DEBUGKPILOT<< "\t\tEMPTY"<<endl;
		return;
	}
	DEBUGKPILOT << "\t\tLast name = " << abAddress.familyName() << endl;
	DEBUGKPILOT << "\t\tFirst name = " << abAddress.givenName() << endl;
	DEBUGKPILOT << "\t\tCompany = " << abAddress.organization() << endl;
	DEBUGKPILOT << "\t\tJob Title = " << abAddress.prefix() << endl;
	DEBUGKPILOT << "\t\tNote = " << abAddress.note() << endl;
	DEBUGKPILOT << "\t\tCategory = " << abAddress.categories().first() << endl;
	DEBUGKPILOT << "\t\tEmail = " << abAddress.emails().join(",") << endl;

	KABC::PhoneNumber::List phs = abAddress.phoneNumbers();
	for (KABC::PhoneNumber::List::Iterator it = phs.begin(); it != phs.end(); ++it) {
		KABC::PhoneNumber phone = *it;
		DEBUGKPILOT << "\t\t" << phone.label() 
			<< "= " << phone.number() << endl;
	}

	KABC::Address::List ads = abAddress.addresses();
	for (KABC::Address::List::Iterator it = ads.begin(); it != ads.end(); ++it) {
		const KABC::Address addr = *it;
		DEBUGKPILOT << "\t\tAddress = " << addr.street() <<endl;
		DEBUGKPILOT << "\t\tLocality = " << addr.locality() <<endl;
		DEBUGKPILOT << "\t\tRegion = " << addr.region() <<endl;
		DEBUGKPILOT << "\t\tPostal code = " << addr.postalCode() <<endl;
		DEBUGKPILOT << "\t\tCountry = " << addr.country() <<endl << endl;
	}
#else
	Q_UNUSED( abAddress );
#endif
}




KABCSync::Settings::Settings() :
	fDateFormat(),
	fCustomMapping(4), // Reserve space for 4 elements, value 0 == CustomField
	fOtherPhone(eOtherPhone),
	fPreferHome(true),
	fFaxTypeOnPC(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home)
{
}

