/* KPilot
**
** Copyright (C) 2000,2001 by Dan Pilone
** Copyright (C) 2002-2003 by Reinhold Kainhofer
** Copyright (C) 2007 by Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/



#include "options.h"

#include <qtimer.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qregexp.h>

#include <kabc/stdaddressbook.h>
#include <kabc/resourcefile.h>
#include <kio/netaccess.h>
#include <ksavefile.h>

#include <pilotSerialDatabase.h>
#include <pilotLocalDatabase.h>

#include "resolutionDialog.h"
#include "resolutionTable.h"
#include "abbrowserSettings.h"
#include "kabcRecord.h"

#include "abbrowser-conduit.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
extern "C"
{
unsigned long version_conduit_address = Pilot::PLUGIN_API;
}


/* This is partly stolen from the boost libraries, partly from
*  "Modern C++ design" for doing compile time checks; we need
*  to make sure that the enum values in KABCSync:: and in the
*  AbbrowserSettings class are the same so that both interpret
*  configuration values the same way.
*/
template<bool> struct EnumerationMismatch;
template<> struct EnumerationMismatch<true>{};

#define CHECK_ENUM(a) (void)sizeof(EnumerationMismatch<((int)KABCSync::a)==((int)AbbrowserSettings::a)>)

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

inline int faxTypeOnPC()
{
	return KABC::PhoneNumber::Fax |
		( (AbbrowserSettings::pilotFax()==0) ?
			KABC::PhoneNumber::Home :
			KABC::PhoneNumber::Work );
}


using namespace KABC;

/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/


AbbrowserConduit::AbbrowserConduit(KPilotLink * o, const char *n, const QStringList & a):
		ConduitAction(o, n, a),
		aBook(0L),
		fAddressAppInfo(0L),
		addresseeMap(),
		syncedIds(),
		abiter(),
		fTicket(0L),
		fCreatedBook(false),
		fBookResource(0L)
{
	FUNCTIONSETUP;
	fConduitName=i18n("Addressbook");
}



AbbrowserConduit::~AbbrowserConduit()
{
	FUNCTIONSETUP;

	if (fTicket)
	{
		DEBUGKPILOT << fname << ": Releasing ticket" << endl;
		aBook->releaseSaveTicket(fTicket);
		fTicket=0L;
	}

	_cleanupAddressBookPointer();
	// unused function warnings.
	compile_time_check();
}



/*********************************************************************
                L O A D I N G   T H E   D A T A
 *********************************************************************/



/* Builds the map which links record ids to uid's of Addressee
*/
void AbbrowserConduit::_mapContactsToPilot(QMap < recordid_t, QString > &idContactMap)
{
	FUNCTIONSETUP;

	idContactMap.clear();

	for(AddressBook::Iterator contactIter = aBook->begin();
		contactIter != aBook->end(); ++contactIter)
	{
		Addressee aContact = *contactIter;
		QString recid = aContact.custom(KABCSync::appString, KABCSync::idString);
		if(!recid.isEmpty())
		{
			recordid_t id = recid.toULong();
			// safety check:  make sure that we don't already have a map for this pilot id.
			// if we do (this can come from a copy/paste in kaddressbook, etc.), then we need
			// to reset our Addressee so that we can assign him a new pilot Id later and sync
			// him properly.  if we don't do this, we'll lose one of these on the pilot.
			if (!idContactMap.contains(id))
			{
				idContactMap.insert(id, aContact.uid());
			}
			else
			{
		DEBUGKPILOT << fname << ": found duplicate pilot key: ["
						<< id << "], removing pilot id from addressee: ["
						<< aContact.realName() << "]" << endl;
				aContact.removeCustom(KABCSync::appString, KABCSync::idString);
				aBook->insertAddressee(aContact);
				abChanged = true;
			}
		}
	}
	DEBUGKPILOT << fname << ": Loaded " << idContactMap.size() <<
	    " addresses from the addressbook. " << endl;
}



bool AbbrowserConduit::_prepare()
{
	FUNCTIONSETUP;

	readConfig();
	syncedIds.clear();
	pilotindex = 0;

	return true;
}



void AbbrowserConduit::readConfig()
{
	FUNCTIONSETUP;
	AbbrowserSettings::self()->readConfig();

	// Conflict page
	SyncAction::ConflictResolution res = (SyncAction::ConflictResolution)AbbrowserSettings::conflictResolution();
	setConflictResolution(res);

	DEBUGKPILOT << fname
		<< ": Reading addressbook "
		<< ( AbbrowserSettings::addressbookType() == AbbrowserSettings::eAbookFile ?
			AbbrowserSettings::fileName() : CSL1("Standard") )
		<< endl;
	DEBUGKPILOT << fname
		<< ": "
		<< " fConflictResolution=" << getConflictResolution()
		<< " fArchive=" << AbbrowserSettings::archiveDeleted()
		<< " fFirstTime=" << isFirstSync()
		<< endl;
	DEBUGKPILOT << fname
		<< ": "
		<< " fPilotStreetHome=" << AbbrowserSettings::pilotStreet()
		<< " fPilotFaxHome=" << AbbrowserSettings::pilotFax()
		<< " eCustom[0]=" << AbbrowserSettings::custom0()
		<< " eCustom[1]=" << AbbrowserSettings::custom1()
		<< " eCustom[2]=" << AbbrowserSettings::custom2()
		<< " eCustom[3]=" << AbbrowserSettings::custom3()
		<< endl;
}



bool isDeleted(const PilotAddress *addr)
{
	if (!addr)
	{
		return true;
	}
	if (addr->isDeleted() && !addr->isArchived())
	{
		return true;
	}
	if (addr->isArchived())
	{
		return !AbbrowserSettings::archiveDeleted();
	}
	return false;
}

bool isArchived(const PilotAddress *addr)
{
	if (addr && addr->isArchived())
	{
		return AbbrowserSettings::archiveDeleted();
	}
	else
	{
		return false;
	}
}



bool AbbrowserConduit::_loadAddressBook()
{
	FUNCTIONSETUP;

	startTickle();
	switch ( AbbrowserSettings::addressbookType() )
	{
		case AbbrowserSettings::eAbookResource:
			DEBUGKPILOT<<"Loading standard addressbook"<<endl;
			aBook = StdAddressBook::self( true );
			fCreatedBook=false;
			break;
		case AbbrowserSettings::eAbookFile:
		{ // initialize the abook with the given file
			DEBUGKPILOT<<"Loading custom addressbook"<<endl;
			KURL kurl(AbbrowserSettings::fileName());
			if(!KIO::NetAccess::download(AbbrowserSettings::fileName(), fABookFile, 0L) &&
				!kurl.isLocalFile())
			{
				emit logError(i18n("You chose to sync with the file \"%1\", which "
							"cannot be opened. Please make sure to supply a "
							"valid file name in the conduit's configuration dialog. "
							"Aborting the conduit.").arg(AbbrowserSettings::fileName()));
				KIO::NetAccess::removeTempFile(fABookFile);
				stopTickle();
				return false;
			}

			aBook = new AddressBook();
			if (!aBook)
			{
				stopTickle();
				return false;
			}
			fBookResource = new ResourceFile(fABookFile, CSL1("vcard") );

			bool r = aBook->addResource( fBookResource );
			if ( !r )
			{
				DEBUGKPILOT << "Unable to open resource for file " << fABookFile << endl;
				KPILOT_DELETE( aBook );
				stopTickle();
				return false;
			}
			fCreatedBook=true;
			break;
		}
		default: break;
	}
	// find out if this can fail for reasons other than a non-existent
	// vcf file. If so, how can I determine if the missing file was the problem
	// or something more serious:
	if ( !aBook || !aBook->load() )
	{
		// Something went wrong, so tell the user and return false to exit the conduit
		emit logError(i18n("Unable to initialize and load the addressbook for the sync.") );
		addSyncLogEntry(i18n("Unable to initialize and load the addressbook for the sync.") );
		WARNINGKPILOT << "Unable to initialize the addressbook for the sync." << endl;
		_cleanupAddressBookPointer();
		stopTickle();
		return false;
	}
	abChanged = false;

	fTicket=aBook->requestSaveTicket();
	if (!fTicket)
	{
		WARNINGKPILOT << "Unable to lock addressbook for writing " << endl;
		emit logError(i18n("Unable to lock addressbook for writing.  Can't sync!"));
		addSyncLogEntry(i18n("Unable to lock addressbook for writing.  Can't sync!"));
		_cleanupAddressBookPointer();
		stopTickle();
		return false;
	}

	fCtrPC->setStartCount(aBook->allAddressees().count());

	// get the addresseMap which maps Pilot unique record(address) id's to
	// a Abbrowser Addressee; allows for easy lookup and comparisons
	if(aBook->begin() == aBook->end())
	{
		setFirstSync( true );
	}
	else
	{
		_mapContactsToPilot(addresseeMap);
	}
	stopTickle();
	return(aBook != 0L);
}

