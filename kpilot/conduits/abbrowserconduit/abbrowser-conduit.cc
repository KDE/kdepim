/* abbrowser-conduit.cc                           KPilot
**
** Copyright (C) 2000,2001 by Dan Pilone
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
** The abbrowser conduit copies addresses from the Pilot's address book to
** the KDE addressbook maintained via the kabc library.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/



#include "options.h"
#include "abbrowser-conduit.moc"

#include <unistd.h>

#include <qtimer.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>
#include <qtextcodec.h>
#include <time.h>


#include <kglobal.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/resource.h>
//#include <kabc/resourcefile.h>
#include <kresources/factory.h>

#include <pilotUser.h>
#include <pilotSerialDatabase.h>

#include "abbrowser-factory.h"
#include "resolutionDialog.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
const char *abbrowser_conduit_id="$Id$";

using namespace KABC;

const QString AbbrowserConduit::flagString=CSL1("Flag");
const QString AbbrowserConduit::appString=CSL1("KPILOT");
const QString AbbrowserConduit::idString=CSL1("RecordID");

bool AbbrowserConduit::fPilotStreetHome=true;
bool AbbrowserConduit::fPilotFaxHome=true;
enum AbbrowserConduit::ePilotOtherEnum AbbrowserConduit::ePilotOther=AbbrowserConduit::eOtherPhone;

enum AbbrowserConduit::eCustomEnum AbbrowserConduit::eCustom[4] = {
	AbbrowserConduit::eCustomField,
	AbbrowserConduit::eCustomField,
	AbbrowserConduit::eCustomField,
	AbbrowserConduit::eCustomField
	} ;



/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/





AbbrowserConduit::AbbrowserConduit(KPilotDeviceLink * o, const char *n, const QStringList & a):
		ConduitAction(o, n, a),
		addresseeMap(),
		syncedIds(),
		aBook(0L),
		abiter(),
		ticket(0L)
{
  FUNCTIONSETUP;
#ifdef DEBUG
  DEBUGCONDUIT<<abbrowser_conduit_id<<endl;
#endif
}



AbbrowserConduit::~AbbrowserConduit()
{
  FUNCTIONSETUP;
}





/*********************************************************************
                L O A D I N G   T H E   D A T A
 *********************************************************************/





/* Builds the map which links record ids to uid's of KABC::Addressee
*/
void AbbrowserConduit::_mapContactsToPilot(QMap < recordid_t, QString > &idContactMap) const
{
	FUNCTIONSETUP;

	idContactMap.clear();

	for(KABC::AddressBook::Iterator contactIter = aBook->begin();
		contactIter != aBook->end(); ++contactIter)
	{
		KABC::Addressee aContact = *contactIter;
		QString recid = aContact.custom(appString, idString);
		if(!recid.isEmpty())
		{
			recordid_t id = recid.toULong();
			idContactMap.insert(id, aContact.uid());
		}
	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Loaded " << idContactMap.size() <<
	    " addresses from the addressbook. " << endl;
#endif
}



bool AbbrowserConduit::_prepare()
{
	FUNCTIONSETUP;

	readConfig();
	syncedIds.clear();

	return true;
}



void AbbrowserConduit::readConfig()
{
	FUNCTIONSETUP;

	KConfigGroupSaver g(fConfig, AbbrowserConduitFactory::group());

	// General page
	fAbookType = (eAbookTypeEnum)fConfig->readNumEntry(
		AbbrowserConduitFactory::fAbookType, 0);
	fAbookFile = fConfig->readEntry(
		AbbrowserConduitFactory::fAbookFile);
	fArchive=fConfig->readBoolEntry(
		AbbrowserConduitFactory::fArchive, true);

	// Conflict page
	SyncAction::eConflictResolution res=(SyncAction::eConflictResolution)fConfig->readNumEntry(
		AbbrowserConduitFactory::fResolution, SyncAction::eUseGlobalSetting);
	if (res!=SyncAction::eUseGlobalSetting) fConflictResolution=res;
	fSmartMerge=fConfig->readBoolEntry(
		AbbrowserConduitFactory::fSmartMerge, true);

	// Fields page
	fPilotStreetHome=!fConfig->readBoolEntry(
		AbbrowserConduitFactory::fStreetType, true);
	fPilotFaxHome=!fConfig->readBoolEntry(
		AbbrowserConduitFactory::fFaxType, true);
	ePilotOther=(ePilotOtherEnum)(fConfig->readNumEntry(
		AbbrowserConduitFactory::fOtherField, eOtherPhone));

	// Custom fields page
	for (int i=0; i<4; i++)
	{
		eCustom[i]=(eCustomEnum)(fConfig->readNumEntry(
			AbbrowserConduitFactory::custom(i), eCustomField) );
		if (eCustom[i]==eCustomBirthdate) eCustom[i]=eCustomField;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Settings "
		<< " fSmartMerge=" << fSmartMerge
		<< " fConflictResolution=" << fConflictResolution
		<< " fPilotStreetHome=" << fPilotStreetHome
		<< " fPilotFaxHome=" << fPilotFaxHome
		<< " fArchive=" << fArchive
		<< " eCustom[0]=" << eCustom[0]
		<< " eCustom[1]=" << eCustom[1]
		<< " eCustom[2]=" << eCustom[2]
		<< " eCustom[3]=" << eCustom[3]
		<< " fFirstTime=" << isFirstSync() << endl;
#endif
}



bool AbbrowserConduit::_loadAddressBook()
{
	FUNCTIONSETUP;
	KConfigGroupSaver g(fConfig, AbbrowserConduitFactory::group());
	switch (fAbookType)
	{
		case eAbookResource:
			aBook = KABC::StdAddressBook::self();
			break;
		case eAbookLocal: { // initialize the abook with the given file
			aBook = new KABC::AddressBook();
			KRES::Factory*resfact=KRES::Factory::self("contact");
			if (aBook && resfact)
			{
				// just give the config object of the conduit, because the filename
				// is stored there under the key "FileName", just as the ResourceFile
				// class expects it to be (I'd like it much more if I could just give
				// the filename instead of having to use a config file)
				KRES::Resource*rawres=resfact->resource("file", fConfig);
				KABC::Resource*abookres=dynamic_cast<KABC::Resource*>(rawres);
				if (abookres)
				{
					abookres->setAddressBook(aBook);
					aBook->addResource(abookres);
//					aBook->setStandardResource(abookres);
					ticket=abookres->requestSaveTicket();
				}
				if (!abookres || !ticket)
				{
					kdWarning()<<k_funcinfo<<": Unable to lock addressbook resource "
						<<"for file "<<fAbookFile<<endl;
					aBook->cleanUp();
					// TODO: Do I need to  delete the ticket manually?
					KPILOT_DELETE(ticket);
					KPILOT_DELETE(rawres);
					KPILOT_DELETE(aBook);
					return false;
				}
			}
			break;}
		default: break;
	}
	if (!aBook )
	{
		// Something went wrong, so tell the user and return false to exit the conduit
		kdWarning()<<k_funcinfo<<": Unable to initialize the addressbook for the sync."<<endl;
		KPILOT_DELETE(aBook);
		return false;
	}
	// TODO: find out if this can fail for reasons other than a non-existent
	// vcf file. If so, how can I determine if the missing file was the problem
	// or something more serious:
	aBook->load();
	abChanged = false;
	// get the addresseMap which maps Pilot unique record(address) id's to
	// a Abbrowser KABC::Addressee; allows for easy lookup and comparisons
	if(aBook->begin() == aBook->end())
	{
		fFirstSync = true;
	}
	else
	{
		_mapContactsToPilot(addresseeMap);
	}
	return(aBook != 0L);
}



bool AbbrowserConduit::_saveAddressBook()
{
	FUNCTIONSETUP;

	bool res=false;

	switch (fAbookType)
	{
		case eAbookResource:
			if (abChanged) res==StdAddressBook::save();
//			if (aBook) aBook->cleanUp();
			break;
		case eAbookLocal: // initialize the abook with the given file
			if (ticket)
			{
				if (abChanged && aBook) res=aBook->save(ticket);
			}
			else
			{
				kdWarning()<<k_funcinfo<<": No ticket available to save the "
				<<"addressbook."<<endl;
			}
			// Don't break, let the default cleanUp and delete the addressbook
		default:
			if (aBook) aBook->cleanUp();
			KPILOT_DELETE(aBook);
			break;
	}

	return false;
}



void AbbrowserConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	int appLen = pack_AddressAppInfo(&fAddressAppInfo, 0, 0);
	unsigned char *buffer = new unsigned char[appLen];
	pack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);
	if (fDatabase) fDatabase->writeAppBlock(buffer, appLen);
	if (fLocalDatabase) fLocalDatabase->writeAppBlock(buffer, appLen);
	delete[] buffer;
}
void AbbrowserConduit::_getAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	unsigned char *buffer = new unsigned char[PilotAddress::APP_BUFFER_SIZE];
	int appLen=fDatabase->readAppBlock(buffer, PilotAddress::APP_BUFFER_SIZE);

	unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);
	delete[]buffer;
	buffer = NULL;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " lastUniqueId" << fAddressAppInfo.category.lastUniqueID << endl;
	for(int i = 0; i < 16; i++)
	{
		DEBUGCONDUIT << fname << " cat " << i << " =" << fAddressAppInfo.category.name[i] << endl;
	}

	for(int x = 0; x < 8; x++)
	{
		DEBUGCONDUIT << fname << " phone[" << x << "] = " << fAddressAppInfo.phoneLabels[x] << endl;
	}
