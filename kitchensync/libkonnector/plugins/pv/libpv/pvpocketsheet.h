/***************************************************************************
                          pvpocketsheet.h  -  description
                             -------------------
    begin                : Don Jul 11 2002
    copyright            : (C) 2002 by Selzer Michael
    email                : selzer@student.uni-kl.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PVPOCKETSHEET_H
#define PVPOCKETSHEET_H

// project includes
#include <pvdataentry.h>

/**
  *@author Selzer Michael, Maurus Erni
  */

namespace CasioPV {

class PVPocketSheet : public PVDataEntry  {
public:
    /**
     * Constructor.
     */
    PVPocketSheet(unsigned int uid);

    /**
     * Destructor.
     */
    virtual ~PVPocketSheet();

    /**
       * Getter for the mode code.
       * @return The mode code of the data entry.
       */
    virtual unsigned int   getModeCode() const;

    /**
       * Setter for the uid.
       * @param uid The uid of the data entry.
       */
    virtual void setUid(unsigned int uid);
    
    /**
       * Getter for the uid.
       * @return The uid of the data entry.
       */
    virtual unsigned int getUid() const;

    /**
       * Setter for the state of an entry.
       * @param state The state of the entry
       */
    virtual void setState(unsigned int state);
    
    /**
       * Getter for the state of an entry.
       * @return The state of the entry
       */
    virtual unsigned int getState();          

    /**
       * Getter for the data.
       * @return Return all of the data.
       */
    virtual const map<unsigned int, string>& getData() const;

    virtual string getSheetData();
    virtual string getXLineData();
    virtual string getYLineData();
    virtual string getCellData();

    virtual void setSheetData( string& value );
    virtual void setXLineData( string& value );
    virtual void setYLineData( string& value );
    virtual void setCellData( string& value );

    /**
       * Setter for the data of a field.
       * @param packet data packet for the field
       * @exception PVDataEntryException
       */
    virtual void setFieldData( datapacket& packet );

    /**
       * Checks if this instance of  pocketsheet is sendable.
       * The fields xxxxxxxxxxxxxx have to be set.
       * @return bool true if all nessecary fields are filled else false.
       */
    virtual bool isSendable();

    /**
       * This method returns if the data entry is a secret area entry
       * @return bool true if secret else false
       */
    virtual bool isSecret();

    /**
       * stream the content
       */
    friend ostream& operator<< (ostream& out, PVPocketSheet& pocketsheet);

    /**
       * Convert the Entry to an XML string
       * @return string The entry as XML string
       */
    string toXML();

    /**
       * Convert the XML string to an Entry
       * @param strXML The XML string to be converted
       */
    void fromXML(string strXML);

private:
    /**
       * map for handle the DataElements
       */
    map<unsigned int, string> m_data;

    /**
       * unsigned int The uid of the entry
       */
    unsigned int m_uid;
    
    /**
       * unsigned int The state of the entry
       */
    unsigned int m_state;

    /**
       * bool for isSecret()
       */
    bool m_isSecret;

    /**
       * This method changes the return code 0x0D as it is received from the PV to 0x0A as it is used by Unix.
       */
    string ChangeReturnCodeToUnix( string& data );

    /**
       * This method changes the return code 0x0A as it is used by Unix to 0x0D as it is used by the PV.
       */
    string ChangeReturnCodeToPV( string& data );

};

}; // namespace CasioPV

#endif