bool AbbrowserConduit::_saveAddressBook()
{
	FUNCTIONSETUP;

	bool saveSuccessful = false;

	fCtrPC->setEndCount(aBook->allAddressees().count());

	Q_ASSERT(fTicket);

	if (abChanged)
	{
		saveSuccessful = aBook->save(fTicket);
	}
	else
	{
		DEBUGKPILOT << fname
			<< "Addressbook not changed, no need to save it" << endl;
	}
	// XXX: KDE4: release ticket in all cases (save no longer releases it)
	if ( !saveSuccessful ) // didn't save, delete ticket manually
	{
		aBook->releaseSaveTicket(fTicket);
	}
	fTicket=0L;

	if ( AbbrowserSettings::addressbookType()!= AbbrowserSettings::eAbookResource )
	{
		KURL kurl(AbbrowserSettings::fileName());
		if(!kurl.isLocalFile())
		{
			DEBUGKPILOT << fname << "Deleting local addressbook tempfile" << endl;
			if(!KIO::NetAccess::upload(fABookFile, AbbrowserSettings::fileName(), 0L)) {
				emit logError(i18n("An error occurred while uploading \"%1\". You can try to upload "
					"the temporary local file \"%2\" manually")
					.arg(AbbrowserSettings::fileName()).arg(fABookFile));
			}
			else {
				KIO::NetAccess::removeTempFile(fABookFile);
			}
			QFile backup(fABookFile + CSL1("~"));
			backup.remove();
		}

	}

	// now try to remove the resource from the addressbook...
	if (fBookResource) 
	{
		bool r = aBook->removeResource( fBookResource );
		if ( !r )
		{
			DEBUGKPILOT << fname <<": Unable to close resource." << endl;
		}
	}

	return saveSuccessful;
}



void AbbrowserConduit::_getAppInfo()
{
	FUNCTIONSETUP;

	delete fAddressAppInfo;
	fAddressAppInfo = new PilotAddressInfo(fDatabase);
	fAddressAppInfo->dump();
}

void AbbrowserConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	if (fDatabase) fAddressAppInfo->writeTo(fDatabase);
	if (fLocalDatabase) fAddressAppInfo->writeTo(fLocalDatabase);
}


void AbbrowserConduit::_cleanupAddressBookPointer()
{
        if (fCreatedBook)
        {
                KPILOT_DELETE(aBook);
                fCreatedBook=false;
        }
        else
        {
                aBook=0L;
	}												         
}




/*********************************************************************
                     D E B U G   O U T P U T
 *********************************************************************/





void AbbrowserConduit::showPilotAddress(const PilotAddress *pilotAddress)
{
	FUNCTIONSETUPL(3);
	if (debug_level < 3)
	{
		return;
	}
	if (!pilotAddress)
	{
		DEBUGKPILOT<< fname << "| EMPTY"<<endl;
		return;
	}
	DEBUGKPILOT << fname << "\n"
			<< pilotAddress->getTextRepresentation(
				fAddressAppInfo,Qt::PlainText) << endl;
}


void AbbrowserConduit::showAddresses(
	const Addressee &pcAddr,
	const PilotAddress *backupAddr,
	const PilotAddress *palmAddr)
{
	FUNCTIONSETUPL(3);
	if (debug_level >= 3)
	{
		DEBUGKPILOT << fname << "abEntry:" << endl;
		KABCSync::showAddressee(pcAddr);
		DEBUGKPILOT << fname << "pilotAddress:" << endl;
		showPilotAddress(palmAddr);
		DEBUGKPILOT << fname << "backupAddress:" << endl;
		showPilotAddress(backupAddr);
		DEBUGKPILOT << fname << "------------------------------------------------" << endl;
	}
}



/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/



/* virtual */ bool AbbrowserConduit::exec()
{
	FUNCTIONSETUP;

	_prepare();

	bool retrieved = false;
	if(!openDatabases(CSL1("AddressDB"), &retrieved))
	{
		emit logError(i18n("Unable to open the addressbook databases on the handheld."));
		return false;
	}
	setFirstSync( retrieved );

	_getAppInfo();

	// Local block
	{
		QString dbpath = fLocalDatabase->dbPathName();
		DEBUGKPILOT << fname << ": Local database path " << dbpath << endl;
	}

	if ( syncMode().isTest() )
	{
		QTimer::singleShot(0, this, SLOT(slotTestRecord()));
		return true;
	}

	if(!_loadAddressBook())
	{
		emit logError(i18n("Unable to open the addressbook."));
		return false;
	}
	setFirstSync( isFirstSync() || (aBook->begin() == aBook->end()) );

	DEBUGKPILOT << fname << ": First sync now " << isFirstSync()
		<< " and addressbook is "
		<< ((aBook->begin() == aBook->end()) ? "" : "non-")
		<< "empty." << endl;

	// perform syncing from palm to abbrowser
	// iterate through all records in palm pilot

	DEBUGKPILOT << fname << ": fullsync=" << isFullSync() << ", firstSync=" <<    isFirstSync() << endl;
	DEBUGKPILOT << fname << ": "
		<< "syncDirection=" << syncMode().name() << ", "
		<< "archive = " << AbbrowserSettings::archiveDeleted() << endl;
	DEBUGKPILOT << fname << ": conflictRes="<< getConflictResolution() << endl;
	DEBUGKPILOT << fname << ": PilotStreetHome=" << AbbrowserSettings::pilotStreet() << ", PilotFaxHOme" << AbbrowserSettings::pilotFax() << endl;

	if (!isFirstSync())
	{
		allIds=fDatabase->idList();
	}

	QValueVector<int> v(4);
	v[0] = AbbrowserSettings::custom0();
	v[1] = AbbrowserSettings::custom1();
	v[2] = AbbrowserSettings::custom2();
	v[3] = AbbrowserSettings::custom3();

	fSyncSettings.setCustomMapping(v);
	fSyncSettings.setFieldForOtherPhone(AbbrowserSettings::pilotOther());
	fSyncSettings.setDateFormat(AbbrowserSettings::customDateFormat());
	fSyncSettings.setPreferHome(AbbrowserSettings::pilotStreet()==0);
	fSyncSettings.setFaxTypeOnPC(faxTypeOnPC());

	/* Note:
	   if eCopyPCToHH or eCopyHHToPC, first sync everything, then lookup
	   those entries on the receiving side that are not yet syncced and delete
	   them. Use slotDeleteUnsyncedPCRecords and slotDeleteUnsyncedHHRecords
	   for this, and no longer purge the whole addressbook before the sync to
	   prevent data loss in case of connection loss. */

	QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));

	return true;
}



