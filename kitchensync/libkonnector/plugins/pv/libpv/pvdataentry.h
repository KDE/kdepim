/***************************************************************************
                          pvdataentry.h  -  Declaration of the abstract
                                            class PVDataEntry. This class
                                            is being implemented by all
                                            data classes (e.g. Contacts, Memos etc.)

                             -------------------
    begin                : Sat Mar 16 2002
    copyright            : (C) 2002 by Selzer Michael, Thomas Bonk
    email                : selzer@student.uni-kl.de, thomas@ghecko.saar.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PVDATAENTRY_H
#define PVDATAENTRY_H

// C++ includes
#include <map>
#include <string>
#include <ostream>
// project includes
#include "datapacket.h"

using namespace std;

/**
  * This class is the abstract base class for all data classes
  * (e.g. Contacts, Memos etc.).
  * @author Selzer Michael, Thomas Bonk, Maurus Erni
  */

namespace CasioPV {

class PVDataEntry
{
  public:
    /**
       * Getter for the mode code.
       * @return The mode code of the data entry.
       */
    virtual unsigned int getModeCode() const = 0;

    /**
       * Setter for the uid.
       * @param uid The uid of the data entry.
       */
    virtual void setUid(unsigned int uid) = 0;

    /**
       * Getter for the uid.
       * @return The uid of the data entry.
       */
    virtual unsigned int getUid() const = 0;

    /**
       * Setter for the state of an entry.
       * @param state The state of the entry
       */
    virtual void setState(unsigned int state) = 0;

    /**
       * Getter for the state of an entry.
       * @return The state of the entry
       */
    virtual unsigned int getState() = 0;

    /**
       * Getter for the data.
       * @return Return all of the data.
       */
    virtual const map<unsigned int, string>& getData() const = 0;

    /**
       * Setter for the data of a field.
       * @param packet data packet for the field
       * @exception PVDataEntryException
       */
    virtual void setFieldData( datapacket& packet ) = 0;

    /**
       * Checks if a dataentry is sendable.
       * @return bool true if all necessary fields are filled else false.
       */
    virtual bool isSendable() = 0;

    /**
       * This method returns if the data entry is a secret area entry.
       * @return bool true if secret else false
       */
    virtual bool isSecret() = 0;

    /**
       * Convert the Entry to an XML string
       * @return string The data entry as an XML string
       */
    virtual string toXML() = 0;

    /**
       * Convert the XML string to an Entry
       * @param strXML The XML string to be converted
       */
    virtual void fromXML(string strXML) = 0;

    /** Possible states of the entries. The states are used in two ways:
      * 1. Getting the modified entries from the PV. The state of the entry is
      *    set depending what was happening with this entry since the last sync.
      * 2. Receiving the synchronized entries from KitchenSync. Depending on the
      *    state, the synchronized entry will be added / modified / removed on
      *    the PV.
      */
    enum State {
      // The state of the data entry is undefined -> state is set to UNDEFINED in constructor
      UNDEFINED,
      // The data entry was 1. added since the last synchronization
      //                    2. added on desktop -> has to be added on PV
      ADDED,
      // The data entry was 1. modified since the last synchronization
      //                    2. modified on desktop -> has to be modified on PV
      MODIFIED,
      // The data entry was 1. removed since the last synchronization
      //                    2. removed on desktop -> has to be removed on PV
      REMOVED
    };
  };
}; // namespace CasioPV

#endif
