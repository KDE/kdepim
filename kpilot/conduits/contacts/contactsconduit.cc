/* contactsconduit.cc			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include "contactsconduit.h"

#include <akonadi/collection.h>
#include <kglobal.h>

#include "idmapping.h"
#include "options.h"
#include "contactsakonadiproxy.h"
#include "contactsakonadirecord.h"
#include "contactshhrecord.h"
#include "contactshhdataproxy.h"
#include "contactssettings.h"
#include "pilotAddress.h"
#include "pilottophonemap.h"

class Settings
{
public:
	enum MappingForOtherPhone {
		eOtherPhone=0,
		eAssistant,
		eBusinessFax,
		eCarPhone,
		eEmail2,
		eHomeFax,
		eTelex,
		eTTYTTDPhone
	};

	enum MappingForCustomField {
		eCustomField=0,
		eCustomBirthdate,
		eCustomURL,
		eCustomIM
	};
	
	Settings() : fDateFormat(),
	fCustomMapping(4), // Reserve space for 4 elements, value 0 == CustomField
	fOtherPhone(eOtherPhone),
	fPreferHome(true),
	fFaxTypeOnPC(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home)
	{}
	
	QString dateFormat() const
	{
		return fDateFormat;
	}
	void setDateFormat(const QString& s)
	{
		fDateFormat = s;
	}

	const QVector<int> &customMapping() const
	{
		return fCustomMapping;
	}
	void setCustomMapping(const QVector<int> &v)
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

	KABC::PhoneNumber::Type faxTypeOnPC() const
	{
		return fFaxTypeOnPC;
	}
	void setFaxTypeOnPC(KABC::PhoneNumber::Type v)
	{
		fFaxTypeOnPC = v;
	}
private:
	QString fDateFormat;
	QVector<int> fCustomMapping;
	int fOtherPhone;
	bool fPreferHome;
	KABC::PhoneNumber::Type fFaxTypeOnPC;
};

class ContactsConduit::Private
{
public:
	Private() : fCollectionId( -1 ), fPrevCollectionId(-2), fContactsHHDataProxy( 0L )
	{
	}
	
	Akonadi::Collection::Id fCollectionId;
	Akonadi::Collection::Id fPrevCollectionId;
	ContactsHHDataProxy* fContactsHHDataProxy;
	Settings fSettings;
};

/* This is partly stolen from the boost libraries, partly from
*  "Modern C++ design" for doing compile time checks; we need
*  to make sure that the enum values and the values in the 
*  ContactsSettings class are the same so that both interpret
*  configuration values the same way.
*/
template<bool> struct EnumerationMismatch;
template<> struct EnumerationMismatch<true>{};

#define CHECK_ENUM(a) (void)sizeof(EnumerationMismatch<((int)Settings::a)==((int)ContactsSettings::a)>)

static inline void compile_time_check()
{
	// Mappings for other phone
	CHECK_ENUM(eOtherPhone);
	CHECK_ENUM(eOtherPhone);
	CHECK_ENUM(eAssistant);
	CHECK_ENUM(eBusinessFax);
	CHECK_ENUM(eCarPhone);
	CHECK_ENUM(eEmail2);
	CHECK_ENUM(eHomeFax);
	CHECK_ENUM(eTelex);
	CHECK_ENUM(eTTYTTDPhone);

	// Mappings for custom fields
	CHECK_ENUM(eCustomField);
	CHECK_ENUM(eCustomBirthdate);
	CHECK_ENUM(eCustomURL);
	CHECK_ENUM(eCustomIM);
}

inline KABC::PhoneNumber::Type faxTypeOnPC()
{
	return KABC::PhoneNumber::Fax |
		( (ContactsSettings::pilotFax() == 0 ) ?
			KABC::PhoneNumber::Home :
			KABC::PhoneNumber::Work );
}

inline bool _equal(const QString & str1, const QString & str2)
{
	return ( str1.isEmpty() && str2.isEmpty() ) || ( str1 == str2 );
}

const QString appString = CSL1( "KPILOT" ); ///< Identifier for the application
const QString flagString = CSL1( "Flag" ); ///< Flags: synced or not
const QString idString = CSL1( "RecordID" ); ///< Record ID on HH for this addressee


