/***************************************************************************
                          pvexpense.h  -  description
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

#ifndef PVEXPENSE_H
#define PVEXPENSE_H

// project includes
#include <pvdataentry.h>

/**
  *@author Selzer Michael, Maurus Erni
  */

namespace CasioPV {

class PVExpense : public PVDataEntry  {
public:
    /**
       * Constructor.
       */
    PVExpense(unsigned int uid);

    /**
       * Destructor.
       */
    virtual ~PVExpense();


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
       * @return Return all of the data in a map, where int is the FieldCode and string the data
       */
    virtual const map<unsigned int, string>& getData() const;

    /**
       * Getter for date.
       * @return Return the date in a string with the format yyyymmdd
       */
    virtual string getDate();

    /**
       * Getter for payment type.
       * @return Return the payment type in a string with max of 14 characters
       */
    virtual string getPaymentType();

    /**
       * Getter for amount.
       * @return Return the amount in a string with max value of +/- 99999999
       * and max two digits of under decimal point
       */
    virtual string getAmount();

    /**
       * Getter for expence type.
       * @return Return the expence type in a string with max of 14 characters
       */
    virtual string getExpenseType();

    /**
       * Getter for note.
       * @return Return the note in a string with up to 2008 characters
       */
    virtual string getNote();




    /**
       * Setter for date.
       * @param value sets the date in a string with the format yyyymmdd
       */
    virtual void setDate( string& value );

    /**
       * Setter for payment type.
       * @param value sets the payment type in a string with max of 14 characters
       * @exception PVDataEntryException
       */
    virtual void setPaymentType( string& value );

    /**
       * Setter for amount.
       * @param value sets the amount in a string with max value of +/- 99999999
       * and max two digits of under decimal point
       */
    virtual void setAmount( string& value );

    /**
       * Setter for expense type.
       * @param value sets the expense type with a string with max of 14 characters
       * @exception PVDataEntryException
       */
    virtual void setExpenseType( string& value );

    /**
       * Setter for note.
       * @param value sets the note in a string with up to 2008 characters
       * @exception PVDataEntryException
       */
    virtual void setNote( string& value );


    /**
       * Setter for the data of a field.
       * @param packet data packet for the field
       * @exception PVDataEntryException
       */
    virtual void setFieldData( datapacket& packet );

    /**
       * Checks if this instance of expense is sendable.
       * The fields date and amount have to be set.
       * @return bool true if all necessary fields are filled else false.
       */
    virtual bool isSendable();

    /**
       * This method returns if the data entry is a secret area entry
       * @return bool true if secret else false
       */
    virtual bool isSecret();

    /**
       * stream the content in a formated way
       */
    friend std::ostream& operator<< (std::ostream& out, PVExpense expense);

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
};

}; // namespace CasioPV

#endif
