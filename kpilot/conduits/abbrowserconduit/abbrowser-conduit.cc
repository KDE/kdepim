// abbrowser-conduit.cc
//
// Copyright (C) 2000,2001 by Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
//
// The abbrowser conduit copies memos from the Pilot's address book to 
// abbrowser (KAddressbook after KDE 2.1) and vice-versa.
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

#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>

#include <pi-appinfo.h>
#include <pi-source.h>

#include "pilotSerialDatabase.h"
#include "abbrowser-factory.h"

#include "abbrowser-conduit.moc"


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *abbrowser_conduit_id =
	"$Id$";



AbbrowserConduit::AbbrowserConduit(KPilotDeviceLink * o,
	const char *n, 
	const QStringList & a) :
	ConduitAction(o, n, a),
	fDCOP(0L),
	fDatabase(0L)
{
	FUNCTIONSETUP;

	fDCOP = KApplication::kApplication()->dcopClient();
	if (!fDCOP)
	{
		kdWarning() << k_funcinfo
			<< ": Can't get DCOP client." << endl;
	}
}

AbbrowserConduit::~AbbrowserConduit()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fDatabase);
}



bool AbbrowserConduit::_startAbbrowser()
{
	FUNCTIONSETUP;

	bool alreadyRunning = true;
	bool foundAbbrowser = false;
	QByteArray sendData;
	QByteArray replyData;
	QCString replyTypeStr;


	KConfigGroupSaver g(fConfig, AbbrowserConduitFactory::group());
	QCString abbrowserName(fConfig->readEntry("AbbrowserName",
			"kaddressbook"));
	QCString abbrowserIface(fConfig->readEntry("AbbrowserIface",
			"KAddressBookIface"));

	/**
	* It's ugly to use defines, I know, but anything else is silly
	* ie. functions have too much overhead. It's undeffed below.
	*/
#define PING_ABBROWSER (fDCOP->call(abbrowserName, \
	abbrowserIface, \
	"interfaces()",  \
	sendData, replyTypeStr, replyData))


	foundAbbrowser = PING_ABBROWSER;
	if (!foundAbbrowser)
	{
		// abbrowser not running, start it
		kapp->startServiceByDesktopName(abbrowserName, QString::null);

#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< "Waiting to run " << abbrowserName << endl;
#endif

		alreadyRunning = false;
	}

	// Now check every second if abbrowser is already running,
	// while keeping the Pilot awake.
	//
	//
	for (int i = 0; !foundAbbrowser && (i < 20); i++)
	{
		sleep(1);
		kapp->processEvents();
		pi_tickle(pilotSocket());

		foundAbbrowser = PING_ABBROWSER;
	}

	if (!foundAbbrowser)
	{
		kdWarning() << fname
			<< " unable to connect to "
			<< abbrowserName
			<< " through dcop; autostart failed" << endl;
	}
	return alreadyRunning;

#undef PING_ABBROWSER
}

void AbbrowserConduit::_stopAbbrowser(bool abAlreadyRunning)
{
	FUNCTIONSETUP;


	KConfigGroupSaver g(fConfig, AbbrowserConduitFactory::group());
	QCString abbrowserName(
		fConfig->readEntry("AbbrowserName", "kaddressbook"));
	QCString abbrowserIface(
		fConfig->readEntry("AbbrowserIface", "KAddressBookIface"));

	if (!abAlreadyRunning)
	{
		QByteArray sendData;
		QByteArray replyData;
		QCString replyTypeStr;

		if (!fDCOP->call(abbrowserName, 
			abbrowserIface,
			"exit()", 
			sendData, replyTypeStr, replyData))
		{
			kdWarning() << k_funcinfo
				<< "Unable to tell " 
				<< abbrowserName 
				<< " to quit."
				<< endl;
		}
	}
}

void AbbrowserConduit::_mapContactsToPilot(
	const QDict < ContactEntry > &contacts,
	QMap < recordid_t, QString > &idContactMap,
	QDict < ContactEntry > &newContacts) const
{
	FUNCTIONSETUP;

	idContactMap.clear();
	newContacts.clear();
	for (QDictIterator < ContactEntry > contactIter(contacts);
		contactIter.current(); ++contactIter)
	{
		ContactEntry *aContact = contactIter.current();

		if (aContact->isNew())
			newContacts.insert(contactIter.currentKey(),
				aContact);
		else
		{
			QString idStr = aContact->getCustomField("KPILOT_ID");

			if (idStr != QString::null)
			{
				recordid_t id = idStr.toULong();

				idContactMap.insert(id,
					contactIter.currentKey());
			}
			else
			{
				kdWarning() << k_funcinfo 
					<< " contact is new but KPILOT_ID is not found in abbrowser contact; BUG!"
					<< endl;
				newContacts.insert(contactIter.currentKey(),
					aContact);
			}
		}
	}
}

bool AbbrowserConduit::_getAbbrowserContacts(QDict < ContactEntry > &contacts)
{
	FUNCTIONSETUP;

	QDict < ContactEntry > entryDict;

	KConfigGroupSaver g(fConfig, AbbrowserConduitFactory::group());
	QCString abbrowserName(fConfig->readEntry("AbbrowserName",
			"kaddressbook"));
	QCString abbrowserIface(fConfig->readEntry("AbbrowserIface",
			"KAddressBookIface"));

	QByteArray noParamData;
	QByteArray replyDictData;
	QCString replyTypeStr;

	if (!fDCOP->call(abbrowserName, abbrowserIface, "getEntryDict()",
			noParamData, replyTypeStr, replyDictData))
	{
		kdWarning() << k_funcinfo
			<< "Unable to call abbrowser getEntryDict()" << endl;
		return false;
	}
	assert(replyTypeStr == "QDict<ContactEntry>");

	QDataStream dictStream(replyDictData, IO_ReadOnly);

	dictStream >> contacts;
	return true;
}

