/* KPilot
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

#include "abbrowser-conduit.moc"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
extern "C"
{
unsigned long version_conduit_address = Pilot::PLUGIN_API;
}


using namespace KABC;

const QString AbbrowserConduit::appString=CSL1("KPILOT");
const QString AbbrowserConduit::flagString=CSL1("Flag");
const QString AbbrowserConduit::idString=CSL1("RecordID");

AddressBook*AbbrowserConduit::aBook=0L;

/// This function just sets the phone number of type "type" to "phone"
static inline void _setPhoneNumber(Addressee &abEntry, int type, const QString &nr)
{
	PhoneNumber phone = abEntry.phoneNumber(type);
	phone.setNumber(nr);
	abEntry.insertPhoneNumber(phone);
}


/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/





AbbrowserConduit::AbbrowserConduit(KPilotLink * o, const char *n, const QStringList & a):
		ConduitAction(o, n, a),
		fAddressAppInfo(0L),
		addresseeMap(),
		syncedIds(),
		abiter(),
		ticket(0L)
{
	FUNCTIONSETUP;
	fConduitName=i18n("Addressbook");
}



AbbrowserConduit::~AbbrowserConduit()
{
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
		QString recid = aContact.custom(appString, idString);
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
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": found duplicate pilot key: ["
						<< id << "], removing pilot id from addressee: ["
						<< aContact.realName() << "]" << endl;
#endif
				aBook->removeAddressee(aContact);
				aContact.removeCustom(appString, idString);
				aBook->insertAddressee(aContact);
				abChanged = true;
			}
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

	DEBUGCONDUIT << fname
		<< ": Reading addressbook "
		<< ( AbbrowserSettings::addressbookType() == AbbrowserSettings::eAbookFile ?
			AbbrowserSettings::fileName() : CSL1("Standard") )
		<< endl;
	DEBUGCONDUIT << fname
		<< ": "
		<< " fConflictResolution=" << getConflictResolution()
		<< " fArchive=" << AbbrowserSettings::archiveDeleted()
		<< " fFirstTime=" << isFirstSync()
		<< endl;
	DEBUGCONDUIT << fname
		<< ": "
		<< " fPilotStreetHome=" << AbbrowserSettings::pilotStreet()
		<< " fPilotFaxHome=" << AbbrowserSettings::pilotFax()
		<< " eCustom[0]=" << AbbrowserSettings::custom0()
		<< " eCustom[1]=" << AbbrowserSettings::custom1()
		<< " eCustom[2]=" << AbbrowserSettings::custom2()
		<< " eCustom[3]=" << AbbrowserSettings::custom3()
		<< endl;
}



bool AbbrowserConduit::isDeleted(const PilotAddress*addr)
{
	if (!addr) return true;
	if (addr->isDeleted() && !addr->isArchived()) return true;
	if (addr->isArchived()) return !AbbrowserSettings::archiveDeleted();
	return false;
}
bool AbbrowserConduit::isArchived(const PilotAddress*addr)
{
	if (addr && addr->isArchived()) return AbbrowserSettings::archiveDeleted();
	else return false;
}
bool AbbrowserConduit::isArchived(const Addressee &addr)
{
	return addr.custom(appString, flagString) == QString::number(SYNCDEL);
}
bool AbbrowserConduit::makeArchived(Addressee &addr)
{
	FUNCTIONSETUP;
	addr.insertCustom(appString, flagString, QString::number(SYNCDEL));
	addr.removeCustom(appString, idString);
	return true;
}



bool AbbrowserConduit::_loadAddressBook()
{
	FUNCTIONSETUP;

	startTickle();
	switch ( AbbrowserSettings::addressbookType() )
	{
		case AbbrowserSettings::eAbookResource:
			DEBUGCONDUIT<<"Loading standard addressbook"<<endl;
			aBook = StdAddressBook::self( true );
			break;
		case AbbrowserSettings::eAbookFile:
		{ // initialize the abook with the given file
			DEBUGCONDUIT<<"Loading custom addressbook"<<endl;
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
			KABC::Resource *res = new ResourceFile(fABookFile, CSL1("vcard") );

			bool r = aBook->addResource( res );
			if ( !r )
			{
				DEBUGCONDUIT << "Unable to open resource for file " << fABookFile << endl;
				KPILOT_DELETE( aBook );
				stopTickle();
				return false;
			}
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
		kdWarning()<<k_funcinfo<<": Unable to initialize the addressbook for the sync."<<endl;
		KPILOT_DELETE(aBook);
		stopTickle();
		return false;
	}
	abChanged = false;
	ticket=aBook->requestSaveTicket();
	if (!ticket)
	{
		kdWarning()<<k_funcinfo<<": Unable to lock addressbook for writing "<<endl;
		KPILOT_DELETE(aBook);
		stopTickle();
		return false;
	}
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
#ifdef DEBUG
			DEBUGCONDUIT<<"Addressbook not changed, freeing ticket"<<endl;
#endif

	bool res=false;

	if (ticket)
	{
		if (abChanged)
		{
			res=aBook->save(ticket);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"Addressbook not changed, no need to save it"<<endl;
#endif
		}
		// XXX: KDE4: release ticket in all cases (save no longer releases it)
		if ( !res ) // didn't save, delete ticket manually
		{
			aBook->releaseSaveTicket(ticket);
		}
		ticket=0;
	}
	else
	{
		kdWarning()<<k_funcinfo<<": No ticket available to save the "
		<<"addressbook."<<endl;
	}
	if ( AbbrowserSettings::addressbookType()!= AbbrowserSettings::eAbookResource )
	{
		KURL kurl(AbbrowserSettings::fileName());
		if(!kurl.isLocalFile())
		{
#ifdef DEBUG
			DEBUGCONDUIT << "Deleting local addressbook tempfile" << endl;
#endif
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

#ifdef DEBUG
		DEBUGCONDUIT<<"Deleting addressbook"<<endl;
#endif
		KPILOT_DELETE(aBook);
	}

	return res;
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

int AbbrowserConduit::getCustom(const int index)
{
	FUNCTIONSETUPL(4);

	int customEnum;
	switch(index) {
		case 0:
			customEnum = AbbrowserSettings::custom0();
			break;
		case 1:
			customEnum = AbbrowserSettings::custom1();
			break;
		case 2:
			customEnum = AbbrowserSettings::custom2();
			break;
		case 3:
			customEnum = AbbrowserSettings::custom3();
			break;
		default:
			customEnum = index;
			break;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname << "Index: " << index << " -> customEnum: "
		<< customEnum << endl;
#endif

	return customEnum;
}

QString AbbrowserConduit::getCustomField(const Addressee &abEntry, const int index)
{
	FUNCTIONSETUPL(4);

	switch (getCustom(index)) {
		case AbbrowserSettings::eCustomBirthdate: {
			QDateTime bdate(abEntry.birthday().date());
			if (!bdate.isValid())
				return abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));
			QString tmpfmt(KGlobal::locale()->dateFormat());
			if (!AbbrowserSettings::customDateFormat().isEmpty())
				KGlobal::locale()->setDateFormat(AbbrowserSettings::customDateFormat());
#ifdef DEBUG
			DEBUGCONDUIT<<"Birthdate: "<<KGlobal::locale()->formatDate(bdate.date())<<" (QDate: "<<bdate.toString()<<endl;
#endif
			QString ret(KGlobal::locale()->formatDate(bdate.date()));
			KGlobal::locale()->setDateFormat(tmpfmt);
			return ret;
		}
		case AbbrowserSettings::eCustomURL:
			return abEntry.url().url();
			break;
		case AbbrowserSettings::eCustomIM:
			return abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"));
			break;
		case AbbrowserSettings::eCustomField:
		default:
			return abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));
			break;
	}
}

void AbbrowserConduit::setCustomField(Addressee &abEntry,  int index, QString cust)
{
	FUNCTIONSETUPL(4);

	switch (getCustom(index)) {
		case AbbrowserSettings::eCustomBirthdate: {
			QDate bdate;
			bool ok=false;
			if (AbbrowserSettings::customDateFormat().isEmpty())
			{
				// empty format means use locale setting
				bdate=KGlobal::locale()->readDate(cust, &ok);
			}
			else
			{
				// use given format
				bdate=KGlobal::locale()->readDate(cust, AbbrowserSettings::customDateFormat(), &ok);
			}

			if (!ok)
			{
				QString format = KGlobal::locale()->dateFormatShort();
				QRegExp re(CSL1("%[yY][^%]*"));
				format.remove(re); // Remove references to year and following punctuation
				bdate = KGlobal::locale()->readDate(cust, format, &ok);
			}
#ifdef DEBUG
			DEBUGCONDUIT<<"Birthdate from "<<index<<"-th custom field: "<<bdate.toString()<<endl;
			DEBUGCONDUIT<<"Is Valid: "<<bdate.isValid()<<endl;
#endif
			if (bdate.isValid())
				return abEntry.setBirthday(bdate);
			else
				return abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("X-Birthday"), cust);
			break; }
		case AbbrowserSettings::eCustomURL: {
			return abEntry.setUrl(cust);
			break;}
		case AbbrowserSettings::eCustomIM: {
			return abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"), cust);
			break;}
		case AbbrowserSettings::eCustomField:
		default: {
			return abEntry.insertCustom(appString, CSL1("CUSTOM")+QString::number(index), cust);
			break;}
	}
	return;
}



QString AbbrowserConduit::getOtherField(const Addressee & abEntry)
{
	switch(AbbrowserSettings::pilotOther())
	{
		case AbbrowserSettings::eOtherPhone:
			return abEntry.phoneNumber(0).number();
		case AbbrowserSettings::eAssistant:
			return abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"));
		case AbbrowserSettings::eBusinessFax:
			return abEntry.phoneNumber(PhoneNumber::Fax | PhoneNumber::Work).number();
		case AbbrowserSettings::eCarPhone:
			return abEntry.phoneNumber(PhoneNumber::Car).number();
		case AbbrowserSettings::eEmail2:
			return abEntry.emails().first();
		case AbbrowserSettings::eHomeFax:
			return abEntry.phoneNumber(PhoneNumber::Fax | PhoneNumber::Home).number();
		case AbbrowserSettings::eTelex:
			return abEntry.phoneNumber(PhoneNumber::Bbs).number();
		case AbbrowserSettings::eTTYTTDPhone:
			return abEntry.phoneNumber(PhoneNumber::Pcs).number();
		default:
			return QString::null;
	}
}
void AbbrowserConduit::setOtherField(Addressee & abEntry, QString nr)
{
//	PhoneNumber phone;
	switch (AbbrowserSettings::pilotOther())
	{
		case AbbrowserSettings::eOtherPhone:
			_setPhoneNumber(abEntry, 0, nr);
			break;
		case AbbrowserSettings::eAssistant:
			abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"), nr);
			break;
		case AbbrowserSettings::eBusinessFax:
			_setPhoneNumber(abEntry, PhoneNumber::Fax | PhoneNumber::Work, nr);
			break;
		case AbbrowserSettings::eCarPhone:
			_setPhoneNumber(abEntry, PhoneNumber::Car, nr);
			break;
		case AbbrowserSettings::eEmail2:
			return abEntry.insertEmail(nr);
		case AbbrowserSettings::eHomeFax:
			_setPhoneNumber(abEntry, PhoneNumber::Fax|PhoneNumber::Home, nr);
			break;
		case AbbrowserSettings::eTelex:
			_setPhoneNumber(abEntry, PhoneNumber::Bbs, nr);
			break;
		case AbbrowserSettings::eTTYTTDPhone:
			_setPhoneNumber(abEntry, PhoneNumber::Pcs, nr);
			break;
	}
}



PhoneNumber AbbrowserConduit::getFax(const Addressee & abEntry)
{
	// *NOTE* If our user has said that they want a fax number to be their
	//        preferred number on their Pilot, this will negate that.  In other
	//        words, this could be a bug waiting to happen if someone wants to
	//        have a fax number be their "preferred number".
	return abEntry.phoneNumber( PhoneNumber::Fax |
		( (AbbrowserSettings::pilotFax()==0) ?(PhoneNumber::Home) :(PhoneNumber::Work)));
}
void AbbrowserConduit::setFax(Addressee & abEntry, QString fax)
{
	_setPhoneNumber(abEntry, PhoneNumber::Fax | ( (AbbrowserSettings::pilotFax()==0) ? PhoneNumber::Home : PhoneNumber::Work ), fax);
}


/** First search for a preferred  address. If we don't have one, search
 *  for home or work as specified in the config dialog. If we don't have
 *  such one, either, search for the other type. If we still have no luck,
 *  return an address with preferred + home/work flag (from config dlg). */
