// abbrowser-conduit.cc
//
// Copyright (C) 2000,2001 by Dan Pilone
// Copyright (C) 2002 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
//
// The abbrowser conduit copies addresses from the Pilot's address book to 
// the KDE addressbook maintained via the kabc library.
//
//
 



#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
#include <unistd.h>
#include <assert.h>
#include <iostream.h>
#include <time.h>

#include <qdir.h>
#include <qtimer.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>

#include <pi-appinfo.h>
#include <pi-source.h>
#include <pi-address.h>

#include <kabc/stdaddressbook.h>
#include <kabc/resource.h> 
#include <kabc/addressbook.h> 
#include <pilotUser.h>
#include <pilotSerialDatabase.h>

#include "abbrowser-factory.h"

#include "abbrowser-conduit.moc"
#include "resolutionDialog.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
const char *abbrowser_conduit_id =
	"$Id$";
const QString AbbrowserConduit::flagString="Flag";
const QString AbbrowserConduit::appString="KPILOT";
const QString AbbrowserConduit::idString="RecordID";

bool AbbrowserConduit::fPilotStreetHome=true;
bool AbbrowserConduit::fPilotFaxHome=true;
enum AbbrowserConduit::ePilotOtherEnum AbbrowserConduit::ePilotOther=AbbrowserConduit::eOtherPhone;



/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/

 
 
 
 
AbbrowserConduit::AbbrowserConduit(KPilotDeviceLink * o,
	const char *n, 
	const QStringList & a) :
	ConduitAction(o, n, a),
	addresseeMap(),
	syncedIds(),
//	recordIds(),
	aBook(0L),
	abiter()
{
	FUNCTIONSETUP;
	(void) abbrowser_conduit_id;
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
void AbbrowserConduit::_mapContactsToPilot( QMap < recordid_t, QString> &idContactMap) const
{
	FUNCTIONSETUP;

	idContactMap.clear();
	
	for (KABC::AddressBook::Iterator contactIter=aBook->begin();
		contactIter != aBook->end(); ++contactIter)
	{
		KABC::Addressee aContact = *contactIter;
		QString recid = aContact.custom(appString, idString);
		if (!recid.isEmpty())
		{
			recordid_t id = recid.toULong();
			idContactMap.insert(id, aContact.uid());
		}
	}
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": Loaded "<<idContactMap.size()<<" addresses from the addressbook "<<dynamic_cast<KABC::StdAddressBook*>(aBook)->fileName()<<endl;
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

	fSmartMerge = fConfig->readBoolEntry(AbbrowserConduitFactory::smartMerge(),true);
	fConflictResolution = (EConflictResolution) fConfig->readNumEntry(AbbrowserConduitFactory::conflictResolution(), eUserChoose);
	fArchive = fConfig->readBoolEntry(AbbrowserConduitFactory::archiveDeletedRecs(), true); 
	fPilotStreetHome=fConfig->readBoolEntry(AbbrowserConduitFactory::streetType(), true);
	fPilotFaxHome = fConfig->readBoolEntry(AbbrowserConduitFactory::faxType(), true);
	syncAction = fConfig->readNumEntry(AbbrowserConduitFactory::syncMode(), SYNC_FAST);
	fFirstTime = fConfig->readBoolEntry(AbbrowserConduitFactory::firstSync(), false);
	ePilotOther = fConfig->readNumEntry(AbbrowserConduitFactory::otherField(), eOtherPhone);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Settings "
		<< " fSmartMerge="<<fSmartMerge
		<< " fConflictResolution="<<fConflictResolution
		<< " fPilotStreetHome="<<fPilotStreetHome
		<< " fPilotFaxHome="<<fPilotFaxHome
		<< " syncAction="<<syncAction
		<< " fArchive="<<fArchive
		<< " fFirstTime="<<fFirstTime<<endl;
#endif
}



bool AbbrowserConduit::_loadAddressBook()
{
	FUNCTIONSETUP;
	aBook=KABC::StdAddressBook::self();
	aBook->load();
	abChanged=false;
	// get the addresseMap which maps Pilot unique record (address) id's to
	//	a Abbrowser KABC::Addressee; allows for easy lookup and comparisons
	if (aBook->begin()==aBook->end()) {
		fFirstTime=true;
	}
	else
	{
		_mapContactsToPilot(addresseeMap);
	}
	return (aBook!=0L );
}



bool AbbrowserConduit::_saveAddressBook()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<"abChanged="<<abChanged<<endl;
	for (KABC::AddressBook::Iterator contactIter=aBook->begin();
		contactIter != aBook->end(); ++contactIter)
	{
		DEBUGCONDUIT<<"Adressee: "<<(*contactIter).realName()<<", uid="<<(*contactIter).uid()<<", resource="<<(*contactIter).resource()<<endl;
	}
	DEBUGCONDUIT<<fname<<" --------------------------------------------"<<endl;
#endif
	
	if (!abChanged) return true;
	return (aBook) && (aBook->saveAll());
}



void AbbrowserConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	unsigned char *buffer =
		new unsigned char[PilotAddress::APP_BUFFER_SIZE];
	int appLen = fDatabase->readAppBlock(buffer,PilotAddress::APP_BUFFER_SIZE);

	unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);
	delete[]buffer;
	buffer = NULL;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " lastUniqueId"
		<< fAddressAppInfo.category.lastUniqueID << endl;
#endif
	for (int i = 0; i < 16; i++)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " cat " << i << " =" <<
			fAddressAppInfo.category.name[i] << endl;
#endif
	}

	for (int x = 0; x < 8; x++)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " phone[" << x << "] = " <<
			fAddressAppInfo.phoneLabels[x] << endl;
#endif
	}
}


QString AbbrowserConduit::getOtherField(const KABC::Addressee&abEntry) 
{
	FUNCTIONSETUP;
	switch (ePilotOther)
	{
		case eOtherPhone:
			return abEntry.phoneNumber(0).number();
		case eAssistant:
			return abEntry.custom( "KADDRESSBOOK", "AssistantsName");
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
			return "";
	}
}