#ifdef DEBUG
void AbbrowserConduit::showContactEntry(const ContactEntry & abAddress)
{
	qDebug("\tAbbrowser Contact Entry");
	qDebug("\t\tLast name = %s", abAddress.getLastName().latin1());
	qDebug("\t\tFirst name = %s", abAddress.getFirstName().latin1());
	qDebug("\t\tCompany = %s", abAddress.getCompany().latin1());
	qDebug("\t\tJob Title = %s", abAddress.getJobTitle().latin1());
	qDebug("\t\tNote = %s", abAddress.getNote().latin1());
	qDebug("\t\tHome phone = %s", abAddress.getHomePhone().latin1());
	qDebug("\t\tWork phone = %s", abAddress.getBusinessPhone().latin1());
	qDebug("\t\tMobile phone = %s", abAddress.getMobilePhone().latin1());
	qDebug("\t\tEmail = %s", abAddress.getEmail().latin1());
	qDebug("\t\tFax = %s", abAddress.getBusinessFax().latin1());
	qDebug("\t\tPager = %s", abAddress.getPager().latin1());
	qDebug("\t\tCategory = %s", abAddress.getFolder().latin1());

}


void AbbrowserConduit::showPilotAddress(const PilotAddress & pilotAddress)
{
	qDebug("\tPilot Address");
	qDebug("\t\tLast name = %s", pilotAddress.getField(entryLastname));
	qDebug("\t\tFirst name = %s", pilotAddress.getField(entryFirstname));
	qDebug("\t\tCompany = %s", pilotAddress.getField(entryCompany));
	qDebug("\t\tJob Title = %s", pilotAddress.getField(entryTitle));
	qDebug("\t\tNote = %s", pilotAddress.getField(entryNote));
	qDebug("\t\tHome phone = %s",
		pilotAddress.getPhoneField(PilotAddress::eHome));
	qDebug("\t\tWork phone = %s",
		pilotAddress.getPhoneField(PilotAddress::eWork));
	qDebug("\t\tMobile phone = %s",
		pilotAddress.getPhoneField(PilotAddress::eMobile));
	qDebug("\t\tEmail = %s",
		pilotAddress.getPhoneField(PilotAddress::eEmail));
	qDebug("\t\tFax = %s",
		pilotAddress.getPhoneField(PilotAddress::eFax));
	qDebug("\t\tPager = %s",
		pilotAddress.getPhoneField(PilotAddress::ePager));
	qDebug("\t\tCategory = %s", pilotAddress.getCategoryLabel());

}
#endif

void AbbrowserConduit::_copy(PilotAddress & toPilotAddr,
	ContactEntry & fromAbEntry)
{
	// don't do a reset since this could wipe out non copied info 
	//toPilotAddr.reset();
	toPilotAddr.setField(entryLastname,
		fromAbEntry.getLastName().latin1());
	QString firstAndMiddle = fromAbEntry.getFirstName();

	if (!fromAbEntry.getMiddleName().isEmpty())
		firstAndMiddle += " " + fromAbEntry.getMiddleName();
	toPilotAddr.setField(entryFirstname, firstAndMiddle.latin1());
	toPilotAddr.setField(entryCompany, fromAbEntry.getCompany().latin1());
	toPilotAddr.setField(entryTitle, fromAbEntry.getJobTitle().latin1());
	toPilotAddr.setField(entryNote, fromAbEntry.getNote().latin1());

	// do email first, to ensure its gets stored
	toPilotAddr.setPhoneField(PilotAddress::eEmail,
		fromAbEntry.getEmail().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eWork,
		fromAbEntry.getBusinessPhone().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eHome,
		fromAbEntry.getHomePhone().latin1());
	toPilotAddr.setPhoneField(PilotAddress::eMobile,
		fromAbEntry.getMobilePhone().latin1());
	if (isPilotFaxHome())
		toPilotAddr.setPhoneField(PilotAddress::eFax,
			fromAbEntry.getHomeFax().latin1());
	else
		toPilotAddr.setPhoneField(PilotAddress::eFax,
			fromAbEntry.getBusinessFax().latin1());

	toPilotAddr.setPhoneField(PilotAddress::ePager,
		fromAbEntry.getPager().latin1());
	toPilotAddr.setShownPhone(PilotAddress::eMobile);
	QString otherMapType = getPilotOtherMap();

	if (!otherMapType.isEmpty())
		toPilotAddr.setPhoneField(PilotAddress::eOther,
			fromAbEntry.findRef(otherMapType).latin1());
	// in future, may want prefs that will map from abbrowser entries
	// to the pilot phone entries so they can do the above assignment and
	// assign the Other entry which is currenty unused
	ContactEntry::Address * homeAddress = fromAbEntry.getHomeAddress();
	if (!homeAddress->isEmpty())
		_setPilotAddress(toPilotAddr, *homeAddress);
	else
	{
		// no home address, try work address
		ContactEntry::Address * workAddress =
			fromAbEntry.getBusinessAddress();
		if (!workAddress->isEmpty())
			_setPilotAddress(toPilotAddr, *workAddress);
		delete workAddress;

		workAddress = NULL;
	}
	delete homeAddress;

	homeAddress = NULL;
}

void AbbrowserConduit::_setPilotAddress(PilotAddress & toPilotAddr,
	const ContactEntry::Address & abAddress)
{
	toPilotAddr.setField(entryAddress, abAddress.getStreet().latin1());
	toPilotAddr.setField(entryCity, abAddress.getCity().latin1());
	toPilotAddr.setField(entryState, abAddress.getState().latin1());
	toPilotAddr.setField(entryZip, abAddress.getZip().latin1());
	toPilotAddr.setField(entryCountry, abAddress.getCountry().latin1());
}