#endif
}

QString AbbrowserConduit::getCustomField(const KABC::Addressee &abEntry, const int index)
{
//	FUNCTIONSETUP;
//	return abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));

	switch (eCustom[index]) {
		case eCustomBirthdate: {
			// TODO: The date things do  not work yet, so disable this for now...
			return abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));


			QDateTime bdate(abEntry.birthday().date());
			if (!bdate.isValid()) {
				// TODO
				return QString::null;
			}

			time_t btime=bdate.toTime_t();
			struct tm*btmtime=localtime(&btime);
			char datestr[500];
			size_t len=strftime(&datestr[0], 500, "%x", btmtime);
			return QString(&datestr[0]);
			break;
		}
		case eCustomURL:
			return abEntry.url().url();
			break;
		case eCustomIM:
			return abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"));
			break;
		case eCustomField:
		default:
			return abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));
			break;
	}
}


void AbbrowserConduit::setCustomField(KABC::Addressee &abEntry,  int index, QString cust)
{
	FUNCTIONSETUP;
//	return abEntry.insertCustom(appString, CSL1("CUSTOM")+QString::number(index), cust);

	// TODO: The date things do  not work yet, so disable this for now...
	switch (eCustom[index]) {
		case eCustomBirthdate: {
			return abEntry.insertCustom(appString, CSL1("CUSTOM")+QString::number(index), cust);
			QDate bdate=QDate::fromString(cust);
#ifdef DEBUG
			DEBUGCONDUIT<<bdate.toString()<<endl;
			DEBUGCONDUIT<<"Is Valid: "<<bdate.isValid()<<endl;
#endif
			if (bdate.isValid()) return abEntry.setBirthday(QDateTime(bdate));
			break;}
		case eCustomURL: {
			return abEntry.setUrl(cust);
			break;}
		case eCustomIM: {
			return abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"), cust);
			break;}
		case eCustomField:
		default: {
			return abEntry.insertCustom(appString, CSL1("CUSTOM")+QString::number(index), cust);
			break;}
	}
	return;
}


QString AbbrowserConduit::getOtherField(const KABC::Addressee & abEntry)
{
	switch(ePilotOther)
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


void AbbrowserConduit::setOtherField(KABC::Addressee & abEntry, QString nr)
{
	KABC::PhoneNumber phone;
	switch(ePilotOther)
	{
		case eOtherPhone:
			phone = abEntry.phoneNumber(0);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eAssistant:
			abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"), nr);
			break;
		case eBusinessFax:
			phone = abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eCarPhone:
			phone = abEntry.phoneNumber(KABC::PhoneNumber::Car);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eEmail2:
			return abEntry.insertEmail(nr);
		case eHomeFax:
			phone = abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eTelex:
			phone = abEntry.phoneNumber(KABC::PhoneNumber::Bbs);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eTTYTTDPhone:
			phone = abEntry.phoneNumber(KABC::PhoneNumber::Pcs);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
	}
}


KABC::PhoneNumber AbbrowserConduit::getFax(const KABC::Addressee & abEntry)
{
	return abEntry.phoneNumber(KABC::PhoneNumber::Fax |
		( (fPilotFaxHome) ?(KABC::PhoneNumber::Home) :(KABC::PhoneNumber::Work)));
}


KABC::Address AbbrowserConduit::getAddress(const KABC::Addressee & abEntry)
{
	return abEntry.address((fPilotStreetHome) ?(KABC::Address::Home) :(KABC::Address::Work));
}




/*********************************************************************
                     D E B U G   O U T P U T
 *********************************************************************/





#ifdef DEBUG
void AbbrowserConduit::showAddressee(const KABC::Addressee & abAddress)
{
	FUNCTIONSETUP;
	DEBUGCONDUIT << "\tAbbrowser Contact Entry" << endl;
	DEBUGCONDUIT << "\t\tLast name = " << abAddress.familyName() << endl;
	DEBUGCONDUIT << "\t\tFirst name = " << abAddress.givenName() << endl;
	DEBUGCONDUIT << "\t\tCompany = " << abAddress.organization() << endl;
	DEBUGCONDUIT << "\t\tJob Title = " << abAddress.title() << endl;
	DEBUGCONDUIT << "\t\tNote = " << abAddress.note() << endl;
	DEBUGCONDUIT << "\t\tHome phone = " << abAddress.phoneNumber(KABC::PhoneNumber::Home).number() << endl;
	DEBUGCONDUIT << "\t\tWork phone = " << abAddress.phoneNumber(KABC::PhoneNumber::Work).number() << endl;
	DEBUGCONDUIT << "\t\tMobile phone = " << abAddress.phoneNumber(KABC::PhoneNumber::Cell).number() << endl;
	DEBUGCONDUIT << "\t\tEmail = " << abAddress.preferredEmail() << endl;
	DEBUGCONDUIT << "\t\tFax = " << getFax(abAddress).number() << endl;
	DEBUGCONDUIT << "\t\tPager = " << abAddress.phoneNumber(KABC::PhoneNumber::Pager).number() << endl;
	DEBUGCONDUIT << "\t\tCategory = " << abAddress.categories().first() << endl;
}



void AbbrowserConduit::showPilotAddress(const PilotAddress & pilotAddress)
{
	FUNCTIONSETUP;
	DEBUGCONDUIT << "\tPilot Address" << endl;
	DEBUGCONDUIT << "\t\tLast name = " << pilotAddress.getField(entryLastname) << endl;
	DEBUGCONDUIT << "\t\tFirst name = " << pilotAddress.getField(entryFirstname) << endl;
	DEBUGCONDUIT << "\t\tCompany = " << pilotAddress.getField(entryCompany) << endl;
	DEBUGCONDUIT << "\t\tJob Title = " << pilotAddress.getField(entryTitle) << endl;
	DEBUGCONDUIT << "\t\tNote = " << pilotAddress.getField(entryNote) << endl;
	DEBUGCONDUIT << "\t\tHome phone = " << pilotAddress.getPhoneField(PilotAddress::eHome) << endl;
	DEBUGCONDUIT << "\t\tWork phone = " << pilotAddress.getPhoneField(PilotAddress::eWork) << endl;
	DEBUGCONDUIT << "\t\tMobile phone = " << pilotAddress.getPhoneField(PilotAddress::eMobile) << endl;
	DEBUGCONDUIT << "\t\tEmail = " << pilotAddress.getPhoneField(PilotAddress::eEmail) << endl;
	DEBUGCONDUIT << "\t\tFax = " << pilotAddress.getPhoneField(PilotAddress::eFax) << endl;
	DEBUGCONDUIT << "\t\tPager = " << pilotAddress.getPhoneField(PilotAddress::ePager) << endl;
	DEBUGCONDUIT << "\t\tOther = " << pilotAddress.getPhoneField(PilotAddress::eOther) << endl;
	DEBUGCONDUIT << "\t\tCategory = " << pilotAddress.getCategoryLabel() << endl;
}
#endif





/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/





/* virtual */ bool AbbrowserConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<abbrowser_conduit_id<<endl;

	if(!fConfig)
	{
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		emit logError(i18n("Unable to load configuration of the addressbook conduit."));
		return false;
	}

	_prepare();

	fFirstSync = false;
	// Database names probably in latin1.
	if(!openDatabases(QString::fromLatin1("AddressDB"), &fFirstSync))
	{
		emit logError(i18n("Unable to open the addressbook databases on the handheld."));
		return false;
	}
	_getAppInfo();
	if(!_loadAddressBook())
	{
		emit logError(i18n("Unable to open the addressbook."));
		return false;
	}
	fFirstSync = fFirstSync || (aBook->begin() == aBook->end());

	// perform syncing from palm to abbrowser
	// iterate through all records in palm pilot
	pilotindex = 0;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": fullsync=" << isFullSync() << ", firstSync=" <<    isFirstSync() << endl;
	DEBUGCONDUIT << fname << ": syncAction=" << syncAction << ", archive = " << fArchive << endl;
	DEBUGCONDUIT << fname << ": smartmerge=" << fSmartMerge << ", conflictRes="<< fConflictResolution << endl;
	DEBUGCONDUIT << fname << ": PilotStreetHome=" << fPilotStreetHome << ", PilotFaxHOme" << fPilotFaxHome << endl;
#endif

	QTimer::singleShot(0, this, SLOT(syncPalmRecToPC()));
	// TODO: maybe start a second timer to kill the sync after, say, 5 Minutes(e.g. non existent slot called...)
	return true;
}