void AbbrowserConduit::setOtherField(KABC::Addressee&abEntry, QString nr) 
{
//FUNCTIONSETUP;
	KABC::PhoneNumber phone;
	switch (ePilotOther)
	{
		case eOtherPhone:
			phone=abEntry.phoneNumber(0);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eAssistant:
			abEntry.insertCustom("KADDRESSBOOK", "AssistantsName", nr);
		case eBusinessFax:
			phone=abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eCarPhone:
			phone=abEntry.phoneNumber(KABC::PhoneNumber::Car);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eEmail2:
			return abEntry.insertEmail(nr);
		case eHomeFax:
			phone=abEntry.phoneNumber(KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eTelex:
			phone=abEntry.phoneNumber(KABC::PhoneNumber::Bbs);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
		case eTTYTTDPhone:
			phone=abEntry.phoneNumber(KABC::PhoneNumber::Pcs);
			phone.setNumber(nr);
			abEntry.insertPhoneNumber(phone);
			break;
	}
}


KABC::PhoneNumber AbbrowserConduit::getFax(const KABC::Addressee &abEntry)
{
	return abEntry.phoneNumber(KABC::PhoneNumber::Fax | ( (fPilotFaxHome)?(KABC::PhoneNumber::Home):(KABC::PhoneNumber::Work)) );
}


KABC::Address AbbrowserConduit::getAddress(const KABC::Addressee &abEntry)
{
	return abEntry.address((fPilotStreetHome)?(KABC::Address::Home):(KABC::Address::Work) );
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
	DEBUGCONDUIT << "\t\tHome phone = "<< pilotAddress.getPhoneField(PilotAddress::eHome) << endl;
	DEBUGCONDUIT << "\t\tWork phone = "<< pilotAddress.getPhoneField(PilotAddress::eWork) << endl;
	DEBUGCONDUIT << "\t\tMobile phone = "<< pilotAddress.getPhoneField(PilotAddress::eMobile) << endl;
	DEBUGCONDUIT << "\t\tEmail = "<< pilotAddress.getPhoneField(PilotAddress::eEmail) << endl;
	DEBUGCONDUIT << "\t\tFax = "<< pilotAddress.getPhoneField(PilotAddress::eFax) << endl;
	DEBUGCONDUIT << "\t\tPager = "<< pilotAddress.getPhoneField(PilotAddress::ePager) << endl;
	DEBUGCONDUIT << "\t\tOther = "<< pilotAddress.getPhoneField(PilotAddress::eOther) << endl;
	DEBUGCONDUIT << "\t\tCategory = " << pilotAddress.getCategoryLabel() << endl;
}
#endif





/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/





/* virtual */ void AbbrowserConduit::exec()
{
	FUNCTIONSETUP;

	KPilotUser*usr;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		goto error;
	}

	_prepare();
	
	usr=fHandle->getPilotUser();
	// changing the PC or using a different Palm Desktop app causes a full sync
	// Use gethostid for this, since JPilot uses 1+(2000000000.0*random()/(RAND_MAX+1.0))
	// as PC_ID, so using JPilot and KPilot is the same as using two differenc PCs
	fFullSync = (syncAction==SYNC_FULL) ||
		((usr->getLastSyncPC()!=(unsigned long) gethostid()) && fConfig->readBoolEntry(AbbrowserConduitFactory::fullSyncOnPCChange(), true));
	
	if (!openDatabases("AddressDB", &fFullSync) ) goto error;
	_setAppInfo();
	if (!_loadAddressBook() ) goto error;
//	recordIds=fDatabase->idList();
	
	// perform syncing from palm to abbrowser
	// iterate through all records in palm pilot
	pilotindex=0;

#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": fullsync="<<fFullSync<<", firstSync="<<fFirstTime<<endl;
	DEBUGCONDUIT<<fname<<": syncAction="<<syncAction<<", archive = "<<fArchive<<endl;
	DEBUGCONDUIT<<fname<<": smartmerge="<< fSmartMerge<<", conflictRes="<<fConflictResolution<<endl;
	DEBUGCONDUIT<<fname<<": PilotStreetHome="<<fPilotStreetHome<<", PilotFaxHOme"<<fPilotFaxHome<<endl;
#endif
	
	QTimer::singleShot(0, this, SLOT(syncPalmRecToPC()));
	// TODO: maybe start a second timer to kill the sync after, say, 5 Minutes (e.g. non existent slot called...)
	return;
	
error:
	emit logError(i18n("Couldn't open the addressbook databases."));
	emit syncDone(this);
}



void AbbrowserConduit::syncPalmRecToPC()
{
	FUNCTIONSETUP;
	PilotRecord*r=0L, *s=0L;
	
	if (fFirstTime || fFullSync)
	{
		r=fDatabase->readRecordByIndex(pilotindex++);
	}
	else
	{
		r=dynamic_cast<PilotSerialDatabase*>(fDatabase)->readNextModifiedRec();
	}
	
	if (!r)
	{
		// TODO: implement nextSyncAction...
		abiter=aBook->begin();
		QTimer::singleShot(0,this, SLOT(syncPCRecToPalm()));
		return;
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"Checking for already synced ID "<<r->getID()<<"...  "<<endl;
#endif
		// already synced, so skip:
		if (syncedIds.contains(r->getID())) 
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"already synced, so skipping"<<endl;
#endif
			QTimer::singleShot(0, this, SLOT(syncPalmRecToPC()));
			return;
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"not yet synced"<<endl;
#endif
		}
	}
	
	bool archiveRecord=(r->getAttrib() & dlpRecAttrArchived);

	KABC::Addressee e;
	s = fLocalDatabase->readRecordById(r->getID());
	if (!s) 
	{
		e=_findMatch(PilotAddress(fAddressAppInfo, r));
	}

	if ( (!s && e.isEmpty())  || fFirstTime)
	{
		// doesn't exist on PC. Either not deleted at all, or deleted with the archive flag on.
		if (!r->isDeleted() || (fArchive && archiveRecord))
		{
			e=_addToPC(r);
//			if (fArchive && !e.isEmpty() ) 
//			{
//				e.insertCustom(appString, flagString, QString::number(SYNCDEL));
//				aBook->insertAddressee(e);
//			}
		}
	}
	else
	{
		if (r->isDeleted())
		{
			if (fArchive && archiveRecord) 
			{
				e=_changeOnPC(r,s);
				e.insertCustom(appString, flagString, QString::number(SYNCDEL));
				aBook->insertAddressee(e);
			}
			else
			{
				_deleteOnPC(r,s);
			}
		}
		else
		{
			e=_changeOnPC(r,s);
		}
	}

	syncedIds.append(r->getID());
	KPILOT_DELETE(r);
	KPILOT_DELETE(s);
	
	QTimer::singleShot(0,this,SLOT(syncPalmRecToPC()));
}



void AbbrowserConduit::syncPCRecToPalm()
{
	FUNCTIONSETUP;
	
	if (abiter==aBook->end() || (*abiter).isEmpty()) {
		pilotindex=0;
		QTimer::singleShot(0,this,SLOT(syncDeletedRecord()));
		return;
	}
	bool ok;
	KABC::Addressee ad=*abiter;
	QString recID(ad.custom(appString, idString));
	recordid_t rid = recID.toLong(&ok);
	if (recID.isEmpty() || !ok || !rid ) 
	{
		// it's a new item (no record ID and not inserted by the Palm -> PC sync), so add it 
		_addToPalm(ad);
	}
	// look into the list of already synced record ids to see if the addressee hasn't already been synced
	else if (syncedIds.contains(rid) )
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": address with id "<<rid<<" already synced."<<endl;
#endif
		abiter++;
		QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));	
		return;
	}
	if (ad.custom(appString, flagString)==QString::number(SYNCDEL))
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": address with id "<<rid<<" marked archived, so don't sync."<<endl;
#endif
		syncedIds.append(rid);
		abiter++;
		QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));	
		return;
	}

	
	PilotRecord*backup=fLocalDatabase->readRecordById(rid);
	// only update if no backup record or the backup record is not equal to the addresse
	PilotAddress pbackupadr(fAddressAppInfo, backup);
	if (!backup || !_equal(pbackupadr, ad) )
	{
		PilotRecord*rec=fDatabase->readRecordById(rid);
		if (!rec) 
		{
			// not found on palm, so it was permanently deleted from the palm. 
			int res=KMessageBox::warningYesNo(0L, 
				i18n("The following record does not exist on the handheld and was probably deleted from the handheld:\n%1\n\n"
				"Also delete it from the PC?").arg(ad.realName()),  
				i18n("Addressbook conduit Conflict"), i18n("Delete from PC"), i18n("Add on handheld")/*, i18n("don't resolve")*/ );
			switch (res) {
				case KMessageBox::Yes:
					// Palm takes precedence -> delete from PC
					_deleteOnPC(rec, backup);
					break;
				case KMessageBox::No:
					// PC takes precedence -> add to palm
					_addToPalm(ad);
					break;
			}
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
	abiter++;
	QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));	
}