KABC::Address AbbrowserConduit::getAddress(const Addressee & abEntry)
{
	int type=(AbbrowserSettings::pilotStreet==0)?(KABC::Address::Home):(KABC::Address::Work);
	KABC::Address ad(abEntry.address(KABC::Address::Pref));
	if (!ad.isEmpty()) return ad;
	ad=abEntry.address(type);
	if (!ad.isEmpty()) return ad;
	ad=abEntry.address((AbbrowserSettings::pilotStreet==0) ?(KABC::Address::Work):(KABC::Address::Home));
	if (!ad.isEmpty()) return ad;

	return abEntry.address(type | KABC::Address::Pref);
}



/**
 * _getCat returns the id of the category from the given categories list.
 * If the address has no categories on the PC, QString::null is returned.
 * If the current category exists in the list of cats, it is returned
 * Otherwise the first cat in the list that exists on the HH is returned
 * If none of the categories exists on the palm, QString::null is returned
 */
QString AbbrowserConduit::_getCatForHH(const QStringList cats, const QString curr) const
{
	FUNCTIONSETUP;
	if (cats.size()<1) return QString::null;
	if (cats.contains(curr)) return curr;
	for(QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it)
	{
		if ( fAddressAppInfo->findCategory( *it, false /* Unknown returns -1 */) >= 0)
		{
			return *it;
		}
	}

	// didn't find anything. return null
	return QString::null;
}
void AbbrowserConduit::_setCategory(Addressee & abEntry, QString cat)
{
	if ( (!cat.isEmpty()))
	// &&  (cat!=QString(fAddressAppInfo.category.name[0])) )
		abEntry.insertCategory(cat);
}