void AbbrowserConduit::syncPalmRecToPC()
{
	FUNCTIONSETUP;
	PilotRecord *r = 0L, *s = 0L;

	if(isFullSync())
	{
		r = fDatabase->readRecordByIndex(pilotindex++);
	}
	else
	{
		r = dynamic_cast <PilotSerialDatabase * >(fDatabase)->readNextModifiedRec();
	}

	if(!r)
	{
		abiter = aBook->begin();
		QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
		return;
	}
	else
	{
		// already synced, so skip:
		if(syncedIds.contains(r->getID()))
		{
#ifdef DEBUG
			DEBUGCONDUIT << "already synced, so skipping" << endl;
#endif
			QTimer::singleShot(0, this, SLOT(syncPalmRecToPC()));
			return;
		}
	}

	bool archiveRecord =(r->getAttrib() & dlpRecAttrArchived);

	KABC::Addressee e;
	s = fLocalDatabase->readRecordById(r->getID());
	if(!s)
	{
		e = _findMatch(PilotAddress(fAddressAppInfo, r));
	}

	if((!s && e.isEmpty()) || isFirstSync())
	{
		// doesn't exist on PC. Either not deleted at all, or deleted with the archive flag on.
		if(!r->isDeleted() ||(fArchive && archiveRecord))
		{
			e = _addToPC(r);
			if(fArchive && archiveRecord && !e.isEmpty() )
			{
				e.insertCustom(appString, flagString, QString::number(SYNCDEL));
				_saveAbEntry(e);
			}
		}
	}
	else
	{
		// if the entry should be archived, it is not marked deleted!!!
		if(r->isDeleted())
		{
			_checkDelete(r,s);
		}
		else
		{
			// if archived, either delete or change and add archived flag. Otherwise just change
			if (archiveRecord && !fArchive)
			{
				// Archived, but archived records are not supposed to be synced, so delete
				_checkDelete(r, s);
			}
			else
			{
				e = _changeOnPC(r, s);
				if(fArchive && archiveRecord && !e.isEmpty() )
				{
					e.insertCustom(appString, flagString, QString::number(SYNCDEL));
					_saveAbEntry(e);
				}
			}
		}
	}

	syncedIds.append(r->getID());
	KPILOT_DELETE(r);
	KPILOT_DELETE(s);

	QTimer::singleShot(0, this, SLOT(syncPalmRecToPC()));
}



void AbbrowserConduit::syncPCRecToPalm()
{
	FUNCTIONSETUP;

	if(abiter == aBook->end() ||(*abiter).isEmpty())
	{
		pilotindex = 0;
		QTimer::singleShot(0, this, SLOT(syncDeletedRecord()));
		return;
	}
	bool ok;
	KABC::Addressee ad = *abiter;
	abiter++;
	QString recID(ad.custom(appString, idString));
	recordid_t rid = recID.toLong(&ok);
	if(recID.isEmpty() || !ok || !rid)
	{
		// it's a new item(no record ID and not inserted by the Palm -> PC sync), so add it
		_addToPalm(ad);
		QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
		return;
	}
	// look into the list of already synced record ids to see if the addressee hasn't already been synced
	else if(syncedIds.contains(rid))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": address with id " << rid <<
			" already synced." << endl;
#endif
		QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
		return;
	}
	if(ad.custom(appString, flagString) == QString::number(SYNCDEL))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": address with id " << rid <<
			" marked archived, so don't sync." << endl;
#endif
		syncedIds.append(rid);
		QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
		return;
	}


	PilotRecord *backup = fLocalDatabase->readRecordById(rid);
	// only update if no backup record or the backup record is not equal to the addressee
	PilotAddress pbackupadr(fAddressAppInfo, backup);
	if(!backup || !_equal(pbackupadr, ad) || isFirstSync() )
	{
		PilotRecord *rec = fDatabase->readRecordById(rid);
		if (!rec && !backup)
		{
			// Entry exists neither on the handheld not in the backup database, although the addressbook entry has a KPilot-ID stored. Assume
			// this comes from a sync with a different Handheld, so delete the handheld ID and restart work on the current item.
			// It's rather unlikely that the entry was deleted from the handheld and the backup database at the same time without
			// being deleted from the addressbook. This can only be the case if the last sync on this computer was done with a
			// different addressbook. In this situation, however it's impossible to decide if that record comes from a sync
			// with a different Handheld or from a deleted record that was synced with a different addressbook.

#ifdef DEBUG
			DEBUGCONDUIT<<"Addressbook entry "<<ad.realName()<<" has a non-existent Record-ID "<<ad.custom(appString, idString)<<", so disregard that ID"<<endl;
#endif
			ad.removeCustom(appString, idString);
			_saveAbEntry(ad);
			abiter--;
			// No need to KPILOT_DELETE anything, as the records could not be created anyway
			QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
			return;
		}

		if(!rec)
		{
			if(isFirstSync()) _addToPalm(ad);
			else _checkDelete(rec, backup);
		}
		else
		{
			// no conflict, just update the record on the handheld
			_changeOnPalm(rec, backup, ad);
		}
		KPILOT_DELETE(rec);
	}
	KPILOT_DELETE(backup);
	syncedIds.append(rid);
	// done with the sync process, go on with the next one:
	QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
}



void AbbrowserConduit::syncDeletedRecord()
{
	FUNCTIONSETUP;

	PilotRecord *s = fLocalDatabase->readRecordByIndex(pilotindex++);
	if(!s || isFirstSync() )
	{
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}

	// already synced, so skip this record:
	if(syncedIds.contains(s->getID()))
	{
		QTimer::singleShot(0, this, SLOT(syncDeletedRecord()));
		return;
	}

	QString uid = addresseeMap[s->getID()];
	KABC::Addressee e = aBook->findByUid(uid);
	if(uid.isEmpty() || e.isEmpty())
	{
#ifdef DEBUG
		DEBUGCONDUIT << "Item " << s->getID() << " deleted from the PC, so delete from Palm too (or do deconfliction)!" << endl;
#endif

		// entry was deleted from addressbook, so delete it from the palm
		// First find out if changed on Palm, and if so, do a deconfliction.
		// This is the case if an entry was changed on the handheld and deleted from the pc.
		// If the user chooses to ignore the conflict, on the next sync we have to check for that
		// conflict here again...
		PilotRecord*r=fDatabase->readRecordById(s->getID());
		PilotAddress adr(fAddressAppInfo, r), backadr(fAddressAppInfo, s);
		if (r && s && !(adr==backadr) )
		{
			_changeOnPC(r, s);
		}
		else
		{
			_deleteFromPalm(s);
		}
		KPILOT_DELETE(r);
	}

	KPILOT_DELETE(s);
	QTimer::singleShot(0, this, SLOT(syncDeletedRecord()));
}

void AbbrowserConduit::cleanup()
{
	FUNCTIONSETUP;

	// Set the appInfoBlock, just in case the category labels changed
	_setAppInfo();
	if(fDatabase)
	{
		fDatabase->resetSyncFlags();
		fDatabase->cleanup();
	}
	if(fLocalDatabase)
	{
		fLocalDatabase->resetSyncFlags();
		fLocalDatabase->cleanup();
	}
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
	_saveAddressBook();
	// TODO: Do I need to free the addressbook?????
//	aBook->cleanUp();
//	KPILOT_DELETE(aBook);
	emit syncDone(this);
}





/*********************************************************************
                 l o w - l e v e l   f u n c t i o n s   f o r
                   adding / removing palm/pc records
 *********************************************************************/





void AbbrowserConduit::_removePilotAddress(PilotAddress & address)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " deleting from palm pilot " << endl;
	showPilotAddress(address);
#endif

	address.makeDeleted();
	PilotRecord *pilotRec = address.pack();
	_deleteFromPalm(pilotRec);
	delete pilotRec;
}



void AbbrowserConduit::_removeAbEntry(KABC::Addressee addressee)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " removing " << addressee.formattedName() << endl;
#endif
	abChanged = true;
	aBook->removeAddressee(addressee);
}



KABC::Addressee AbbrowserConduit::_saveAbEntry(KABC::Addressee & abEntry)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT<<"Before _saveAbEntry, abEntry.custom="<<abEntry.custom(appString, idString)<<endl;
#endif
	if(!abEntry.custom(appString, idString).isEmpty())
	{
		addresseeMap.insert(abEntry.custom(appString, idString).toLong(), abEntry.uid());
	}