void AbbrowserConduit::_copy(ContactEntry & toAbEntry,
	const PilotAddress & fromPiAddr)
{
	// copy straight forward values
	toAbEntry.setLastName(fromPiAddr.getField(entryLastname));
	toAbEntry.setFirstName(fromPiAddr.getField(entryFirstname));
	toAbEntry.setCompany(fromPiAddr.getField(entryCompany));
	toAbEntry.setJobTitle(fromPiAddr.getField(entryTitle));
	toAbEntry.setNote(fromPiAddr.getField(entryNote));
	toAbEntry.setName();

	// copy the phone stuff
	toAbEntry.setEmail(fromPiAddr.getPhoneField(PilotAddress::eEmail));
	toAbEntry.setHomePhone(fromPiAddr.getPhoneField(PilotAddress::eHome));
	toAbEntry.setBusinessPhone(fromPiAddr.
		getPhoneField(PilotAddress::eWork));
	toAbEntry.setMobilePhone(fromPiAddr.
		getPhoneField(PilotAddress::eMobile));
	if (isPilotFaxHome())
		toAbEntry.setHomeFax(fromPiAddr.
			getPhoneField(PilotAddress::eFax));
	else
		toAbEntry.setBusinessFax(fromPiAddr.
			getPhoneField(PilotAddress::eFax));
	toAbEntry.setPager(fromPiAddr.getPhoneField(PilotAddress::ePager));
	if (!getPilotOtherMap().isEmpty())
		toAbEntry.replaceValue(getPilotOtherMap(),
			fromPiAddr.getPhoneField(PilotAddress::eOther));

	//in future, probably the address assigning to work or home should
	// be a prefs option
	// for now, just assign to home since that's what I'm using it for
	ContactEntry::Address * homeAddress = toAbEntry.getHomeAddress();
	homeAddress->setStreet(fromPiAddr.getField(entryAddress));
	homeAddress->setCity(fromPiAddr.getField(entryCity));
	homeAddress->setState(fromPiAddr.getField(entryState));
	homeAddress->setZip(fromPiAddr.getField(entryZip));
	homeAddress->setCountry(fromPiAddr.getField(entryCountry));
	delete homeAddress;

	homeAddress = NULL;

	// copy the fromPiAddr pilot id to the custom field KPilot_Id;
	// pilot id may be zero (since it could be new) but couldn't hurt
	// to even assign it to zero; let's us know what state the
	// toAbEntry is in
	toAbEntry.setCustomField("KPILOT_ID",
		QString::number(fromPiAddr.getID()));
}


void AbbrowserConduit::_addToAbbrowser(const PilotAddress & address)
{
	ContactEntry entry;

	_copy(entry, address);
	_saveAbEntry(entry, QString::null);
}

void AbbrowserConduit::_addToPalm(const QString & key, ContactEntry & entry)
{
	PilotAddress pilotAddress(fAddressAppInfo);

	_copy(pilotAddress, entry);
	if (_savePilotAddress(pilotAddress, entry))
		_saveAbEntry(entry, key);
}

bool AbbrowserConduit::_conflict(const QString & str1,
	const QString & str2, bool & mergeNeeded, QString & mergedStr) const
{
	mergeNeeded = false;
	if (str1.isEmpty() && str2.isEmpty())
		return false;
	if (str1.isEmpty() || str2.isEmpty())
	{
		mergeNeeded = true;
		if (str1 == QString::null)
			mergedStr = str2;
		else
			mergedStr = str1;
		return false;
	}
	if (str1 != str2)
		return true;
	return false;
}

