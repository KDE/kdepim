/* contactshhdataproxy.cc			KPilot
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

#include "contactshhdataproxy.h"

#include "contactshhrecord.h"
#include "options.h"
#include "pilotAddress.h"
#include "pilottophonemap.h"
#include "pilotRecord.h"

class ContactsHHDataProxy::Private
{
public:
	Private() : fAddressInfo( 0L )
	{
	}
	
	PilotAddressInfo* fAddressInfo;
};


bool ContactsHHDataProxy::createDataStore()
{
	// TODO: Implement
	return false;
}

ContactsHHDataProxy::ContactsHHDataProxy( PilotDatabase *db ) : HHDataProxy( db )
	, d( new Private )
{
}

HHRecord* ContactsHHDataProxy::createHHRecord( PilotRecord *rec )
{
	QString category = fAppInfo->categoryName( rec->category() );
	return new ContactsHHRecord( rec, category );
}

PilotAppInfoBase* ContactsHHDataProxy::readAppInfo()
{
	FUNCTIONSETUP;
	
	if( fDatabase && fDatabase->isOpen() )
	{
		d->fAddressInfo = new PilotAddressInfo( fDatabase );
		
		return d->fAddressInfo;
	}

	return 0;
}

void ContactsHHDataProxy::setPhoneNumbers( PilotAddress &a
	, const KABC::PhoneNumber::List &list )
{
	FUNCTIONSETUP;

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
				DEBUGKPILOT << "Found pilot type: ["
					<< pilotPhoneType << "] ("
					<< d->fAddressInfo->phoneLabel( (PilotAddressInfo::EPhoneType)pilotPhoneType)
					<< ") for PhoneNumber: ["
					<< phone.number() << ']';

				phoneType = (PilotAddressInfo::EPhoneType) pilotPhoneType;
				break;
			}
		}
		PhoneSlot fieldSlot =
			a.setPhoneField(phoneType, phone.number(), PilotAddress::NoFlags);

		// if this is the preferred phone number, then set it as such
		if (fieldSlot.isValid() && (phone.type() & KABC::PhoneNumber::Pref))
		{
			DEBUGKPILOT << "Found preferred PhoneNumber."
				<< "setting showPhone to index: ["
				<< fieldSlot << "], PhoneNumber: ["
				<< phone.number() << ']';
			a.setShownPhone( fieldSlot );
		}

		if (!fieldSlot.isValid())
		{
			DEBUGKPILOT << "Phone listing overflowed.";
		}
	}

	DEBUGKPILOT << "Pilot's showPhone now: [" << a.getShownPhone() << ']';

	// after setting the numbers, make sure that something sensible is set as the
	// shownPhone on the Pilot if nothing is yet...
	QString pref = a.getField(a.getShownPhone());
	if (!a.getShownPhone().isValid() || pref.isEmpty())
	{
		DEBUGKPILOT << "Pilot's showPhone: ["
			<< a.getShownPhone()
			<< "] not properly set to a default.";

		for (PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i)
		{
			pref = a.getField(i);
			if (!pref.isEmpty())
			{
				a.setShownPhone( i );
				DEBUGKPILOT << "Pilot's showPhone now: ["
					<< a.getShownPhone()
					<< "], and that's final.";
				break;
			}
		}
	}
}
