// knotes-conduit.h
//
// Copyright (C) 2000 Gregory Stern
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$


#ifndef _ABBROWSER_CONDUIT_H
#define _ABBROWSER_CONDUIT_H

#include "baseConduit.h"

#include <qmap.h>
#include <qlist.h>
#include <contactentry.h>
#include <pilotAddress.h>

class DCOPClient;
class AbbrowserConduit : public BaseConduit
    {
    public:
      AbbrowserConduit(BaseConduit::eConduitMode mode);
      AbbrowserConduit(BaseConduit::eConduitMode mode,
		       BaseConduit::DatabaseSource dbSource);
      virtual ~AbbrowserConduit();
      
      virtual void doSync();
      virtual void doBackup();
      virtual QWidget* aboutAndSetup();
      
      virtual const char* dbInfo();
      virtual void doTest();

      enum EConflictResolution { eUserChoose=0, eKeepBothInAbbrowser,
				 ePilotOverides, eAbbrowserOverides,
				 eDoNotResolve };
      EConflictResolution getResolveConflictOption() const
		{ return fConflictResolution; }
      bool doSmartMerge() const { return fSmartMerge; }
      /** @return the Abbrowser Contact field to map the pilot "other" phone
       *  field to (such as BusinessFax, etc)
       */
      const QString &getPilotOtherMap() const { return fPilotOtherMap; }
      bool isPilotStreetHome() const { return fPilotStreetHome; }
      bool isPilotFaxHome() const { return fPilotFaxHome; }
      /** @return flag if should close abbrowser if it was already running
       *  upon exit
       */
      bool closeAbbrowserIfOpen() const { return fCloseAbIfOpen; }
      bool backupDone() const { return fBackupDone; }
      
    private:
      /** Do the preperations before doSync or doBackup.
       *  Start abbrowser, set the pilot app info, assign the fDcop variable,
       *  and get the contacts from abbrowser over dcop */
      bool _prepare(QDict<ContactEntry> &abbrowserContacts,
		    QMap<recordid_t, QString> &idContactMap,
		    QList<ContactEntry> &newContacts,
		    bool &abAlreadyRunning);
      /**
       * Read the global KPilot config file for settings
       * particular to the AbbrowserConduit conduit.
       */
      void readConfig();
      /** Start the Abbrowser application; if can't start exit's application
       @return true if already running, false if not
       */
      bool _startAbbrowser();
      void _stopAbbrowser(bool abAlreadyRunning);
      void _saveAbChanges();
      void _setAppInfo();
      void _addToAbbrowser(const PilotAddress &address);
      void _addToPalm(ContactEntry &entry);
      void _handleConflict(PilotAddress *piAddress, ContactEntry *abEntry,
			   const QString &abKey);
      void _removePilotAddress(PilotAddress &address);
      void _removeAbEntry(const QString &key);
      void _saveAbEntry(ContactEntry &abEntry, const QString &key);
      /** @return true if the abbEntry's pilot id was changed */
      bool _savePilotAddress(PilotAddress &address, ContactEntry &abEntry);
      bool _getAbbrowserContacts(QDict<ContactEntry> &contacts);
      void _copy(PilotAddress &toPilotAddr, ContactEntry &fromAbEntry);
      void _copy(ContactEntry &toAbEntry, const PilotAddress &fromPilotAddr);
      void _setPilotAddress(PilotAddress &toPilotAddr,
			    const ContactEntry::Address &abAddress);
      bool _equal(const PilotAddress &piAddress,
		  ContactEntry &abEntry) const;
      ContactEntry *_findMatch(const QDict<ContactEntry> &entries,
			       const PilotAddress &pilotAddress,
			       QString &contactKey) const;
      /** Given a list of contacts, creates the pilot id to contact key map
       *  and a list of new contacts in O(n) time (single pass)
       */
      void _mapContactsToPilot(const QDict<ContactEntry> &contacts,
			       QMap<recordid_t, QString> &idContactMap,
			       QList<ContactEntry> &newContacts) const;

      /** Output to console, for debugging only */
      static void showContactEntry(const ContactEntry &abAddress);
      /** Output to console, for debugging only */
      static void showPilotAddress(const PilotAddress &pilotAddress);
      bool _conflict(const QString &str1, const QString &str2,
		     bool &mergeNeeded, QString &mergedStr) const;
      ContactEntry *_syncPilotEntry(PilotAddress &pilotAddress,
		  const QDict<ContactEntry> &abbrowserContacts);
      bool _smartMerge(PilotAddress &pilotAddress, ContactEntry &abEntry);
      void _backupDone();
      const char *_getKabFieldForOther(const QString &desc) const;
      int _getCatId(int catIndex) const;
      
      DCOPClient *fDcop;
      struct AddressAppInfo fAddressAppInfo;

      bool fSmartMerge;
      EConflictResolution fConflictResolution;
      QString fPilotOtherMap;
      bool fPilotStreetHome;
      bool fPilotFaxHome;
      bool fCloseAbIfOpen;
      bool fBackupDone;
    };
// Revision 1.0  2000/12/27 00:22:28  new conduit
// New AbbrowserConduit
//
#endif