bool AbbrowserConduit::_smartMerge(PilotAddress & outPilotAddress,
	ContactEntry & outAbEntry)
{
	PilotAddress pilotAddress(outPilotAddress);
	ContactEntry abEntry(outAbEntry);

	bool mergeNeeded = false;
	QString mergedStr;

	if (_conflict(pilotAddress.getField(entryLastname),
			abEntry.getLastName(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setField(entryLastname, mergedStr.latin1());
		abEntry.setLastName(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryFirstname),
			abEntry.getFirstName(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setField(entryFirstname, mergedStr.latin1());
		abEntry.setFirstName(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryCompany),
			abEntry.getCompany(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setField(entryCompany, mergedStr.latin1());
		abEntry.setCompany(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryTitle),
			abEntry.getJobTitle(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setField(entryTitle, mergedStr.latin1());
		abEntry.setJobTitle(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryNote),
			abEntry.getNote(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setField(entryNote, mergedStr.latin1());
		abEntry.setNote(mergedStr);
	}

	if (_conflict(pilotAddress.getPhoneField(PilotAddress::eWork),
			abEntry.getBusinessPhone(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eWork,
			mergedStr.latin1());
		abEntry.setBusinessPhone(mergedStr);
	}

	if (_conflict(pilotAddress.getPhoneField(PilotAddress::eHome),
			abEntry.getHomePhone(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eHome,
			mergedStr.latin1());
		abEntry.setHomePhone(mergedStr);
	}

	if (_conflict(pilotAddress.getPhoneField(PilotAddress::eEmail),
			abEntry.getEmail(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eEmail,
			mergedStr.latin1());
		abEntry.setEmail(mergedStr);
	}

	if (_conflict(pilotAddress.getPhoneField(PilotAddress::eMobile),
			abEntry.getMobilePhone(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eMobile,
			mergedStr.latin1());
		abEntry.setMobilePhone(mergedStr);
	}

	if (isPilotFaxHome())
		if (_conflict(pilotAddress.getPhoneField(PilotAddress::eFax),
				abEntry.getHomeFax(), mergeNeeded, mergedStr))
			return false;
		else if (_conflict(pilotAddress.
				getPhoneField(PilotAddress::eFax),
				abEntry.getBusinessFax(), mergeNeeded,
				mergedStr))
			return false;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eFax,
			mergedStr.latin1());
		if (isPilotFaxHome())
			abEntry.setHomeFax(mergedStr);
		else
			abEntry.setBusinessFax(mergedStr);
	}

	if (_conflict(pilotAddress.getPhoneField(PilotAddress::ePager),
			abEntry.getPager(), mergeNeeded, mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::ePager,
			mergedStr.latin1());
		abEntry.setPager(mergedStr);
	}

	if (_conflict(pilotAddress.getPhoneField(PilotAddress::eOther),
			abEntry.findRef(fPilotOtherMap), mergeNeeded,
			mergedStr))
		return false;
	if (mergeNeeded)
	{
		pilotAddress.setPhoneField(PilotAddress::eOther,
			mergedStr.latin1());
		abEntry.replaceValue(fPilotOtherMap, mergedStr);
	}

	ContactEntry::Address * abAddress = abEntry.getHomeAddress();
	if (_conflict(pilotAddress.getField(entryAddress),
			abAddress->getStreet(), mergeNeeded, mergedStr))
	{
		delete abAddress;

		abAddress = abEntry.getBusinessAddress();

		if (_conflict(pilotAddress.getField(entryAddress),
				abAddress->getStreet(), mergeNeeded,
				mergedStr))
		{
			delete abAddress;

			return false;
		}
	}
	if (mergeNeeded)
	{
		pilotAddress.setField(entryAddress, mergedStr.latin1());
		abAddress->setStreet(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryCity), abAddress->getCity(),
			mergeNeeded, mergedStr))
	{
		delete abAddress;

		return false;
	}
	if (mergeNeeded)
	{
		pilotAddress.setField(entryCity, mergedStr.latin1());
		abAddress->setCity(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryState),
			abAddress->getState(), mergeNeeded, mergedStr))
	{
		delete abAddress;

		return false;
	}
	if (mergeNeeded)
	{
		pilotAddress.setField(entryState, mergedStr.latin1());
		abAddress->setState(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryZip), abAddress->getZip(),
			mergeNeeded, mergedStr))
	{
		delete abAddress;

		return false;
	}
	if (mergeNeeded)
	{
		pilotAddress.setField(entryZip, mergedStr.latin1());
		abAddress->setZip(mergedStr);
	}

	if (_conflict(pilotAddress.getField(entryCountry),
			abAddress->getCountry(), mergeNeeded, mergedStr))
	{
		delete abAddress;

		return false;
	}
	if (mergeNeeded)
	{
		pilotAddress.setField(entryCountry, mergedStr.latin1());
		abAddress->setCountry(mergedStr);
	}

	delete abAddress;

	abEntry.setCustomField("KPILOT_ID",
		QString::number(pilotAddress.id()));
	abEntry.setName();

	abEntry.setFolder(pilotAddress.getCategoryLabel());

	outPilotAddress = pilotAddress;
	outAbEntry = abEntry;

	return true;
}

/** There was a conflict between the two fields; either could be null,
 *  or both have been modified
 */
void AbbrowserConduit::_handleConflict(PilotAddress * pilotAddress,
	ContactEntry * abEntry, const QString & abKey)
{
	FUNCTIONSETUP;

	if (pilotAddress && abEntry)
	{
		if (doSmartMerge() && _smartMerge(*pilotAddress, *abEntry))
		{

#ifdef DEBUG
			DEBUGCONDUIT << fname 
				<< " Both records exist and "
				   "both were changed => MERGE done"
				<< endl;
#endif

			if (getMode() != Backup)
				_savePilotAddress(*pilotAddress, *abEntry);
			_saveAbEntry(*abEntry, abKey);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname 
				<< ": Both records exist but both "
				   "were changed => conflict, "
				   " unable to merge; keeping both."
				<< endl;
			showPilotAddress(*pilotAddress);
			showContactEntry(*abEntry);
#endif


			switch (getResolveConflictOption())
			{
			case eUserChoose:
				// implement later with conflict dialog...
			case eKeepBothInAbbrowser:
				_addToAbbrowser(*pilotAddress);
				break;
			case ePilotOverides:
				_addToAbbrowser(*pilotAddress);
				_removeAbEntry(abKey);
				break;
			case eAbbrowserOverides:
				// in future, should see what the config wants to do for
				// deconfliction; for now will just keep both
				if (getMode() != Backup)
				{
					_addToPalm(abKey, *abEntry);
					_removePilotAddress(*pilotAddress);
				}
				break;
			case eDoNotResolve:
			default:
				// do nothing
				break;
			}
		}
	}
	else if (pilotAddress)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname <<
			" ContactEntry was deleted but pilotAddress was modified"
			<< endl;
		showPilotAddress(*pilotAddress);
#endif
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname <<
			" PilotAddress was deleted but ConactEntry was modified"
			<< endl;
		showContactEntry(*abEntry);
#endif
	}
}

void AbbrowserConduit::_removePilotAddress(PilotAddress & address)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " deleting from palm pilot " << endl;
	showPilotAddress(address);
#endif

	address.makeDeleted();
	PilotRecord *pilotRec = address.pack();

	fDatabase->writeRecord(pilotRec);

	delete pilotRec;

	pilotRec = 0L;
}

void AbbrowserConduit::_removeAbEntry(const QString & key)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " removing key " << key << endl;
#endif

	KConfigGroupSaver g(fConfig,AbbrowserConduitFactory::group());

	QCString abbrowserName(fConfig->readEntry("AbbrowserName", 
		"kaddressbook"));
	QCString abbrowserIface(fConfig->readEntry("AbbrowserIface",
		"KAddressBookIface"));

	QByteArray sendData;
	QByteArray replyData;
	QCString replyTypeStr;
	QDataStream out(sendData, IO_WriteOnly);

	if (!key.isEmpty())
	{
		// new entry; just send the contact entry
		out << key;
		if (!fDCOP->call(abbrowserName, abbrowserIface,
				"removeEntry(QString)",
				sendData, replyTypeStr, replyData))
		{
			kdWarning() << "Unable to call "
				<< abbrowserName << " removeEnty" << endl;
			// Don't exit after all: this Sync should possibly continue
			//
			// qApp->exit(1);
		}
	}
}

void AbbrowserConduit::_saveAbEntry(ContactEntry & abEntry,
	const QString & key)
{
	FUNCTIONSETUP;

	// We may need to operate in a KDE 2.1 environment,
	// where the address book is called "abbrowser" instead
	// of "kaddressbook".
	//
	//
	KConfigGroupSaver g(fConfig,AbbrowserConduitFactory::group());
	QCString abbrowserName(fConfig->readEntry("AbbrowserName", 
		"kaddressbook"));
	QCString abbrowserIface(fConfig->readEntry("AbbrowserIface",
		"KAddressBookIface"));


	// this marks that this field has been synced
	abEntry.setModified(false);

#ifdef DEBUG
	DEBUGCONDUIT << fname 
		<< " saving to " 
		<< abbrowserName
		<< abEntry.getFullName()
		<< " " 
		<< abEntry.getCompany() << endl;
#endif

	// save over kdcop to abbrowser
	QByteArray sendData;
	QByteArray replyData;
	QCString replyTypeStr;
	QDataStream out(sendData, IO_WriteOnly);

	if (key == QString::null)
	{
		// new entry; just send the contact entry
		out << abEntry;
		if (!fDCOP->call(abbrowserName, abbrowserIface,
				"addEntry(ContactEntry)",
				sendData, replyTypeStr, replyData))
		{
			kdWarning() << "Unable to call abbrowser addEntry" <<
				endl;
			// qApp->exit(1);
		}
	}
	else
	{
		// change entry, send contact and key
		out << key;
		out << abEntry;
		if (!fDCOP->call(abbrowserName, abbrowserIface,
				"changeEntry(QString, ContactEntry)",
				sendData, replyTypeStr, replyData))
		{
			kdWarning() << "Unable to call "
				<< abbrowserName << " changeEntry" << endl;
			// qApp->exit(1);
		}
	}
}

void AbbrowserConduit::_saveAbChanges()
{
	QByteArray sendData;
	QByteArray replyData;
	QCString replyTypeStr;

	// We may need to operate in a KDE 2.1 environment,
	// where the address book is called "abbrowser" instead
	// of "kaddressbook".
	//
	//
	KConfigGroupSaver g(fConfig,AbbrowserConduitFactory::group());
	QCString abbrowserName(fConfig->readEntry("AbbrowserName", 
		"kaddressbook"));
	QCString abbrowserIface(fConfig->readEntry("AbbrowserIface",
		"KAddressBookIface"));


	if (!fDCOP->call(abbrowserName, abbrowserIface,
			"save()", sendData, replyTypeStr, replyData))
	{
		kdWarning() << "Unable to save kaddressbook" << endl;
		// qApp->exit(1);
	}
}

bool AbbrowserConduit::_savePilotAddress(PilotAddress & address,
	ContactEntry & abEntry)
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
	delete pilotRec;

	// pilotId == 0 if using local db, so don't overwrite the valid id
	if (pilotId != 0)
		address.setID(pilotId);

	recordid_t abId = 0;

	if (abEntry.getCustomField("KPILOT_ID") != QString::null)
		abId = abEntry.getCustomField("KPILOT_ID").toUInt();
	if (abId != address.id())
	{
		QString abIdStr = QString::number(address.id());

		abEntry.setCustomField("KPILOT_ID", abIdStr);
		return true;
	}

	return false;
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

bool AbbrowserConduit::_equal(const PilotAddress & piAddress,
	ContactEntry & abEntry) const
{
	bool mergeNeeded = false;	// not needed here, but in merge func
	QString mergedStr;	// not needed here, but in merge func

	if (_conflict(piAddress.getField(entryLastname),
			abEntry.getLastName(), mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getField(entryFirstname),
			abEntry.getFirstName(), mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getField(entryTitle), abEntry.getJobTitle(),
			mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getField(entryCompany), abEntry.getCompany(),
			mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getField(entryNote), abEntry.getNote(),
			mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getCategoryLabel(), abEntry.getFolder(),
			mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getPhoneField(PilotAddress::eWork),
			abEntry.getBusinessPhone(), mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getPhoneField(PilotAddress::eHome),
			abEntry.getHomePhone(), mergeNeeded, mergedStr))
		return false;
	if (_conflict(piAddress.getPhoneField(PilotAddress::eEmail),
			abEntry.findRef(fPilotOtherMap), mergeNeeded,
			mergedStr))
		return false;
	if (_conflict(piAddress.getPhoneField(PilotAddress::eOther),
			abEntry.getEmail(), mergeNeeded, mergedStr))
		return false;
	if (isPilotFaxHome())
		if (_conflict(piAddress.getPhoneField(PilotAddress::eFax),
				abEntry.getHomeFax(), mergeNeeded, mergedStr))
			return false;
		else if (_conflict(piAddress.
				getPhoneField(PilotAddress::eFax),
				abEntry.getBusinessFax(), mergeNeeded,
				mergedStr))
			return false;
	if (_conflict(piAddress.getPhoneField(PilotAddress::eMobile),
			abEntry.getMobilePhone(), mergeNeeded, mergedStr))
		return false;
	ContactEntry::Address * address = abEntry.getHomeAddress();
	if (_conflict(piAddress.getField(entryAddress), address->getStreet(),
			mergeNeeded, mergedStr))
	{
		delete address;

		address = abEntry.getBusinessAddress();
		if (_conflict(piAddress.getField(entryAddress),
				address->getStreet(), mergeNeeded, mergedStr))
		{
			delete address;

			return false;
		}
	}
	if (_conflict(piAddress.getField(entryCity), address->getCity(),
			mergeNeeded, mergedStr))
	{
		delete address;

		return false;
	}
	if (_conflict(piAddress.getField(entryState), address->getState(),
			mergeNeeded, mergedStr))
	{
		delete address;

		return false;
	}
	if (_conflict(piAddress.getField(entryZip), address->getZip(),
			mergeNeeded, mergedStr))
	{
		delete address;

		return false;
	}
	if (_conflict(piAddress.getField(entryCountry), address->getCountry(),
			mergeNeeded, mergedStr))
	{
		delete address;

		return false;
	}

	delete address;

	return true;
}

ContactEntry *AbbrowserConduit::_findMatch(const QDict < ContactEntry >
	&entries, const PilotAddress & pilotAddress,
	QString & contactKey) const
{
	bool piFirstEmpty = (pilotAddress.getField(entryFirstname) == 0L);
	bool piLastEmpty = (pilotAddress.getField(entryLastname) == 0L);
	bool piCompanyEmpty = (pilotAddress.getField(entryCompany) == 0L);

	// return not found if not matching keys
	if (piFirstEmpty && piLastEmpty && piCompanyEmpty)
		return 0L;

	contactKey = QString::null;
	// for now just loop throug all entries; in future, probably better
	// to create a map for first and last name, then just do O(1) calls
	for (QDictIterator < ContactEntry > iter(entries); iter.current();
		++iter)
	{
		ContactEntry *abEntry = iter.current();

		// do quick empty check's
		if (piFirstEmpty != abEntry->getFirstName().isEmpty() ||
			piLastEmpty != abEntry->getLastName().isEmpty() ||
			piCompanyEmpty != abEntry->getCompany().isEmpty())
			continue;

		if (piFirstEmpty && piLastEmpty)
		{
			if (abEntry->getCompany() ==
				pilotAddress.getField(entryCompany))
			{
				contactKey = iter.currentKey();
				return abEntry;
			}
		}
		else
			// the first and last name must be equal; they are equal
			// if they are both empty or the strings match
		if (((piLastEmpty && abEntry->getLastName().isEmpty())
				|| (abEntry->getLastName() ==
					pilotAddress.
					getField(entryLastname)))
			&& ((piFirstEmpty
					&& abEntry->getFirstName().
					isEmpty())
				|| (abEntry->getFirstName() ==
					pilotAddress.
					getField(entryFirstname))))
		{
			contactKey = iter.currentKey();
			return abEntry;
		}

	}
	return 0L;
}

ContactEntry *AbbrowserConduit::_syncPilotEntry(PilotAddress & pilotAddress,
	const QDict < ContactEntry > &abbrowserContacts,
	QString * outAbKey, bool deleteIfNotFound)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname <<
		" trying to sync the existing pilotAddress to the kaddressbook entries"
		<< endl;
#endif

	QString abKey;

	// look for the possible match of names
	ContactEntry *abEntry =
		_findMatch(abbrowserContacts, pilotAddress, abKey);
	if (abEntry)
	{
		// if already found in kaddressbook and kpilot, just assign
		// the kpilot id and save
		if (_equal(pilotAddress, *abEntry))
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname <<
				" both records already exist and are equal, just assigning the KPILOT_ID to the kaddressbook entry"
				<< endl;
#endif
			abEntry->setCustomField("KPILOT_ID",
				QString::number(pilotAddress.getID()));
			_saveAbEntry(*abEntry, abKey);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname <<
				" both records exist (no id match) but conflict"
				<< endl;
#endif
			_handleConflict(&pilotAddress, abEntry, abKey);
		}
	}
	else			// if not found in the kaddressbook contacts, add it
	{
		bool addPalm = true;

		if (deleteIfNotFound)
		{
			if (!pilotAddress.isModified())
			{
				_removePilotAddress(pilotAddress);
				addPalm = false;
			}
			// else
			// two possible cases: modified on palm and
			// deleted in kaddressbook or just new in palm
			// assume new in palm and add it
		}
		if (addPalm)
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname <<
				" adding new pilot record to kaddressbook => "
				<< endl;
			showPilotAddress(pilotAddress);
#endif
			_addToAbbrowser(pilotAddress);
		}
	}
	if (outAbKey)
		*outAbKey = abKey;
	return abEntry;
}