void AbbrowserConduit::slotPalmRecToPC()
{
	FUNCTIONSETUP;
	PilotRecord *palmRec = 0L, *backupRec = 0L;

	if ( syncMode() == SyncMode::eCopyPCToHH )
	{
		DEBUGKPILOT << fname << ": Done; change to PCtoHH phase." << endl;
		abiter = aBook->begin();
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	if(isFullSync())
	{
		palmRec = fDatabase->readRecordByIndex(pilotindex++);
	}
	else
	{
		palmRec = fDatabase->readNextModifiedRec();
	}

	// no record means we're done going in this direction, so switch to
	// PC->Palm
	if(!palmRec)
	{
		abiter = aBook->begin();
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	// already synced, so skip:
	if(syncedIds.contains(palmRec->id()))
	{
		KPILOT_DELETE(palmRec);
		QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));
		return;
	}

	backupRec = fLocalDatabase->readRecordById(palmRec->id());
	PilotRecord*compareRec=(backupRec)?(backupRec):(palmRec);
	Addressee e = _findMatch(PilotAddress(compareRec));

	PilotAddress*backupAddr=0L;
	if (backupRec)
	{
		backupAddr=new PilotAddress(backupRec);
	}

	PilotAddress*palmAddr=0L;
	if (palmRec)
	{
		palmAddr=new PilotAddress(palmRec);
	}

	syncAddressee(e, backupAddr, palmAddr);

	syncedIds.append(palmRec->id());
	KPILOT_DELETE(palmAddr);
	KPILOT_DELETE(backupAddr);
	KPILOT_DELETE(palmRec);
	KPILOT_DELETE(backupRec);

	QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));
}



void AbbrowserConduit::slotPCRecToPalm()
{
	FUNCTIONSETUP;

	if ( (syncMode()==SyncMode::eCopyHHToPC) ||
		abiter == aBook->end() || (*abiter).isEmpty() )
	{
		DEBUGKPILOT << fname << ": Done; change to delete records." << endl;
		pilotindex = 0;
		QTimer::singleShot(0, this, SLOT(slotDeletedRecord()));
		return;
	}

	PilotRecord *palmRec=0L, *backupRec=0L;
	Addressee ad = *abiter;

	abiter++;

	// If marked as archived, don't sync!
	if (KABCSync::isArchived(ad))
	{
		DEBUGKPILOT << fname << ": address with id " << ad.uid() <<
			" marked archived, so don't sync." << endl;
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}


	QString recID(ad.custom(KABCSync::appString, KABCSync::idString));
	bool ok;
	recordid_t rid = recID.toLong(&ok);
	if (recID.isEmpty() || !ok || !rid)
	{
		DEBUGKPILOT << fname << ": This is a new record." << endl;
		// it's a new item(no record ID and not inserted by the Palm -> PC sync), so add it
		syncAddressee(ad, 0L, 0L);
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	// look into the list of already synced record ids to see if the addressee hasn't already been synced
	if (syncedIds.contains(rid))
	{
		DEBUGKPILOT << ": address with id " << rid << " already synced." << endl;
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}


	backupRec = fLocalDatabase->readRecordById(rid);
	// only update if no backup record or the backup record is not equal to the addressee

	PilotAddress*backupAddr=0L;
	if (backupRec)
	{
		backupAddr=new PilotAddress(backupRec);
	}
	if(!backupRec || isFirstSync() || !_equal(backupAddr, ad)  )
	{
		DEBUGKPILOT << fname << ": Updating entry." << endl;
		palmRec = fDatabase->readRecordById(rid);
		PilotAddress *palmAddr = 0L;
		if (palmRec)
		{
			palmAddr = new PilotAddress(palmRec);
		}
		else
		{
		DEBUGKPILOT << fname << ": No HH record with id " << rid << endl;
		}
		syncAddressee(ad, backupAddr, palmAddr);
		// update the id just in case it changed
		if (palmRec) rid=palmRec->id();
		KPILOT_DELETE(palmRec);
		KPILOT_DELETE(palmAddr);
	}
	else
	{
		DEBUGKPILOT << fname << ": Entry not updated." << endl;
	}
	KPILOT_DELETE(backupAddr);
	KPILOT_DELETE(backupRec);
	
	DEBUGKPILOT << fname << ": adding id:["<< rid << "] to syncedIds." << endl;

	syncedIds.append(rid);
	// done with the sync process, go on with the next one:
	QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
}



void AbbrowserConduit::slotDeletedRecord()
{
	FUNCTIONSETUP;

	PilotRecord *backupRec = fLocalDatabase->readRecordByIndex(pilotindex++);
	if(!backupRec || isFirstSync() )
	{
		KPILOT_DELETE(backupRec);
		QTimer::singleShot(0, this, SLOT(slotDeleteUnsyncedPCRecords()));
		return;
	}

	recordid_t id = backupRec->id();

	QString uid = addresseeMap[id];
	Addressee e = aBook->findByUid(uid);

		DEBUGKPILOT << fname << ": now looking at palm id: ["
					<< id << "], kabc uid: [" << uid << "]." << endl;

	PilotAddress*backupAddr=0L;
	if (backupRec)
	{
		backupAddr=new PilotAddress(backupRec);
	}
	PilotRecord*palmRec=fDatabase->readRecordById(id);

	if ( e.isEmpty() )
	{
		DEBUGKPILOT << fname << ": no Addressee found for this id." << endl;
		DEBUGKPILOT << fname << "\n"
			<< backupAddr->getTextRepresentation( 
				fAddressAppInfo,Qt::PlainText) << endl;

		if (palmRec) {
		DEBUGKPILOT << fname << ": deleting from database on palm." << endl;
			fDatabase->deleteRecord(id);
			fCtrHH->deleted();
		}
		DEBUGKPILOT << fname << ": deleting from backup database." << endl;
		fLocalDatabase->deleteRecord(id);

		// because we just deleted a record, we need to go back one
		pilotindex--;
	}

	KPILOT_DELETE(palmRec);
	KPILOT_DELETE(backupAddr);
	KPILOT_DELETE(backupRec);
	QTimer::singleShot(0, this, SLOT(slotDeletedRecord()));
}



void AbbrowserConduit::slotDeleteUnsyncedPCRecords()
{
	FUNCTIONSETUP;
	if ( syncMode()==SyncMode::eCopyHHToPC )
	{
		QStringList uids;
		RecordIDList::iterator it;
		QString uid;
		for ( it = syncedIds.begin(); it != syncedIds.end(); ++it)
		{
			uid=addresseeMap[*it];
			if (!uid.isEmpty()) uids.append(uid);
		}
		// TODO: Does this speed up anything?
		// qHeapSort( uids );
		AddressBook::Iterator abit;
		for (abit = aBook->begin(); abit != aBook->end(); ++abit)
		{
			if (!uids.contains((*abit).uid()))
			{
				DEBUGKPILOT<<"Deleting addressee "<<(*abit).realName()<<" from PC (is not on HH, and syncing with HH->PC direction)"<<endl;
				abChanged = true;
				// TODO: Can I really remove the current iterator???
				aBook->removeAddressee(*abit);
				fCtrPC->deleted();
			}
		}
	}
	QTimer::singleShot(0, this, SLOT(slotDeleteUnsyncedHHRecords()));
}



void AbbrowserConduit::slotDeleteUnsyncedHHRecords()
{
	FUNCTIONSETUP;
	if ( syncMode()==SyncMode::eCopyPCToHH )
	{
		RecordIDList ids=fDatabase->idList();
		RecordIDList::iterator it;
		for ( it = ids.begin(); it != ids.end(); ++it )
		{
			if (!syncedIds.contains(*it))
			{
				DEBUGKPILOT<<"Deleting record with ID "<<*it<<" from handheld (is not on PC, and syncing with PC->HH direction)"<<endl;
				fDatabase->deleteRecord(*it);
				fCtrHH->deleted();
				fLocalDatabase->deleteRecord(*it);
			}
		}
	}
	QTimer::singleShot(0, this, SLOT(slotCleanup()));
}


void AbbrowserConduit::slotCleanup()
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

	// Write out the sync maps
	QString syncFile = fLocalDatabase->dbPathName() + CSL1(".sync");
	DEBUGKPILOT << fname << ": Writing sync map to " << syncFile << endl;
	KSaveFile map( syncFile );
	if ( map.status() == 0 )
	{
		DEBUGKPILOT << fname << ": Writing sync map ..." << endl;
		(*map.dataStream()) << addresseeMap ;
		map.close();
	}
	// This also picks up errors from map.close()
	if ( map.status() != 0 )
	{
		WARNINGKPILOT << "Could not make backup of sync map." << endl;
	}

	_saveAddressBook();
	delayDone();
}



