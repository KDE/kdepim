/***************************************************************************
                          pvschedule.h  -  description
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

#ifndef PVSCHEDULE_H
#define PVSCHEDULE_H

// project includes
#include <pvdataentry.h>

/**
  *@author Selzer Michael, Maurus Erni
  */

namespace CasioPV {

class PVSchedule : public PVDataEntry  {
public:
    /**
     * Constructor.
     */
  PVSchedule(unsigned int uid);

    /**
     * Destructor.
     */
  virtual ~PVSchedule();

    /**
       * Getter for the mode code.
       * @return The mode code of the data entry.
       */
    virtual unsigned int getModeCode() const;

    /**
       * Getter for the uid.
       * @return The uid of the data entry.
       */
    virtual unsigned int getUid() const;

    /**
       * Getter for the data.
       * @return Return all of the data.
       */
    virtual const map<unsigned int, string>& getData() const;

    /**
       * Getter for date.
       * @return Return the date in a string with the format yyyymmdd
       */
    virtual string getDate();

    /**
       * Getter for start time.
       * @return Return the start time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
       * hours 00-23
       * minutes 00-59
       */
    virtual string getStartTime();

    /**
       * Getter for end time.
       * @return Return the end time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
       * hours 00-23
       * minutes 00-59
       */
    virtual string getEndTime();

    /**
       * Getter for alarm time.
       * @return Return the alarm time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
       * hours 00-23
       * minutes 00-59
       */
    virtual string getAlarmTime();

    /**
       * Getter for description.
       * @return Return the description in a string with up to 2046 characters
       */
    virtual string getDescription();

    /**
       * Setter for date.
       * @param value sets the date in a string with the format yyyymmdd
       */
    virtual void setDate( string& value );

    /**
       * Setter for start time.
       * @param value sets the start time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
       * hours 00-23
       * minutes 00-59
       */
    virtual void setStartTime( string& value );

    /**
       * Setter for end time.
       * @param value sets the end time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
       * hours 00-23
       * minutes 00-59
       */
    virtual void setEndTime( string& value );

    /**
       * Setter for alarm time.
       * @param value sets the alarm time in a string with 4 bytes in the format hhdd. So every value is between 0-9 (0x30-0x39)
       * hours 00-23
       * minutes 00-59
       */
    virtual void setAlarmTime( string& value );

    /**
       * Setter for description.
       * @param value sets the description in a string with up to 2046 characters
       * @exception PVDataEntryException
       */
    virtual void setDescription( string& value );


    /**
       * Setter for the data of a field.
       * @param packet data packet for the field
       * @exception PVDataEntryException
       */
    virtual void setFieldData( datapacket& packet );

    /**
       * Checks if this instance of schedule is sendable.
       * The fields date, start time and description have to be set.
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
    friend std::ostream& operator<< (std::ostream& out, PVSchedule& schedule);

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
       * bool for isSecret()
       */
    bool m_isSecret;
};

}; // namespace CasioPV

#endif
