/***************************************************************************
                          pvcontact.h  -  Declaration of the class
                                                 for a contact.
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

#ifndef PVCONTACT_H
#define PVCONTACT_H

#include <sstream>

// project includes
#include <pvdataentry.h>

/**
  *@author Selzer Michael, Thomas Bonk, Maurus Erni
  */

namespace CasioPV {

class PVContact : public PVDataEntry  {
public:
    /**
     * Constructor.
     */
    PVContact( unsigned int modeCode, unsigned int uid );

    /**
     * Destructor.
     */
    virtual ~PVContact();


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
       * Getter for name.
       * @return Return the name in a string with up to 2036 characters
       */
    virtual string getName();

    /**
       * Getter for home number.
       * @return Return the home number in a string with up to 2036 characters
       */
    virtual string getHomeNumber();

    /**
       * Getter for business number.
       * @return Return the business number in a string with up to 2036 characters
       */
    virtual string getBusinessNumber();

    /**
       * Getter for fax number.
       * @return Return the fax number in a string with up to 2036 characters
       */
  virtual string getFaxNumber();

    /**
       * Getter for business fax.
       * @return Return the business fax in a string with up to 2036 characters
       */
    virtual string getBusinessFax();

    /**
       * Getter for mobile.
       * @return Return the mobile in a string with up to 2036 characters
       */
    virtual string getMobile();

    /**
       * Getter for address.
       * @return Return the address in a string with up to 2036 characters
       */
    virtual string getAddress();

    /**
       * Getter for email.
       * @return Return the email in a string with up to 2036 characters
       */
    virtual string getEmail();

    /**
       * Getter for employer.
       * @return Return the employer in a string with up to 2036 characters
       */
    virtual string getEmployer();

    /**
       * Getter for business address.
       * @return Return the business address in a string with up to 2036 characters
       */
  virtual string getBusinessAddress();

    /**
       * Getter for department.
       * @return Return the department in a string with up to 2036 characters
       */
  virtual string getDepartment();

    /**
       * Getter for position.
       * @return Return the position in a string with up to 2036 characters
       */
    virtual string getPosition();

    /**
       * Getter for note.
       * @return Return the note in a string with up to 2036 characters
       */
    virtual string getNote();

    /**
       * Setter for name.
       * @param value sets the name in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setName( string& value );

    /**
       * Setter for home number.
       * @param value sets the home number in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setHomeNumber( string& value );

    /**
       * Setter for business number.
       * @param value sets the business number in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setBusinessNumber( string& value );

    /**
       * Setter for fax number.
       * @param value sets the fax number in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setFaxNumber( string& value );

    /**
       * Setter for business fax.
       * @param value sets the business fax in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setBusinessFax( string& value );

    /**
       * Setter for mobile.
       * @param value sets the mobile in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setMobile( string& value );

    /**
       * Setter for address.
       * @param value sets the address in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setAddress( string& value );

    /**
       * Setter for email.
       * @param value sets the email in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setEmail( string& value );

    /**
       * Setter for employer.
       * @param value sets the employer in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setEmployer( string& value );

    /**
       * Setter for business address.
       * @param value sets the business address in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setBusinessAddress( string& value );

    /**
       * Setter for department.
       * @param value sets the department in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setDepartment( string& value );

    /**
       * Setter for position.
       * @param value sets the position in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setPosition( string& value );

    /**
       * Setter for note.
       * @param value sets the note in a string with up to 2036 characters
       * @exception PVDataEntryException
       */
    virtual void setNote( string& value );


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
    friend std::ostream& operator<< (std::ostream& out, PVContact& contact);

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
