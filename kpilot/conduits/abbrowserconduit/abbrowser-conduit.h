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
      
      virtual const char* dbInfo() ; // { return NULL; }
      virtual void doTest();

    private:
      /**
       * Read the global KPilot config file for settings
       * particular to the AbbrowserConduit conduit.
       */
      void readConfig();
      /** Start the Abbrowser application */
      void _startAbbrowser();
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
      ContactEntry *_findMatch(const QDict<ContactEntry> entries,
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
      bool _smartMerge(PilotAddress &pilotAddress, ContactEntry &abEntry);
      
      
      DCOPClient *fDcop;
      struct AddressAppInfo fAddressAppInfo;
      
    };
// Revision 1.0  2000/12/27 00:22:28  new conduit
// New AbbrowserConduit
//
#endif