/*********************************************************************
              G E N E R A L   S Y N C   F U N C T I O N
         These functions modify the Handheld and the addressbook
 *********************************************************************/



bool AbbrowserConduit::syncAddressee(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;
	showAddresses(pcAddr, backupAddr, palmAddr);

	if ( syncMode() == SyncMode::eCopyPCToHH )
	{
		if (pcAddr.isEmpty())
		{
			return _deleteAddressee(pcAddr, backupAddr, palmAddr);
		}
		else
		{
			return _copyToHH(pcAddr, backupAddr, palmAddr);
		}
	}

	if ( syncMode() == SyncMode::eCopyHHToPC )
	{
		if (!palmAddr)
		{
			return _deleteAddressee(pcAddr, backupAddr, palmAddr);
		}
		else
		{
			return _copyToPC(pcAddr, backupAddr, palmAddr);
		}
	}

	if ( !backupAddr || isFirstSync() )
	{
			DEBUGKPILOT<< fname << ": Special case: no backup." << endl;
		/*
		Resolution matrix (0..does not exist, E..exists, D..deleted flag set, A..archived):
		  HH    PC  | Resolution
		  ------------------------------------------------------------
		   0     A  |  -
		   0     E  |  PC -> HH, reset ID if not set correctly
		   D     0  |  delete (error, should never occur!!!)
		   D     E  |  CR (ERROR)
		   E/A   0  |  HH -> PC
		   E/A   E/A|  merge/CR
		 */
		if  (!palmAddr && KABCSync::isArchived(pcAddr) )
		{
			return true;
		}
		else if (!palmAddr && !pcAddr.isEmpty())
		{
			DEBUGKPILOT << fname << ": case: 1a"<<endl;
			// PC->HH
			bool res=_copyToHH(pcAddr, 0L, 0L);
			return res;
		}
		else if (!palmAddr && pcAddr.isEmpty())
		{
			DEBUGKPILOT << fname << ": case: 1b"<<endl;
			// everything's empty -> ERROR
			return false;
		}
		else if ( (isDeleted(palmAddr) || isArchived(palmAddr)) && pcAddr.isEmpty())
		{
			DEBUGKPILOT << fname << ": case: 1c"<<endl;
			if (isArchived(palmAddr))
				return _copyToPC(pcAddr, 0L, palmAddr);
			else
				// this happens if you add a record on the handheld and delete it again before you do the next sync
				return _deleteAddressee(pcAddr, 0L, palmAddr);
		}
		else if ((isDeleted(palmAddr)||isArchived(palmAddr)) && !pcAddr.isEmpty())
		{
			DEBUGKPILOT << fname << ": case: 1d"<<endl;
			// CR (ERROR)
			return _smartMergeAddressee(pcAddr, 0L, palmAddr);
		}
		else if (pcAddr.isEmpty())
		{
			DEBUGKPILOT << fname << ": case: 1e"<<endl;
			// HH->PC
			return _copyToPC(pcAddr, 0L, palmAddr);
		}
		else
		{
			DEBUGKPILOT << fname << ": case: 1f"<<endl;
			// Conflict Resolution
			return _smartMergeAddressee(pcAddr, 0L, palmAddr);
		}
	} // !backupAddr
	else
	{
			DEBUGKPILOT << fname << ": case: 2"<<endl;
		/*
		Resolution matrix:
		  1) if HH.(empty| (deleted &! archived) ) -> { if (PC==B) -> delete, else -> CR }
		     if HH.archied -> {if (PC==B) -> copyToPC, else -> CR }
		     if PC.empty -> { if (HH==B) -> delete, else -> CR }
		     if PC.archived -> {if (HH==B) -> delete on HH, else CR }
		  2) if PC==HH -> { update B, update ID of PC if needed }
		  3) if PC==B -> { HH!=PC, thus HH modified, so copy HH->PC }
		     if HH==B -> { PC!=HH, thus PC modified, so copy PC->HH }
		  4) else: all three addressees are different -> CR
		*/

		if (!palmAddr || isDeleted(palmAddr) )
		{
			DEBUGKPILOT << fname << ": case: 2a"<<endl;
			if (_equal(backupAddr, pcAddr) || pcAddr.isEmpty())
			{
				return _deleteAddressee(pcAddr, backupAddr, 0L);
			}
			else
			{
				return _smartMergeAddressee(pcAddr, backupAddr, 0L);
			}
		}
		else if (pcAddr.isEmpty())
		{
			DEBUGKPILOT << fname << ": case: 2b"<<endl;
			if (*palmAddr == *backupAddr)
			{
				return _deleteAddressee(pcAddr, backupAddr, palmAddr);
			}
			else
			{
				return _smartMergeAddressee(pcAddr, backupAddr, palmAddr);
			}
		}
		else if (_equal(palmAddr, pcAddr))
		{
			DEBUGKPILOT << fname << ": case: 2c"<<endl;
			// update Backup, update ID of PC if neededd
			return _writeBackup(palmAddr);
		}
		else if (_equal(backupAddr, pcAddr))
		{
			DEBUGKPILOT << fname << ": case: 2d"<<endl;
			DEBUGKPILOT << fname << ": Flags: "<<palmAddr->attributes()<<", isDeleted="<<
				isDeleted(palmAddr)<<", isArchived="<<isArchived(palmAddr)<<endl;
			if (isDeleted(palmAddr))
				return _deleteAddressee(pcAddr, backupAddr, palmAddr);
			else
				return _copyToPC(pcAddr, backupAddr, palmAddr);
		}
		else if (*palmAddr == *backupAddr)
		{
			DEBUGKPILOT << fname << ": case: 2e"<<endl;
			return _copyToHH(pcAddr, backupAddr, palmAddr);
		}
		else
		{
			DEBUGKPILOT << fname << ": case: 2f"<<endl;
			// CR, since all are different
			return _smartMergeAddressee(pcAddr, backupAddr, palmAddr);
		}
	} // backupAddr
	return false;
}



