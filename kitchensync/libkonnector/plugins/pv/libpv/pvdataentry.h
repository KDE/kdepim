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
#include <ostream.h>
// project includes
#include <datapacket.h>


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
       * Getter for the uid.
       * @return The uid of the data entry.
       */
    virtual unsigned int getUid() const = 0;

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
       * @return bool true if all nessecary fields are filled else false.
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

};

}; // namespace CasioPV

#endif
