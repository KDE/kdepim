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
#include <kabc/address.h>

#include <pilotAddress.h>
#include <plugin.h>
#include <kcrash.h>


#define SYNCNONE 0
#define SYNCMOD 1
#define SYNCDEL 3

using namespace KABC;
class ResolutionTable;

typedef QValueList<recordid_t> RecordIDList;

class AbbrowserConduit : public ConduitAction
{
Q_OBJECT
public:
	AbbrowserConduit(KPilotDeviceLink *o,const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~AbbrowserConduit();

/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/
	virtual bool exec();
protected slots:
	void slotPalmRecToPC();
	void slotPCRecToPalm();
	void slotDeletedRecord();
	void slotDeleteUnsyncedPCRecords();
	void slotDeleteUnsyncedHHRecords();
	void slotCleanup();

private:

	/********************************************************/
	/* Handle the configuration                             */
	/********************************************************/

	/* Read the global KPilot config file for settings
	 * particular to the AbbrowserConduit conduit. */
	void readConfig();
	/**
	*  @return the Abbrowser Contact field to map the pilot "other" phone
	*  field to (such as BusinessFax, etc)
	*/
	static bool isPilotStreetHome()  { return fPilotStreetHome; };
	static bool isPilotFaxHome()  { return fPilotFaxHome; };
	static bool isDeleted(const PilotAddress*addr);
	static bool isArchived(const PilotAddress*addr);
	static bool isArchived(const Addressee &addr);
	static bool makeArchived(Addressee &addr);



	/********************************************************/
	/* Loading and saving the addressbook and database      */
	/********************************************************/


	/* Given a list of contacts, creates the pilot id to contact key map
	 * and a list of new contacts in O(n) time (single pass) */
	void _mapContactsToPilot( QMap < recordid_t, QString> &idContactMap) const;
	/* Do the preperations before doSync or doBackup.
	 * Load contacts, set the pilot */
	bool _prepare();
	/* Load the contacts from the addressbook.
	 * @return true if successful, false if not */
	bool _loadAddressBook();
	/* Save the contacts back to the addressbook.
	 * @return true if successful, false if not */
	bool _saveAddressBook();
	void _getAppInfo();
	void _setAppInfo();



	/********************************************************/
	/* Handle special fields of the Addressees              */
	/********************************************************/
	static QString getCustomField(const Addressee &abEntry, const int index);
	static void setCustomField(Addressee &abEntry, int index, QString cust);
	static QString getOtherField(const Addressee&abEntry);
	static void setOtherField(Addressee&abEntry, QString nr);
	static PhoneNumber getFax(const Addressee &abEntry);
	static void setFax(Addressee &abEntry, QString fax);
	static KABC::Address getAddress(const Addressee &abEntry);
	QString _getCatForHH(const QStringList cats, const QString curr) const;
	void _setCategory(Addressee &abEntry, QString cat);



/*********************************************************************
                     D E B U G   O U T P U T
 *********************************************************************/
#ifdef DEBUG
	/**
	* Output to console, for debugging only
	*/
	static void showAddressee(const Addressee &abAddress);
	/**
	* Output to console, for debugging only
	*/
	static void showPilotAddress(PilotAddress *pilotAddress);
#endif
	void showAdresses(Addressee &pcAddr, PilotAddress *backupAddr,
		PilotAddress *palmAddr);



/*********************************************************************
              G E N E R A L   S Y N C   F U N C T I O N
         These functions modify the Handheld and the addressbook
 *********************************************************************/
	bool syncAddressee(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr);
	bool _copyToHH(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr);
	bool _copyToPC(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr);
	bool _writeBackup(PilotAddress *backup);
	bool _deleteAddressee(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr);


/*********************************************************************
                 l o w - l e v e l   f u n c t i o n s   f o r
                   adding / removing palm/pc records
 *********************************************************************/
	bool _savePalmAddr(PilotAddress *palmAddr, Addressee &pcAddr);
	bool _savePCAddr(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr);


/*********************************************************************
                   C O P Y   R E C O R D S
 *********************************************************************/
	int _compare(const QString & str1, const QString & str2) const;
	typedef enum eqFlagsType
	{
		eqFlagsName=0x1,
		eqFlagsAdress=0x2,
		eqFlagsPhones=0x4,
		eqFlagsNote=0x8,
		eqFlagsCategory=0x10,
		eqFlagsFlags=0x20,
		eqFlagsCustom=0x40,
		eqFlagsAll=0xFFFF,
		eqFlagsAlmostAll=eqFlagsName|eqFlagsAdress|eqFlagsPhones|eqFlagsNote|eqFlagsCustom
	};
	bool _equal(const PilotAddress *piAddress, const Addressee &abEntry,
		enum eqFlagsType flags=eqFlagsAll) const;
	void _copy(PilotAddress *toPilotAddr, Addressee &fromAbEntry);
	void _setPilotAddress(PilotAddress *toPilotAddr, const KABC::Address & abAddress);
	void _copyPhone(Addressee &toAbEntry, PhoneNumber phone, QString palmphone);
	void _copy(Addressee &toAbEntry, PilotAddress *fromPiAddr);

/*********************************************************************
 C O N F L I C T   R E S O L U T I O N   a n d   M E R G I N G
 *********************************************************************/
	/** smartly merge the given field for the given entry. use the
	 *  backup record to determine which record has been modified
	 *  @pc, @backup, @palm ... entries of the according databases
	 *  @returns string of the merged entries.
	 */
	QString _smartMergeString(const QString &pc, const QString & backup,
		const QString & palm, eConflictResolution confRes);
	bool _buildResolutionTable(ResolutionTable*tab, const Addressee &pcAddr,
		PilotAddress *backupAddr, PilotAddress *palmAddr);
	bool _applyResolutionTable(ResolutionTable*tab, Addressee &pcAddr,
		PilotAddress *backupAddr, PilotAddress *palmAddr);
	bool _smartMergeTable(ResolutionTable*tab);
	/** Merge the palm and the pc entries with the additional
	 *  information of the backup record. Calls _smartMerge
	 *  which does the actual syncing of the data structures.
	 *  According to the return value of _smartMerge, this function
	 *  writes the data back to the palm/pc.
	 *  return value: no meaning yet
	 */
	bool _smartMergeAddressee(Addressee &pcAddr, PilotAddress *backupAddr,
		PilotAddress *palmAddr);
	Addressee _findMatch(const PilotAddress & pilotAddress) const;


/********************************************************/
/*   D A T A   M E M B E R S ,   S E T T I N G S        */
/********************************************************/

	struct AddressAppInfo fAddressAppInfo;

/*	eConflictResolution fEntryResolution;*/
	static bool fPilotStreetHome, fPilotFaxHome;
	static bool fArchive;
	static enum ePilotOtherEnum
	{
		eOtherPhone,
		eAssistant,
		eBusinessFax,
		eCarPhone,
		eEmail2,
		eHomeFax,
		eTelex,
		eTTYTTDPhone
	} ePilotOther;
	int pilotindex;
	bool abChanged;
	static const QString appString;
	static const QString flagString;
	static const QString idString;
	/** addresseeMap maps record ids to IDs of Addressees. This is used to speed up searching the local addressbook */
	QMap < recordid_t, QString> addresseeMap;
	RecordIDList syncedIds, allIds;
	static AddressBook* aBook;
	AddressBook::Iterator abiter;
	// for a local file, we need to obtain a saveTicket when opening the abook
	Ticket*ticket;

	QString fAbookFile;
	enum eAbookTypeEnum {
		eAbookResource=0,
		eAbookLocal
	} fAbookType;

	static enum eCustomEnum {
		eCustomField,
		eCustomBirthdate,
		eCustomURL,
		eCustomIM
	} eCustom[4];
	static QString fCustomFmt;

	static void cleanupOnCrash(int);
} ;

#endif