bool AbbrowserConduit::_copyToHH(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;

	if (pcAddr.isEmpty()) return false;
	PilotAddress*paddr=palmAddr;
	bool paddrcreated=false;
	if (!paddr)
	{
		paddr=new PilotAddress();
		paddrcreated=true;
		fCtrHH->created();
	}
	else
	{
		fCtrHH->updated();
	}
	KABCSync::copy(*paddr, pcAddr, *fAddressAppInfo, fSyncSettings);

	DEBUGKPILOT << fname << "palmAddr->id=" << paddr->id()
		<< ", pcAddr.ID=" << pcAddr.custom(KABCSync::appString, KABCSync::idString) << endl;

	if(_savePalmAddr(paddr, pcAddr))
	{
		_savePCAddr(pcAddr, backupAddr, paddr);
	}
	if (paddrcreated) KPILOT_DELETE(paddr);
	return true;
}



bool AbbrowserConduit::_copyToPC(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;
	if (!palmAddr)
	{
		return false;
	}
	// keep track of CUD's...
	if (pcAddr.isEmpty())
	{
		fCtrPC->created();
	}
	else
	{
		fCtrPC->updated();
	}
	showPilotAddress(palmAddr);

	KABCSync::copy(pcAddr, *palmAddr, *fAddressAppInfo, fSyncSettings);
	if (isArchived(palmAddr))
	{
		KABCSync::makeArchived(pcAddr);
	}

	_savePCAddr(pcAddr, backupAddr, palmAddr);
	_writeBackup(palmAddr);
	return true;
}



bool AbbrowserConduit::_writeBackup(PilotAddress *backup)
{
	FUNCTIONSETUP;
	if (!backup) return false;

	showPilotAddress(backup);

	PilotRecord *pilotRec = backup->pack();
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);
	return true;
}



bool AbbrowserConduit::_deleteAddressee(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;

	if (palmAddr)
	{
		if (!syncedIds.contains(palmAddr->id())) {
		DEBUGKPILOT << fname << ": adding id:["<< palmAddr->id() << "] to syncedIds." << endl;
			syncedIds.append(palmAddr->id());
		}
		fDatabase->deleteRecord(palmAddr->id());
		fCtrHH->deleted();
		fLocalDatabase->deleteRecord(palmAddr->id());
	}
	else if (backupAddr)
	{
		if (!syncedIds.contains(backupAddr->id())) {
		DEBUGKPILOT << fname << ": adding id:["<< backupAddr->id() << "] to syncedIds." << endl;
			syncedIds.append(backupAddr->id());
		}
		fLocalDatabase->deleteRecord(backupAddr->id());
	}
	if (!pcAddr.isEmpty())
	{
		DEBUGKPILOT << fname << " removing " << pcAddr.formattedName() << endl;
		abChanged = true;
		aBook->removeAddressee(pcAddr);
		fCtrPC->deleted();
	}
	return true;
}



/*********************************************************************
                 l o w - l e v e l   f u n c t i o n s   f o r
                   adding / removing palm/pc records
 *********************************************************************/



bool AbbrowserConduit::_savePalmAddr(PilotAddress *palmAddr, Addressee &pcAddr)
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname << ": Saving to pilot " << palmAddr->id()
		<< " " << palmAddr->getField(entryFirstname)
		<< " " << palmAddr->getField(entryLastname)<< endl;

	PilotRecord *pilotRec = palmAddr->pack();
	DEBUGKPILOT << fname << ": record with id=" << pilotRec->id()
		<< " len=" << pilotRec->size() << endl;
	recordid_t pilotId = fDatabase->writeRecord(pilotRec);
	DEBUGKPILOT << fname << ": Wrote "<<pilotId<<": ID="<<pilotRec->id()<<endl;
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);

	// pilotId == 0 if using local db, so don't overwrite the valid id
	if(pilotId != 0)
	{
		palmAddr->setID(pilotId);
		if (!syncedIds.contains(pilotId)) {
		DEBUGKPILOT << fname << ": adding id:["<< pilotId << "] to syncedIds." << endl;
			syncedIds.append(pilotId);
		}
	}

	recordid_t abId = 0;
	abId = pcAddr.custom(KABCSync::appString, KABCSync::idString).toUInt();
	if(abId != pilotId)
	{
		pcAddr.insertCustom(KABCSync::appString, KABCSync::idString, QString::number(pilotId));
		return true;
	}

	return false;
}



bool AbbrowserConduit::_savePCAddr(Addressee &pcAddr, PilotAddress*,
	PilotAddress*)
{
	FUNCTIONSETUP;

	DEBUGKPILOT<<"Before _savePCAddr, pcAddr.custom="<<pcAddr.custom(KABCSync::appString, KABCSync::idString)<<endl;
	QString pilotId = pcAddr.custom(KABCSync::appString, KABCSync::idString);
	long pilotIdL = pilotId.toLong();
	if(!pilotId.isEmpty())
	{
		// because we maintain a mapping between pilotId -> kabc uid, whenever we add
		// a new relationship, we have to remove any old mapping that would tie a different
		// pilot id -> this kabc uid
		QMap < recordid_t, QString>::iterator it;
		for ( it = addresseeMap.begin(); it != addresseeMap.end(); ++it ) {
			QString kabcUid = it.data();
			if (kabcUid == pcAddr.uid()) {
				addresseeMap.remove(it);
				break;
			}
		}

		// now put the new mapping in
		addresseeMap.insert(pilotIdL,  pcAddr.uid());
	}

	aBook->insertAddressee(pcAddr);

	abChanged = true;
	return true;
}




/*********************************************************************
                   C O P Y   R E C O R D S
 *********************************************************************/



bool AbbrowserConduit::_equal(const PilotAddress *piAddress, const Addressee &abEntry,
	enum eqFlagsType flags) const
{
	FUNCTIONSETUP;

	// empty records are never equal!
	if (!piAddress) {
		DEBUGKPILOT << fname  << ": no pilot address passed" << endl;
		return false;
	}
	if (abEntry.isEmpty()) {
		DEBUGKPILOT << fname  << ":abEntry.isEmpty()" << endl;
		return false;
	}
	//  Archived records match anything so they won't be copied to the HH again
	if (flags & eqFlagsFlags)
		if (isArchived(piAddress) && KABCSync::isArchived(abEntry) ) return true;

	if (flags & eqFlagsName)
	{
		if(!_equal(abEntry.familyName(), piAddress->getField(entryLastname)))
		{
		DEBUGKPILOT << fname  << ": last name not equal" << endl;
			return false;
		}
		if(!_equal(abEntry.givenName(), piAddress->getField(entryFirstname)))
		{
		DEBUGKPILOT << fname  << ": first name not equal" << endl;
			return false;
		}
		if(!_equal(abEntry.prefix(), piAddress->getField(entryTitle)))
		{
		DEBUGKPILOT << fname  << ": title/prefix not equal" << endl;
			return false;
		}
		if(!_equal(abEntry.organization(), piAddress->getField(entryCompany)))
		{
		DEBUGKPILOT << fname  << ": company/organization not equal" << endl;
			return false;
		}
	}
	if (flags & eqFlagsNote)
		if(!_equal(abEntry.note(), piAddress->getField(entryNote)))
	{
		DEBUGKPILOT << fname  << ": note not equal" << endl;
			return false;
	}

