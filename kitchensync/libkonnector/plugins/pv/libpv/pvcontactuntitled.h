/***************************************************************************
                          pvcontactuntitled.h  -  description
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

#ifndef PVCONTACTUNTITLED_H
#define PVCONTACTUNTITLED_H

#include <sstream>

#include <pvdataentry.h>

/**
  *@author Selzer Michael, Maurus Erni
  */

namespace CasioPV {

class PVContactUntitled : public PVDataEntry  {
public:
    /**
     * Constructor.
     */
    PVContactUntitled( unsigned int modeCode, unsigned int uid );

    /**
     * Destructor.
     */
    virtual ~PVContactUntitled();


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
       * Getter for field1.
       * @return Return the field1 in a string with up to 2036 characters
       */
    virtual string getField1();

    /**
       * Getter for field2.
       * @return Return the field2 in a string with up to 2036 characters
       */
    virtual string getField2();

    /**
       * Getter for field3.
       * @return Return the field3 in a string with up to 2036 characters
       */
    virtual string getField3();

    /**
       * Getter for field4.
       * @return Return the field4 in a string with up to 2036 characters
       */
  virtual string getField4();

    /**
       * Getter for field5.
       * @return Return the field5 in a string with up to 2036 characters
       */
    virtual string getField5();

    /**
       * Getter for field6.
       * @return Return the field6 in a string with up to 2036 characters
       */
    virtual string getField6();

    /**
       * Getter for field7.
       * @return Return the field7 in a string with up to 2036 characters
       */
    virtual string getField7();

    /**
       * Getter for field8.
       * @return Return the field8 in a string with up to 2036 characters
       */
    virtual string getField8();

    /**
       * Getter for field9.
       * @return Return the field9 in a string with up to 2036 characters
       */
    virtual string getField9();

    /**
       * Getter for field10.
       * @return Return the field10 in a string with up to 2036 characters
       */
  virtual string getField10();

    /**
       * Getter for field11.
       * @return Return the field11 in a string with up to 2036 characters
       */
    virtual string getField11();

    /**
       * Getter for field12.
       * @return Return the field12 in a string with up to 2036 characters
       */
    virtual string getField12();

    /**
       * Getter for field13.
       * @return Return the field13 in a string with up to 2036 characters
       */
    virtual string getField13();

    /**
       * Setter for field1.
       * @param value sets the name in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField1( string& value );

    /**
       * Setter for field2.
       * @param value sets the home number in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField2( string& value );

    /**
       * Setter for field3.
       * @param value sets the business number in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField3( string& value );

    /**
       * Setter for field4.
       * @param value sets the fax number in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField4( string& value );

    /**
       * Setter for field5.
       * @param value sets the business fax in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField5( string& value );

    /**
       * Setter for field6.
       * @param value sets the mobile in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField6( string& value );

    /**
       * Setter for field7.
       * @param value sets the address in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField7( string& value );

    /**
       * Setter for field8.
       * @param value sets the email in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField8( string& value );

    /**
       * Setter for field9.
       * @param value sets the employer in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField9( string& value );

    /**
       * Setter for field10.
       * @param value sets the business address in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField10( string& value );

    /**
       * Setter for field11.
       * @param value sets the department in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField11( string& value );

    /**
       * Setter for field12.
       * @param value sets the position in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField12( string& value );

    /**
       * Setter for field13.
       * @param value sets the note in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setField13( string& value );


    /**
       * Getter for the data.
       * @return Return all of the data.
       */
    virtual const map<unsigned int, string>& getData() const;

    /**
       * Setter for the data of a field.
       * @param packet data packet for the field
       * @exception PVDataEntryException
       */
    virtual void setFieldData( datapacket& packet );

    /**
       * Checks if this instance of reminder is sendable.
       * The field name have to be set if this have the modecode private contact.
       * If this have the modecode for a business contacte the field employer have to be set, too.
       * @return bool true if all necessary fields are filled else false.
       */
    virtual bool isSendable();

    /**
       * This method returns if the data entry is a secret area entry
       * @return bool true if secret false else
       */
    virtual bool isSecret();

    /**
       * stream the content
       */
    friend std::ostream& operator<< (std::ostream& out, PVContactUntitled& contact);

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