#ifdef DEBUG
	DEBUGCONDUIT<<"Before insertAddressee, abEntry.custom="<<abEntry.custom(appString, idString)<<endl;
#endif

	aBook->insertAddressee(abEntry);
#ifdef DEBUG
	DEBUGCONDUIT<<"After insertAddressee, abEntry.custom="<<abEntry.custom(appString, idString)<<endl;
#endif

	abChanged = true;
	return abEntry;
}



bool AbbrowserConduit::_savePilotAddress(PilotAddress & address, KABC::Addressee & abEntry)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname <<
		" saving to pilot " << address.id()
		<< " " << address.getField(entryFirstname)
		<< " " << address.getField(entryLastname)
		<< " " << address.getField(entryCompany) << endl;
#endif

	PilotRecord *pilotRec = address.pack();
#ifdef DEBUG
	DEBUGCONDUIT<<"PilotRec vor writeRecord: ID="<<pilotRec->getID()<<", address.id="<<address.id()<<endl;
#endif
	recordid_t pilotId = fDatabase->writeRecord(pilotRec);
#ifdef DEBUG
	DEBUGCONDUIT<<"PilotRec nach writeRecord ("<<pilotId<<": ID="<<pilotRec->getID()<<endl;
#endif
	pilotRec->setID(pilotId);
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);

	// pilotId == 0 if using local db, so don't overwrite the valid id
	if(pilotId != 0) address.setID(pilotId);

	recordid_t abId = 0;

	abId = abEntry.custom(appString, idString).toUInt();
	if(abId != pilotId)
	{
		abEntry.insertCustom(appString, idString, QString::number(pilotId));
		return true;
	}

	return false;
}






bool AbbrowserConduit::_saveBackupAddress(PilotAddress & backup)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	showPilotAddress(backup);
#endif
	PilotRecord *pilotRec = backup.pack();
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);
	return true;
}






KABC::Addressee AbbrowserConduit::_addToAbbrowser(const PilotAddress & address)
{
	FUNCTIONSETUP;
	KABC::Addressee entry;

// If a record has been deleted on pda without archiving option,
// flags modify and deleted will be set but the contents are empty.
// We shouldn't add such zombies to database:
	if((address.isModified() && address.isDeleted())
			&&(address.getField(entryLastname).isEmpty())
			&&(address.getField(entryFirstname).isEmpty()))
		return entry;

	_copy(entry, address);
	return _saveAbEntry(entry);
}





/*********************************************************************
              A D D   /   C H A N G E   R E C O R D S
 *********************************************************************/





// -----------------------------------------------------------
// Palm => PC
// -----------------------------------------------------------

KABC::Addressee AbbrowserConduit::_addToPC(PilotRecord * r)
{
	return _changeOnPC(r, NULL);
}



KABC::Addressee AbbrowserConduit::_changeOnPC(PilotRecord * rec, PilotRecord * backup)
{
	FUNCTIONSETUP;
	PilotAddress padr(fAddressAppInfo, rec);
#ifdef DEBUG
	showPilotAddress(padr);
#endif
	struct AddressAppInfo ai = fAddressAppInfo;
	PilotAddress pbackupadr(ai, backup);
	KABC::Addressee ad;

#ifdef DEBUG
	DEBUGCONDUIT << "---------------------------------" << endl;
	DEBUGCONDUIT << "Now syncing " <<
		padr.getField(entryFirstname) << " " <<
		padr.getField(entryLastname) << " / backup: " <<
		pbackupadr.getField(entryFirstname) << " " <<
		pbackupadr.getField(entryLastname) << endl;
#endif

	if(backup) ad = _findMatch(pbackupadr);
	if(ad.isEmpty()) ad = _findMatch(padr);
#ifdef DEBUG
	DEBUGCONDUIT << "ad.custom=" << ad.custom(appString, idString) << endl;
#endif

	if(ad.isEmpty())
	{
#ifdef DEBUG
		DEBUGCONDUIT << "ad.isEmpty() " << endl;
#endif
		if(!backup)
		{
			// not found, so add
			ad = _addToAbbrowser(padr);
			fLocalDatabase->writeRecord(rec);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT <<
				"not a new entry, but PC entry does not exist => deconfliction of "
				<< padr.getField(entryLastname) << endl;
#endif
			KABC::Addressee ab;
			switch(getEntryResolution(ad, pbackupadr, padr))
			{
				case SyncAction::eHHOverrides:
					_addToAbbrowser(padr);
					break;
				case SyncAction::ePCOverrides:
					_removePilotAddress(padr);
					break;
				case SyncAction::ePreviousSyncOverrides:
					ab = _addToAbbrowser(pbackupadr);
					if(_savePilotAddress(pbackupadr, ab)) _saveAbEntry(ab);
					break;
				case SyncAction::eDoNothing:
				default:
					break;
			}
		}
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << "!ad.isEmpty()" << endl;
		showAddressee(ad);
#endif
		PilotAddress backupadr(fAddressAppInfo, backup);
		_mergeEntries(padr, backupadr, ad);

	}
	return ad;
}



bool AbbrowserConduit::_deleteOnPC(PilotRecord * rec, PilotRecord * backup)
{
	FUNCTIONSETUP;
	recordid_t id;
	if(rec) id = rec->getID();
	else if(backup) id = backup->getID();
	else id = 0;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": deleting record with id " << id << ", rec " <<
		(rec != NULL) << ", backup " <<(backup != NULL) << endl;
#endif
	if(!id) return false;

	KABC::Addressee ad = aBook->findByUid(addresseeMap[id]);
	PilotAddress backupAdr(fAddressAppInfo, backup);

	if((!backup) || !_equal(backupAdr, ad))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": record with id " << id <<
			" is either new on PC or was changed, but is is requested to be removed. Ignoring this request"
			<< endl;
#endif
		// TODO: Conflict!!!
	}
	if(!ad.isEmpty())
	{
		_removeAbEntry(ad);
		//aBook->removeAddressee(ad);
	}
	if(!rec)
	{
		backup->makeDeleted();	//setAttrib(backup->getAttrib() | dlpRecAttrDeleted );
		fLocalDatabase->writeRecord(backup);
	}
	else
	{
		fLocalDatabase->writeRecord(rec);
	}
	return true;
}



// -----------------------------------------------------------
// PC => Palm
// -----------------------------------------------------------



void AbbrowserConduit::_addToPalm(KABC::Addressee & entry)
{
	FUNCTIONSETUP;
	PilotAddress pilotAddress(fAddressAppInfo);

	_copy(pilotAddress, entry);
#ifdef DEBUG
	DEBUGCONDUIT<<"pilotAddress.id="<<pilotAddress.getID()<<", abEntry.ID="<<entry.custom(appString, idString)<<endl;
#endif

	if(_savePilotAddress(pilotAddress, entry))
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"Vor _saveAbEntry, pilotAddress.id="<<pilotAddress.getID()<<", abEntry.ID="<<entry.custom(appString, idString)<<endl;
#endif
		_saveAbEntry(entry);
	}
}



void AbbrowserConduit::_changeOnPalm(PilotRecord * rec, PilotRecord * backuprec, KABC::Addressee & ad)
{
	FUNCTIONSETUP;
	PilotAddress padr(fAddressAppInfo);
	PilotAddress pbackupadr(fAddressAppInfo);

	if(rec) padr = PilotAddress(fAddressAppInfo, rec);
	if(backuprec) pbackupadr = PilotAddress(fAddressAppInfo, backuprec);
#ifdef DEBUG
	DEBUGCONDUIT << "---------------------------------" << endl;
	DEBUGCONDUIT << "Now syncing " << padr.getField(entryLastname) << " / backup: " << pbackupadr.getField(entryLastname) << endl;
#endif
	_mergeEntries(padr, pbackupadr, ad);
}



void AbbrowserConduit::_deleteFromPalm(PilotRecord * rec)
{
	FUNCTIONSETUP;
	rec->makeDeleted();
	recordid_t pilotId = fDatabase->writeRecord(rec);
	rec->setID(pilotId);
	fLocalDatabase->writeRecord(rec);
	syncedIds.append(rec->getID());
}




/*********************************************************************
                   C O P Y   R E C O R D S
 *********************************************************************/