	if (flags & eqFlagsCategory)
	{
		// Check that the name of the category of the HH record
		// is one matching the PC record.
		QString addressCategoryLabel = fAddressAppInfo->categoryName(piAddress->category());
		QString cat = KABCSync::bestMatchedCategoryName(abEntry.categories(),
			*fAddressAppInfo, piAddress->category());
		if(!_equal(cat, addressCategoryLabel))
		{
			DEBUGKPILOT << fname  << ": category not equal" << endl;
			return false;
		}
	}

	if (flags & eqFlagsPhones)
	{
		// first, look for missing e-mail addresses on either side
		QStringList abEmails(abEntry.emails());
		QStringList piEmails(piAddress->getEmails());

		if (abEmails.count() != piEmails.count())
		{
		DEBUGKPILOT << fname  << ": email count not equal" << endl;
			return false;
		}
		for (QStringList::Iterator it = abEmails.begin(); it != abEmails.end(); it++) {
			if (!piEmails.contains(*it))
			{
		DEBUGKPILOT << fname  << ": pilot e-mail missing" << endl;
			return false;
			}
		}
		for (QStringList::Iterator it = piEmails.begin(); it != piEmails.end(); it++) {
			if (!abEmails.contains(*it))
			{
		DEBUGKPILOT << fname  << ": kabc e-mail missing" << endl;
			return false;
			}
		}

		// now look for differences in phone numbers.  Note:  we can't just compare one
		// of each kind of phone number, because there's no guarantee that if the user
		// has more than one of a given type, we're comparing the correct two.

		PhoneNumber::List abPhones(abEntry.phoneNumbers());
		PhoneNumber::List piPhones = KABCSync::getPhoneNumbers(*piAddress);
		// first make sure that all of the pilot phone numbers are in kabc
		for (PhoneNumber::List::Iterator it = piPhones.begin(); it != piPhones.end(); it++) {
			PhoneNumber piPhone = *it;
			bool found=false;
			for (PhoneNumber::List::Iterator it = abPhones.begin(); it != abPhones.end(); it++) {
				PhoneNumber abPhone = *it;
				// see if we have the same number here...
				// * Note * We used to check for preferred number matching, but
				//     this seems to have broke in kdepim 3.5 and I don't have time to
				//     figure out why, so we won't check to see if preferred number match
				if ( _equal(piPhone.number(), abPhone.number()) ) {
					found = true;
					break;
				}
			}
			if (!found) {
		DEBUGKPILOT << fname  << ": not equal because kabc phone not found." << endl;
				return false;
			}
		}
		// now the other way.  *cringe*  kabc has the capacity to store way more addresses
		// than the Pilot, so this might give false positives more than we'd want....
		for (PhoneNumber::List::Iterator it = abPhones.begin(); it != abPhones.end(); it++) {
			PhoneNumber abPhone = *it;
			bool found=false;
			for (PhoneNumber::List::Iterator it = piPhones.begin(); it != piPhones.end(); it++) {
				PhoneNumber piPhone = *it;
				if ( _equal(piPhone.number(), abPhone.number()) ) {
					found = true;
					break;
				}
			}
			if (!found)
			{
				DEBUGKPILOT << fname  << ": not equal because pilot phone not found." << endl;
				return false;
			}
		}

		if(!_equal(KABCSync::getFieldForHHOtherPhone(abEntry,fSyncSettings),
			piAddress->getPhoneField(PilotAddressInfo::eOther)))
		{
			DEBUGKPILOT << fname  << ": not equal because of other phone field." << endl;
		   	return false;
		}
	}

	if (flags & eqFlagsAdress)
	{
		KABC::Address address = KABCSync::getAddress(abEntry,fSyncSettings);
		if(!_equal(address.street(), piAddress->getField(entryAddress)))
		{
			DEBUGKPILOT << fname  << ": address not equal" << endl;
			return false;
		}
		if(!_equal(address.locality(), piAddress->getField(entryCity)))
		{
			DEBUGKPILOT << fname  << ": city not equal" << endl;
			return false;
		}
		if(!_equal(address.region(), piAddress->getField(entryState)))
		{
		DEBUGKPILOT << fname  << ": state not equal" << endl;
			return false;
		}
		if(!_equal(address.postalCode(), piAddress->getField(entryZip)))
		{
		DEBUGKPILOT << fname  << ": zip not equal" << endl;
			return false;
		}
		if(!_equal(address.country(), piAddress->getField(entryCountry)))
		{
		DEBUGKPILOT << fname  << ": country not equal" << endl;
			return false;
		}
	}

	if (flags & eqFlagsCustom)
	{
		unsigned int customIndex = 0;
		unsigned int hhField = entryCustom1;

		for ( ; customIndex<4; ++customIndex,++hhField )
		{
			if (!_equal(KABCSync::getFieldForHHCustom(customIndex, abEntry, fSyncSettings),
				piAddress->getField(hhField)))
			{
				DEBUGKPILOT << fname << ": Custom field " << customIndex
					<< " (HH field " << hhField << ") differs." << endl;
				return false;
			}
		}
	}

	// if any side is marked archived, but the other is not, the two
	// are not equal.
	if ( (flags & eqFlagsFlags) && (isArchived(piAddress) || KABCSync::isArchived(abEntry) ) )
	{
		DEBUGKPILOT << fname  << ": archived flags don't match" << endl;
		return false;
	}

	return true;
}










/*********************************************************************
 C O N F L I C T   R E S O L U T I O N   a n d   M E R G I N G
 *********************************************************************/



/** smartly merge the given field for the given entry. use the backup record to determine which record has been modified
	@pc, @backup, @palm ... entries of the according databases
	@returns string of the merged entries.
*/
QString AbbrowserConduit::_smartMergeString(const QString &pc, const QString & backup,
	const QString & palm, ConflictResolution confRes)
{
	FUNCTIONSETUP;

	// if both entries are already the same, no need to do anything
	if(pc == palm) return pc;

	// If this is a first sync, we don't have a backup record, so
	if(isFirstSync() || backup.isEmpty()) {
		if (pc.isEmpty() && palm.isEmpty() ) return QString::null;
		if(pc.isEmpty()) return palm;
		if(palm.isEmpty()) return pc;
	} else {
		// only one side modified, so return that string, no conflict
		if(palm == backup) return pc;
		if(pc == backup) return palm;
	}

	DEBUGKPILOT<<"pc="<<pc<<", backup="<<backup<<", palm="<<
		palm<<", ConfRes="<<confRes<<endl;
	DEBUGKPILOT<<"Use conflict resolution :"<<confRes<<
		", PC="<<SyncAction::ePCOverrides<<endl;
	switch(confRes) {
		case SyncAction::ePCOverrides: return pc; break;
		case SyncAction::eHHOverrides: return palm; break;
		case SyncAction::ePreviousSyncOverrides: return backup; break;
		default: break;
	}
	return QString::null;
}



