/* conduit.cc			KPilot
**
** Copyright (C) 2008 by Jason 'vanRijn' Kasper <vR@movingparts.net>
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

#include "memofileconduit.h"

#include <kglobal.h>

#include "idmapping.h"
#include "options.h"
#include "memofilepcdataproxy.h"
#include "memofilepcrecord.h"
#include "memofilehhrecord.h"
#include "memofilehhdataproxy.h"
#include "memofilesettings.h"
#include "pilotMemo.h"


class MemofileConduit::Private
{
public:
	Private() :
           _DEFAULT_MEMODIR( QDir::homePath() + CSL1( "/MyMemos" ) ),
	   fMemoAppInfo( 0L ),
	   _memofiles( 0L ),
           fHHDataProxy( 0 ),
           fPCDataProxy( 0 )
	{
	}
        // configuration settings...
	QString	_DEFAULT_MEMODIR;
	QString	_memo_directory;
	bool	_sync_private;

        PilotMemoInfo	*fMemoAppInfo;
	QList<PilotMemo> fMemoList;

        MemofileHHDataProxy *fHHDataProxy;
        MemofilePCDataProxy *fPCHDataProxy;

	// our categories
	MemoCategoryMap fCategories;

	Memofiles * _memofiles;
};


inline bool _equal(const QString & str1, const QString & str2)
{
	return ( str1.isEmpty() && str2.isEmpty() ) || ( str1 == str2 );
}


MemofileConduit::MemofileConduit( KPilotLink *o, const QVariantList &a)
	: RecordConduit( o, a, CSL1( "MemoDB" ), CSL1( "Memofile Conduit" )),
          d( new MemofileConduit::Private )
{
}

MemofileConduit::~MemofileConduit()
{
	KPILOT_DELETE( d );
}

void MemofileConduit::loadSettings()
{
	FUNCTIONSETUP;

	MemofileSettings::self()->readConfig();

       	QString dir(MemofileConduitSettings::directory());
	if (dir.isEmpty()) {
		dir = _DEFAULT_MEMODIR;

		DEBUGKPILOT
			<< ": no directory given to us.  defaulting to: ["
			<< _DEFAULT_MEMODIR
			<< "]";
	}

	d->_memo_directory = dir;
	d->_sync_private = MemofileConduitSettings::syncPrivate();


	DEBUGKPILOT
		<< ": Settings... "
		<< "  directory: [" << d->_memo_directory
		<< "], sync private: [" << d->_sync_private
		<< "]";

	return true;
}

bool MemofileConduit::initDataProxies()
{
	FUNCTIONSETUP;

	if( !fDatabase )
	{
		addSyncLogEntry( i18n( "Error: Handheld database is not loaded." ) );
		return false;
	}

        fHHDataProxy = new MemofileHHDataProxy( fDatabase );
	fBackupDataProxy = new MemofileHHDataProxy( fLocalDatabase );
	fPCDataProxy = new MemofilePCDataProxy( fMapping );

	fHHDataProxy->loadAllRecords();
	fBackupDataProxy->loadAllRecords();
	fPCDataProxy->loadAllRecords();

	return true;
}

bool MemofileConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;

	// empty records are never equal!
	if ( !pcRec || !hhRec )
	{
       		DEBUGKPILOT  << "pcRec or hhRec are null";
		return false;
	}

	const MemofilePCRecord* mPCRec = static_cast<const MemofilePCRecord*>( pcRec );
	const ContactsHHRecord* mHHrec = static_cast<const ContactsHHRecord*>( hhRec );

       	if ( !mPCRec || !mHHRec )
	{
       		DEBUGKPILOT  << "mPCRec or mHHRec are null";
                return false;
	}

	if ( !_equal( mHHRec.title(), mPCRec.title() ) )
	{
		DEBUGKPILOT  << "title not equal [pc,hh]: [" << mPCRec.title()
			<< ", " << mHHRec.title() << "]";
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

Record* MemofileConduit::createPCRecord( const HHRecord *hhRec )
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

HHRecord* MemofileConduit::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;

	HHRecord* hhRec = new ContactsHHRecord( PilotAddress().pack(), "Unfiled" );
	copy( pcRec, hhRec );

	Q_ASSERT( equal( pcRec, hhRec ) );

	return hhRec;
}

void MemofileConduit::_copy( const Record *from, HHRecord *to )
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

void MemofileConduit::_copy( const HHRecord* from, Record *to  )
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