ContactsConduit::ContactsConduit( KPilotLink *o, const QVariantList &a )
	: RecordConduit( o, a, CSL1( "AddressDB" ), CSL1( "Contacts Conduit" ) )
	, d( new ContactsConduit::Private )
{
}

ContactsConduit::~ContactsConduit()
{
	KPILOT_DELETE( d );
}

void ContactsConduit::loadSettings()
{
	FUNCTIONSETUP;
	
	ContactsSettings::self()->readConfig();
	d->fCollectionId = ContactsSettings::akonadiCollection();
	d->fPrevCollectionId = ContactsSettings::prevAkonadiCollection();
}

bool ContactsConduit::initDataProxies()
{
	FUNCTIONSETUP;
	
	if( !fDatabase )
	{
		addSyncLogEntry( i18n( "Error: Handheld database is not loaded." ) );
		return false;
	}
	
	if( d->fCollectionId < 0 )
	{
		addSyncLogEntry( i18n( "Error: No valid akonadi collection configured." ) );
		return false;
	}
	
	if( d->fPrevCollectionId != d->fCollectionId )
	{
		// TODO: Enable this in trunk.
		//addSyncLogEntry( i18n( "Note: Collection has changed since last sync, removing mapping." ) );
		DEBUGKPILOT << "Note: Collection has changed since last sync, removing mapping.";
		fMapping.remove();
	}
	
	d->fContactsHHDataProxy = new ContactsHHDataProxy( fDatabase );
	ContactsAkonadiProxy* pcDataProxy = new ContactsAkonadiProxy( fMapping );
	pcDataProxy->setCollectionId( d->fCollectionId );
	
	fHHDataProxy = d->fContactsHHDataProxy;
	fBackupDataProxy = new ContactsHHDataProxy( fLocalDatabase );
	fPCDataProxy = pcDataProxy;
	
	// At this point we should be able to read the backup and handheld database.
	// However, it might be that Akonadi is not started.
	fHHDataProxy->loadAllRecords();
	fBackupDataProxy->loadAllRecords();
	if( fPCDataProxy->isOpen() ) 
	{
		fPCDataProxy->loadAllRecords();
	}
	
	return true;
}

void ContactsConduit::syncFinished()
{
	ContactsSettings::self()->readConfig();
	ContactsSettings::self()->setPrevAkonadiCollection(d->fCollectionId);
	ContactsSettings::self()->writeConfig();
}