bool AbbrowserConduit::_equal(const PilotAddress & piAddress, KABC::Addressee & abEntry) const
{
	if(_compare(abEntry.familyName(), piAddress.getField(entryLastname))) return false;
	if(_compare(abEntry.givenName(), piAddress.getField(entryFirstname))) return false;
	if(_compare(abEntry.title(), piAddress.getField(entryTitle))) return false;
	if(_compare(abEntry.organization(), piAddress.getField(entryCompany))) return false;
	if(_compare(abEntry.note(), piAddress.getField(entryNote))) return false;

	QString cat = _getCatForHH(abEntry.categories(), piAddress.getCategoryLabel());
	if(_compare(cat, piAddress.getCategoryLabel())) return false;

	if(_compare(abEntry.phoneNumber(KABC::PhoneNumber::Work).number(),
			piAddress.getPhoneField(PilotAddress::eWork))) return false;
	if(_compare(abEntry.phoneNumber(KABC::PhoneNumber::Home).number(),
			piAddress.getPhoneField(PilotAddress::eHome))) return false;
	if(_compare(getOtherField(abEntry),
			piAddress.getPhoneField(PilotAddress::eOther))) return false;
	if(_compare(abEntry.preferredEmail(),
			piAddress.getPhoneField(PilotAddress::eEmail))) return false;
	if(_compare(getFax(abEntry).number(),
			piAddress.getPhoneField(PilotAddress::eFax))) return false;
	if(_compare(abEntry.phoneNumber(KABC::PhoneNumber::Cell).number(),
			piAddress.getPhoneField(PilotAddress::eMobile))) return false;

	KABC::Address address = getAddress(abEntry);
	if(_compare(address.street(), piAddress.getField(entryAddress))) return false;
	if(_compare(address.locality(), piAddress.getField(entryCity))) return false;
	if(_compare(address.region(), piAddress.getField(entryState))) return false;
	if(_compare(address.postalCode(), piAddress.getField(entryZip))) return false;
	if(_compare(address.country(), piAddress.getField(entryCountry))) return false;

	if(
		_compare(getCustomField(abEntry, 0), piAddress.getField(entryCustom1)) ||
		_compare(getCustomField(abEntry, 1), piAddress.getField(entryCustom2)) ||
		_compare(getCustomField(abEntry, 2), piAddress.getField(entryCustom3)) ||
		_compare(getCustomField(abEntry, 3), piAddress.getField(entryCustom4)))
	{
		return false;
	}

	return true;
}



void AbbrowserConduit::_copy(PilotAddress & toPilotAddr, KABC::Addressee & fromAbEntry)
{
	FUNCTIONSETUP;
	// don't do a reset since this could wipe out non copied info
	//toPilotAddr.reset();
	toPilotAddr.setField(entryLastname, fromAbEntry.familyName());
	QString firstAndMiddle = fromAbEntry.givenName();
	if(!fromAbEntry.additionalName().isEmpty()) firstAndMiddle += CSL1(" ") + fromAbEntry.additionalName();
	toPilotAddr.setField(entryFirstname, firstAndMiddle);
	toPilotAddr.setField(entryCompany, fromAbEntry.organization());
	toPilotAddr.setField(entryTitle, fromAbEntry.title());
	toPilotAddr.setField(entryNote, fromAbEntry.note());

	// do email first, to ensure its gets stored
	toPilotAddr.setPhoneField(PilotAddress::eEmail, fromAbEntry.preferredEmail());
	toPilotAddr.setPhoneField(PilotAddress::eWork,
		fromAbEntry.phoneNumber(KABC::PhoneNumber::Work).number());
	toPilotAddr.setPhoneField(PilotAddress::eHome,
		fromAbEntry.phoneNumber(KABC::PhoneNumber::Home).number());
	toPilotAddr.setPhoneField(PilotAddress::eMobile,
		fromAbEntry.phoneNumber(KABC::PhoneNumber::Cell).number());
	toPilotAddr.setPhoneField(PilotAddress::eFax, getFax(fromAbEntry).number());
	toPilotAddr.setPhoneField(PilotAddress::ePager,
		fromAbEntry.phoneNumber(KABC::PhoneNumber::Pager).number());
	toPilotAddr.setPhoneField(PilotAddress::eOther, getOtherField(fromAbEntry));
	toPilotAddr.setShownPhone(PilotAddress::eMobile);

	KABC::Address homeAddress = getAddress(fromAbEntry);
	_setPilotAddress(toPilotAddr, homeAddress);

	// Process the additional entries from the Palm(the palm database app block tells us the name of the fields)
	toPilotAddr.setField(entryCustom1, getCustomField(fromAbEntry, 0));
	toPilotAddr.setField(entryCustom2, getCustomField(fromAbEntry, 1));
	toPilotAddr.setField(entryCustom3, getCustomField(fromAbEntry, 2));
	toPilotAddr.setField(entryCustom4, getCustomField(fromAbEntry, 3));

	toPilotAddr.setCategory(_getCatForHH(fromAbEntry.categories(), toPilotAddr.getCategoryLabel()));
}



/**
 * _getCat returns the id of the category from the given categories list. If none of the categories exist
 * on the palm, the "Nicht abgelegt"(don't know the english name) is used.
 */
QString AbbrowserConduit::_getCatForHH(const QStringList cats, const QString curr) const
{
	FUNCTIONSETUP;
	int j;
	if (cats.size()<1) return QString::null;
	if (cats.contains(curr)) return curr;
	for(QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it)
	{
		for(j = 1; j <= 15; j++)
		{
			QString catName = PilotAppCategory::codec()->
				toUnicode(fAddressAppInfo.category.name[j]);
			if(!(*it).isEmpty() && !_compare(*it, catName))
			{
				return catName;
			}
		}
	}
	// If we have a free label, return the first possible cat
	QString lastName(fAddressAppInfo.category.name[15]);
	if (lastName.isEmpty()) return cats.first();
	return QString::null;
}



void AbbrowserConduit::_setPilotAddress(PilotAddress & toPilotAddr, const KABC::Address & abAddress)
{
	toPilotAddr.setField(entryAddress, abAddress.street());
	toPilotAddr.setField(entryCity, abAddress.locality());
	toPilotAddr.setField(entryState, abAddress.region());
	toPilotAddr.setField(entryZip, abAddress.postalCode());
	toPilotAddr.setField(entryCountry, abAddress.country());
}


void AbbrowserConduit::_copyPhone(KABC::Addressee & toAbEntry,
			      KABC::PhoneNumber phone, QString palmphone)
{
	if(!palmphone.isEmpty())
	{
		phone.setNumber(palmphone);
		toAbEntry.insertPhoneNumber(phone);
	}
	else
	{
		toAbEntry.removePhoneNumber(phone);
	}
}

void AbbrowserConduit::_setCategory(Addressee & abEntry, QString cat)
{
	if(!cat.isEmpty() ) abEntry.insertCategory(cat);
}

void AbbrowserConduit::_copy(KABC::Addressee & toAbEntry, const PilotAddress & fromPiAddr)
{
	FUNCTIONSETUP;
	// copy straight forward values
	toAbEntry.setFamilyName(fromPiAddr.getField(entryLastname));
	toAbEntry.setGivenName(fromPiAddr.getField(entryFirstname));
	toAbEntry.setOrganization(fromPiAddr.getField(entryCompany));
	toAbEntry.setTitle(fromPiAddr.getField(entryTitle));
	toAbEntry.setNote(fromPiAddr.getField(entryNote));

	// copy the phone stuff
	toAbEntry.removeEmail(toAbEntry.preferredEmail());
	toAbEntry.insertEmail(fromPiAddr.getPhoneField(PilotAddress::eEmail));

	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(KABC::PhoneNumber::Home),
		fromPiAddr.getPhoneField(PilotAddress::eHome));
	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(KABC::PhoneNumber::Work),
		fromPiAddr.getPhoneField(PilotAddress::eWork));
	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(KABC::PhoneNumber::Cell),
		fromPiAddr.getPhoneField(PilotAddress::eMobile));
	_copyPhone(toAbEntry,
		getFax(toAbEntry),
		fromPiAddr.getPhoneField(PilotAddress::eFax));
	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(KABC::PhoneNumber::Pager),
		fromPiAddr.getPhoneField(PilotAddress::ePager));
	setOtherField(toAbEntry, fromPiAddr.getPhoneField(PilotAddress::eOther));

	KABC::Address homeAddress = getAddress(toAbEntry);
	homeAddress.setStreet(fromPiAddr.getField(entryAddress));
	homeAddress.setLocality(fromPiAddr.getField(entryCity));
	homeAddress.setRegion(fromPiAddr.getField(entryState));
	homeAddress.setPostalCode(fromPiAddr.getField(entryZip));
	homeAddress.setCountry(fromPiAddr.getField(entryCountry));
	toAbEntry.insertAddress(homeAddress);

	setCustomField(toAbEntry, 0, fromPiAddr.getField(entryCustom1));
	setCustomField(toAbEntry, 1, fromPiAddr.getField(entryCustom2));
	setCustomField(toAbEntry, 2, fromPiAddr.getField(entryCustom3));
	setCustomField(toAbEntry, 3, fromPiAddr.getField(entryCustom4));

	// copy the fromPiAddr pilot id to the custom field KPilot_Id;
	// pilot id may be zero(since it could be new) but couldn't hurt
	// to even assign it to zero; let's us know what state the
	// toAbEntry is in
	toAbEntry.insertCustom(appString, idString, QString::number(fromPiAddr.getID()));


	int cat = fromPiAddr.getCat();
	QString category;
	if (0 < cat && cat <= 15) category = fAddressAppInfo.category.name[cat];
	_setCategory(toAbEntry, category);
