#ifndef _ABBROWSER_CONDUIT_H
#define _ABBROWSER_CONDUIT_H
/* abbrowser-conduit.h                           KPilot
**
** Copyright (C) 2000,2001 by Dan Pilone
** Copyright (C) 2000 Gregory Stern
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <qmap.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include <pilotAddress.h>
#include <plugin.h>



#define SYNCNONE 0
#define SYNCMOD 1
#define SYNCDEL 3

#define SYNC_FULL 1
#define SYNC_FAST 0
#define SYNC_FIRST 2

#define CHANGED_NONE 0
#define CHANGED_PC 1
#define CHANGED_PALM 2
#define CHANGED_BOTH CHANGED_PC|CHANGED_PALM
#define CHANGED_ADD  0x100
#define CHANGED_NORES 0x200
#define CHANGED_DUPLICATE CHANGED_ADD|CHANGED_NORES|CHANGED_BOTH


using namespace KABC;

class AbbrowserConduit : public ConduitAction
{
Q_OBJECT
public:
	AbbrowserConduit(
		KPilotDeviceLink *o,
		const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~AbbrowserConduit();
	virtual bool exec();

protected slots:
	void syncDeletedRecord();
	void syncPCRecToPalm();
	void syncPalmRecToPC();
	void cleanup();

private:
	enum EConflictResolution {
		eUserChoose=0,
		eDoNotResolve,
		ePilotOverides,
		eAbbrowserOverides,
		eRevertToBackup,
		eKeepBothInAbbrowser
	};
	EConflictResolution getResolveConflictOption() const { return fConflictResolution; }
	bool doSmartMerge() const { return fSmartMerge; }
	
	EConflictResolution getEntryResolution(const KABC::Addressee & abEntry, const PilotAddress &backupAddress, const PilotAddress & pilotAddress);
	EConflictResolution getFieldResolution(const QString &entry, const QString &field, const QString &palm, const QString &backup, const QString &pc);
	EConflictResolution ResolutionDialog(QString Title, 
		QString Text, 
		QStringList &lst, 
		QString remember=QString::null, bool*rem=0L) const;

	int _conflict(const QString &entry, const QString &field, const QString &pc, const QString &backup, 
			const QString &palm, bool & mergeNeeded, QString & mergedStr);
	int _compare(const QString &str1, const QString &str2) const;

	/**
	*  @return the Abbrowser Contact field to map the pilot "other" phone
	*  field to (such as BusinessFax, etc)
	*/
	static bool isPilotStreetHome()  { return fPilotStreetHome; }
	static bool isPilotFaxHome()  { return fPilotFaxHome; }



	/**
	*  Do the preperations before doSync or doBackup.
	*  Load contacts, set the pilot 
	*/
	bool _prepare();

	/**
	* Read the global KPilot config file for settings
	* particular to the AbbrowserConduit conduit.
	*/
	void readConfig();

	/**
	*  Load the contacts from the addressbook.
	*  @return true if successful, false if not
	*/
	bool _loadAddressBook();
	/**
	*  Save the contacts back to the addressbook.
	*  @return true if successful, false if not
	*/
	bool _saveAddressBook();
	
	static QString getOtherField(const KABC::Addressee&abEntry);
	static void setOtherField(KABC::Addressee&abEntry, QString nr);
	static QString getCustomField(const KABC::Addressee &abEntry, const int index);
	static void setCustomField(KABC::Addressee &abEntry,  int index, QString cust);
	static KABC::PhoneNumber getFax(const KABC::Addressee &abEntry);
	static KABC::Address getAddress(const KABC::Addressee &abEntry);
	

	void _setAppInfo();
	KABC::Addressee _addToAbbrowser(const PilotAddress & address);
	int _mergeEntries(PilotAddress &pilotAddress, PilotAddress &backupAddress, KABC::Addressee &abEntry);
	int _handleConflict(PilotAddress &piAddress, PilotAddress &backup, KABC::Addressee &abEntry);
	int _smartMerge(PilotAddress & outPilotAddress, const PilotAddress & backupAddress, KABC::Addressee & outAbEntry);
	int _smartMergePhone(KABC::Addressee &abEntry, const PilotAddress &backupAddress, 
		PilotAddress &pilotAddress, PilotAddress::EPhoneType PalmFlag, KABC::PhoneNumber phone, QString
		 thisName, QString name);
	int _smartMergeEntry(QString abEntry, const PilotAddress &backupAddress, PilotAddress &pilotAddress, int PalmFlag, QString thisName, QString name, QString &mergedString);
	int _smartMergeCategories(KABC::Addressee &abAddress, const PilotAddress &backupAddress, PilotAddress &pilotAddress, QString thisName, QString name, QString &mergedString);

	void _removePilotAddress(PilotAddress &address);
	void _removeAbEntry(KABC::Addressee addressee);
	KABC::Addressee _saveAbEntry(KABC::Addressee &abEntry);
	void _checkDelete(PilotRecord *r, PilotRecord *s);

   /** 
	*  @return true if the abbEntry's pilot id was changed 
	*/
	bool _savePilotAddress(PilotAddress &address, KABC::Addressee &abEntry);
	bool _saveBackupAddress(PilotAddress & backup);
	
	void _copyPhone(KABC::Addressee &toAbEntry, KABC::PhoneNumber phone, QString palmphone);
	void _copy(PilotAddress &toPilotAddr, KABC::Addressee &fromAbEntry);
	void _copy(KABC::Addressee &toAbEntry, const PilotAddress &fromPilotAddr);
	void _setPilotAddress(PilotAddress &toPilotAddr, const KABC::Address &abAddress);
	bool _equal(const PilotAddress &piAddress, KABC::Addressee &abEntry) const;
	KABC::Addressee _findMatch(const PilotAddress & pilotAddress) const;
	int _getCat(const QStringList cats) const ;
	void _setCategory(KABC::Addressee &abEntry, QString cat);

   /** 
	*  Given a list of contacts, creates the pilot id to contact key map
	*  and a list of new contacts in O(n) time (single pass)
	*/
   void _mapContactsToPilot( QMap < recordid_t, QString> &idContactMap) const;