/*********************************************************************
                     D E B U G   O U T P U T
 *********************************************************************/



void AbbrowserConduit::showAddressee(const Addressee & abAddress)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << "\tAbbrowser Contact Entry" << endl;
	if (abAddress.isEmpty()) {
		DEBUGCONDUIT<< "\t\tEMPTY"<<endl;
		return;
	}
	DEBUGCONDUIT << "\t\tLast name = " << abAddress.familyName() << endl;
	DEBUGCONDUIT << "\t\tFirst name = " << abAddress.givenName() << endl;
	DEBUGCONDUIT << "\t\tCompany = " << abAddress.organization() << endl;
	DEBUGCONDUIT << "\t\tJob Title = " << abAddress.prefix() << endl;
	DEBUGCONDUIT << "\t\tNote = " << abAddress.note() << endl;
	DEBUGCONDUIT << "\t\tHome phone = " << abAddress.phoneNumber(PhoneNumber::Home).number() << endl;
	DEBUGCONDUIT << "\t\tWork phone = " << abAddress.phoneNumber(PhoneNumber::Work).number() << endl;
	DEBUGCONDUIT << "\t\tMobile phone = " << abAddress.phoneNumber(PhoneNumber::Cell).number() << endl;
	DEBUGCONDUIT << "\t\tEmail = " << abAddress.preferredEmail() << endl;
	DEBUGCONDUIT << "\t\tFax = " << getFax(abAddress).number() << endl;
	DEBUGCONDUIT << "\t\tPager = " << abAddress.phoneNumber(PhoneNumber::Pager).number() << endl;
	DEBUGCONDUIT << "\t\tCategory = " << abAddress.categories().first() << endl;
#else
	Q_UNUSED( abAddress );
#endif
}



void AbbrowserConduit::showPilotAddress(PilotAddress *pilotAddress)
{
	FUNCTIONSETUPL(3);
#ifdef DEBUG
	if (debug_level >= 3)
	{
	if (!pilotAddress) {
		DEBUGCONDUIT<< fname << "| EMPTY"<<endl;
		return;
	}
	DEBUGCONDUIT << fname << "\n"
			<< pilotAddress->getTextRepresentation(false) << endl;
	}
#else
	Q_UNUSED( pilotAddress );
#endif
}


