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
      virtual ~AbbrowserConduit();
      
      virtual void doSync();
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
      void _addToAbbrowser(const PilotAddress &address);
      void _addToPalm(ContactEntry &entry);
      void _handleConflict(PilotAddress *piAddress, ContactEntry *abEntry);
      void _removePilotAddress(PilotAddress &address);
      void _removeAbEntry(ContactEntry &abEntry);
      void _saveAbEntry(ContactEntry &abEntry);
      void _savePilotAddress(PilotAddress &address, ContactEntry &abEntry);
      bool _getAbbrowserContacts(QDict<ContactEntry> &contacts);
      void _copy(PilotAddress &toPilotAddr, const ContactEntry &fromAbEntry);
      void _copy(ContactEntry &toAbEntry, const PilotAddress &fromPilotAddr);
      void _setPilotAddress(PilotAddress &toPilotAddr,
			    const ContactEntry::Address &abAddress);
      
      /** Given a list of contacts, creates the pilot id to contact map
       *  and a list of new contacts in O(n) time
       */
      void _mapContactsToPilot(const QDict<ContactEntry> &contacts,
			       QMap<recordid_t, ContactEntry *> &idContactMap,
			       QList<ContactEntry> &newContacts) const;

      /** Output to console, for debugging only */
      static void showContactEntry(const ContactEntry &abAddress);
      /** Output to console, for debugging only */
      static void showPilotAddress(const PilotAddress &pilotAddress);
      
      
      DCOPClient *fDcop;
      struct AddressAppInfo fAddressAppInfo;
      
    };
// Revision 1.0  2000/12/27 00:22:28  new conduit
// New AbbrowserConduit
//
#endif
