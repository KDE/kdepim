/***************************************************************************
                          casiopv.h  -  description
                             -------------------
    begin                : Thu Dec 13 2001
    copyright            : (C) 2001 by Selzer Michael
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

#ifndef CASIOPV_H
#define CASIOPV_H

// C++ includes
#include <string>
#include <list>
// project includes
#include <protocol.h>
#include <pvdataentry.h>

namespace CasioPV {

/**This is a struct for the modified list which is returned by @ref CheckForModifiedEntries .
  *@author Selzer Michael, Maurus Erni
  */

struct modifiedDataEntry {
    /**
       * holds the number of the entry, if this is 0xFFFFFF or as int 16777215 it is a new entry.
       */
    unsigned int number;

    /**
       * if the entry is modified this is true else false.
       */
    bool modified;
};

typedef list<modifiedDataEntry> ModifyList;

/**This is a class for the syncronization with the casio pv
  *@author Selzer Michael
  */

class CasioPV {
  public:
    /**
       * Standard constructor.
       */
    CasioPV();
    /**
       * Constructor.
       * @param port a string which contains the path to the device file
       * @exception CasioPVException
       */
    CasioPV( string port );
    /**
       * Destructor.
       */
    ~CasioPV();

    /**
       * This method opens the serial port for the communication with the PV.
       * @param port The port where the PV is connected.
       * @exception CasioPVException
       */
    void OpenPort(string port);
    
/**
       * This method closes the serial port.
       * @exception CasioPVException
       */
    void ClosePort();

    /**
       * Make a wake up call to the protocol
       * @exception CasioPVException
       */
    void WakeUp();

    /**
       * This method waits for the link packet from the PV, which will be send if the WakeUp() method is called
       * or if the Start-button on the craddle is pressed
       * @param speed can be 38400, 19200 or 9600 (for the PV-450X)
       * @param pvpin sets the pvpin code (only numbers are allowed) as a string
       * @exception CasioPVException
       */
    void WaitForLink( int speed, string pvpin = "" );

    /**
       * Get number of data in a section.
       * @param dataCondition specified through the ModeCode.
       * @return number of data as an unsigned int (number of entries).
       * @exception CasioPVException
       */
    unsigned int GetNumberOfData( int dataCondition );

    /**
       * This method gets the data for the corresponding section
       * which is internaly handled through the data condition (mode code)
       * @param PVDataEntry there can only be derivated classes used because this is an interface
       * the data will be stored in this class
       * @param dataOrder this is the number of the entry which will be downloaded
       * @exception CasioPVException
       */
    void GetEntry( PVDataEntry& dataEntry, unsigned int dataOrder );

    /**
       * This method downloads a specified entry.     not tested !!! no command code known !!
       */
    void GetSpecifiedEntry( PVDataEntry& dataEntry, unsigned int dataOrder );

    /**
       * This method adds the content of a class derivated from PVDataEntry in the corresponding section
       * which is internaly handled through the data condition (mode code) and sets the number of the entry to the lowest free number.
       * @param PVDataEntry there can only derivated classes be used because this is an interface
       * @exception CasioPVException
       * @return returns the number for the new data entry.
       */
    unsigned int AddEntry( PVDataEntry& dataEntry );

    /**
       * This method appends the content of a class derivated from PVDataEntry in the corresponding section
       * which is internaly handled through the data condition (mode code)
       * @param PVDataEntry there can only derivated classes be used because this is an interface
       * @exception CasioPVException
       */
    void AppendEntry( PVDataEntry& dataEntry );

    /**
       * This method checks if the data in the specified section have changed since the last syncronisation.
       * @param dataCondition specified through the ModeCode
       * @return ModifyList this is a typedef for list<modifiedEntry>.
       */
    ModifyList CheckForModifiedEntries( unsigned int dataCondition );

    /**
       * This method gets a modified data entry and sets the modify flag to false.
       * The modified entries are reported through CheckForModifiedData.
       * @param PVDataEntry there can only derivated classes be used because this is an interface
       */
    void GetModifiedEntry( PVDataEntry& dataEntry, unsigned int dataOrder );

    /**
       * This method gets a new data entry and sets the number of the entry to the lowest free number.
       * The new entries are reported through CheckForModifiedData.
       * @param PVDataEntry there can only derivated classes be used because this is an interface
       * @return returns the number for the new data entry.
       */
    unsigned int GetNewEntry( PVDataEntry& dataEntry );

    /**
       * This method deletes a specified data entry on the PV.
       * @param dataCondition specified through the ModeCode
       * @param dataOrder this is the number of the entry which will be deleted
       */
    void DeleteEntry( unsigned int dataCondition, unsigned int dataOrder );

    /**
       * This method modifies the data of a specified entry in a section
       * @param dataCondition specified through the ModeCode
       * @param dataOrder this is the number of the entry which will be deleted
       */
    void ModifyEntry( PVDataEntry& dataEntry, unsigned int dataOrder );

    /**
       * This method changes the optional code of the UserID. This should be used in case of sync with the PV.
       * @param userid the new userid which will be stored on the PV
       */
    void ChangeOptionalCode( string optionalcode );

    /**
       * This method releases the link.
       * This is phase 3 as descriped in the documentation.
       */
    void ReleaseLink();

    /**
       * This method returns if the PV is in the secret area.
       * @return true if the PV is in the secet area else false.
       */
    bool isInSecretArea();

    /**
       * Getter for the actual model code.
       */
    string GetModelCode();

    /**
       * Getter for the actual optional code.
       */
    string GetOptionalCode();


  private:
    Protocol::Protocol* m_protocol;
    bool m_portopen;

    /**
       * This list holds the list with the modified or added entries. Get this with CheckForModifiedEntries.
       */
    list<modifiedDataEntry> m_modifiedList;

};

}; // namespace CasioPV

#endif