void AbbrowserConduit::showAdresses(Addressee &pcAddr, PilotAddress *backupAddr,
	PilotAddress *palmAddr)
{
#ifdef DEBUG
	FUNCTIONSETUPL(3);
	if (debug_level >= 3)
	{
		DEBUGCONDUIT << fname << "abEntry:" << endl;
		showAddressee(pcAddr);
		DEBUGCONDUIT << fname << "pilotAddress:" << endl;
		showPilotAddress(palmAddr);
		DEBUGCONDUIT << fname << "backupAddress:" << endl;
		showPilotAddress(backupAddr);
		DEBUGCONDUIT << fname << "------------------------------------------------" << endl;
	}
#else
	Q_UNUSED(pcAddr);
	Q_UNUSED(backupAddr);
	Q_UNUSED(palmAddr);
#endif
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
		DEBUGCONDUIT << fname << ": Local database path " << dbpath << endl;
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

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": First sync now " << isFirstSync()
		<< " and addressbook is "
		<< ((aBook->begin() == aBook->end()) ? "" : "non-")
		<< "empty." << endl;
#endif

	// perform syncing from palm to abbrowser
	// iterate through all records in palm pilot

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": fullsync=" << isFullSync() << ", firstSync=" <<    isFirstSync() << endl;
	DEBUGCONDUIT << fname << ": "
		<< "syncDirection=" << syncMode().name() << ", "
		<< "archive = " << AbbrowserSettings::archiveDeleted() << endl;
	DEBUGCONDUIT << fname << ": conflictRes="<< getConflictResolution() << endl;
	DEBUGCONDUIT << fname << ": PilotStreetHome=" << AbbrowserSettings::pilotStreet() << ", PilotFaxHOme" << AbbrowserSettings::pilotFax() << endl;
#endif

	if (!isFirstSync())
		allIds=fDatabase->idList();

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
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Done; change to PCtoHH phase." << endl;
#endif
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
	Addressee e = _findMatch(PilotAddress(fAddressAppInfo, compareRec));

	PilotAddress*backupAddr=0L;
	if (backupRec) backupAddr=new PilotAddress(fAddressAppInfo, backupRec);
	PilotAddress*palmAddr=0L;
	if (palmRec) palmAddr=new PilotAddress(fAddressAppInfo, palmRec);

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
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Done; change to delete records." << endl;
#endif
		pilotindex = 0;
		QTimer::singleShot(0, this, SLOT(slotDeletedRecord()));
		return;
	}

	PilotRecord *palmRec=0L, *backupRec=0L;
	Addressee ad = *abiter;

	abiter++;

	// If marked as archived, don't sync!
	if (isArchived(ad))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": address with id " << ad.uid() <<
			" marked archived, so don't sync." << endl;
#endif
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}


	QString recID(ad.custom(appString, idString));
	bool ok;
	recordid_t rid = recID.toLong(&ok);
	if (recID.isEmpty() || !ok || !rid)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": This is a new record." << endl;
#endif
		// it's a new item(no record ID and not inserted by the Palm -> PC sync), so add it
		syncAddressee(ad, 0L, 0L);
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	// look into the list of already synced record ids to see if the addressee hasn't already been synced
	if (syncedIds.contains(rid))
	{
#ifdef DEBUG
		DEBUGCONDUIT << ": address with id " << rid << " already synced." << endl;
#endif
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}


	backupRec = fLocalDatabase->readRecordById(rid);
	// only update if no backup record or the backup record is not equal to the addressee

	PilotAddress*backupAddr=0L;
	if (backupRec) backupAddr=new PilotAddress(fAddressAppInfo, backupRec);
	if(!backupRec || isFirstSync() || !_equal(backupAddr, ad)  )
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Updating entry." << endl;
#endif
		palmRec = fDatabase->readRecordById(rid);
		PilotAddress *palmAddr = 0L;
		if (palmRec)
		{
			palmAddr = new PilotAddress(fAddressAppInfo, palmRec);
		}
		else
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": No HH record with id " << rid << endl;
#endif
		}
		syncAddressee(ad, backupAddr, palmAddr);
		// update the id just in case it changed
		if (palmRec) rid=palmRec->id();
		KPILOT_DELETE(palmRec);
		KPILOT_DELETE(palmAddr);
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Entry not updated." << endl;
#endif
	}
	KPILOT_DELETE(backupAddr);
	KPILOT_DELETE(backupRec);
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": adding id:["<< rid << "] to syncedIds." << endl;
#endif
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

#ifdef DEBUG
		DEBUGCONDUIT << fname << ": now looking at palm id: ["
					<< id << "], kabc uid: [" << uid << "]." << endl;
#endif

	PilotAddress*backupAddr=0L;
	if (backupRec) backupAddr=new PilotAddress(fAddressAppInfo, backupRec);
	PilotRecord*palmRec=fDatabase->readRecordById(id);

	if ( e.isEmpty() ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": no Addressee found for this id." << endl;
		DEBUGCONDUIT << fname << "\n"
				<< backupAddr->getTextRepresentation(false) << endl;
#endif

		if (palmRec) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": deleting from database on palm." << endl;
#endif
			fDatabase->deleteRecord(id);
		}
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": deleting from backup database." << endl;
#endif
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
#ifdef DEBUG
				DEBUGCONDUIT<<"Deleting addressee "<<(*abit).realName()<<" from PC (is not on HH, and syncing with HH->PC direction)"<<endl;
#endif
				abChanged = true;
				// TODO: Can I really remove the current iterator???
				aBook->removeAddressee(*abit);
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
#ifdef DEBUG
				DEBUGCONDUIT<<"Deleting record with ID "<<*it<<" from handheld (is not on PC, and syncing with PC->HH direction)"<<endl;
#endif
				fDatabase->deleteRecord(*it);
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
	DEBUGCONDUIT << fname << ": Writing sync map to " << syncFile << endl;
	KSaveFile map( syncFile );
	if ( map.status() == 0 )
	{
		DEBUGCONDUIT << fname << ": Writing sync map ..." << endl;
		(*map.dataStream()) << addresseeMap ;
		map.close();
	}
	// This also picks up errors from map.close()
	if ( map.status() != 0 )
	{
		kdWarning() << k_funcinfo << ": Could not make backup of sync map." << endl;
	}

	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
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
	showAdresses(pcAddr, backupAddr, palmAddr);

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
#ifdef DEBUG
			DEBUGCONDUIT<< fname << ": Special case: no backup." << endl;