bool ContactsConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUPL(5);
	
	// empty records are never equal!
	if( !pcRec || !hhRec )
	{
		return false;
	}
	
	const ContactsAkonadiRecord* aRec
		= static_cast<const ContactsAkonadiRecord*>( pcRec );
	const ContactsHHRecord* cHHrec = static_cast<const ContactsHHRecord*>( hhRec );
	
	PilotAddress piAddress = cHHrec->pilotAddress();
	KABC::Addressee abEntry = aRec->addressee();

	if( !_equal( abEntry.familyName(), piAddress.getField( entryLastname ) ) )
	{
		DEBUGKPILOT  << "last name not equal [pc,hh]: [" << abEntry.familyName()
			<< ", " << piAddress.getField( entryLastname ) << "]";
		return false;
	}
	if(!_equal( abEntry.givenName(), piAddress.getField(entryFirstname) ) )
	{
		DEBUGKPILOT  << "first name not equal";
		return false;
	}
	if(!_equal( abEntry.prefix(), piAddress.getField(entryTitle) ) )
	{
		DEBUGKPILOT  << "title/prefix not equal";
		return false;
	}
	if(!_equal( abEntry.organization(), piAddress.getField(entryCompany) ) )
	{
		DEBUGKPILOT  << "company/organization not equal";
		return false;
	}
	
	if( !_equal( abEntry.note(), piAddress.getField( entryNote) ) )
	{
			DEBUGKPILOT  << "note not equal";
			return false;
	}

	// Check that the name of the category of the HH record
	// is one matching the PC record.
	QString cat = fHHDataProxy->bestMatchCategory( pcRec->categories()
		, hhRec->category() );
		
	if( hhRec->category() != "Unfiled" && !_equal( cat, hhRec->category() ) )
	{
		DEBUGKPILOT  << "category not equal: " << cat << " " << hhRec->category();
		return false;
	}

	// first, look for missing e-mail addresses on either side
	QStringList abEmails( abEntry.emails() );
	QStringList piEmails( piAddress.getEmails() );

	if( abEmails.count() != piEmails.count() )
	{
		DEBUGKPILOT  << "email count not equal";
		return false;
	}
	
	foreach( const QString& abEmail, abEmails )
	{
		if( !piEmails.contains( abEmail ) )
		{
			DEBUGKPILOT  << "pilot e-mail missing";
			return false;
		}
	}
	
	foreach( const QString& piEmail, piEmails )
	{
		if( !abEmails.contains( piEmail ) )
		{
			DEBUGKPILOT  << "kabc e-mail missing";
			return false;
		}
	}

	// now look for differences in phone numbers.  Note:  we cannot just compare one
	// of each kind of phone number, because there's no guarantee that if the user
	// has more than one of a given type, we're comparing the correct two.

	KABC::PhoneNumber::List abPhones( abEntry.phoneNumbers() );
	KABC::PhoneNumber::List piPhones = getPhoneNumbers( piAddress );
	
	// first make sure that all of the pilot phone numbers are in kabc
	foreach( const KABC::PhoneNumber& piPhone, piPhones )
	{
		bool found = false;
		foreach( const KABC::PhoneNumber& abPhone, abPhones )
		{
			// see if we have the same number here...
			// * Note * We used to check for preferred number matching, but
			//     this seems to have broke in kdepim 3.5 and I don't have time to
			//     figure out why, so we won't check to see if preferred number match
			if( _equal( piPhone.number(), abPhone.number() ) ) {
				found = true;
				break;
			}
		}
		if (!found)
		{
			DEBUGKPILOT  << "not equal because kabc phone not found.";
			return false;
		}
	}
	
	// now the other way.  *cringe*  kabc has the capacity to store way more addresses
	// than the Pilot, so this might give false positives more than we'd want....
	foreach( const KABC::PhoneNumber& abPhone, abPhones )
	{
		bool found = false;
		
		foreach( const KABC::PhoneNumber& piPhone, piPhones )
		{
			if( _equal( piPhone.number(), abPhone.number() ) )
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			DEBUGKPILOT  << "not equal because pilot phone not found.";
			return false;
		}
	}

	// Only check the other field if it is not empty on the palm.
	if( !piAddress.getPhoneField( PilotAddressInfo::eOther ).isEmpty() )
	{
		if( !_equal( getFieldForHHOtherPhone( abEntry ),
			piAddress.getPhoneField( PilotAddressInfo::eOther ) ) )
		{
			DEBUGKPILOT  << "not equal because of other phone field.";
			return false;
		}
	}

	KABC::Address address = getAddress( abEntry );
	if( !_equal(address.street(), piAddress.getField( entryAddress) ) )
	{
		DEBUGKPILOT  << "address not equal";
		return false;
	}
	
	if( !_equal( address.locality(), piAddress.getField( entryCity ) ) )
	{
		DEBUGKPILOT  << "city not equal";
		return false;
	}
	
	if( !_equal( address.region(), piAddress.getField( entryState ) ) )
	{
		DEBUGKPILOT  << "state not equal";
		return false;
	}
	
	if( !_equal( address.postalCode(), piAddress.getField( entryZip ) ) )
	{
		DEBUGKPILOT  << "zip not equal";
		return false;
	}
	
	if( !_equal( address.country(), piAddress.getField( entryCountry ) ) )
	{
		DEBUGKPILOT  << "country not equal";
		return false;
	}

	unsigned int customIndex = 0;
	unsigned int hhField = entryCustom1;

	for ( ; customIndex < 4; ++customIndex, ++hhField )
	{
		if( !_equal( getFieldForHHCustom( customIndex, abEntry ),
			piAddress.getField( hhField ) ) )
		{
			DEBUGKPILOT <<"Custom field" << customIndex
				<< " (HH field " << hhField << ") differs.";
			return false;
		}
	}
	
	return true;
}