#ifdef DEBUG
	showAddressee(toAbEntry);
#endif
}





/*********************************************************************
 C O N F L I C T   R E S O L U T I O N   a n d   M E R G I N G
 *********************************************************************/





/** smartly merge the given field for the given entry. use the backup record to determine which record has been modified
	entry... string representation of the record the field belongs to. Used for deconfliction dialog
	field ... string representation of the conflicting field. Used for deconfliction dialog
	pc, backup, palm ... entries of the according databases
	mergeNeeded ... set to true, if the field needed to be merged
	mergedStr ... contains the result if the field needed to be merged
	return value ... true if the merge could be done.
	                 false if the entry could not be merged
			 (e.g. user chose to add both records or no resolution at all)
*/
int AbbrowserConduit::_conflict(const QString & entry, const QString & field, const QString & palm,
	const QString & backup, const QString & pc, bool & mergeNeeded, QString & mergedStr)
{
	FUNCTIONSETUP;
	mergeNeeded = false;
	QString bckup = backup;

	// if both entries are already the same, no need to do anything
	if(pc == palm) return CHANGED_NONE;

	// If this is a first sync, we don't have a backup record, so
	if(isFirstSync())
	{
		bckup = QString();
		if(pc.isEmpty())
		{
			mergeNeeded = true;
			mergedStr = palm;
			return CHANGED_PC;
		}
		if(palm.isEmpty())
		{
			mergeNeeded = true;
			mergedStr = pc;
			return CHANGED_PALM;
		}
	}
	else
	{
		// only pc modified, so return that string, no conflict
		if(palm == backup)
		{
			mergeNeeded = true;
			mergedStr = pc;
			return CHANGED_PALM;
		}
		// only palm modified, so return that string, no conflict
		if(pc == backup)
		{
			mergeNeeded = true;
			mergedStr = palm;
			return CHANGED_PC;
		}
	}

	// We need to do some deconfliction. Use already chosen resolution option if possible
	SyncAction::eConflictResolution fieldres = getFieldResolution(entry, field, palm, bckup, pc);
#ifdef DEBUG
	DEBUGCONDUIT << "fieldres=" << fieldres << endl;
#endif
	switch(fieldres)
	{
		case SyncAction::ePCOverrides:
			mergeNeeded = true;
			mergedStr = pc;
			return CHANGED_PALM;
			break;
		case SyncAction::eHHOverrides:
			mergeNeeded = true;
			mergedStr = palm;
			return CHANGED_PC;
			break;
		case SyncAction::ePreviousSyncOverrides:
			mergeNeeded = true;
			mergedStr = backup;
			return CHANGED_BOTH;
			break;
		case SyncAction::eDuplicate:
			return CHANGED_DUPLICATE;
		case SyncAction::eDoNothing:
		default:
			return CHANGED_NORES;
	}
	return CHANGED_NONE;
}



int AbbrowserConduit::_compare(const QString & str1, const QString & str2) const
{
	if(str1.isEmpty() && str2.isEmpty()) return 0;
	else return str1.compare(str2);
}


int AbbrowserConduit::_smartMergePhone(KABC::Addressee & abEntry,
	const PilotAddress & backupAddress,
	PilotAddress & pilotAddress,
	PilotAddress::EPhoneType PalmFlag,
	KABC::PhoneNumber phone, QString thisName,
	QString name)
{
	bool mergeNeeded = false;
	QString mergedStr;

	int res = _conflict(thisName, name,
		pilotAddress.getPhoneField(PalmFlag),
		backupAddress.getPhoneField(PalmFlag),
		phone.number(), mergeNeeded, mergedStr);
	if(res & CHANGED_NORES) return res;
	if(mergeNeeded)
	{
		pilotAddress.setPhoneField(PalmFlag, mergedStr);
		phone.setNumber(mergedStr);
		abEntry.insertPhoneNumber(phone);
	}
	return -1;
}

int AbbrowserConduit::_smartMergeEntry(QString abEntry,
	const PilotAddress & backupAddress,
	PilotAddress & pilotAddress, int PalmFlag,
	QString thisName, QString name,
	QString & mergedString)
{
	bool mergeNeeded = false;
	QString mergedStr;
	mergedString = QString();

	int res = _conflict(thisName, name,
		pilotAddress.getField(PalmFlag),
		backupAddress.getField(PalmFlag),
		abEntry, mergeNeeded, mergedStr);
	if(res & CHANGED_NORES) return res;
	if(mergeNeeded)
	{
#ifdef DEBUG
		DEBUGCONDUIT << "Merged string=" << mergedStr << endl;
#endif
		pilotAddress.setField(PalmFlag, mergedStr);
		mergedString = mergedStr;
	}
	return -1;
}


int AbbrowserConduit::_smartMergeCategories(KABC::Addressee & abEntry,
	const PilotAddress & backupAddress,
	PilotAddress & pilotAddress,
	QString thisName, QString name,
	QString & mergedString)
{
	QString abAddressCat = _getCatForHH(abEntry.categories(), pilotAddress.getCategoryLabel());
	bool mergeNeeded = false;
	QString mergedStr;
	mergedString = QString();

	int res = _conflict(thisName, name,
		pilotAddress.getCategoryLabel(),
		backupAddress.getCategoryLabel(),
		abAddressCat, mergeNeeded, mergedStr);
	if(res & CHANGED_NORES) return res;
	if(mergeNeeded)
	{
		pilotAddress.setCategory(mergedStr);
		_setCategory(abEntry, mergedStr);
		mergedString = mergedStr;
	}
	return -1;
}


void AbbrowserConduit::showAdresses(PilotAddress & pilotAddress,
	const PilotAddress & backupAddress, KABC::Addressee & abEntry)
{
#ifdef DEBUG
	DEBUGCONDUIT << "abEntry:" << endl;
	showAddressee(abEntry);
	DEBUGCONDUIT << "pilotAddress:" << endl;
	showPilotAddress(pilotAddress);
	DEBUGCONDUIT << "backupAddress:" << endl;
	showPilotAddress(backupAddress);
	DEBUGCONDUIT << "------------------------------------------------" << endl;
#endif
}