#endif
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
		if  (!palmAddr && isArchived(pcAddr) )
		{
			return true;
		}
		else if (!palmAddr && !pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1a"<<endl;
#endif
			// PC->HH
			bool res=_copyToHH(pcAddr, 0L, 0L);
			return res;
		}
		else if (!palmAddr && pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1b"<<endl;
#endif
			// everything's empty -> ERROR
			return false;
		}
		else if ( (isDeleted(palmAddr) || isArchived(palmAddr)) && pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1c"<<endl;
#endif
			if (isArchived(palmAddr))
				return _copyToPC(pcAddr, 0L, palmAddr);
			else
				// this happens if you add a record on the handheld and delete it again before you do the next sync
				return _deleteAddressee(pcAddr, 0L, palmAddr);
		}
		else if ((isDeleted(palmAddr)||isArchived(palmAddr)) && !pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1d"<<endl;
#endif
			// CR (ERROR)
			return _smartMergeAddressee(pcAddr, 0L, palmAddr);
		}
		else if (pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1e"<<endl;
#endif
			// HH->PC
			return _copyToPC(pcAddr, 0L, palmAddr);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1f"<<endl;
#endif
			// Conflict Resolution
			return _smartMergeAddressee(pcAddr, 0L, palmAddr);
		}
	} // !backupAddr
	else
	{
#ifdef DEBUG
			DEBUGCONDUIT<<"2"<<endl;
#endif
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
#ifdef DEBUG
			DEBUGCONDUIT<<"2a"<<endl;
#endif
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
#ifdef DEBUG
			DEBUGCONDUIT<<"2b"<<endl;
#endif
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
#ifdef DEBUG
			DEBUGCONDUIT<<"2c"<<endl;
#endif
			// update Backup, update ID of PC if neededd
			return _writeBackup(palmAddr);
		}
		else if (_equal(backupAddr, pcAddr))
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2d"<<endl;
			DEBUGCONDUIT<<"Flags: "<<palmAddr->attributes()<<", isDeleted="<<
				isDeleted(palmAddr)<<", isArchived="<<isArchived(palmAddr)<<endl;
#endif
			if (isDeleted(palmAddr))
				return _deleteAddressee(pcAddr, backupAddr, palmAddr);
			else
				return _copyToPC(pcAddr, backupAddr, palmAddr);
		}
		else if (*palmAddr == *backupAddr)
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2e"<<endl;
#endif
			return _copyToHH(pcAddr, backupAddr, palmAddr);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2f"<<endl;
#endif
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
		paddr=new PilotAddress(fAddressAppInfo);
		paddrcreated=true;
	}
	_copy(paddr, pcAddr);
#ifdef DEBUG
	DEBUGCONDUIT << fname << "palmAddr->id="<<paddr->id()<<", pcAddr.ID="<<
		pcAddr.custom(appString, idString)<<endl;
#endif

	if(_savePalmAddr(paddr, pcAddr))
	{
#ifdef DEBUG
		DEBUGCONDUIT<< fname << "Vor _saveAbEntry, palmAddr->id="<<
		paddr->id()<<", pcAddr.ID="<<pcAddr.custom(appString, idString)<<endl;
#endif
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

	showPilotAddress(palmAddr);

	_copy(pcAddr, palmAddr);
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
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": adding id:["<< palmAddr->id() << "] to syncedIds." << endl;
#endif
			syncedIds.append(palmAddr->id());
		}
		fDatabase->deleteRecord(palmAddr->id());
		fLocalDatabase->deleteRecord(palmAddr->id());
	}
	else if (backupAddr)
	{
		if (!syncedIds.contains(backupAddr->id())) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": adding id:["<< backupAddr->id() << "] to syncedIds." << endl;
#endif
			syncedIds.append(backupAddr->id());
		}
		fLocalDatabase->deleteRecord(backupAddr->id());
	}
	if (!pcAddr.isEmpty())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " removing " << pcAddr.formattedName() << endl;
#endif
		abChanged = true;
		aBook->removeAddressee(pcAddr);
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

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Saving to pilot " << palmAddr->id()
		<< " " << palmAddr->getField(entryFirstname)
		<< " " << palmAddr->getField(entryLastname)<< endl;
#endif

	PilotRecord *pilotRec = palmAddr->pack();
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": record with id=" << pilotRec->id()
		<< " len=" << pilotRec->size() << endl;
#endif
	recordid_t pilotId = fDatabase->writeRecord(pilotRec);
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Wrote "<<pilotId<<": ID="<<pilotRec->id()<<endl;
#endif
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);

	// pilotId == 0 if using local db, so don't overwrite the valid id
	if(pilotId != 0)
	{
		palmAddr->setID(pilotId);
		if (!syncedIds.contains(pilotId)) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": adding id:["<< pilotId << "] to syncedIds." << endl;
#endif
			syncedIds.append(pilotId);
		}
	}

	recordid_t abId = 0;
	abId = pcAddr.custom(appString, idString).toUInt();
	if(abId != pilotId)
	{
		pcAddr.insertCustom(appString, idString, QString::number(pilotId));
		return true;
	}

	return false;
}



bool AbbrowserConduit::_savePCAddr(Addressee &pcAddr, PilotAddress*,
	PilotAddress*)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT<<"Before _savePCAddr, pcAddr.custom="<<pcAddr.custom(appString, idString)<<endl;