Record* ContactsConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;
	
	Akonadi::Item item;
	item.setPayload<KABC::Addressee>( KABC::Addressee() );
	item.setMimeType( "text/directory" );
		
	Record* rec = new ContactsAkonadiRecord( item, fMapping.lastSyncedDate() );
	copy( hhRec, rec );
	
	Q_ASSERT( equal( rec, hhRec ) );
	
	return rec;
}

HHRecord* ContactsConduit::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;
	
	HHRecord* hhRec = new ContactsHHRecord( PilotAddress().pack(), "Unfiled" );
	copy( pcRec, hhRec );
	
	Q_ASSERT( equal( pcRec, hhRec ) );
	
	return hhRec;
}

void ContactsConduit::_copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	const ContactsAkonadiRecord* aFrom 
		= static_cast<const ContactsAkonadiRecord*>( from );
	ContactsHHRecord* hhTo = static_cast<ContactsHHRecord*>( to );
	
	KABC::Addressee fromAbEntry = aFrom->addressee();
	PilotAddress toPilotAddr = hhTo->pilotAddress();
	
	toPilotAddr.setDeleted( false );

	// don't do a reset since this could wipe out non copied info
	//toPilotAddr.reset();
	toPilotAddr.setField( entryLastname, fromAbEntry.familyName() );
	toPilotAddr.setField( entryFirstname, fromAbEntry.givenName() );
	toPilotAddr.setField( entryCompany, fromAbEntry.organization() );
	toPilotAddr.setField( entryTitle, fromAbEntry.prefix() );
	toPilotAddr.setField( entryNote, fromAbEntry.note() );

	// do email first, to ensure they get stored
	toPilotAddr.setEmails( fromAbEntry.emails() );

	// now in one fell swoop, set all phone numbers from the Addressee.  Note,
	// we don't need to differentiate between Fax numbers here--all Fax numbers
	// (Home Fax or Work Fax or just plain old Fax) will get synced to the Pilot
	d->fContactsHHDataProxy->setPhoneNumbers( toPilotAddr, fromAbEntry.phoneNumbers() );

	// Other field is an oddball and if the user has more than one field set
	// as "Other" then only one will be carried over.
	QString oth = getFieldForHHOtherPhone( fromAbEntry );
	DEBUGKPILOT << "putting: ["<<oth<<"] into Palm's other";
	toPilotAddr.setPhoneField( PilotAddressInfo::eOther,
		oth, PilotAddress::Replace );

	KABC::Address homeAddress = getAddress( fromAbEntry );
	setAddress( toPilotAddr, homeAddress );

	// Process the additional entries from the Palm(the palm database app block 
	// tells us the name of the fields)
	unsigned int customIndex = 0;
	unsigned int hhField = entryCustom1;

	for ( ; customIndex<4; ++customIndex, ++hhField )
	{
		toPilotAddr.setField( hhField
			, getFieldForHHCustom( customIndex, fromAbEntry ) );
	}
	
	hhTo->setPilotAddress( toPilotAddr );
}