void AbbrowserConduit::syncDeletedRecord() 
{
	FUNCTIONSETUP;

	PilotRecord *r = fLocalDatabase->readRecordByIndex(pilotindex++);
	if (!r || fFirstTime)
	{
		QTimer::singleShot(0, this,SLOT(cleanup()));
		return;
	}
	
	// already synced, so skip this record:
	if (syncedIds.contains(r->getID())) 
	{
		QTimer::singleShot(0, this, SLOT(syncDeletedRecord()));
		return;
	}
#ifdef DEBUG
	DEBUGCONDUIT<<"Item "<<r->getID()<<" not yet synced, see if we find it in the addressbook"<<endl;
#endif

	QString uid = addresseeMap[r->getID()];
	KABC::Addressee e = aBook->findByUid(uid);
	if (uid.isEmpty() || e.isEmpty())
	{
#ifdef DEBUG
	DEBUGCONDUIT<<"Item "<<r->getID()<<" deleted from the PC, so delete from Palm too!"<<endl;
#endif
		
		// entry was deleted from addressbook, so delete it from the palm
		_deleteFromPalm(r);
	}

	KPILOT_DELETE(r);
	QTimer::singleShot(0,this,SLOT(syncDeletedRecord()));

}

void AbbrowserConduit::cleanup() 
{
	FUNCTIONSETUP;

	if (fDatabase) 
	{
		fDatabase->resetSyncFlags();
		fDatabase->cleanup();
	}
	if (fLocalDatabase) 
	{
		fLocalDatabase->resetSyncFlags();
		fLocalDatabase->cleanup();
	}
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
	_saveAddressBook();
	// TODO: Do I need to free the addressbook?????
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
	abChanged=true;
	aBook->removeAddressee(addressee);
}



KABC::Addressee AbbrowserConduit::_saveAbEntry(KABC::Addressee &abEntry)
{
	FUNCTIONSETUP;

	// TODO: Clear a modified flag (if existent)
	if (!abEntry.custom(appString, idString).isEmpty())
	{
		addresseeMap.insert(abEntry.custom(appString, idString).toLong(), abEntry.uid());
	}
#ifdef DEBUG
	else
	{
		DEBUGCONDUIT<<fname<<": WARNING: saveAbEntry without Pilot id set (uid: "<<abEntry.uid()<<")"<<endl;
	}
#endif
	  
// TODO: Remove this terrible hack when insertAddressee finally sets a default resource to new addressees
// BEGIN BAD HACK
//	if (!abEntry.resource()) 
//		abEntry.setResource(aBook->resources().first());
// END BAD HACK
	aBook->insertAddressee(abEntry);
	abChanged=true;
	return abEntry;
}



bool AbbrowserConduit::_savePilotAddress(PilotAddress & address,
	KABC::Addressee &abEntry)
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
	recordid_t pilotId = fDatabase->writeRecord(pilotRec);
	pilotRec->setID(pilotId);
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);

	// pilotId == 0 if using local db, so don't overwrite the valid id
	if (pilotId != 0) 
		address.setID(pilotId);

	recordid_t abId = 0;

//	if (!abEntry.custom(appString, idString).isEmpty())
		abId = abEntry.custom(appString, idString).toUInt();
	if (abId != address.id())
	{
		abEntry.insertCustom(appString, idString, QString::number(address.id()) );
		return true;
	}

	return false;
}