#endif
	QString pilotId = pcAddr.custom(appString, idString);
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
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": no pilot address passed" << endl;
#endif
		return false;
	}
	if (abEntry.isEmpty()) {
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ":abEntry.isEmpty()" << endl;
#endif
		return false;
	}
	//  Archived records match anything so they won't be copied to the HH again
	if (flags & eqFlagsFlags)
		if (isArchived(piAddress) && isArchived(abEntry) ) return true;

	if (flags & eqFlagsName)
	{
		if(!_equal(abEntry.familyName(), piAddress->getField(entryLastname)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": last name not equal" << endl;
#endif
			return false;
		}
		// goofiness that we do in _copy(), duplicated here for your viewing pleasure... *grrr*
		QString firstAndMiddle = abEntry.givenName();
		if(!abEntry.additionalName().isEmpty()) firstAndMiddle += CSL1(" ") + abEntry.additionalName();

		if(!_equal(firstAndMiddle, piAddress->getField(entryFirstname)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": first name not equal" << endl;
#endif
			return false;
		}
		if(!_equal(abEntry.prefix(), piAddress->getField(entryTitle)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": title/prefix not equal" << endl;
#endif
			return false;
		}
		if(!_equal(abEntry.organization(), piAddress->getField(entryCompany)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": company/organization not equal" << endl;
#endif
			return false;
		}
	}
	if (flags & eqFlagsNote)
		if(!_equal(abEntry.note(), piAddress->getField(entryNote)))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": note not equal" << endl;
#endif
			return false;
	}

	if (flags & eqFlagsNote)
	{
		QString cat = _getCatForHH(abEntry.categories(), piAddress->getCategoryLabel());
		if(!_equal(cat, piAddress->getCategoryLabel()))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": category not equal" << endl;
#endif
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
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": email count not equal" << endl;
#endif
			return false;
		}
		for (QStringList::Iterator it = abEmails.begin(); it != abEmails.end(); it++) {
			if (!piEmails.contains(*it))
			{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": pilot e-mail missing" << endl;
#endif
			return false;
			}
		}
		for (QStringList::Iterator it = piEmails.begin(); it != piEmails.end(); it++) {
			if (!abEmails.contains(*it))
			{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": kabc e-mail missing" << endl;
#endif
			return false;
			}
		}

		// now look for differences in phone numbers.  Note:  we can't just compare one
		// of each kind of phone number, because there's no guarantee that if the user
		// has more than one of a given type, we're comparing the correct two.

		PhoneNumber::List abPhones(abEntry.phoneNumbers());
		PhoneNumber::List piPhones(piAddress->getPhoneNumbers());
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
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": not equal because kabc phone not found." << endl;
#endif
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
			if (!found) {
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": not equal because pilot phone not found." << endl;
#endif
				return false;
			}
		}

		if(!_equal(getOtherField(abEntry),
		   piAddress->getPhoneField(PilotAddress::eOther, false))) {
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": not equal because of other phone field." << endl;
#endif
		   	return false;
		}
	}

	if (flags & eqFlagsAdress)
	{
		KABC::Address address = getAddress(abEntry);
		if(!_equal(address.street(), piAddress->getField(entryAddress)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": address not equal" << endl;
#endif
			return false;
		}
		if(!_equal(address.locality(), piAddress->getField(entryCity)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": city not equal" << endl;
#endif
			return false;
		}
		if(!_equal(address.region(), piAddress->getField(entryState)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": state not equal" << endl;
#endif
			return false;
		}
		if(!_equal(address.postalCode(), piAddress->getField(entryZip)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": zip not equal" << endl;
#endif
			return false;
		}
		if(!_equal(address.country(), piAddress->getField(entryCountry)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": country not equal" << endl;
#endif
			return false;
		}
	}

	if (flags & eqFlagsCustom)
	{
		if(!_equal(getCustomField(abEntry, 0),
			piAddress->getField(entryCustom1)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": custom1 not equal" << endl;
#endif
			return false;
		}
		if(!_equal(getCustomField(abEntry, 1),
			piAddress->getField(entryCustom2)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": custom2 not equal" << endl;
#endif
			return false;
		}
		if(!_equal(getCustomField(abEntry, 2),
			piAddress->getField(entryCustom3)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": custom3 not equal" << endl;
#endif
			return false;
		}
		if(!_equal(getCustomField(abEntry, 3),
			piAddress->getField(entryCustom4)))
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": custom4 not equal" << endl;
#endif
			return false;
		}
	}

	// if any side is marked archived, but the other is not, the two
	// are not equal.
	if (flags & eqFlagsFlags)
		if (isArchived(piAddress) || isArchived(abEntry) )
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname  << ": archived flags don't match" << endl;
#endif
			return false;
	}

	return true;
}



void AbbrowserConduit::_copy(PilotAddress *toPilotAddr, Addressee &fromAbEntry)
{
	FUNCTIONSETUP;
	if (!toPilotAddr) return;

	toPilotAddr->setDeleted(false);

	// don't do a reset since this could wipe out non copied info
	//toPilotAddr->reset();
	toPilotAddr->setField(entryLastname, fromAbEntry.familyName());
	QString firstAndMiddle = fromAbEntry.givenName();
	if(!fromAbEntry.additionalName().isEmpty()) firstAndMiddle += CSL1(" ") + fromAbEntry.additionalName();
	toPilotAddr->setField(entryFirstname, firstAndMiddle);
	toPilotAddr->setField(entryCompany, fromAbEntry.organization());
	toPilotAddr->setField(entryTitle, fromAbEntry.prefix());
	toPilotAddr->setField(entryNote, fromAbEntry.note());

	// do email first, to ensure they get stored
	toPilotAddr->setEmails(fromAbEntry.emails());
	// now in one fell swoop, set all phone numbers from the Addressee.  Note,
	// we don't need to differentiate between Fax numbers here--all Fax numbers
	// (Home Fax or Work Fax or just plain old Fax) will get synced to the Pilot
	toPilotAddr->setPhoneNumbers(fromAbEntry.phoneNumbers());
	// Other field is an oddball and if the user has more than one field set
	// as "Other" then only one will be carried over.
	toPilotAddr->setPhoneField(PilotAddress::eOther, getOtherField(fromAbEntry), false);

	KABC::Address homeAddress = getAddress(fromAbEntry);
	_setPilotAddress(toPilotAddr, homeAddress);

	// Process the additional entries from the Palm(the palm database app block tells us the name of the fields)
	toPilotAddr->setField(entryCustom1, getCustomField(fromAbEntry, 0));
	toPilotAddr->setField(entryCustom2, getCustomField(fromAbEntry, 1));
	toPilotAddr->setField(entryCustom3, getCustomField(fromAbEntry, 2));
	toPilotAddr->setField(entryCustom4, getCustomField(fromAbEntry, 3));

	toPilotAddr->setCategory(_getCatForHH(fromAbEntry.categories(), toPilotAddr->getCategoryLabel()));

	if (isArchived(fromAbEntry))
	{
		toPilotAddr->setArchived( true );
	}
	else
	{
		toPilotAddr->setArchived( false );
	}
}



void AbbrowserConduit::_setPilotAddress(PilotAddress *toPilotAddr, const KABC::Address & abAddress)
{
	toPilotAddr->setField(entryAddress, abAddress.street());
	toPilotAddr->setField(entryCity, abAddress.locality());
	toPilotAddr->setField(entryState, abAddress.region());
	toPilotAddr->setField(entryZip, abAddress.postalCode());
	toPilotAddr->setField(entryCountry, abAddress.country());
}



void AbbrowserConduit::_copyPhone(Addressee &toAbEntry,
			      PhoneNumber phone, QString palmphone)
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



void AbbrowserConduit::_copy(Addressee &toAbEntry, PilotAddress *fromPiAddr)
{
	FUNCTIONSETUP;
	if (!fromPiAddr) return;
	// copy straight forward values
	toAbEntry.setFamilyName(fromPiAddr->getField(entryLastname));
	toAbEntry.setGivenName(fromPiAddr->getField(entryFirstname));
	toAbEntry.setOrganization(fromPiAddr->getField(entryCompany));
	toAbEntry.setPrefix(fromPiAddr->getField(entryTitle));
	toAbEntry.setNote(fromPiAddr->getField(entryNote));

	// set the formatted name
	// TODO this is silly and should be removed soon.
	toAbEntry.setFormattedName(toAbEntry.realName());

	// copy the phone stuff
	// first off, handle the e-mail addresses as a group and separate from
	// the other phone number fields
	toAbEntry.setEmails(fromPiAddr->getEmails());

	// going from Pilot to kabc, we need to clear out all phone records in kabc
	// so that they can be set from the Pilot.  If we do not do this, then records
	// will be left in kabc when they are removed from the Pilot and we'll look
	// broken.
	PhoneNumber::List old = toAbEntry.phoneNumbers();
	for (PhoneNumber::List::Iterator it = old.begin(); it != old.end(); ++it) {
		PhoneNumber phone = *it;
		toAbEntry.removePhoneNumber(phone);
	}

	// now, get the phone numbers from the Pilot and set them one at a time in kabc
	PhoneNumber::List phones = fromPiAddr->getPhoneNumbers();
	for (PhoneNumber::List::Iterator it = phones.begin(); it != phones.end(); ++it) {
		PhoneNumber phone = *it;
		// fax is an odd-ball because while the Pilot has a single "Fax" field, kabc has
		// both a "Home Fax" and a "Work Fax" field and we need to tell kabc
		// what our user has asked us to.
		if (phone.type() & PhoneNumber::Fax) {
			_copyPhone(toAbEntry, getFax(toAbEntry), phone.number());
		} else {
			_copyPhone(toAbEntry, toAbEntry.phoneNumber(phone.type()),
				phone.number());
		}
	}

	// Note:  this is weird, and it may cause data to not be synced if there is
	// more than one "Other" field being used on the Pilot, since only one will
	// be synced in either direction.
	setOtherField(toAbEntry, fromPiAddr->getPhoneField(PilotAddress::eOther, false));

	KABC::Address homeAddress = getAddress(toAbEntry);
	homeAddress.setStreet(fromPiAddr->getField(entryAddress));
	homeAddress.setLocality(fromPiAddr->getField(entryCity));
	homeAddress.setRegion(fromPiAddr->getField(entryState));
	homeAddress.setPostalCode(fromPiAddr->getField(entryZip));
	homeAddress.setCountry(fromPiAddr->getField(entryCountry));
	toAbEntry.insertAddress(homeAddress);

	setCustomField(toAbEntry, 0, fromPiAddr->getField(entryCustom1));
	setCustomField(toAbEntry, 1, fromPiAddr->getField(entryCustom2));
	setCustomField(toAbEntry, 2, fromPiAddr->getField(entryCustom3));
	setCustomField(toAbEntry, 3, fromPiAddr->getField(entryCustom4));

	// copy the fromPiAddr pilot id to the custom field KPilot_Id;
	// pilot id may be zero(since it could be new) but couldn't hurt
	// to even assign it to zero; let's us know what state the
	// toAbEntry is in
	toAbEntry.insertCustom(appString, idString, QString::number(fromPiAddr->id()));


	int cat = fromPiAddr->category();
	QString category = fAddressAppInfo->categoryName(cat);
	_setCategory(toAbEntry, category);
#ifdef DEBUG
	showAddressee(toAbEntry);
#endif
	if (isArchived(fromPiAddr))
		makeArchived(toAbEntry);
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

#ifdef DEBUG
	DEBUGCONDUIT<<"pc="<<pc<<", backup="<<backup<<", palm="<<
		palm<<", ConfRes="<<confRes<<endl;
	DEBUGCONDUIT<<"Use conflict resolution :"<<confRes<<
		", PC="<<SyncAction::ePCOverrides<<endl;
#endif
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
	appendGen(desc, abfield, getPhoneField(PilotAddress::palmfield, false))
#define appendPhone(desc, abfield, palmfield) \
	appendGenPhone(desc, pcAddr.phoneNumber(PhoneNumber::abfield).number(), palmfield)


	appendAddr(i18n("Last name"), pcAddr.familyName(), entryLastname);
	appendAddr(i18n("First name"), pcAddr.givenName(), entryFirstname);
	appendAddr(i18n("Organization"), pcAddr.organization(), entryCompany);
	appendAddr(i18n("Title"), pcAddr.prefix(), entryTitle);
	appendAddr(i18n("Note"), pcAddr.note(), entryNote);
	appendAddr(i18n("Custom 1"), getCustomField(pcAddr, 0), entryCustom1);
	appendAddr(i18n("Custom 2"), getCustomField(pcAddr, 1), entryCustom2);
	appendAddr(i18n("Custom 3"), getCustomField(pcAddr, 2), entryCustom3);
	appendAddr(i18n("Custom 4"), getCustomField(pcAddr, 3), entryCustom4);
	appendPhone(i18n("Work Phone"), Work, eWork);
	appendPhone(i18n("Home Phone"), Home, eHome);
	appendPhone(i18n("Mobile Phone"), Cell, eMobile);
	appendGenPhone(i18n("Fax"), getFax(pcAddr).number(), eFax);
	appendPhone(i18n("Pager"), Pager, ePager);
	appendGenPhone(i18n("Other"), getOtherField(pcAddr), eOther);
	appendGenPhone(i18n("Email"), pcAddr.preferredEmail(), eEmail);

	KABC::Address abAddress = getAddress(pcAddr);
	appendAddr(i18n("Address"), abAddress.street(), entryAddress);
	appendAddr(i18n("City"), abAddress.locality(), entryCity);
	appendAddr(i18n("Region"), abAddress.region(), entryState);
	appendAddr(i18n("Postal code"), abAddress.postalCode(), entryZip);
	appendAddr(i18n("Country"), abAddress.country(), entryCountry);

	appendGen(i18n("Category"),
		_getCatForHH(pcAddr.categories(), (palmAddr)?(palmAddr->getCategoryLabel()):(QString::null)),
		getCategoryLabel());

#undef appendGen
#undef appendAddr
#undef appendGenPhone
#undef appendPhone

	return true;
}



bool AbbrowserConduit::_applyResolutionTable(ResolutionTable*tab, Addressee &pcAddr,
	PilotAddress *backupAddr, PilotAddress *palmAddr)
{
	FUNCTIONSETUP;
	if (!tab) return false;
	if (!palmAddr) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Empty palmAddr after conf res. ERROR!!!!"<<endl;
#endif
		kdWarning()<<"Empty palmAddr after conf res. ERROR!!!!"<<endl;
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
	SETGENFIELD(setCustomField(pcAddr, abfield, item->fResolved), palmfield)
#define SETGENPHONE(abfield, palmfield) \
	if (item) { \
		abfield; \
		palmAddr->setPhoneField(PilotAddress::palmfield, item->fResolved, false); \
	}\
	item=tab->next();
#define SETPHONEFIELD(abfield, palmfield) \
	SETGENPHONE(_setPhoneNumber(pcAddr, PhoneNumber::abfield, item->fResolved), palmfield)
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
	SETGENPHONE(setFax(pcAddr, item->fResolved), eFax);
	SETPHONEFIELD(Pager, ePager);
	SETGENPHONE(setOtherField(pcAddr, item->fResolved), eOther);

	// TODO: fix email
	if (item) {
		palmAddr->setPhoneField(PilotAddress::eEmail, item->fResolved, false);
		if (backupAddr)
			pcAddr.removeEmail(backupAddr->getPhoneField(PilotAddress::eEmail, false));
		pcAddr.removeEmail(palmAddr->getPhoneField(PilotAddress::eEmail, false));
		pcAddr.insertEmail(item->fResolved, true);
	}
	item=tab->next();

	KABC::Address abAddress = getAddress(pcAddr);
	SETADDRESSFIELD(setStreet, entryAddress);
	SETADDRESSFIELD(setLocality, entryCity);
	SETADDRESSFIELD(setRegion, entryState);
	SETADDRESSFIELD(setPostalCode, entryZip);
	SETADDRESSFIELD(setCountry, entryCountry);
	pcAddr.insertAddress(abAddress);

	// TODO: Is this correct?
	if (item) {
		palmAddr->setCategory(item->fResolved);
		_setCategory(pcAddr, item->fResolved);
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
			pcAddr.removeCustom(appString, idString);
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
			_copy(pcAddr, backupAddr);
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
				pAddr=new PilotAddress(fAddressAppInfo);
				pAddrCreated=true;
			}
			result &= _applyResolutionTable(&tab, pcAddr, backupAddr, pAddr);
showAdresses(pcAddr, backupAddr, pAddr);
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
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": PilotRecord has id " << pilotAddress.id() << ", mapped to " << id << endl;
#endif
		if(!id.isEmpty())
		{
			Addressee res(aBook->findByUid(id));
			if(!res.isEmpty()) return res;
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": PilotRecord has id " << pilotAddress.id() << ", but could not be found in the addressbook" << endl;
#endif
		}
	}

	for(AddressBook::Iterator iter = aBook->begin(); iter != aBook->end(); ++iter)
	{
		Addressee abEntry = *iter;
		QString recID(abEntry.custom(appString, idString));
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
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Could not find any addressbook enty matching " << pilotAddress.getField(entryLastname) << endl;
#endif
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
	PilotAddress a(fAddressAppInfo,r);
	KPILOT_DELETE(r);

	// Process this record.
	showPilotAddress(&a);

	// Schedule more work.
	++pilotindex;
	QTimer::singleShot(0, this, SLOT(slotTestRecord()));
}