void ContactsConduit::_copy( const HHRecord* from, Record *to  )
{
	FUNCTIONSETUP;
	
	const ContactsHHRecord* hhFrom = static_cast<const ContactsHHRecord*>( from );
	ContactsAkonadiRecord* aTo = static_cast<ContactsAkonadiRecord*>( to );
	
	PilotAddress fromPiAddr = hhFrom->pilotAddress();
	KABC::Addressee toAbEntry = aTo->addressee();
	
	// copy straight forward values
	toAbEntry.setFamilyName( fromPiAddr.getField( entryLastname ) );
	toAbEntry.setGivenName( fromPiAddr.getField( entryFirstname ) );
	toAbEntry.setOrganization( fromPiAddr.getField( entryCompany ) );
	toAbEntry.setPrefix( fromPiAddr.getField( entryTitle ) );
	toAbEntry.setNote( fromPiAddr.getField( entryNote ) );

	// set the formatted name
	// TODO this is silly and should be removed soon.
	toAbEntry.setFormattedName( toAbEntry.realName() );

	// copy the phone stuff
	// first off, handle the e-mail addresses as a group and separate from
	// the other phone number fields
	toAbEntry.setEmails( fromPiAddr.getEmails() );

	// going from Pilot to kabc, we need to clear out all phone records in kabc
	// so that they can be set from the Pilot.  If we do not do this, then records
	// will be left in kabc when they are removed from the Pilot and we'll look
	// broken.
	KABC::PhoneNumber::List old = toAbEntry.phoneNumbers();
	for( KABC::PhoneNumber::List::Iterator it = old.begin(); it != old.end(); ++it )
	{
		KABC::PhoneNumber phone = *it;
		toAbEntry.removePhoneNumber(phone);
	}

	// now, get the phone numbers from the Pilot and set them one at a time in kabc
	KABC::PhoneNumber::List phones = getPhoneNumbers( fromPiAddr );
	for( KABC::PhoneNumber::List::Iterator it = phones.begin(); it != phones.end(); ++it )
	{
		KABC::PhoneNumber phone = *it;
		// check for fax number if it is one, set the type per the user's direction
		if( phone.type() & KABC::PhoneNumber::Fax )
		{
			phone.setType( d->fSettings.faxTypeOnPC() );
		}
		
		toAbEntry.insertPhoneNumber( phone );
	}

	// Note:  this is weird, and it may cause data to not be synced if there is
	// more than one "Other" field being used on the Pilot, since only one will
	// be synced in either direction.
	setFieldFromHHOtherPhone( toAbEntry
		, fromPiAddr.getPhoneField( PilotAddressInfo::eOther ) );

	// going from Pilot to kabc, we need to clear out all addresses in kabc
	// so that they can be set from the Pilot.  If we do not do this, then records
	// will be left in kabc when they are removed from the Pilot and we'll look
	// broken.
	KABC::Address::List oAddr = toAbEntry.addresses();
	for( KABC::Address::List::Iterator it = oAddr.begin(); it != oAddr.end(); ++it )
	{
		const KABC::Address addr = *it;
		toAbEntry.removeAddress(addr);
	}
	
	KABC::Address homeAddress = getAddress( toAbEntry );
	homeAddress.setStreet( fromPiAddr.getField( entryAddress ) );
	homeAddress.setLocality( fromPiAddr.getField( entryCity ) );
	homeAddress.setRegion( fromPiAddr.getField( entryState) );
	homeAddress.setPostalCode( fromPiAddr.getField( entryZip ) );
	homeAddress.setCountry( fromPiAddr.getField( entryCountry ) );
	toAbEntry.insertAddress( homeAddress );

	unsigned int customIndex = 0;
	unsigned int hhField = entryCustom1;

	for ( ; customIndex < 4; ++customIndex, ++hhField )
	{
		setFieldFromHHCustom( customIndex, toAbEntry, fromPiAddr.getField( hhField ) );
	}

	// copy the fromPiAddr pilot id to the custom field KPilot_Id;
	// pilot id may be zero(since it could be new) but couldn't hurt
	// to even assign it to zero; let's us know what state the
	// toAbEntry is in
	// NOTE: I don't think this is necessary anymore.
	toAbEntry.insertCustom( appString, idString, from->id() );

	if( from->category() != CSL1( "Unfiled" ) )
	{
		toAbEntry.insertCategory( from->category() );
	}
	
	aTo->setAddressee( toAbEntry );
}

/** Protected methods **/

/** First search for a preferred  address. If we don't have one, search
 *  for home or work as specified in the config dialog. If we don't have
 *  such one, either, search for the other type. If we still have no luck,
 *  return an address with preferred + home/work flag (from config dlg). */
KABC::Address ContactsConduit::getAddress( const KABC::Addressee &abEntry ) const
{
	// preferhome == (AbbrowserSettings::pilotStreet==0)

	// Check for preferred address first
	KABC::Address ad( abEntry.address( KABC::Address::Pref ) );
	if( !ad.isEmpty() ) return ad;

	// Look for home or work, whichever is preferred
	KABC::Address::Type type = d->fSettings.preferHome() ? KABC::Address::Home 
		: KABC::Address::Work;
	ad = abEntry.address( type );
	if( !ad.isEmpty() ) return ad;

	// Switch preference if still none found
	type = !d->fSettings.preferHome() ? KABC::Address::Home : KABC::Address::Work;
	ad = abEntry.address( type );
	if( !ad.isEmpty() ) return ad;

	// Last-ditch attempt; see if there is a preferred home or work address
	type = d->fSettings.preferHome() ? KABC::Address::Home : KABC::Address::Work;
	return abEntry.address( type | KABC::Address::Pref );
}