bool AbbrowserConduit::_prepare(QDict < ContactEntry > &abbrowserContacts,
	QMap < recordid_t, QString > &idContactMap,
	QDict < ContactEntry > &newContacts, bool & abAlreadyRunning)
{
	FUNCTIONSETUP;

	readConfig();

	if (!fDCOP)
		fDCOP = KApplication::kApplication()->dcopClient();
	if (!fDCOP)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " unable to connect to dcop" << endl;
#endif
		return false;
	}
	abAlreadyRunning = _startAbbrowser();
	_setAppInfo();


	// get the contacts from kaddressbook
	if (!_getAbbrowserContacts(abbrowserContacts))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": unable to get contacts" << endl;
#endif
		return false;
	}

	// get the idContactMap and the newContacts
	// - the idContactMap maps Pilot unique record (address) id's to
	//   a Abbrowser ContactEntry; allows for easy lookup and comparisons
	// - created from the list of Abbrowser Contacts
	_mapContactsToPilot(abbrowserContacts, idContactMap, newContacts);

#ifdef DEBUG
	DEBUGCONDUIT << fname << " mapped pilot contacts, ready to sync" << endl;
#endif

	return true;
}

#if 0
void AbbrowserConduit::_backupDone()
{
	FUNCTIONSETUP;

	if (!fBackupDone)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " setting FIRST_TIME_SYNCING to false"
			<< endl;
#endif
		fBackupDone = true;

		KConfigGroupSaver g(fConfig,AbbrowserConduitFactory::group());
		fConfig->writeEntry(AbbrowserConduitFactory::firstTime(),
			!fBackupDone);
		fConfig->sync();
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname << " stop" << endl;
#endif
}
#endif