bool AbbrowserConduit::_buildResolutionTable(ResolutionTable*tab, const Addressee &pcAddr,
	PilotAddress *backupAddr, PilotAddress *palmAddr)
{
	FUNCTIONSETUP;
	if (!tab) return false;
	tab->setAutoDelete( TRUE );
	tab->labels[0]=i18n("Item on PC");
	tab->labels[1]=i18n("Handheld");
	tab->labels[2]=i18n("Last sync");
	if (!pcAddr.isEmpty())
		tab->fExistItems=(eExistItems)(tab->fExistItems|eExistsPC);
	if (backupAddr)
		tab->fExistItems=(eExistItems)(tab->fExistItems|eExistsBackup);
	if (palmAddr)
		tab->fExistItems=(eExistItems)(tab->fExistItems|eExistsPalm);

#define appendGen(desc, abfield, palmfield) \
	tab->append(new ResolutionItem(desc, tab->fExistItems, \
		(!pcAddr.isEmpty())?(abfield):(QString::null), \
		(palmAddr)?(palmAddr->palmfield):(QString::null), \
		(backupAddr)?(backupAddr->palmfield):(QString::null) ))
#define appendAddr(desc, abfield, palmfield) \
	appendGen(desc, abfield, getField(palmfield))
#define appendGenPhone(desc, abfield, palmfield) \
	appendGen(desc, abfield, getPhoneField(PilotAddressInfo::palmfield))
#define appendPhone(desc, abfield, palmfield) \
	appendGenPhone(desc, pcAddr.phoneNumber(PhoneNumber::abfield).number(), palmfield)


	appendAddr(i18n("Last name"), pcAddr.familyName(), entryLastname);
	appendAddr(i18n("First name"), pcAddr.givenName(), entryFirstname);
	appendAddr(i18n("Organization"), pcAddr.organization(), entryCompany);
	appendAddr(i18n("Title"), pcAddr.prefix(), entryTitle);
	appendAddr(i18n("Note"), pcAddr.note(), entryNote);

	appendAddr(i18n("Custom 1"), KABCSync::getFieldForHHCustom(0, pcAddr, fSyncSettings), entryCustom1);
	appendAddr(i18n("Custom 2"), KABCSync::getFieldForHHCustom(1, pcAddr, fSyncSettings), entryCustom2);
	appendAddr(i18n("Custom 3"), KABCSync::getFieldForHHCustom(2, pcAddr, fSyncSettings), entryCustom3);
	appendAddr(i18n("Custom 4"), KABCSync::getFieldForHHCustom(3, pcAddr, fSyncSettings), entryCustom4);

	appendPhone(i18n("Work Phone"), Work, eWork);
	appendPhone(i18n("Home Phone"), Home, eHome);
	appendPhone(i18n("Mobile Phone"), Cell, eMobile);
	appendGenPhone(i18n("Fax"), pcAddr.phoneNumber(faxTypeOnPC()).number(), eFax);
	appendPhone(i18n("Pager"), Pager, ePager);
	appendGenPhone(i18n("Other"), KABCSync::getFieldForHHOtherPhone(pcAddr,fSyncSettings), eOther);
	appendGenPhone(i18n("Email"), pcAddr.preferredEmail(), eEmail);

	KABC::Address abAddress = KABCSync::getAddress(pcAddr,fSyncSettings);
	appendAddr(i18n("Address"), abAddress.street(), entryAddress);
	appendAddr(i18n("City"), abAddress.locality(), entryCity);
	appendAddr(i18n("Region"), abAddress.region(), entryState);
	appendAddr(i18n("Postal code"), abAddress.postalCode(), entryZip);
	appendAddr(i18n("Country"), abAddress.country(), entryCountry);

	QString palmAddrCategoryLabel;
	if (palmAddr)
	{
		 palmAddrCategoryLabel = fAddressAppInfo->categoryName(palmAddr->category());
	}
	QString backupAddrCategoryLabel;
	if (backupAddr)
	{
		backupAddrCategoryLabel = fAddressAppInfo->categoryName(backupAddr->category());
	}
	int category = palmAddr ? palmAddr->category() : 0;
	tab->append(new ResolutionItem(
		i18n("Category"),
		tab->fExistItems,
		!pcAddr.isEmpty() ?
			KABCSync::bestMatchedCategoryName(pcAddr.categories(), *fAddressAppInfo, category) :
			QString::null,
		palmAddrCategoryLabel,
		backupAddrCategoryLabel));
#undef appendGen
#undef appendAddr
#undef appendGenPhone
#undef appendPhone

	return true;
}


/// This function just sets the phone number of type "type" to "phone"
static inline void setPhoneNumber(Addressee &abEntry, int type, const QString &nr)
{
	PhoneNumber phone = abEntry.phoneNumber(type);
	phone.setNumber(nr);
	abEntry.insertPhoneNumber(phone);
}