int AbbrowserConduit::_smartMerge(PilotAddress & outPilotAddress,
	const PilotAddress & backupAddress, KABC::Addressee & outAbEntry)
{
	FUNCTIONSETUP;
	fEntryResolution = getConflictResolution();
	PilotAddress pilotAddress(outPilotAddress);
	KABC::Addressee abEntry(outAbEntry);
	QString thisName(outAbEntry.realName());
	bool mergeNeeded = false;
	QString mergedStr;
	int res;

	res = _smartMergeEntry(abEntry.familyName(), backupAddress, pilotAddress,
		(int) entryLastname, thisName, i18n("last name"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abEntry.setFamilyName(mergedStr);

	res = _smartMergeEntry(abEntry.givenName(), backupAddress, pilotAddress,
		(int) entryFirstname, thisName, i18n("first name"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abEntry.setGivenName(mergedStr);

	res = _smartMergeEntry(abEntry.organization(), backupAddress, pilotAddress,
		(int) entryCompany, thisName, i18n("organization"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abEntry.setOrganization(mergedStr);

	res = _smartMergeEntry(abEntry.title(), backupAddress, pilotAddress,
		(int) entryTitle, thisName, i18n("title"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abEntry.setTitle(mergedStr);

	res = _smartMergeEntry(abEntry.note(), backupAddress, pilotAddress,
		(int) entryNote, thisName, i18n("note"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abEntry.setNote(mergedStr);










	res = _smartMergeEntry(getCustomField(abEntry, 0), backupAddress, pilotAddress,
		entryCustom1, thisName, i18n("custom 1"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) setCustomField(abEntry, 0, mergedStr);

	res = _smartMergeEntry(getCustomField(abEntry, 1), backupAddress, pilotAddress,
		entryCustom2, thisName, i18n("custom 2"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) setCustomField(abEntry, 1, mergedStr);

	res = _smartMergeEntry(getCustomField(abEntry, 2), backupAddress, pilotAddress,
		entryCustom3, thisName, i18n("custom 3"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) setCustomField(abEntry, 2, mergedStr);

	res = _smartMergeEntry(getCustomField(abEntry, 3), backupAddress, pilotAddress,
		entryCustom4, thisName, i18n("custom 4"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) setCustomField(abEntry, 3, mergedStr);









	res = _smartMergePhone(abEntry, backupAddress, pilotAddress,
		PilotAddress::eWork, abEntry.phoneNumber(KABC::PhoneNumber::Work),
		thisName, i18n("work phone"));
	if(res >= 0) return res;
	res = _smartMergePhone(abEntry, backupAddress, pilotAddress,
		PilotAddress::eHome, abEntry.phoneNumber(KABC::PhoneNumber::Home),
		thisName, i18n("home phone"));
	if(res >= 0) return res;
	res = _smartMergePhone(abEntry, backupAddress, pilotAddress,
		PilotAddress::eMobile, abEntry.phoneNumber(KABC::PhoneNumber::Cell),
		thisName, i18n("mobile phone"));
	if(res >= 0) return res;
	res = _smartMergePhone(abEntry, backupAddress, pilotAddress,
		PilotAddress::eFax, getFax(abEntry), thisName, i18n("fax"));
	if(res >= 0) return res;
	res = _smartMergePhone(abEntry, backupAddress, pilotAddress,
		PilotAddress::ePager, abEntry.phoneNumber(KABC::PhoneNumber::Pager),
		thisName, i18n("pager"));
	if(res >= 0) return res;

	res = _conflict(thisName, i18n("other"),
		pilotAddress.getPhoneField(PilotAddress::eOther),
		backupAddress.getPhoneField(PilotAddress::eOther),
		getOtherField(abEntry), mergeNeeded, mergedStr);
	if(res & CHANGED_NORES) return res;
	if(mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eOther, mergedStr);
		setOtherField(abEntry, mergedStr);
	}

	res = _conflict(thisName, i18n("email"),
		pilotAddress.getPhoneField(PilotAddress::eEmail),
		backupAddress.getPhoneField(PilotAddress::eEmail),
		abEntry.preferredEmail(), mergeNeeded, mergedStr);
	if(res & CHANGED_NORES) return res;
	if(mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eEmail, mergedStr);
		abEntry.removeEmail(backupAddress.getPhoneField(PilotAddress::eEmail));
		abEntry.removeEmail(pilotAddress.getPhoneField(PilotAddress::eEmail));
		abEntry.insertEmail(mergedStr, true);
	}



	KABC::Address abAddress = getAddress(abEntry);

	res = _smartMergeEntry(abAddress.street(),
		backupAddress, pilotAddress, entryAddress,
		thisName, i18n("address"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abAddress.setStreet(mergedStr);

	res = _smartMergeEntry(abAddress.locality(),
		backupAddress, pilotAddress, entryCity,
		thisName, i18n("city"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abAddress.setLocality(mergedStr);

	res = _smartMergeEntry(abAddress.region(),
		backupAddress, pilotAddress, entryState,
		thisName, i18n("region"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abAddress.setRegion(mergedStr);

	res = _smartMergeEntry(abAddress.postalCode(),
		backupAddress, pilotAddress, entryZip,
		thisName, i18n("postal code"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abAddress.setPostalCode(mergedStr);

	res = _smartMergeEntry(abAddress.country(),
		backupAddress, pilotAddress, entryCountry,
		thisName, i18n("country"), mergedStr);
	if(res >= 0) return res;
	else if(!mergedStr.isEmpty()) abAddress.setCountry(mergedStr);

	abEntry.insertAddress(abAddress);

	res = _smartMergeCategories(abEntry,
		backupAddress, pilotAddress,
		thisName, i18n("category"), mergedStr);
	if(res >= 0) return res;

	abEntry.insertCustom(appString, idString, QString::number(pilotAddress.id()));


	outPilotAddress = pilotAddress;
	outAbEntry = abEntry;

	return CHANGED_BOTH;
}


/** Merge the palm and the pc entries with the additional information of the backup record. Calls _handleConflict
 * which does the actual syncing of the data structures. According to the return value of _handleConflict, this function
 * writes the data back to the palm/pc.
 *  return value: no meaning yet
 */
int AbbrowserConduit::_mergeEntries(PilotAddress & pilotAddress, PilotAddress & backupAddress, KABC::Addressee & abEntry)
{
	FUNCTIONSETUP;

	int res = _handleConflict(pilotAddress, backupAddress, abEntry);
	if(res & CHANGED_NORES)
	{
		if(res & CHANGED_DUPLICATE)
		{
			if(res & CHANGED_PALM)
			{
				// Set the Palm ID to 0 so we don't overwrite the existing record.
				abEntry.insertCustom(appString, idString, QString::number(0));
				_addToPalm(abEntry);
			}
			if(res & CHANGED_PC)
			{
				_addToAbbrowser(pilotAddress);
				_saveBackupAddress(pilotAddress);
			}
		}
	}
	else
	{
		if(res & CHANGED_PALM)
		{
			_savePilotAddress(pilotAddress, abEntry);
		}
		if(res & CHANGED_PC)
		{
			_saveAbEntry(abEntry);
		}
		_saveBackupAddress(pilotAddress);

		// Duplication handles this on its own.
		QString idStr(abEntry.custom(appString, idString));

		if(idStr.isEmpty() ||(idStr != QString::number(pilotAddress.getID())))
		{
			abEntry.insertCustom(appString, idString, QString::number(pilotAddress.getID()));
			_saveAbEntry(abEntry);
		}
	}
	return 0;
}


/** There was a conflict between the two fields; either could be null,
 *  or both have been modified
 *  return value: returns either of the CHANGED_* constants to determine which records need to be written back
 */
int AbbrowserConduit::_handleConflict(PilotAddress & pilotAddress, PilotAddress & backupAddress, KABC::Addressee & abEntry)
{
	FUNCTIONSETUP;

	if(abEntry.isEmpty())
	{
		_copy(abEntry, pilotAddress);
		return CHANGED_PC | CHANGED_ADD;
	}
	// TODO: if pilotAddress is empty????

	if(_equal(pilotAddress, abEntry)) return CHANGED_NONE;

	if(pilotAddress == backupAddress)
	{
		if(!_equal(backupAddress, abEntry))
		{
			_copy(pilotAddress, abEntry);
			return CHANGED_PALM;
		}
		else
		{
			// This should never be called, since that would mean
			// pilotAddress and backupAddress are equal, which we already checked!!!!
			return CHANGED_NONE;
		}
	}
	else
	{
		if(_equal(backupAddress, abEntry))
		{
			_copy(abEntry, pilotAddress);
			return CHANGED_PC;
		}
		else
		{
			// Both pc and palm were changed => merge, override, duplicate or ignore.
			if(doSmartMerge())
			{
				PilotAddress pAdr(pilotAddress);
				KABC::Addressee abEnt(abEntry);
				int res = _smartMerge(pilotAddress, backupAddress, abEntry);
				switch(res)
				{
					case CHANGED_NORES:
					case CHANGED_DUPLICATE:
						pilotAddress = pAdr;
						abEntry = abEnt;
				}
				return res;
			}
			else
			{			// no smart merge
				switch(getEntryResolution(abEntry, backupAddress, pilotAddress))
				{
					case SyncAction::eDuplicate:
						return CHANGED_DUPLICATE;
					case SyncAction::eHHOverrides:
						_copy(abEntry, pilotAddress);
						return CHANGED_PC;
					case SyncAction::ePCOverrides:
						_copy(pilotAddress, abEntry);
						return CHANGED_PALM;
					case SyncAction::ePreviousSyncOverrides:
						_copy(abEntry, backupAddress);
						pilotAddress = backupAddress;
						return CHANGED_BOTH;
					case SyncAction::eDoNothing:
						return CHANGED_NORES;
						default:
						return CHANGED_NONE;
				}
			}			// smart merge
		}			// backup == abook
	}				// pilot== backup

	return CHANGED_NONE;
}

SyncAction::eConflictResolution AbbrowserConduit::getFieldResolution(const QString & entry, const QString & field,
	const QString & palm, const QString & backup, const QString & pc)
{
	FUNCTIONSETUP;
	SyncAction::eConflictResolution res = fEntryResolution;
	if(res == SyncAction::eAskUser)
	{
		res = getConflictResolution();
	}
	switch(res)
	{
		case SyncAction::eDuplicate:
		case SyncAction::eHHOverrides:
		case SyncAction::ePCOverrides:
		case SyncAction::eDoNothing:
			return res;
			break;
		case SyncAction::ePreviousSyncOverrides:
			if(backup.isNull()) return SyncAction::eDoNothing;
			else return res;
			break;
		case SyncAction::eAskUser:
		default:
			QStringList lst;
			lst <<
				i18n("Leave untouched") <<
				i18n("Handheld overrides") <<
				i18n("PC overrides");
			if(!backup.isNull()) lst << i18n("Use the value from the last sync");
			lst << i18n("Duplicate both");
			bool remember = FALSE;
			res = ResolutionDialog(i18n("Address conflict"),
				i18n("<html><p>The field \"%1\" of the entry \"%2\" was changed on the handheld and on the PC.</p>"
				"<table border=0>"
				"<tr><td><b>Handheld:</b></td><td>%3</td></tr>"
				"<tr><td><b>PC:</b></td><td>%4</td></tr>"
				"<tr><td><b>last sync:</b></td><td>%5</td></tr>"
				"</table>"
				"<p>How should this conflict be resolved?</p></html>").arg(field).arg(entry).arg(palm).arg(pc).arg(backup),
				lst, i18n("Apply to all fields of this entry"), &remember);
			// If we don't have a backup, the item does not appear in the dialog.
			// Instead Duplicate will be the 4th radiobutton, so adjust  its index.
			if(backup.isNull() && (res == SyncAction::ePreviousSyncOverrides) )
				res = SyncAction::eDuplicate;
			if(remember)
			{
				fEntryResolution = res;
			}
			return res;
	}
}

SyncAction::eConflictResolution AbbrowserConduit::getEntryResolution(const KABC::Addressee & abEntry, const PilotAddress &backupAddress, const PilotAddress & pilotAddress)
{
	FUNCTIONSETUP;
	SyncAction::eConflictResolution res = getConflictResolution();
	switch(res)
	{
		case SyncAction::eDuplicate:
		case SyncAction::eHHOverrides:
		case SyncAction::ePCOverrides:
		case SyncAction::ePreviousSyncOverrides:
		case SyncAction::eDoNothing:
			return res;
			break;
		case SyncAction::eAskUser:
		default:
			QStringList lst;
			lst << i18n("Leave untouched") <<
				i18n("Handheld overrides") <<
				i18n("PC overrides") <<
				i18n("Use the value from the last sync");

			bool remember = FALSE;

			PilotAddress emptyAddress(fAddressAppInfo);
			bool backupEmpty=backupAddress.isDeleted() || emptyAddress==backupAddress;
			bool pilotEmpty=pilotAddress.isDeleted() || emptyAddress==pilotAddress;
			QString backupEntryString;
			if(!backupEmpty) backupEntryString=CSL1("%1 %2").arg(backupAddress.getField(entryFirstname)).arg(backupAddress.getField(entryLastname));
			else backupEntryString=i18n("(deleted)");
			QString pilotEntryString;
			if(!pilotEmpty) pilotEntryString=CSL1("%1 %2").arg(pilotAddress.getField(entryFirstname)).arg(pilotAddress.getField(entryLastname));
			else pilotEntryString=i18n("(deleted)");

			if(!abEntry.isEmpty() && !pilotEmpty) lst << i18n("Duplicate both");

#ifdef DEBUG
			DEBUGCONDUIT<<"pilotAddress.firstName="<<pilotAddress.getField(entryFirstname)<<", pilotAddress.LastName="<<pilotAddress.getField(entryLastname)<<", pilotEmpty="<<pilotEmpty<<", pilotEntryString="<<pilotEntryString<<endl;
			DEBUGCONDUIT<<"backupAddress.firstName="<<backupAddress.getField(entryFirstname)<<", backupAddress.LastName="<<backupAddress.getField(entryLastname)<<", backupEmpty="<<backupEmpty<<", backupEntryString="<<backupEntryString<<endl;
#endif
			res = ResolutionDialog(i18n("Address conflict"),
				i18n("<html><p>The following record was changed on the handheld and on the PC. </p>"
					"<table border=0>"
					"<tr><td><b>Handheld:</b></td><td>%1</td></tr>"
					"<tr><td><b>PC:</b></td><td>%2</td></tr>"
					"<tr><td><b>last sync:</b></td><td>%3</td></tr>"
					"</table>"
					"<p>How should this conflict be resolved?</p></html>")
					.arg(pilotEntryString)
					.arg(abEntry.isEmpty()? i18n("(deleted)") : abEntry.realName())
					.arg(backupEntryString)
				, lst, i18n("Remember my choice for all other records"), &remember);
			if(remember)
			{
				fConflictResolution = res;
			}
			return res;
	}
}


SyncAction::eConflictResolution AbbrowserConduit::ResolutionDialog(QString Title, QString Text, QStringList & lst, QString remember, bool * rem) const
{
	FUNCTIONSETUP;
	ResolutionDlg *dlg = new ResolutionDlg(0L, fHandle, Title, Text, lst, remember);
	if(dlg->exec() == KDialogBase::Cancel)
	{
		delete dlg;
		return SyncAction::eDoNothing;
	}
	else
	{
		SyncAction::eConflictResolution res = (SyncAction::eConflictResolution)(dlg->ResolutionButtonGroup->id(dlg->ResolutionButtonGroup->selected()) + 1);

		if(!remember.isEmpty() && rem)
		{
			*rem = dlg->rememberCheck->isChecked();
		}
		delete dlg;
		return res;
	}
}


// TODO: right now entries are equal if both first/last name and organization are
//  equal. This rules out two entries for the same person(e.g. real home and weekend home)
//  or two persons with the same name where you don't know the organization.!!!
KABC::Addressee AbbrowserConduit::_findMatch(const PilotAddress & pilotAddress) const
{
	FUNCTIONSETUP;
	// TODO: also search with the pilotID
	// first, use the pilotID to UID map to find the appropriate record
	if( !isFirstSync() && (pilotAddress.id() > 0) )
	{
		QString id(addresseeMap[pilotAddress.id()]);
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": PilotRecord has id " << pilotAddress.id() << ", mapped to " << id << endl;
#endif
		if(!id.isEmpty())
		{
			KABC::Addressee res(aBook->findByUid(id));
			if(!res.isEmpty()) return res;
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": PilotRecord has id " << pilotAddress.id() << ", but could not be found in the addressbook" << endl;
#endif
		}
	}

	bool piFirstEmpty =(pilotAddress.getField(entryFirstname).isEmpty());
	bool piLastEmpty =(pilotAddress.getField(entryLastname).isEmpty());
	bool piCompanyEmpty =(pilotAddress.getField(entryCompany).isEmpty());

	// return not found if either one is empty on one side but not on the other
	if(piFirstEmpty && piLastEmpty && piCompanyEmpty)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": entry has empty first, last name and company! Will not search in Addressbook" << endl;
#endif
		return KABC::Addressee();
	}

	// for now just loop throug all entries; in future, probably better
	// to create a map for first and last name, then just do O(1) calls
	for(KABC::AddressBook::Iterator iter = aBook->begin(); iter != aBook->end(); ++iter)
	{
		KABC::Addressee abEntry = *iter;
		// do quick empty check's
		if(piFirstEmpty != abEntry.givenName().isEmpty() ||
			piLastEmpty != abEntry.familyName().isEmpty() ||
			piCompanyEmpty != abEntry.organization().isEmpty())
		{
			continue;
		}
		if(piFirstEmpty && piLastEmpty)
		{
			if(abEntry.organization() == pilotAddress.getField(entryCompany))
			{
				return *iter;
			}
		}
		else
			// the first and last name must be equal; they are equal
			// if they are both empty or the strings match
			if(
				((piLastEmpty && abEntry.familyName().isEmpty()) || (abEntry.familyName() == pilotAddress.getField(entryLastname))) &&
				((piFirstEmpty && abEntry.givenName().isEmpty()) || (abEntry.givenName() == pilotAddress.getField(entryFirstname))) )
			{
				return *iter;
			}

	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Could not find any addressbook enty matching " << pilotAddress.getField(entryLastname) << endl;
#endif
	return KABC::Addressee();
}



void AbbrowserConduit::_checkDelete(PilotRecord* r, PilotRecord *s)
{
	FUNCTIONSETUP;
	// Find out if changed on the PC, and if so, do a deconfliction.
	bool archiveRecord =(r)?(r->getAttrib() & dlpRecAttrArchived):false;

	KABC::Addressee e;
	PilotAddress adr(fAddressAppInfo, r), badr(fAddressAppInfo, s);
	if (r) e = _findMatch(adr);
	else if (s) e=_findMatch(badr);

	if (!e.isEmpty() && !_equal(badr, e))
	{ // entry was changed on the PC => deconflict

		switch(getEntryResolution(e, badr, adr))
		{
			case SyncAction::eHHOverrides:
				_deleteOnPC(r, s);
				break;
			case SyncAction::ePCOverrides:
				_copy(adr, e);
				// undelete the pilot record before changing to make sure it doesn't stay deleted!
				adr.setAttrib(adr.getAttrib() & ~dlpRecAttrDeleted);
				if (_savePilotAddress(adr, e)) _saveAbEntry(e);
				break;
			case SyncAction::ePreviousSyncOverrides:
				_copy(e, badr);
				_savePilotAddress(badr, e);
				_saveAbEntry(e);
				break;
			case SyncAction::eDoNothing:
			default:
				break;
		}
	}
	else
	{
		if(fArchive && archiveRecord)
		{
			e = _changeOnPC(r, s);
			e.insertCustom(appString, flagString, QString::number(SYNCDEL));
			aBook->insertAddressee(e);
		}
		else
		{
			_deleteOnPC(r, s);
		}
	}
}