QString ContactsConduit::getFieldForHHCustom( const unsigned int index
	, const KABC::Addressee &abEntry ) const
{
	FUNCTIONSETUPL(4);

	QString retval;

	if( index > 3 )
	{
		WARNINGKPILOT <<"Bad index number" << index;
		retval.clear();
	}
	if( d->fSettings.customMapping().count() != 4 )
	{
		WARNINGKPILOT <<"Mapping does not have 4 elements." << index;
		retval.clear();
	}

	switch( d->fSettings.custom(index) )
	{
	case Settings::eCustomBirthdate:
		if( d->fSettings.dateFormat().isEmpty() )
		{
			retval = KGlobal::locale()->formatDate(abEntry.birthday().date());
		}
		else
		{
			QString tmpfmt(KGlobal::locale()->dateFormat());
			KGlobal::locale()->setDateFormat( d->fSettings.dateFormat() );
			QString ret( KGlobal::locale()->formatDate( abEntry.birthday().date() ) );
			KGlobal::locale()->setDateFormat( tmpfmt );
			retval = ret;
		}
		break;
	case Settings::eCustomURL:
		retval = abEntry.url().url();
		break;
	case Settings::eCustomIM:
		retval = abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"));
		break;
	case Settings::eCustomField:
	default:
		retval = abEntry.custom( appString, CSL1( "CUSTOM" ) + QString::number( index ) );
		break;
	}

	return retval;
}

QString ContactsConduit::getFieldForHHOtherPhone( const KABC::Addressee & abEntry ) const
{
	switch( d->fSettings.fieldForOtherPhone() )
	{
		case Settings::eOtherPhone:
			return abEntry.phoneNumber(0).number();
		case Settings::eAssistant:
			return abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"));
		case Settings::eBusinessFax:
			return abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work).number();
		case Settings::eCarPhone:
			return abEntry.phoneNumber(KABC::PhoneNumber::Car).number();
		case Settings::eEmail2:
			return abEntry.emails().first();
		case Settings::eHomeFax:
			return abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home).number();
		case Settings::eTelex:
			return abEntry.phoneNumber(KABC::PhoneNumber::Bbs).number();
		case Settings::eTTYTTDPhone:
			return abEntry.phoneNumber(KABC::PhoneNumber::Pcs).number();
		default:
			return QString();
	}
}

KABC::PhoneNumber::List ContactsConduit::getPhoneNumbers( const PilotAddress &a ) const
{
	FUNCTIONSETUP;

	KABC::PhoneNumber::List list;
	QString test;

	PhoneSlot shownPhone = a.getShownPhone();

	DEBUGKPILOT << "Preferred pilot index is: ["
		<< shownPhone << "], preferred phone number is: ["
		<< a.getField(shownPhone) << ']';

	for (PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i)
	{
		// skip email entries
		if ( a.getPhoneType( i ) == PilotAddressInfo::eEmail )
		{
			continue;
		}

		// only look at this if the field is populated
		if( a.getField( i ).isEmpty() )
		{
			continue;
		}

		KABC::PhoneNumber::Type phoneType = pilotToPhoneMap[a.getPhoneType(i)];

		// only populate a PhoneNumber if we have a corresponding type
		// XXX: shouldn't that be > 0 to avoid the badly-mapped types?
		if( phoneType >= 0 )
		{
			// if this is the preferred phone number, set it as such
			if( shownPhone == i )
			{
				phoneType |= KABC::PhoneNumber::Pref;
				DEBUGKPILOT <<"Found preferred pilot index: ["
					<< i << "], text: [" << a.getField( i ) << ']';
			}
			KABC::PhoneNumber ph( a.getField( i ), phoneType );
			list.append(ph);
		}
		else
		{
			DEBUGKPILOT <<"Pilot phone number: ["
				<< a.getField( i ) << "], index: [" << i << "], type: ["
				<< phoneType << "], has no corresponding PhoneNumber type.";
		}
	}

	DEBUGKPILOT << "Returning " << list.count() << " phone numbers.";

	return list;
}