#if 0
void AbbrowserConduit::doBackup()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " start" << endl;
#endif

	QDict < ContactEntry > abbrowserContacts;
	QMap < recordid_t, QString > idContactMap;
	QDict < ContactEntry > newContacts;
	bool abAlreadyRunning;

	if (!_prepare(abbrowserContacts, idContactMap, newContacts,
			abAlreadyRunning))
		return;

	PilotRecord *record = 0L;

	// iterate through all records in palm pilot
	int recIndex = 0;

	//for (int catIndex = 0; catIndex < 16;catIndex++)
	//for (record = readNextRecordInCategory(_getCatId(catIndex));
	//record != NULL;
	//record = readNextRecordInCategory(_getCatId(catIndex)))
	for (record = readRecordByIndex(recIndex); record != NULL;
		record = readRecordByIndex(recIndex))
	{
		PilotAddress pilotAddress(fAddressAppInfo, record);
		QString abKey = QString::null;

		// if already stored in the kaddressbook
		if (idContactMap.contains(pilotAddress.id()))
		{
			abKey = idContactMap[pilotAddress.id()];
			ContactEntry *abEntry = abbrowserContacts[abKey];

			assert(abEntry != NULL);

			// if equal, do nothing since it is already there
			if (!_equal(pilotAddress, *abEntry))
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname << " id = " <<
					pilotAddress.
					id() <<
					" match but not equal; pilot = '" <<
					pilotAddress.
					getField(entryFirstname) << "' '" <<
					pilotAddress.
					getField(entryLastname) <<
					"' abEntry = '" << abEntry->
					findRef("fn").latin1() << "'" << endl;
#endif

				// if not equal, let the user choose what to do
				_handleConflict(&pilotAddress, abEntry,
					abKey);
			}
		}
		else
			_syncPilotEntry(pilotAddress, abbrowserContacts);
		recIndex++;
	}

	_saveAbChanges();
	_stopAbbrowser(abAlreadyRunning);
	_backupDone();
}
#endif

