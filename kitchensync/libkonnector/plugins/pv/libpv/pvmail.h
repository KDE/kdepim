/***************************************************************************
                          pvmail.h  -  description
                             -------------------
    begin                : Die Sep 10 2002
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

#ifndef PVMAIL_H
#define PVMAIL_H

// project includes
#include <pvdataentry.h>

/** This class is for mail and sms.
  *@author Selzer Michael, Maurus Erni
  */

namespace CasioPV {

class PVMail : public PVDataEntry  {
public:
    /**
     * Constructor.
     */
    PVMail( unsigned int modeCode, unsigned int uid );

    /**
     * Destructor.
     */
    virtual ~PVMail();

    /**
       * Getter for the mode code.
       * @return The mode code of the data entry.
       */
    virtual unsigned int getModeCode() const;

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

    /**
       * Getter for mail data.
       * @return Return the mail data in binary format with up to 32KB.
       */
    virtual string getMailData();



    /**
       * Setter for mail data.
       * @param value sets the mail data in binary format with up to 32KB.
       * @exception PVDataEntryException
       */
    virtual void setMailData( string& value );

    /**
       * Setter for the data of a field.
       * @param packet data packet for the field
       * @exception PVDataEntryException
       */
    virtual void setFieldData( datapacket& packet );

    /**
       * Checks if this instance of sms is sendable.
       * The field mail data have to be set.
       * @return bool true if all necessary fields are filled else false.
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
    friend std::ostream& operator<< (std::ostream& out, PVMail& mail);

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

    unsigned int m_modeCode;
    bool m_continued;

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

};

}; // namespace CasioPV

#endif