void ContactsConduit::setAddress( PilotAddress &toPilotAddr
	, const KABC::Address &abAddress ) const
{
	toPilotAddr.setField( entryAddress, abAddress.street() );
	toPilotAddr.setField( entryCity, abAddress.locality() );
	toPilotAddr.setField( entryState, abAddress.region() );
	toPilotAddr.setField( entryZip, abAddress.postalCode() );
	toPilotAddr.setField( entryCountry, abAddress.country() );
}

void ContactsConduit::setFieldFromHHCustom(
	const unsigned int index,
	KABC::Addressee &abEntry,
	const QString &value )
{
	FUNCTIONSETUPL(4);

	if (index>3)
	{
		WARNINGKPILOT <<"Bad index number" << index;
		return;
	}
	if ( d->fSettings.customMapping().count() != 4)
	{
		WARNINGKPILOT <<"Mapping does not have 4 elements." << index;
		return;
	}

	switch( d->fSettings.custom( index ) )
	{
	case Settings::eCustomBirthdate:
	{
		QDate bdate;
		bool ok = false;
		if( d->fSettings.dateFormat().isEmpty() )
		{
			// empty format means use locale setting
			bdate = KGlobal::locale()->readDate( value, &ok );
		}
		else
		{
			// use given format
			bdate = KGlobal::locale()->readDate( value, d->fSettings.dateFormat(), &ok );
		}

		if (!ok)
		{
			QString format = KGlobal::locale()->dateFormatShort();
			QRegExp re(CSL1("%[yY][^%]*"));
			format.remove(re); // Remove references to year and following punctuation
			bdate = KGlobal::locale()->readDate(value, format, &ok);
		}
		DEBUGKPILOT << "Birthdate from" << index <<"-th custom field:"
			<< bdate.toString();
		DEBUGKPILOT <<"Is Valid:" << bdate.isValid();
		if (bdate.isValid())
		{
			abEntry.setBirthday( QDateTime(bdate) );
		}
		else
		{
			abEntry.insertCustom( CSL1( "KADDRESSBOOK" ), CSL1( "X-Birthday" ), value );
		}
		break;
	}
	case Settings::eCustomURL:
		abEntry.setUrl( value );
		break;
	case Settings::eCustomIM:
		abEntry.insertCustom( CSL1( "KADDRESSBOOK" ), CSL1( "X-IMAddress" ), value );
		break;
	case Settings::eCustomField:
	default:
		abEntry.insertCustom( appString, CSL1( "CUSTOM" ) + QString::number( index ), value );
		break;
	}
}

void ContactsConduit::setFieldFromHHOtherPhone( KABC::Addressee & abEntry
	, const QString &nr )
{
	FUNCTIONSETUP;
	
	// Don't try to do things with empty strings.
	if( nr.isEmpty() )
	{
		return;
	}
	
	KABC::PhoneNumber::Type phoneType = 0;
	switch( d->fSettings.fieldForOtherPhone() )
	{
	// One very special case which doesn't even map to a real phone type in KABC
	case Settings::eAssistant:
		abEntry.insertCustom( CSL1( "KADDRESSBOOK" ), CSL1( "AssistantsName" ), nr );
		return;
	// Special case: map phone to email, needs different handling.
	case Settings::eEmail2:
		abEntry.insertEmail(nr);
		return;
	// Remaining cases all map to various phone types
	case Settings::eOtherPhone:
		phoneType = 0;
		break;
	case Settings::eBusinessFax:
		phoneType = KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work;
		break;
	case Settings::eHomeFax:
		phoneType = KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home;
		break;
	case Settings::eCarPhone:
		phoneType = KABC::PhoneNumber::Car;
		break;
	case Settings::eTelex:
		phoneType = KABC::PhoneNumber::Bbs;
		break;
	case Settings::eTTYTTDPhone:
		phoneType = KABC::PhoneNumber::Pcs;
		break;
	default:
		WARNINGKPILOT <<"Unknown phone mapping" << d->fSettings.fieldForOtherPhone();
		phoneType = 0;
	}
	
	KABC::PhoneNumber phone = abEntry.phoneNumber(phoneType);
	phone.setNumber (nr );
	// Double-check in case there was no phonenumber of given type
	phone.setType( phoneType );
	abEntry.insertPhoneNumber( phone );
}