int AbbrowserConduit::_getCatId(int catIndex) const
{
	return fAddressAppInfo.category.ID[catIndex];
}

void AbbrowserConduit::_removeFromSync(const QString & key,
	QDict < ContactEntry > &newContacts,
	QMap < recordid_t, QString > &idContactMap) const
{
	bool inMap = !newContacts.remove(key);

	if (inMap)
	{
		bool found = false;
		recordid_t foundId;

		for (QMap < recordid_t, QString >::Iterator mapIter =
			idContactMap.begin();
			!found && mapIter != idContactMap.end(); ++mapIter)
		{
			if (mapIter.data() == key)
			{
				found = true;
				foundId = mapIter.key();
			}
		}
		if (found)
			idContactMap.remove(foundId);
	}
}


/* virtual */ void AbbrowserConduit::exec()
{
	FUNCTIONSETUP;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo
			<< ": No config file was set!"
			<< endl;
		return;
	}

	QDict < ContactEntry > abbrowserContacts;
	QMap < recordid_t, QString > idContactMap;
	QDict < ContactEntry > newContacts;
	bool abAlreadyRunning;


	if (!_prepare(abbrowserContacts, 
		idContactMap, 
		newContacts,
		abAlreadyRunning))
	{
		kdWarning() << k_funcinfo
			<< ": Can't prepare the kaddressbook for syncing."
			<< endl;
		return;
	}

	fDatabase = new PilotSerialDatabase(pilotSocket(),
		"AddressDB",
		this,
		"AddressDB");

	// perform syncing from palm to abbrowser
	PilotRecord *record = 0L;

	// iterate through all records in palm pilot
	int recIndex = 0;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< " about to readRecordByIndex "
		<< recIndex
		<< endl;