#ifdef DEBUG
	/** 
	* Output to console, for debugging only 
	*/
	static void showAddressee(const KABC::Addressee &abAddress);
	/** 
	* Output to console, for debugging only 
	*/
	static void showPilotAddress(const PilotAddress &pilotAddress);
#endif

	KABC::Addressee _addToPC(PilotRecord *r);
	KABC::Addressee _changeOnPC(PilotRecord*rec, PilotRecord*backup);
	bool _deleteOnPC(PilotRecord*rec,PilotRecord*backup);

	void _addToPalm(KABC::Addressee &entry);
	void _changeOnPalm(PilotRecord *rec, PilotRecord* backuprec, KABC::Addressee &ad);
	void _deleteFromPalm(PilotRecord*rec);

	struct AddressAppInfo fAddressAppInfo;

	bool fSmartMerge;
	EConflictResolution fConflictResolution, fEntryResolution;
	static bool fPilotStreetHome, fPilotFaxHome;
	bool fFullSync, fFirstTime, fArchive;
   static enum  ePilotOtherEnum
	{
		eOtherPhone,
		eAssistant,
//		eBusiness2,
		eBusinessFax,
		eCarPhone,
		eEmail2,
		eHomeFax,
//		eHomePhone2,
		eTelex,
		eTTYTTDPhone
	} ePilotOther;
	int syncAction; 
	int pilotindex;
	bool abChanged;
	static const QString appString;
	static const QString flagString;
	static const QString idString;
	QMap < recordid_t, QString> addresseeMap;
	QValueList <recordid_t> syncedIds;
	KABC::AddressBook* aBook;
	KABC::AddressBook::Iterator abiter;
	
	static enum eCustomEnum {
		eCustomField,
		eCustomBirthdate,
		eCustomURL,
		eCustomIM
	} eCustom[4];
	
	
	
	
	void showAdresses(PilotAddress & pilotAddress, const PilotAddress & backupAddress, KABC::Addressee & abEntry);
} ;

#endif