bool AbbrowserConduit::_saveBackupAddress(PilotAddress & backup)
{
	FUNCTIONSETUP;

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
	if ( (address.isModified() && address.isDeleted())
		 && (address.getField(entryLastname) == 0)
		 && (address.getField(entryFirstname) == 0)
			  )
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
 
KABC::Addressee AbbrowserConduit::_addToPC(PilotRecord *r)
{
	FUNCTIONSETUP;
	return _changeOnPC(r, NULL);
}



KABC::Addressee AbbrowserConduit::_changeOnPC(PilotRecord*rec, PilotRecord*backup)
{
	FUNCTIONSETUP;
	PilotAddress padr(fAddressAppInfo, rec);
	showPilotAddress(padr);
	struct AddressAppInfo ai=fAddressAppInfo;
	PilotAddress pbackupadr(ai, backup);
	KABC::Addressee ad;
	
#ifdef DEBUG
DEBUGCONDUIT<<"---------------------------------"<<endl;
DEBUGCONDUIT<<"Now syncing "<<padr.getField(entryFirstname)<<" "<<padr.getField(entryLastname)<<" / backup: "<<pbackupadr.getField(entryFirstname)<<" "<<pbackupadr.getField(entryLastname)<<endl;
#endif

	if (backup) ad=_findMatch(pbackupadr);
	if (ad.isEmpty()) ad=_findMatch(padr);
#ifdef DEBUG
//	if (ad.isEmpty())
		DEBUGCONDUIT<<"ad.isEmpty()="<<ad.isEmpty()<<endl;
#endif

	if (ad.isEmpty() ) 
	{
		if (!backup) 
		{
			// not found, so add
			ad=_addToAbbrowser(padr);
			fLocalDatabase->writeRecord(rec);
		}
		else
		{
#ifdef DEBUG
DEBUGCONDUIT<<"not a new entry, but PC entry does not exist => deconfliction of "<<padr.getField(entryLastname)<<endl;
#endif
			KABC::Addressee ab;
			switch (getEntryResolution(ad, padr)) 
			{
				case ePilotOverides:
					_addToAbbrowser(padr);
					break;
				case eAbbrowserOverides:
					_removePilotAddress(padr);
					break;
				case eRevertToBackup:
					ab=_addToAbbrowser(pbackupadr);
					if (_savePilotAddress(pbackupadr, ab))
						_saveAbEntry(ab);
					break;
				case eDoNotResolve:
				default:
					break;
			}
		}
	}
	else
	{
#ifdef DEBUG
DEBUGCONDUIT<<"vor insertCustom"<<endl;
#endif
		ad.insertCustom(appString, idString, QString::number(padr.getID()));
#ifdef DEBUG
DEBUGCONDUIT<<"Not equal"<<endl;
#endif
		PilotAddress backupadr(fAddressAppInfo, backup);
		_mergeEntries(padr, backupadr, ad);
	}
	return ad;
}



bool AbbrowserConduit::_deleteOnPC(PilotRecord*rec,PilotRecord*backup) 
{
	FUNCTIONSETUP;
	recordid_t id;
	if (rec) id=rec->getID();
	else if (backup) id=backup->getID();
	else id=0;
	
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": deleting record with id "<<id<<", rec "<<(rec!=NULL)<<", backup "<<(backup!=NULL)<<endl;
#endif
	if (!id) return false;
	
	KABC::Addressee ad=aBook->findByUid(addresseeMap[id]);
	PilotAddress backupAdr(fAddressAppInfo, backup);
	
	if ( (!backup) || !_equal(backupAdr, ad) ) 
	{
		// TODO: Conflict!!!
	}
	if (!ad.isEmpty())
	{
		_removeAbEntry(ad);
		//aBook->removeAddressee(ad);
	}
	if (!rec) {
		backup->makeDeleted();//setAttrib(backup->getAttrib() | dlpRecAttrDeleted );
		fLocalDatabase->writeRecord(backup);
	}
	else
	{
		fLocalDatabase->writeRecord(rec);
	}
	return true;
}



// -----------------------------------------------------------
// Palm => PC 
// -----------------------------------------------------------



void AbbrowserConduit::_addToPalm(KABC::Addressee & entry)
{
	FUNCTIONSETUP;
	PilotAddress pilotAddress(fAddressAppInfo);

	_copy(pilotAddress, entry);
	
	if (_savePilotAddress(pilotAddress, entry))
		_saveAbEntry(entry);
}



void AbbrowserConduit::_changeOnPalm(PilotRecord *rec, PilotRecord* backuprec, KABC::Addressee &ad)
{
	FUNCTIONSETUP;
	PilotAddress padr(fAddressAppInfo);
	PilotAddress pbackupadr(fAddressAppInfo);
	
	if (rec) padr=PilotAddress(fAddressAppInfo, rec);
	if (backuprec) pbackupadr=PilotAddress(fAddressAppInfo, backuprec);
#ifdef DEBUG
	DEBUGCONDUIT<<"---------------------------------"<<endl;
	DEBUGCONDUIT<<"Now syncing "<<padr.getField(entryLastname)<<" / backup: "<<pbackupadr.getField(entryLastname)<<endl;
#endif
	_mergeEntries(padr, pbackupadr, ad);
}



void AbbrowserConduit::_deleteFromPalm(PilotRecord*rec) 
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

 



bool AbbrowserConduit::_equal(const PilotAddress & piAddress,
	KABC::Addressee & abEntry) const
{
	FUNCTIONSETUP;
	// TODO: also check the PilotID
	if (_compare( abEntry.familyName(), piAddress.getField(entryLastname))  ) 
	{
DEBUGCONDUIT<<"Family name not equal: "<< abEntry.familyName()<<" vs. " <<piAddress.getField(entryLastname)<<endl;
		return false;
	} 
	if (_compare( abEntry.givenName(), piAddress.getField(entryFirstname) ) )
	{
		return false;
	} 
	if (_compare( abEntry.title(), piAddress.getField(entryTitle)))
	{
		return false;
	} 
	if (_compare( abEntry.organization(), piAddress.getField(entryCompany)))
	{
		return false;
	} 
	if (_compare( abEntry.note(), piAddress.getField(entryNote)))
	{
		return false;
	} 
	int cat=_getCat(abEntry.categories() );
	if (_compare( fAddressAppInfo.category.name[cat], piAddress.getCategoryLabel()))
	{
		return false;
	} 
	if (_compare( abEntry.phoneNumber(KABC::PhoneNumber::Work).number(), piAddress.getPhoneField(PilotAddress::eWork)))
	{
		return false;
	} 
	if (_compare( abEntry.phoneNumber(KABC::PhoneNumber::Home).number(), piAddress.getPhoneField(PilotAddress::eHome)))
	{
		return false;
	} 
	if (_compare( getOtherField(abEntry), piAddress.getPhoneField(PilotAddress::eOther)))
	{
		return false;
	} 
	if (_compare( abEntry.preferredEmail(), piAddress.getPhoneField(PilotAddress::eEmail)) )
	{
		return false;
	} 
	if (_compare( getFax(abEntry).number(), piAddress.getPhoneField(PilotAddress::eFax)))
	{
		return false;
	}
	if (_compare( abEntry.phoneNumber(KABC::PhoneNumber::Cell).number(), piAddress.getPhoneField(PilotAddress::eMobile) ) )
	{
		return false;
	} 
	KABC::Address address = getAddress(abEntry);
	if (_compare( address.street(), piAddress.getField(entryAddress)))
	{
		return false;
	}
	if (_compare( address.locality(), piAddress.getField(entryCity)))
	{
		return false;
	} 
	if (_compare( address.region(), piAddress.getField(entryState)))
	{
		return false;
	} 
	if (_compare( address.postalCode(), piAddress.getField(entryZip)))
	{
		return false;
	} 
	if (_compare( address.country(), piAddress.getField(entryCountry)))
	{
		return false;
	} 

	if (	_compare( abEntry.custom(appString, "CUSTOM1"), piAddress.getField(entryCustom1)) ||
			_compare( abEntry.custom(appString, "CUSTOM2"), piAddress.getField(entryCustom2)) ||
			_compare( abEntry.custom(appString, "CUSTOM3"), piAddress.getField(entryCustom3)) ||
			_compare( abEntry.custom(appString, "CUSTOM4"), piAddress.getField(entryCustom4))  )
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
	toPilotAddr.setField(entryLastname, fromAbEntry.familyName().latin1());
	QString firstAndMiddle = fromAbEntry.givenName();


	if (!fromAbEntry.additionalName().isEmpty())
		firstAndMiddle += " " + fromAbEntry.additionalName();
	toPilotAddr.setField(entryFirstname, firstAndMiddle.latin1());
	toPilotAddr.setField(entryCompany, fromAbEntry.organization().latin1());
	toPilotAddr.setField(entryTitle, fromAbEntry.title().latin1());
	toPilotAddr.setField(entryNote, fromAbEntry.note().latin1());

	// do email first, to ensure its gets stored
	toPilotAddr.setPhoneField(PilotAddress::eEmail, fromAbEntry.preferredEmail().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eWork, fromAbEntry.phoneNumber(KABC::PhoneNumber::Work).number().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eHome, fromAbEntry.phoneNumber(KABC::PhoneNumber::Home).number().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eMobile, fromAbEntry.phoneNumber(KABC::PhoneNumber::Cell).number().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eFax, getFax(fromAbEntry).number().latin1());
	toPilotAddr.setPhoneField(PilotAddress::ePager, fromAbEntry.phoneNumber(KABC::PhoneNumber::Pager).number().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eOther, getOtherField(fromAbEntry).latin1());
	toPilotAddr.setShownPhone(PilotAddress::eMobile);

	// in future, may want prefs that will map from abbrowser entries
	// to the pilot phone entries so they can do the above assignment and
	// assign the Other entry which is currenty unused
	// TODO: really use home address by default?? Should add some config option for this
//	KABC::Address homeAddress = fromAbEntry.address(KABC::Address::Home);
	KABC::Address homeAddress = getAddress(fromAbEntry);
	if (!homeAddress.isEmpty()) 
		_setPilotAddress(toPilotAddr, homeAddress);
/*	else
	{
		// no home address, try work address
		KABC::Address workAddress =
			fromAbEntry.address(KABC::Address::Work);
		if (!workAddress.isEmpty())
			_setPilotAddress(toPilotAddr, workAddress);
	}*/
	
	// TODO: Process the additional entries from the Palm (the palm database app block tells us the name of the fields)
	toPilotAddr.setField(entryCustom1, fromAbEntry.custom(appString, "CUSTOM1"));
	toPilotAddr.setField(entryCustom2, fromAbEntry.custom(appString, "CUSTOM2"));
	toPilotAddr.setField(entryCustom3, fromAbEntry.custom(appString, "CUSTOM3"));
	toPilotAddr.setField(entryCustom4, fromAbEntry.custom(appString, "CUSTOM4"));
	
	
	//TODO: sync categories
	toPilotAddr.setCat(_getCat(fromAbEntry.categories()));
//showPilotAddress(toPilotAddr);
}

/** 
 * _getCat returns the id of the category from the given categories list. If none of the categories exist 
 * on the palm, the "Nicht abgelegt" (don't know the english name) is used.
 */
	int AbbrowserConduit::_getCat(const QStringList cats) const
{
	FUNCTIONSETUP;
	int j;
	for ( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it ) {
		for (j=1; j<=15; j++) 
		{
			if (!(*it).isEmpty() && ! _compare(*it, fAddressAppInfo.category.name[j]) ) 
			{
				return j;
			}
		}
	}
	return 0;
}



void AbbrowserConduit::_setPilotAddress(PilotAddress & toPilotAddr, const KABC::Address & abAddress)
{
	FUNCTIONSETUP;
	toPilotAddr.setField(entryAddress, abAddress.street().latin1());
	toPilotAddr.setField(entryCity, abAddress.locality().latin1());
	toPilotAddr.setField(entryState, abAddress.region().latin1());
	toPilotAddr.setField(entryZip, abAddress.postalCode().latin1());
	toPilotAddr.setField(entryCountry, abAddress.country().latin1());
}


void AbbrowserConduit::_copyPhone(KABC::Addressee &toAbEntry, KABC::PhoneNumber phone, QString palmphone)
{
//	phone=toAbEntry.phoneNumber(KABC::PhoneNumber::Fax);
//	palmphone=fromPiAddr.getPhoneField(PilotAddress::eFax);
	if (!palmphone.isEmpty())
	{
		phone.setNumber(palmphone);
		toAbEntry.insertPhoneNumber(phone);
	}
	else
	{
		toAbEntry.removePhoneNumber(phone);
	}
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

	_copyPhone(toAbEntry, toAbEntry.phoneNumber(KABC::PhoneNumber::Home), fromPiAddr.getPhoneField(PilotAddress::eHome));
	_copyPhone(toAbEntry, toAbEntry.phoneNumber(KABC::PhoneNumber::Work), fromPiAddr.getPhoneField(PilotAddress::eWork));
	_copyPhone(toAbEntry, toAbEntry.phoneNumber(KABC::PhoneNumber::Cell), fromPiAddr.getPhoneField(PilotAddress::eMobile));
	_copyPhone(toAbEntry, getFax(toAbEntry), fromPiAddr.getPhoneField(PilotAddress::eFax));
	_copyPhone(toAbEntry, toAbEntry.phoneNumber(KABC::PhoneNumber::Pager), fromPiAddr.getPhoneField(PilotAddress::ePager));
	setOtherField(toAbEntry, fromPiAddr.getPhoneField(PilotAddress::eOther));

	// TODO: in future, probably the address assigning to work or home should be a prefs option
	// for now, just assign to home since that's what I'm using it for
	KABC::Address homeAddress = getAddress(toAbEntry);//KABC::Address::Home);
	homeAddress.setStreet(fromPiAddr.getField(entryAddress));
	homeAddress.setLocality(fromPiAddr.getField(entryCity));
	homeAddress.setRegion(fromPiAddr.getField(entryState));
	homeAddress.setPostalCode(fromPiAddr.getField(entryZip));
	homeAddress.setCountry(fromPiAddr.getField(entryCountry));
	toAbEntry.insertAddress(homeAddress);

	toAbEntry.insertCustom(appString, "CUSTOM1", fromPiAddr.getField(entryCustom1));
	toAbEntry.insertCustom(appString, "CUSTOM2", fromPiAddr.getField(entryCustom2));
	toAbEntry.insertCustom(appString, "CUSTOM3", fromPiAddr.getField(entryCustom3));
	toAbEntry.insertCustom(appString, "CUSTOM4", fromPiAddr.getField(entryCustom4));

	// copy the fromPiAddr pilot id to the custom field KPilot_Id;
	// pilot id may be zero (since it could be new) but couldn't hurt
	// to even assign it to zero; let's us know what state the
	// toAbEntry is in
	toAbEntry.insertCustom(appString, idString, QString::number(fromPiAddr.getID()));
	
	// TODO: Sync categories
	// first remove all categories and then add only the appropriate one
	for (int j=1; j<=15; j++) 
	{
		toAbEntry.removeCategory(fAddressAppInfo.category.name[j]);
	}
	int cat=fromPiAddr.getCat();
	if (0<cat && cat<=15) 
	{
		toAbEntry.insertCategory( fAddressAppInfo.category.name[cat] );
	}
//showAddressee(toAbEntry);
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
	return value ... true if the merge could be done. false if the entry could not be merged (e.g. use chose to add both records or no resolution at all.
*/
int AbbrowserConduit::_conflict(const QString &entry, const QString &field, const QString &palm, 
	const QString &backup, const QString &pc, bool & mergeNeeded, QString & mergedStr)
{
	FUNCTIONSETUP;
	mergeNeeded = false;

		// if both entries are already the same, no need to do anything
	if (pc == palm) return CHANGED_NONE;
		// only pc modified, so return that string, no conflict
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": conflicting entries for "<<field<<" of "<<entry<<" are: palm="<<palm<<", backup="<<backup<<", pc="<<pc<<endl;
#endif
	if (palm == backup) {
		mergeNeeded=true;
		mergedStr=pc;
		return CHANGED_PALM;
	}
		// only palm modified, so return that string, no conflict
	if (pc == backup) {
		mergeNeeded=true;
		mergedStr=palm;
		return CHANGED_PC;
	}
	
	// all three differ => conflict => deconfliction. Use already chosen resolution option if possible
	EConflictResolution fieldres=getFieldResolution(entry, field, palm, backup, pc);
#ifdef DEBUG
	DEBUGCONDUIT<<"fieldres="<<fieldres<<endl;
#endif
	switch ( fieldres )
	{
		case eAbbrowserOverides:
			mergeNeeded=true;
			mergedStr=pc;
			return CHANGED_PALM;
			break;
		case ePilotOverides:
			mergeNeeded=true;
			mergedStr=palm;
			return CHANGED_PC;
			break;
		case eRevertToBackup:
			mergeNeeded=true;
			mergedStr=backup;
			return CHANGED_BOTH;
			break;
		case eKeepBothInAbbrowser:
			return CHANGED_DUPLICATE;
		case eDoNotResolve:
		default:
			return CHANGED_NORES;
	}
	return CHANGED_NONE;
}



int AbbrowserConduit::_compare(const QString &str1, const QString &str2) const
{
	 FUNCTIONSETUP;
#ifdef DEBUG
	if (str1.isEmpty() && str2.isEmpty() )  {}
	else 
	{
		if (str1.compare(str2)) 
		{
			DEBUGCONDUIT<<"Not equal: "<< str1<<" vs. " <<str2<<endl;
		}
	}
#endif
	
	if (str1.isEmpty() && str2.isEmpty() ) return 0;
	else return str1.compare(str2);
}

//	phone=abEntry.phoneNumber(KABC::PhoneNumber::Work);
//	res=_conflict(thisName, i18n("work phone"), pilotAddress.getPhoneField(PilotAddress::eWork), backupAddress.getPhoneField(PilotAddress::eWork), phone.number(), mergeNeeded, mergedStr);
//	if (res & CHANGED_NORES) return res;
//	if (mergeNeeded)
//	{
//		pilotAddress.setPhoneField(PilotAddress::eWork, mergedStr.latin1());
//		phone.setNumber(mergedStr);
//		abEntry.insertPhoneNumber(phone);
//	}


int AbbrowserConduit::_smartMergePhone(KABC::Addressee &abEntry, const PilotAddress &backupAddress, 
	PilotAddress &pilotAddress, PilotAddress::EPhoneType PalmFlag, KABC::PhoneNumber phone, 
	QString thisName, QString name)
{
	bool mergeNeeded=false;
	QString mergedStr;
	
//	KABC::PhoneNumber phone=abEntry.phoneNumber(PCFlag);
	int res=_conflict(thisName, name, pilotAddress.getPhoneField(PalmFlag), backupAddress.getPhoneField(PalmFlag), phone.number(), mergeNeeded, mergedStr);
	if (res & CHANGED_NORES) return res;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PalmFlag, mergedStr.latin1());
		phone.setNumber(mergedStr);
		abEntry.insertPhoneNumber(phone);
	}
	return -1;
}

int AbbrowserConduit::_smartMergeEntry(QString abEntry, const PilotAddress &backupAddress, PilotAddress &pilotAddress, int PalmFlag, QString thisName, QString name, QString &mergedString)
{
	bool mergeNeeded=false;
	QString mergedStr;
	mergedString=QString();
DEBUGCONDUIT<<"PalmFlag="<<PalmFlag<<", Palm: "<<pilotAddress.getField(PalmFlag)<<", Backup: "<<backupAddress.getField(PalmFlag)<<endl;
	
	int res=_conflict(thisName, name, pilotAddress.getField(PalmFlag), backupAddress.getField(PalmFlag), abEntry, mergeNeeded, mergedStr);
	if (res & CHANGED_NORES) return res;
	if (mergeNeeded)
	{
		pilotAddress.setField(entryCompany, mergedStr.latin1());
		mergedString=mergedStr;
	}
	return -1;
}


void AbbrowserConduit::showAdresses(PilotAddress & pilotAddress, const PilotAddress & backupAddress, KABC::Addressee & abEntry) {
#ifdef DEBUG
	DEBUGCONDUIT<<"abEntry:"<<endl;
	showAddressee(abEntry);
	DEBUGCONDUIT<<"pilotAddress:"<<endl;
	showPilotAddress(pilotAddress);
	DEBUGCONDUIT<<"backupAddress:"<<endl;
	showPilotAddress(backupAddress);
	DEBUGCONDUIT<<"------------------------------------------------"<<endl;
#endif
}

int AbbrowserConduit::_smartMerge(PilotAddress & outPilotAddress, const PilotAddress & backupAddress, KABC::Addressee & outAbEntry)
{
	FUNCTIONSETUP;
	fEntryResolution=getResolveConflictOption();
	PilotAddress pilotAddress(outPilotAddress);
	// TODO: Is the AddressID copied too?????
	KABC::Addressee abEntry(outAbEntry);
	QString thisName(outAbEntry.realName());
	bool mergeNeeded = false;
	QString mergedStr;
	int res;
//	KABC::PhoneNumber phone;

#ifdef DEBUG
//	DEBUGCONDUIT<<endl<<"************************************************************************"<<"Smartly merging:"<<endl;
//	showAdresses(pilotAddress, backupAddress, abEntry);
#endif

	res=_smartMergeEntry(abEntry.familyName(), backupAddress, pilotAddress, (int)entryLastname, thisName, i18n("last name"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.setFamilyName(mergedStr);

	res=_smartMergeEntry(abEntry.givenName(), backupAddress, pilotAddress, (int)entryFirstname, thisName, i18n("first name"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.setGivenName(mergedStr);

	res=_smartMergeEntry(abEntry.organization(), backupAddress, pilotAddress, (int)entryCompany, thisName, i18n("organization"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.setOrganization(mergedStr);

	res=_smartMergeEntry(abEntry.title(), backupAddress, pilotAddress, (int)entryTitle, thisName, i18n("title"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.setTitle(mergedStr);

	res=_smartMergeEntry(abEntry.note(), backupAddress, pilotAddress, (int)entryNote, thisName, i18n("note"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.setNote(mergedStr);

	res=_smartMergeEntry(abEntry.custom(appString, "CUSTOM1"), backupAddress, pilotAddress, entryCustom1, thisName, i18n("custom 1"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.insertCustom(appString, "CUSTOM1", mergedStr);

	res=_smartMergeEntry(abEntry.custom(appString, "CUSTOM2"), backupAddress, pilotAddress, entryCustom2, thisName, i18n("custom 2"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.insertCustom(appString, "CUSTOM2", mergedStr);

	res=_smartMergeEntry(abEntry.custom(appString, "CUSTOM3"), backupAddress, pilotAddress, entryCustom3, thisName, i18n("custom 3"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.insertCustom(appString, "CUSTOM3", mergedStr);

	res=_smartMergeEntry(abEntry.custom(appString, "CUSTOM4"), backupAddress, pilotAddress, entryCustom4, thisName, i18n("custom 4"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abEntry.insertCustom(appString, "CUSTOM4", mergedStr);

#ifdef DEBUG
//	DEBUGCONDUIT<<endl<<endl<<"After personal fields:"<<endl<<endl;
//	showAdresses(pilotAddress, backupAddress, abEntry);
#endif

	res=_smartMergePhone(abEntry, backupAddress, pilotAddress, PilotAddress::eWork, abEntry.phoneNumber(KABC::PhoneNumber::Work), thisName, i18n("work phone"));
	if (res>=0) return res;
	res=_smartMergePhone(abEntry, backupAddress, pilotAddress, PilotAddress::eHome, abEntry.phoneNumber(KABC::PhoneNumber::Home), thisName, i18n("home phone"));
	if (res>=0) return res;
	res=_smartMergePhone(abEntry, backupAddress, pilotAddress, PilotAddress::eMobile, abEntry.phoneNumber(KABC::PhoneNumber::Cell), thisName, i18n("mobile phone"));
	if (res>=0) return res;
	res=_smartMergePhone(abEntry, backupAddress, pilotAddress, PilotAddress::eFax, getFax(abEntry), thisName, i18n("fax"));
	if (res>=0) return res;
	res=_smartMergePhone(abEntry, backupAddress, pilotAddress, PilotAddress::ePager, abEntry.phoneNumber(KABC::PhoneNumber::Pager), thisName, i18n("pager"));
	if (res>=0) return res;
	
	res=_conflict(thisName, i18n("other"), 
		pilotAddress.getPhoneField(PilotAddress::eOther), 
		backupAddress.getPhoneField(PilotAddress::eOther), 
		getOtherField(abEntry), mergeNeeded, mergedStr);
	if (res & CHANGED_NORES) return res;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eOther, mergedStr.latin1());
		setOtherField(abEntry, mergedStr);
	}

	res=_conflict(thisName, i18n("email"), 
		pilotAddress.getPhoneField(PilotAddress::eEmail), 
		backupAddress.getPhoneField(PilotAddress::eEmail), 
		abEntry.preferredEmail(), mergeNeeded, mergedStr);
	if (res & CHANGED_NORES) return res;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eEmail, mergedStr.latin1());
		abEntry.removeEmail(backupAddress.getPhoneField(PilotAddress::eEmail));
		abEntry.removeEmail(pilotAddress.getPhoneField(PilotAddress::eEmail));
		abEntry.insertEmail(mergedStr, true);
	}

	

#ifdef DEBUG
//	DEBUGCONDUIT<<endl<<endl<<"After phone fields:"<<endl<<endl;
//	showAdresses(pilotAddress, backupAddress, abEntry);
#endif
	
	
	KABC::Address abAddress=getAddress(abEntry);
/*	if (isPilotStreetHome()) abAddress = abEntry.address(KABC::Address::Home);
	else abAddress = abEntry.address(KABC::Address::Work);*/
	
	res=_smartMergeEntry(abAddress.street(), backupAddress, pilotAddress, entryAddress, thisName, i18n("address"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abAddress.setStreet(mergedStr);

	res=_smartMergeEntry(abAddress.locality(), backupAddress, pilotAddress, entryCity, thisName, i18n("city"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abAddress.setLocality(mergedStr);

	res=_smartMergeEntry(abAddress.region(), backupAddress, pilotAddress, entryState, thisName, i18n("region"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abAddress.setRegion(mergedStr);

	res=_smartMergeEntry(abAddress.postalCode(), backupAddress, pilotAddress, entryZip, thisName, i18n("postal code"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abAddress.setPostalCode(mergedStr);

	res=_smartMergeEntry(abAddress.country(), backupAddress, pilotAddress, entryCountry, thisName, i18n("country"), mergedStr);
	if (res>=0) return res;
	else if (!mergedStr.isEmpty()) abAddress.setCountry(mergedStr);

	abEntry.insertAddress(abAddress);

	abEntry.insertCustom(appString, idString, QString::number(pilotAddress.id()));
	// TODO: Merge category
	abEntry.insertCategory(pilotAddress.getCategoryLabel());


#ifdef DEBUG
//	DEBUGCONDUIT<<endl<<endl<<"After merge fields:"<<endl<<endl;
//	showAdresses(pilotAddress, backupAddress, abEntry);
#endif
	
	outPilotAddress = pilotAddress;
	outAbEntry = abEntry;

	return CHANGED_BOTH;
}


/** Merge the palm and the pc entries with the additional information of the backup record. Calls _handleConflict 
 * which does the actual syncing of the data structures. According to the return value of _handleConflict, this function
 * writes the data back to the palm/pc.
 *  return value: no meaning yet
 */
int AbbrowserConduit::_mergeEntries(PilotAddress &pilotAddress, PilotAddress &backupAddress, KABC::Addressee &abEntry)
{
	FUNCTIONSETUP;
	int res=_handleConflict(pilotAddress, backupAddress, abEntry);
	if (res & CHANGED_NORES)
	{
		if (res & CHANGED_DUPLICATE)
		{
#ifdef DEBUG
DEBUGCONDUIT<<"res & CHANGED_ADD"<<endl;
#endif
			if (res & CHANGED_PALM) 
			{
#ifdef DEBUG
DEBUGCONDUIT<<"res & CHANGED_PALM"<<endl;
#endif
				_addToPalm(abEntry);
			}
			if (res & CHANGED_PC)
			{
#ifdef DEBUG
DEBUGCONDUIT<<"res & CHANGED_PC"<<endl;
#endif
				_addToAbbrowser(pilotAddress);
//				_saveBackupAddress(pilotAddress);
			}
		}
	}
	else
	{
#ifdef DEBUG
DEBUGCONDUIT<<" ! res & CHANGED_ADD"<<endl;
#endif
		if (res & CHANGED_PALM)
		{
#ifdef DEBUG
DEBUGCONDUIT<<"res & CHANGED_PALM"<<endl;
#endif
			_savePilotAddress(pilotAddress, abEntry);
		}
		if (res & CHANGED_PC)
		{
#ifdef DEBUG
DEBUGCONDUIT<<"res & CHANGED_PC"<<endl;
#endif
			_saveAbEntry(abEntry);
			_saveBackupAddress(pilotAddress);
		}
	}
	return 0;
}


/** There was a conflict between the two fields; either could be null,
 *  or both have been modified
 *  return value: returns either of the CHANGED_* constants to determine which records need to be written back
 */
int AbbrowserConduit::_handleConflict(PilotAddress &pilotAddress, PilotAddress &backupAddress, KABC::Addressee &abEntry)
{
	FUNCTIONSETUP;
#ifdef DEBUG
DEBUGCONDUIT<<"abEntry.isEmpty()="<<abEntry.isEmpty()<<endl;
DEBUGCONDUIT<<"abEntry.givenName()="<<abEntry.givenName()<<endl;
#endif

	
	if (abEntry.isEmpty()) 
	{
		_copy(abEntry, pilotAddress);
		return CHANGED_PC | CHANGED_ADD;
	}
	// TODO: if pilotAddress is empty????

	if (_equal(pilotAddress, abEntry)) return CHANGED_NONE;
	
#ifdef DEBUG
DEBUGCONDUIT<<"Not equal, so compare pilotAddress with backupAddress"<<endl;
#endif
	if (pilotAddress == backupAddress) {
#ifdef DEBUG
DEBUGCONDUIT<<"pilotAddress==backupAddress"<<endl;
#endif
		if (!_equal(backupAddress, abEntry)) {
#ifdef DEBUG
DEBUGCONDUIT<<"!_equal(backupAddress, abEntry)"<<endl;
#endif
			_copy(pilotAddress, abEntry);
			return CHANGED_PALM;
		} else {
#ifdef DEBUG
			DEBUGCONDUIT<<fname<<": pilot==backup and backup==abEntry, but pilot!=abEntry !!!!! BUG, BUG, BUG"<<endl;
#endif
			// This should never be called, since that would mean pilotAddress and backupAddress are equal, which we already checked!!!!
			return CHANGED_NONE; 
		}
	} else {
#ifdef DEBUG
DEBUGCONDUIT<<"pilotAddress!=backupAddress"<<endl;
#endif
		if (_equal(backupAddress, abEntry)) {
#ifdef DEBUG
DEBUGCONDUIT<<"_equal(backupAddress, abEntry)"<<endl;
#endif
			_copy(abEntry, pilotAddress);
			return CHANGED_PC;
		} else {
#ifdef DEBUG
DEBUGCONDUIT<<"!_equal(backupAddress, abEntry), Attempt a merge..."<<endl;
#endif
			// Both pc and palm were changed => merge, override, duplicate or ignore.
			if (doSmartMerge()) {
				PilotAddress pAdr(pilotAddress);
				KABC::Addressee abEnt(abEntry);
				int res=_smartMerge(pilotAddress, backupAddress, abEntry);
				switch (res) {
					case CHANGED_NORES:
					case CHANGED_DUPLICATE:
						pilotAddress=pAdr;
						abEntry=abEnt;
				}
//					case eUserChoose:
//					case eKeepBothInAbbrowser:
//						pilotAddress=pAdr;
//						abEntry=abEnt;
//						return ADD_BOTH; break;
//					case eRevertToBackup: return CHANGED_BOTH; break;
//					case ePilotOverides: return CHANGED_PC; break;
//					case eAbbrowserOverides: return CHANGED_PALM; break;
//					case eDoNotResolve:
//					default:
//						pilotAddress=pAdr;
//						abEntry=abEnt;
//						return CHANGED_NONE;
//						break;
//				}
				return res;
			} else { // no smart merge
				switch ( getEntryResolution(abEntry, pilotAddress) ) {
					case eKeepBothInAbbrowser: 
						return CHANGED_DUPLICATE; 
					case ePilotOverides: 
						_copy(abEntry, pilotAddress);
						return CHANGED_PC; 
					case eAbbrowserOverides: 
						_copy(pilotAddress, abEntry);
						return CHANGED_PALM; 
					case eRevertToBackup: 
						_copy(abEntry, backupAddress);
						pilotAddress=backupAddress;
						return CHANGED_BOTH; 
					case eDoNotResolve:
						return CHANGED_NORES;
					default:
						return CHANGED_NONE;
				}
			} // smart merge
		} // backup == abook
	} // pilot== backup
	
	return CHANGED_NONE;
}

AbbrowserConduit::EConflictResolution AbbrowserConduit::getFieldResolution(const QString &entry, const QString &field, const QString &palm, const QString &backup, const QString &pc)
{
	FUNCTIONSETUP;
	EConflictResolution res=fEntryResolution;
	if (res==eUserChoose) 
	{
		res=getResolveConflictOption();
	}
	switch (res) {
		case eKeepBothInAbbrowser:
		case ePilotOverides:
		case eAbbrowserOverides:
		case eRevertToBackup:
		case eDoNotResolve:
			return res; break;
		case eUserChoose:
		default:
			QStringList lst;
			lst <<
					i18n("Leave untouched")<<
					i18n("Handheld overrides") << 
					i18n("PC overrides") <<
					i18n("Use the value from the last sync") <<
					i18n("Duplicate both");
			bool remember=FALSE;
			res=ResolutionDialog(i18n("Address conflict"), i18n("<html><p>The field \"%1\" of the entry \"%2\" was changed on the handheld and on the PC.</p>"
					"<table border=0>"
					"<tr><td><b>Handheld:</b></td><td>%3</td></tr>"
					"<tr><td><b>PC:</b></td><td>%4</td></tr>"
					"<tr><td><b>last sync:</b></td><td>%5</td></tr>"
					"</table>"
					"<p>How should this conflict be resolved?</p></html>")
					.arg(field)
					.arg(entry)
					.arg(palm)
					.arg(pc)
					.arg(backup),
					lst, i18n("Apply to all fields of this entry"), &remember);
			if (remember) 
			{
				fEntryResolution=res;
			}
			return res;
			
	}
}

AbbrowserConduit::EConflictResolution AbbrowserConduit::getEntryResolution(const KABC::Addressee & abEntry, const PilotAddress &pilotAddress) 
{
	FUNCTIONSETUP;
	EConflictResolution res=getResolveConflictOption();
	switch (res) {
		case eKeepBothInAbbrowser:
		case ePilotOverides:
		case eAbbrowserOverides:
		case eRevertToBackup:
		case eDoNotResolve:
			return res; break;
		case eUserChoose:
		default:
			QStringList lst;
			lst <<
					i18n("Leave untouched") <<
					i18n("Handheld overrides") << 
					i18n("PC overrides") <<
					i18n("Use the value from the last sync");
			if (!abEntry.isEmpty())  lst << i18n("Duplicate both");
			bool remember=FALSE;
			res=ResolutionDialog(i18n("Address conflict"), i18n("<html><p>The following record was changed on the handheld and on the PC. </p>"
					"<table border=0>"
					"<tr><td><b>Handheld:</b></td><td>%1 %2</td></tr>"
					"<tr><td><b>PC:</b></td><td>%3</td></tr>"
					"</table>"
					"<p>How should this conflict be resolved?</p></html>")
					.arg(pilotAddress.getField(entryFirstname))
					.arg(pilotAddress.getField(entryLastname))
					.arg(abEntry.isEmpty()?i18n("(deleted)"):abEntry.realName()), 
					lst, i18n("Remember my choice for all other records"), &remember);
			if (remember)
			{
				fConflictResolution=res;
			}
			return res;
	}
}


AbbrowserConduit::EConflictResolution AbbrowserConduit::ResolutionDialog(QString Title, QString Text, QStringList &lst, QString remember, bool*rem) const
{
	FUNCTIONSETUP;
	ResolutionDlg*dlg=new ResolutionDlg(0L, fHandle, Title, Text, lst, remember);
	if (dlg->exec()==KDialogBase::Cancel) 
	{
		delete dlg;
		return eDoNotResolve;
	}
	else
	{
		EConflictResolution res=(EConflictResolution)(dlg->ResolutionButtonGroup->id(dlg->ResolutionButtonGroup->selected()) )+1;
cout<<"res="<<res<<endl;
		if (!remember.isEmpty() && rem)
		{
			*rem=dlg->rememberCheck->isChecked();
		}
		delete dlg;
		return res;
	}
}


// TODO: right now entries are equal if both first/last name and organization are 
//		 equal. This rules out two entries for the same person (e.g. real home and weekend home)
//		 or two persons with the same name where you don't know the organization.!!!
KABC::Addressee AbbrowserConduit::_findMatch(const PilotAddress & pilotAddress) const
{
	FUNCTIONSETUP;
	// TODO: also search with the pilotID
	// first, use the pilotID to UID map to find the appropriate record
	if (!fFirstTime && (pilotAddress.id()>0) )
	{
		QString id(addresseeMap[pilotAddress.id()] );
#ifdef DEBUG
			DEBUGCONDUIT<<fname<<": PilotRecord has id "<<pilotAddress.id()<<", mapped to "<<id<<endl;
#endif			
		if (!id.isEmpty())
		{
			KABC::Addressee res(aBook->findByUid(id));
			if (!res.isEmpty()) return res;
#ifdef DEBUG
			DEBUGCONDUIT<<fname<<": PilotRecord has id "<<pilotAddress.id()<<", but could not be found in the addressbook"<<endl;
#endif			
		}
	}
#ifdef DEBUG
			DEBUGCONDUIT<<fname<<": PilotRecord has id "<<pilotAddress.id()<<endl;
#endif			
	
	bool piFirstEmpty = (pilotAddress.getField(entryFirstname) == 0L);
	bool piLastEmpty = (pilotAddress.getField(entryLastname) == 0L);
	bool piCompanyEmpty = (pilotAddress.getField(entryCompany) == 0L);

	// return not found if not matching keys
	if (piFirstEmpty && piLastEmpty && piCompanyEmpty)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": entry has empty first, last name and company! Will not search in Addressbook"<<endl;
#endif
		return KABC::Addressee();
	}

	// for now just loop throug all entries; in future, probably better
	// to create a map for first and last name, then just do O(1) calls
	for (KABC::AddressBook::Iterator iter=aBook->begin(); iter!=aBook->end(); ++iter)
	{
		KABC::Addressee abEntry = *iter;
		// do quick empty check's
		if (piFirstEmpty != abEntry.givenName().isEmpty() ||
			piLastEmpty != abEntry.familyName().isEmpty() ||
			piCompanyEmpty != abEntry.organization().isEmpty())
		{
			continue;
		}
		if (piFirstEmpty && piLastEmpty)
		{
			if (abEntry.organization() ==
				pilotAddress.getField(entryCompany))
			{
				return *iter;
			}
		}
		else
			// the first and last name must be equal; they are equal
			// if they are both empty or the strings match
		if (((piLastEmpty && abEntry.familyName().isEmpty())
				|| (abEntry.familyName() == pilotAddress.getField(entryLastname)))
			&& ((piFirstEmpty	&& abEntry.givenName().isEmpty())
				|| (abEntry.givenName() == pilotAddress.getField(entryFirstname))))
		{
			return *iter;
		}

	}
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": Could not find any addressbook enty matching "<<pilotAddress.getField(entryLastname)<<endl;
#endif
	return KABC::Addressee();
}



/*
int AbbrowserConduit::_getCatId(int catIndex) const
{
	FUNCTIONSETUP;
	return fAddressAppInfo.category.ID[catIndex];
}
*/


// $Log$
// Revision 1.42  2002/08/16 18:54:06  kainhofe
// Tried to fix the conduit, but KABC has changed so much that the conduit was totaly broken again. I will not touch the conduit any more until KABC is really solid and doesn't change any more!!!!
//
// Revision 1.41  2002/08/15 21:40:14  kainhofe
// some more work in the addressbook conduit. Does not yet work
//
// Revision 1.40  2002/07/23 00:52:02  kainhofe
// Reorder the resolution methods
//
// Revision 1.39  2002/07/20 18:50:45  kainhofe
// added a terrible hack to add new contacts to the addressbook. Need to fix kabc for this...
//
// Revision 1.38  2002/07/11 13:27:28  mhunter
// Corrected typographical errors
//
// Revision 1.37  2002/07/09 22:40:18  kainhofe
// backup database fixes, prevent duplicate vcal entries, fixed the empty record that was inserted on the palm on every sync
//
// Revision 1.36  2002/07/04 23:47:00  kainhofe
// Phone, email and address entries are no longer duplicated on a sync.
//
// Revision 1.35  2002/07/01 23:25:46  kainhofe
// implemented categories syncing, many things seem to work, but still every sync creates an empty zombie.
//
// Revision 1.34  2002/06/30 22:17:50  kainhofe
// some cleanup. Changes from the palm are still not applied to the pc, pc->palm still disabled.
//
// Revision 1.33  2002/06/30 16:23:23  kainhofe
// Started rewriting the addressbook conduit to use libkabc instead of direct dcop communication with abbrowser. Palm->PC is enabled (but still creates duplicate addresses), the rest is completely untested and thus disabled for now
//