bool AbbrowserConduit::_applyResolutionTable(ResolutionTable*tab, Addressee &pcAddr,
	PilotAddress *backupAddr, PilotAddress *palmAddr)
{
	FUNCTIONSETUP;
	if (!tab) return false;
	if (!palmAddr) {
		WARNINGKPILOT << "Empty palmAddr after conflict resolution." << endl;
		return false;
	}

	ResolutionItem*item=tab->first();
#define SETGENFIELD(abfield, palmfield) \
	if (item) {\
		abfield; \
		palmAddr->setField(palmfield, item->fResolved); \
	}\
	item=tab->next();
#define SETFIELD(abfield, palmfield) \
	SETGENFIELD(pcAddr.set##abfield(item->fResolved), palmfield)
#define SETCUSTOMFIELD(abfield, palmfield) \
	SETGENFIELD(KABCSync::setFieldFromHHCustom(abfield, pcAddr, item->fResolved, fSyncSettings), palmfield)
#define SETGENPHONE(abfield, palmfield) \
	if (item) { \
		abfield; \
		palmAddr->setPhoneField(PilotAddressInfo::palmfield, item->fResolved, PilotAddress::Replace); \
	}\
	item=tab->next();
#define SETPHONEFIELD(abfield, palmfield) \
	SETGENPHONE(setPhoneNumber(pcAddr, PhoneNumber::abfield, item->fResolved), palmfield)
#define SETADDRESSFIELD(abfield, palmfield) \
	SETGENFIELD(abAddress.abfield(item->fResolved), palmfield)

	SETFIELD(FamilyName, entryLastname);
	SETFIELD(GivenName, entryFirstname);
	SETFIELD(Organization, entryCompany);
	SETFIELD(Prefix, entryTitle);
	SETFIELD(Note, entryNote);

	SETCUSTOMFIELD(0, entryCustom1);
	SETCUSTOMFIELD(1, entryCustom2);
	SETCUSTOMFIELD(2, entryCustom3);
	SETCUSTOMFIELD(3, entryCustom4);

	SETPHONEFIELD(Work, eWork);
	SETPHONEFIELD(Home, eHome);
	SETPHONEFIELD(Cell, eMobile);
	SETGENPHONE(setPhoneNumber(pcAddr, faxTypeOnPC(), item->fResolved), eFax);
	SETPHONEFIELD(Pager, ePager);
	SETGENPHONE(KABCSync::setFieldFromHHOtherPhone(pcAddr, item->fResolved, fSyncSettings), eOther);

	// TODO: fix email
	if (item)
	{
		palmAddr->setPhoneField(PilotAddressInfo::eEmail, item->fResolved, PilotAddress::Replace);
		if (backupAddr)
		{
			pcAddr.removeEmail(backupAddr->getPhoneField(PilotAddressInfo::eEmail));
		}
		pcAddr.removeEmail(palmAddr->getPhoneField(PilotAddressInfo::eEmail));
		pcAddr.insertEmail(item->fResolved, true);
	}
	item=tab->next();

	KABC::Address abAddress = KABCSync::getAddress(pcAddr, fSyncSettings);
	SETADDRESSFIELD(setStreet, entryAddress);
	SETADDRESSFIELD(setLocality, entryCity);
	SETADDRESSFIELD(setRegion, entryState);
	SETADDRESSFIELD(setPostalCode, entryZip);
	SETADDRESSFIELD(setCountry, entryCountry);
	pcAddr.insertAddress(abAddress);

	// TODO: Is this correct?
	if (item)
	{
		palmAddr->setCategory( fAddressAppInfo->findCategory(item->fResolved) );
		KABCSync::setCategory(pcAddr, item->fResolved);
	}


#undef SETGENFIELD
#undef SETFIELD
#undef SETCUSTOMFIELD
#undef SETGENPHONE
#undef SETPHONEFIELD
#undef SETADDRESSFIELD

	return true;
}



bool AbbrowserConduit::_smartMergeTable(ResolutionTable*tab)
{
	FUNCTIONSETUP;
	if (!tab) return false;
	bool noconflict=true;
	ResolutionItem*item;
	for ( item = tab->first(); item; item = tab->next() )
	{
		// try to merge the three strings
		item->fResolved=_smartMergeString(item->fEntries[0],
			item->fEntries[2], item->fEntries[1], getConflictResolution());
		// if a conflict occurred, set the default to something sensitive:
		if (item->fResolved.isNull() && !(item->fEntries[0].isEmpty() &&
			item->fEntries[1].isEmpty() && item->fEntries[2].isEmpty() ) )
		{
			item->fResolved=item->fEntries[0];
			noconflict=false;
		}
		if (item->fResolved.isNull()) item->fResolved=item->fEntries[1];
		if (item->fResolved.isNull()) item->fResolved=item->fEntries[2];
	}
	return  noconflict;
}



/** Merge the palm and the pc entries with the additional information of
 *  the backup.
 *  return value: no meaning yet
 */
bool AbbrowserConduit::_smartMergeAddressee(Addressee &pcAddr,
	PilotAddress *backupAddr, PilotAddress *palmAddr)
{
	FUNCTIONSETUP;

	// Merge them, then look which records have to be written to device or abook
	int res = SyncAction::eAskUser;
 	bool result=true;
	ResolutionTable tab;

	result &= _buildResolutionTable(&tab, pcAddr, backupAddr, palmAddr);
	// Now attempt a smart merge. If that fails, let conflict resolution do the job
	bool mergeOk=_smartMergeTable(&tab);

	if (!mergeOk)
	{
		QString dlgText;
		if (!palmAddr)
		{
			dlgText=i18n("The following address entry was changed, but does no longer exist on the handheld. Please resolve this conflict:");
		}
		else if (pcAddr.isEmpty())
		{
			dlgText=i18n("The following address entry was changed, but does no longer exist on the PC. Please resolve this conflict:");
		}
		else
		{
			dlgText=i18n("The following address entry was changed on the handheld as well as on the PC side. The changes could not be merged automatically, so please resolve the conflict yourself:");
		}
		ResolutionDlg*resdlg=new ResolutionDlg(0L, fHandle, i18n("Address conflict"), dlgText, &tab);
		resdlg->exec();
		KPILOT_DELETE(resdlg);
	}
	res=tab.fResolution;

	// Disallow some resolution under certain conditions, fix wrong values:
	switch (res) {
		case SyncAction::eHHOverrides:
			if (!palmAddr) res=SyncAction::eDelete;
			break;
		case SyncAction::ePCOverrides:
			if (pcAddr.isEmpty()) res=SyncAction::eDelete;
			break;
		case SyncAction::ePreviousSyncOverrides:
			if (!backupAddr) res=SyncAction::eDoNothing;
			break;
	}

	PilotAddress*pAddr=palmAddr;
	bool pAddrCreated=false;
	// Now that we have done a possible conflict resolution, apply the changes
	switch (res) {
		case SyncAction::eDuplicate:
			// Set the Palm ID to 0 so we don't overwrite the existing record.
			pcAddr.removeCustom(KABCSync::appString, KABCSync::idString);
			result &= _copyToHH(pcAddr, 0L, 0L);
			{
			Addressee pcadr;
			result &= _copyToPC(pcadr, backupAddr, palmAddr);
			}
			break;
		case SyncAction::eDoNothing:
			break;
		case SyncAction::eHHOverrides:
				result &= _copyToPC(pcAddr, backupAddr, palmAddr);
				break;
		case SyncAction::ePCOverrides:
			result &= _copyToHH(pcAddr, backupAddr, pAddr);
			break;
		case SyncAction::ePreviousSyncOverrides:
			KABCSync::copy(pcAddr, *backupAddr, *fAddressAppInfo, fSyncSettings);
			if (palmAddr && backupAddr) *palmAddr=*backupAddr;
			result &= _savePalmAddr(backupAddr, pcAddr);
			result &= _savePCAddr(pcAddr, backupAddr, backupAddr);
			break;
		case SyncAction::eDelete:
			result &= _deleteAddressee(pcAddr, backupAddr, palmAddr);
			break;
		case SyncAction::eAskUser:
		default:
			if (!pAddr)
			{
				pAddr=new PilotAddress();
				pAddrCreated=true;
			}
			result &= _applyResolutionTable(&tab, pcAddr, backupAddr, pAddr);
showAddresses(pcAddr, backupAddr, pAddr);
			// savePalmAddr sets the RecordID custom field already
			result &= _savePalmAddr(pAddr, pcAddr);
			result &= _savePCAddr(pcAddr, backupAddr, pAddr);
			if (pAddrCreated) KPILOT_DELETE(pAddr);
			break;
	}

	return result;
}



// TODO: right now entries are equal if both first/last name and organization are
//  equal. This rules out two entries for the same person(e.g. real home and weekend home)
//  or two persons with the same name where you don't know the organization.!!!
Addressee AbbrowserConduit::_findMatch(const PilotAddress & pilotAddress) const
{
	FUNCTIONSETUP;
	// TODO: also search with the pilotID
	// first, use the pilotID to UID map to find the appropriate record
	if( !isFirstSync() && (pilotAddress.id() > 0) )
	{
		QString id(addresseeMap[pilotAddress.id()]);
		DEBUGKPILOT << fname << ": PilotRecord has id " << pilotAddress.id() << ", mapped to " << id << endl;
		if(!id.isEmpty())
		{
			Addressee res(aBook->findByUid(id));
			if(!res.isEmpty()) return res;
			DEBUGKPILOT << fname << ": PilotRecord has id " << pilotAddress.id() << ", but could not be found in the addressbook" << endl;
		}
	}

	for(AddressBook::Iterator iter = aBook->begin(); iter != aBook->end(); ++iter)
	{
		Addressee abEntry = *iter;
		QString recID(abEntry.custom(KABCSync::appString, KABCSync::idString));
		bool ok;
		if (!recID.isEmpty() )
		{
			recordid_t rid = recID.toLong(&ok);
			if (ok && rid)
			{
				if (rid==pilotAddress.id()) return abEntry;// yes, we found it
				// skip this addressee, as it can an other corresponding address on the handheld
				if (allIds.contains(rid)) continue;
			}
		}

		if (_equal(&pilotAddress, abEntry, eqFlagsAlmostAll))
		{
			return abEntry;
		}
	}
	DEBUGKPILOT << fname << ": Could not find any addressbook enty matching " << pilotAddress.getField(entryLastname) << endl;
	return Addressee();
}

void AbbrowserConduit::slotTestRecord()
{
	FUNCTIONSETUP;

	// Get a record and interpret it as an address.
	PilotRecord *r = fDatabase->readRecordByIndex( pilotindex );
	if (!r)
	{
		delayDone();
		return;
	}
	PilotAddress a(r);
	KPILOT_DELETE(r);

	// Process this record.
	showPilotAddress(&a);

	// Schedule more work.
	++pilotindex;
	QTimer::singleShot(0, this, SLOT(slotTestRecord()));
}