#endif

	for (record = fDatabase->readRecordByIndex(recIndex);
		record != NULL ;
		record = fDatabase->readRecordByIndex(recIndex))
	{
		PilotAddress pilotAddress(fAddressAppInfo, record);
		QString abKey = QString::null;

		//   if record not in abbrowser
		if (!idContactMap.contains(pilotAddress.id()))
		{
			// if pilotAddress id == 0, then probably syncing locally
			// and the pilotAddress has already been added
			if (pilotAddress.id() != 0)
			{
				// assume that it was deleted from the abbrowser since a
				// backup should have been done before the first sync;
				//
				// was = fBackupDone
				bool deleteIfNotFound = false;
				QString abKey;
				ContactEntry *syncedContact =
					_syncPilotEntry(pilotAddress,
					abbrowserContacts,
					&abKey, deleteIfNotFound);

				// if a match was found, remove it from the
				// the newContacts or idContactMap structures
				if (syncedContact && abKey != QString::null)
				{
#ifdef DEBUG
					DEBUGCONDUIT << fname
						<<
						" Merged address to unmapped contact "
						<< syncedContact->
						getFullName() << endl;
#endif

					_removeFromSync(abKey, newContacts,
						idContactMap);
				}
			}
		}
		else
		{
			// get the key and the associated contact
			abKey = idContactMap[pilotAddress.id()];
			ContactEntry *abEntry = abbrowserContacts[abKey];

			assert(abEntry != NULL);

			// the record exists in the abbrowser and the palm
			if (pilotAddress.isModified()
				&& abEntry->isModified())
			{
				// query the user what to do...
				_handleConflict(&pilotAddress, abEntry,
					abKey);
			}
			else	// record is either modified in the abbrowser or the palm
				// or not modified at all
			if (pilotAddress.isModified())
			{
				if (pilotAddress.isDeleted())
					_removeAbEntry(abKey);
				else
				{
					// update abbrowser
					_copy(*abEntry, pilotAddress);
					_saveAbEntry(*abEntry, abKey);
				}
			}
			else if (abEntry->isModified())
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname <<
					"abEntry is modified but pilot wasn't; abEntry => "
					<< endl;
				showContactEntry(*abEntry);
#endif
				// update pilot
				_copy(pilotAddress, *abEntry);
				_savePilotAddress(pilotAddress, *abEntry);
				_saveAbEntry(*abEntry, abKey);
			}
			// else not modified at either end, leave alone

			// remove the id from the map, that way, we can see
			// if any address that has a valid id on the kab db
			// but wasn't found in the pilot db
			idContactMap.remove(pilotAddress.id());
		}
		delete record;

		record = NULL;
		recIndex++;	// increment loop
	}			// end pilot record loop

	// add all new entries from abbrowser to the palm pilot
	for (QDictIterator < ContactEntry > newAbIter(newContacts);
		newAbIter.current(); ++newAbIter)
		_addToPalm(newAbIter.currentKey(), *newAbIter.current());

	// there could be kab contacts that have an old pilot id (other palm)
	// but should be synced with this new pilot
	for (QMap < recordid_t, QString >::Iterator oldKabIter =
		idContactMap.begin(); oldKabIter != idContactMap.end();
		++oldKabIter)
	{
		QString abKey = idContactMap[oldKabIter.key()];
		ContactEntry *abEntry = abbrowserContacts[abKey];

		assert(abKey != NULL);
#ifdef DEBUG
		DEBUGCONDUIT << fname <<
			" adding kab contact that has an old pilot id " <<
			endl;
		showContactEntry(*abEntry);
#endif
		_addToPalm(abKey, *abEntry);
	}

	_saveAbChanges();
	_stopAbbrowser(abAlreadyRunning);

	fDatabase->resetSyncFlags();
	KPILOT_DELETE(fDatabase);
	emit syncDone(this);
}

void AbbrowserConduit::doTest()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " start" << endl;

	QDict < ContactEntry > abbrowserContacts;
	QMap < recordid_t, QString > idContactMap;
	QDict < ContactEntry > newContacts;
	bool abAlreadyRunning;

	if (!_prepare(abbrowserContacts, idContactMap, newContacts,
			abAlreadyRunning))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " Test failed" << endl;
#endif
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " Test passed!" << endl;
#endif
		_stopAbbrowser(abAlreadyRunning);
	}
#endif
}

const char *AbbrowserConduit::_getKabFieldForOther(const QString & desc) const
{
	if (desc == "Assistant")
		return "X-AssistantsPhone";
	if (desc == "Other Phone")
		return "X-OtherPhone";
	if (desc == "Business Phone 2")
		return "X-BusinessPhone2";
	if (desc == "Business Fax")
		return "X-BusinessFax";
	if (desc == "Car Phone")
		return "X-CarPhone";
	if (desc == "Email 2")
		return "X-E-mail2";
	if (desc == "Home Fax")
		return "X-HomeFax";
	if (desc == "Home Phone 2")
		return "X-HomePhone2";
	if (desc == "Telex")
		return "X-Telex";
	if (desc == "TTY/TDD Phone")
		return "X-TtyTddPhone";
	return "X-OtherPhone";
}

void AbbrowserConduit::readConfig()
{
	FUNCTIONSETUP;

	KConfigGroupSaver g(fConfig,AbbrowserConduitFactory::group());

	fSmartMerge = fConfig->readBoolEntry(
		AbbrowserConduitFactory::smartMerge(),true);
	fConflictResolution = (EConflictResolution) 
		fConfig->readNumEntry(
			AbbrowserConduitFactory::conflictResolution(),
			eDoNotResolve);

	QString other = fConfig->readEntry(
		AbbrowserConduitFactory::mapOther(),"Other Phone");
	fPilotOtherMap = _getKabFieldForOther(other);

	QString prefsStr = fConfig->readEntry(
		AbbrowserConduitFactory::streetType(),"Home Street");

	fPilotStreetHome = true;
	prefsStr = prefsStr.left(prefsStr.find(' '));
	if (prefsStr != "Home")
	{
		fPilotStreetHome = false;
	}

	prefsStr = fConfig->readEntry(
		AbbrowserConduitFactory::faxType(),"Home Fax");
	fPilotFaxHome = true;
	prefsStr = prefsStr.left(prefsStr.find(' '));
	if (prefsStr != "Home")
	{
		fPilotFaxHome = false;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Settings "
		<< "fSmartMerge=" << fSmartMerge
		<< " fConflictResolution=" << fConflictResolution
		<< " fPilotOtherMap=" << fPilotOtherMap
		<< " fPilotStreetHome=" << fPilotStreetHome
		<< " fPilotFaxHome=" << fPilotFaxHome << endl;
#endif
}
