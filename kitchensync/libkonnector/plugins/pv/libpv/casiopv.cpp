/***************************************************************************
                          casiopv.cpp  -  description
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

// C++ includes
#include <iostream>
#include <map>
#include <set>
// project includes
#include "FieldCode.h"
#include "casiopv.h"
#include "protocolexception.h"
#include "casiopvexception.h"

using namespace std;

#ifdef DEBUG
#define debugout( out ) { std::cout << out << std::endl; }
#else
#define debugout( out ) { /* out */ }
#endif

/**
   * Standard Constructor.
   */
CasioPV::CasioPV::CasioPV()
{
  debugout( "BEGIN:CasioPV::CasioPV()" );

  m_protocol = new Protocol::Protocol();
  m_portopen = false;

  debugout( "END:CasioPV::CasioPV()" );
}

/**
   * Constructor with port.
   */
CasioPV::CasioPV::CasioPV( string port ){
  debugout( "BEGIN:CasioPV::CasioPV()" );

  try
  {
    m_protocol = new Protocol::Protocol( port );
  }
  catch (ProtocolException e)
  {
    cerr << "ERROR: " << e.getMessage() << endl;
    if ( e.getErrorCode() == 2000 ) throw CasioPVException( "CasioPV::CasioPV : Can't open port", 1001);
  }
  m_portopen = true;

  debugout( "END:CasioPV::CasioPV()" );
}

/**
   * Destructor.
   */
CasioPV::CasioPV::~CasioPV(){
  debugout( "BEGIN: CasioPV::~CasioPV()" );

  delete( m_protocol );

  debugout( "END: CasioPV::~CasioPV()" );
}

/**
   * Opens the serial port for the communication with the PV.
   */
void CasioPV::CasioPV::OpenPort(string port)
{
  debugout( "BEGIN: CasioPV::OpenPort()" );

  try
  {
    m_protocol->GetSerial().OpenPort( port );
  }
  catch (SerialException e)
  {
    cerr << "ERROR: " << e.getMessage() << endl;
    if ( e.getErrorCode() == 2000 ) throw CasioPVException( "CasioPV::CasioPV : Can't open port", 1001);
  }
  m_portopen = true;

  debugout( "END: CasioPV::OpenPort()" );
}

/**
   * Closes the serial port for the communication with the PV.
   */
void CasioPV::CasioPV::ClosePort()
{
  debugout( "BEGIN: CasioPV::ClosePort()" );

  try
  {
    m_protocol->GetSerial().ClosePort();
  }
  catch (SerialException e)
  {
    cerr << "ERROR: " << e.getMessage() << endl;
    if ( e.getErrorCode() == 2000 ) throw CasioPVException( "CasioPV::CasioPV : Can't close port", 1001);
  }
  m_portopen = false;

  debugout( "END: CasioPV::ClosePort()" );
}

/**
   * Make a wake up call to the protocol
   */
void CasioPV::CasioPV::WakeUp(){
  debugout( "BEGIN:CasioPV::WakeUp()" );

  m_protocol->WakeUp();

  debugout( "END:CasioPV::WakeUp()" );
}

/**
   * This method waits for the link packet from the PV, which will be send if the WakeUp() method is called
   * or if the Start-button on the craddle is pressed
   * @param speed can be 38400, 19200 or 9600 (for the PV-450X)
   * @param pvpin sets the pvpin code (only numbers are allowed) as a string
   * @exception CasioPVException
   */
void CasioPV::CasioPV::WaitForLink( int speed, string pvpin ){
  debugout( "BEGIN:CasioPV::WaitForLink( int speed, string pvpin )" );

  unsigned int command;

  m_protocol->SetPVPIN( pvpin );

  m_protocol->SetRequestedSpeed( speed );
// !!!!!!!!!!!!!!!!!!!!!!! have to add time out !!!!!!!!!!!!!!!!!!!!!!!!!!
  try {
    m_protocol->RecieveLinkPacket();                                            // recieve calling up
  } catch (ProtocolException e) {
    cerr << "ERROR: " << e.getMessage() << endl;
    if ( e.getErrorCode() == 2000 ) throw CasioPVException( "CasioPV::WaitForLink : didn't recieve calling up !timeout!", 1001);
  }
  m_protocol->SendACK();

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve protocol level exchange
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();
  do {
    m_protocol->SendCommandPacket(PROTOCOL_LEVEL_EXCHANGE_R);     // send protocol level exchange respond
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve UserID
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();
  do {
    m_protocol->SendCommandPacket(USER_ID_EXCHANGE_R);                  // send UserID respond
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve Com speed settings
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();
  do {
    m_protocol->SendCommandPacket(COMMUNICATION_SPEED_SETTING_R); // send Com speed settings respond
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve Phase transition
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();
  do {
    m_protocol->SendCommandPacket(PHASE_TRANSITION_COMMAND_R);      // send Phase transition for 01
  } while (!m_protocol->RecieveACK());

  m_protocol->EndPhase1();

  debugout( "END:CasioPV::WaitForLink( int speed, string pvpin )" );
}

/**
   * Get number of data in a section
   * @param dataCondition specified through the ModeCode
   * @return number of data as an unsigned int (number of entries).
   * @exception CasioPVException
   */
unsigned int CasioPV::CasioPV::GetNumberOfData( int dataCondition ){
  debugout( "BEGIN:CasioPV::GetNumberOfData( int )" );

  int NoOfData = 0;
  datapacket packet;
  unsigned int command;

  do {
    m_protocol->SendCommandPacket( NUMBER_OF_DATA_REQUEST, dataCondition );
  } while ( !m_protocol->RecieveACK() );

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve start data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  m_protocol->RecieveDataPacket( packet );                                      // recieve number of data

  if ( packet.fieldCode == NUMBER_OF_DATA ) {
//    debugout( "number of data = " << packet.data );
    for (int i = 0; i<=5; i++) {
      NoOfData *=16;
      // This calculation is the same as in Protocol::IntToHex
      packet.data[i] -= 48;
      if ( packet.data[i] >= 17 ) packet.data[i] -= 7;
      NoOfData += packet.data[i];
    }
  } else {
    throw CasioPVException( "CasioPV::GetNumberOfData : Didn't recieve the number of data!!!!!", 1002);
  }
  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve end data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();
  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve number of data request result
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  debugout( "END:CasioPV::GetNumberOfData( int )" );
  return  NoOfData;
}

/**
   * This method gets the data for the corresponding section
   * which is internaly handled through the data condition (mode code)
   * @param PVDataEntry there can only be derivated classes used because this is an interface
   * the data will be stored in this class
   * @param DataOrder this is the number of the entry which will be downloaded
   * @exception CasioPVException
   */
void CasioPV::CasioPV::GetEntry( PVDataEntry& dataEntry, unsigned int dataOrder ) {
  debugout( "BEGIN:CasioPV::GetEntry( PVDataEntry&, unsigned int )" );

  datapacket packet;
  unsigned int command;

  do {
    m_protocol->SendCommandPacket( DATA_SEND_REQUEST, dataEntry.getModeCode(), dataOrder );
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve start data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  if ( m_protocol->RecieveOrder() == 0x02 ) {
    do {
      m_protocol->RecieveDataPacket( packet, true );                              // recieve data         check the checksum!!!!!!!!!!!
      dataEntry.setFieldData( packet );
    } while ( m_protocol->RecieveOrder() == 0x02 );
  } else {
    throw CasioPVException( "CasioPV::GetEntry : Communication error!!", 1003 );
  }

  bool checkorder = true;
  while ( !m_protocol->RecieveCommandPacket( command, checkorder ) ) {          // recieve end data block
    m_protocol->SendNAK();
    checkorder = false;
  }
  m_protocol->SendACK();

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve result command
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  debugout( "END:CasioPV::GetEntry( PVDataEntry&, unsigned int )" );
}

/**
  * This method downloads a specified entry.
  */
void CasioPV::CasioPV::GetSpecifiedEntry( PVDataEntry& dataEntry, unsigned int dataOrder ){
  debugout( "BEGIN:CasioPV::GetSpecifiedEntry( PVDataEntry&, unsigned int )" );

  datapacket packet;
  unsigned int command;

  do {
    m_protocol->SendCommandPacket( 0x8033, dataEntry.getModeCode(), dataOrder );
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve start data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  if ( m_protocol->RecieveOrder() == 0x02 ) {
    do {
      m_protocol->RecieveDataPacket( packet, true );                              // recieve data
      dataEntry.setFieldData( packet );
    } while ( m_protocol->RecieveOrder() == 0x02 );
  } else {
    throw CasioPVException( "CasioPV::GetData : Communication error!!", 1003 );
  }

  bool checkorder = true;
  while ( !m_protocol->RecieveCommandPacket( command, checkorder ) ) {          // recieve end data block
    m_protocol->SendNAK();
    checkorder = false;
  }
  m_protocol->SendACK();

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve result command
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  debugout( "END:CasioPV::GetSpecifiedEntry( PVDataEntry&, unsigned int )" );
}

/**
   * This method adds the content of a class derivated from PVDataEntry in the corresponding section
   * which is internaly handled through the data condition (mode code) and sets the number of the entry to the lowest free number.
   * @param PVDataEntry there can only derivated classes be used because this is an interface
   * @exception CasioPVException
   * @return returns the number for the new data entry.
   */
unsigned int CasioPV::CasioPV::AddEntry( PVDataEntry& dataEntry ) {
  debugout( "BEGIN:CasioPV::AddEntry( PVDataEntry&, unsigned int )"  );

  datapacket packet;
  unsigned int command;
  unsigned int dataOrder = 1;
  set<unsigned int> entrynumberlist;

  // search the lowest possible number for the new entry
  for ( ModifyList::iterator iter = m_modifiedList.begin(); iter !=m_modifiedList.end(); iter++) {
    entrynumberlist.insert( (*iter).number );
  }
  for ( set<unsigned int>::iterator iter = entrynumberlist.begin(); iter !=entrynumberlist.end(); iter++ ) {
    if ( dataOrder ==  *iter ) dataOrder++;
  }

  if ( !dataEntry.isSendable() ) throw CasioPVException( "CasioPV::AddEntry : This DataEntry is not sendable. Some fields are missing", 1005 );

  do {
    m_protocol->SendCommandPacket( ADD_DATA, dataEntry.getModeCode(), dataOrder );
  } while (!m_protocol->RecieveACK());

  do {
    m_protocol->SendCommandPacket( START_DATA_BLOCK );                  // send start data block
  } while (!m_protocol->RecieveACK());

  for (map<unsigned int, string>::const_iterator entrydata = dataEntry.getData().begin(); entrydata != dataEntry.getData().end(); entrydata++ ){
    datapacket packet;
    packet.fieldCode = entrydata->first;
    string data = entrydata->second;

    int count = (data.size()%1024 == 0) ? (unsigned int)data.size()/1024 : (unsigned int)data.size()/1024 + 1;

    // truncate data in count packets
    for (int i = 0; i<count; i++) {
      packet.continued = ((data.size()-(1024*i)) > 1024 );
      packet.data.assign( data, (unsigned int)(1024*i), packet.continued?1024:data.size()%1024 );
      m_protocol->SendDataPacket( packet );                                     // send data
    }
  }

/*  do {
    m_protocol->SendCommandPacket( DATA_BLOCK_CHECK );            // send data block check
  } while (!m_protocol->RecieveACK());*/

  do {
    m_protocol->SendCommandPacket( END_DATA_BLOCK );                    // send end data block
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve result command
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  debugout( "END:CasioPV::AddEntry( PVDataEntry&, unsigned int )" );
  return dataOrder;
}

/**
   * This method appends the content of a class derivated from PVDataEntry in the corresponding section
   * which is internaly handled through the data condition (mode code)
   * @param PVDataEntry there can only derivated classes be used because this is an interface
   * @exception CasioPVException
   */
void CasioPV::CasioPV::AppendEntry( PVDataEntry& dataEntry ){
  debugout( "BEGIN:CasioPV::AppendEntry( PVDataEntry& )" );

//  unsigned int command;

  if ( !dataEntry.isSendable() ) throw CasioPVException( "CasioPV::AppendEntry : This DataEntry is not sendable. Some fields are missing", 1005 );

  do {                                                                          // send append registration command
    m_protocol->SendCommandPacket( APPEND_REGISTRATION, dataEntry.getModeCode() );
  } while (!m_protocol->RecieveACK());

  do {
    m_protocol->SendCommandPacket( START_DATA_BLOCK );                // send start data block
  } while (!m_protocol->RecieveACK());

  for (map<unsigned int, string>::const_iterator entrydata = dataEntry.getData().begin(); entrydata != dataEntry.getData().end(); entrydata++ ){
    datapacket packet;
    packet.fieldCode = entrydata->first;
    string data = entrydata->second;

    int count = (data.size()%1024 == 0) ? (unsigned int)data.size()/1024 : (unsigned int)data.size()/1024 + 1;

    // truncate data in count packets
    for (int i = 0; i<count; i++) {
      packet.continued = ((data.size()-(1024*i)) > 1024 );
      packet.data.assign( data, (unsigned int)(1024*i), packet.continued?1024:data.size()%1024 );
      m_protocol->SendDataPacket( packet );                                     // send data
    }
  }

/*  do {
    m_protocol->SendCommandPacket( DATA_BLOCK_CHECK );            // send data block check
  } while (!m_protocol->RecieveACK());*/

  do {
    m_protocol->SendCommandPacket( END_DATA_BLOCK );                    // send end data block
  } while (!m_protocol->RecieveACK());

// !!!!!!!!!!!!!!!!!!!! recieve result command like GetData ?????????????????? looks like a bug in the protocol
/*  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve result command
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();*/

  debugout( "END:CasioPV::AppendEntry( PVDataEntry& )" );
}

/**
   * This method checks if the data in the specified section have changed since the last syncronisation.
   * @param dataCondition specified through the ModeCode
   * @return ModifyList this is a typedef for list<modifiedEntry>.
   */
CasioPV::ModifyList CasioPV::CasioPV::CheckForModifiedEntries( unsigned int dataCondition ){
  debugout( "BEGIN:CasioPV::CheckForModifiedEntries( unsigned int )" );

  int NoOfEntry = 0;
  datapacket packet;
  unsigned int command;
  packet.continued = true;
  string modifiedData = "";
  modifiedDataEntry modifiedEntry;

  do {
    m_protocol->SendCommandPacket( CHECK_FOR_MODIFIED_DATA, dataCondition );
  } while ( !m_protocol->RecieveACK() );

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve start data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  while ( packet.continued ) {
    unsigned int ro =  m_protocol->RecieveOrder();
    switch ( ro ) {
      case 0x02 : {
          m_protocol->RecieveDataPacket( packet, true );                          // recieve list of modified data
          modifiedData += packet.data;
        }
        break;
      case 0x01 : {
          while ( !m_protocol->RecieveCommandPacket( command, true ) ) {        // recieve data block check
            m_protocol->SendNAK();
          }
          m_protocol->SendACK();
        }
        break;
      default : throw CasioPVException( "CasioPV::CheckForModifiedEntries : Didn't recieve the modified data list !!!!!", 1006 );
    }
  }

  if ( packet.fieldCode == DATA_CHANGED ) {
    for (unsigned int i = 0; i<modifiedData.length()/8; i++) {
      NoOfEntry = 0;
      for (unsigned int j = i*8; j<=5+i*8; j++) {
        NoOfEntry *=16;
        // This calculation is the same as in Protocol::IntToHex
        modifiedData[j] -= 48;
        if ( modifiedData[j] >= 17 ) modifiedData[j] -= 7;
        NoOfEntry += modifiedData[j];
      }
      modifiedEntry.number = NoOfEntry;
      modifiedEntry.modified = ( modifiedData[7+(i*8)] == '1' );
      m_modifiedList.push_back( modifiedEntry );
    }
  } else {
    throw CasioPVException( "CasioPV::CheckForModifiedEntries : Didn't recieve the modified data list !!!!!", 1006 );
  }

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve end data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve numbers of modified data result
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  debugout( "END:CasioPV::CheckForModifiedEntries( unsigned int )" );
  return m_modifiedList;
}

/**
   * This method gets a modified data entry and sets the modify flag to false.
   * The modified entries are reported through CheckForModifiedData.
   */
void CasioPV::CasioPV::GetModifiedEntry( PVDataEntry& dataEntry, unsigned int dataOrder ){
  debugout( "BEGIN:CasioPV::GetModifiedEntry( PVDataEntry&, unsigned int )" );

  datapacket packet;
  unsigned int command;

  do {
    m_protocol->SendCommandPacket( SEND_MODIFIED_DATA_REQUEST, dataEntry.getModeCode(), dataOrder );
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve start data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  if ( command == START_DATA_BLOCK ) {
    while ( m_protocol->RecieveOrder() == 0x02 ) {
      m_protocol->RecieveDataPacket( packet, true );                              // recieve data
      dataEntry.setFieldData( packet );
    }

    bool checkorder = true;
    while ( !m_protocol->RecieveCommandPacket( command, checkorder ) ) {        // recieve end data block
      m_protocol->SendNAK();
      checkorder = false;
    }
    m_protocol->SendACK();

    while ( !m_protocol->RecieveCommandPacket( command ) ) {                  // recieve result command
      m_protocol->SendNAK();
    }
    m_protocol->SendACK();
  }

  debugout( "END:CasioPV::GetModifiedEntry( PVDataEntry&, unsigned int )" );
}

/**
   * This method gets a new data entry and sets the number of the entry to the lowest free number.
   * The new entries are reported through CheckForModifiedData.
   * @param PVDataEntry there can only derivated classes be used because this is an interface
   * @return returns the number for the new data entry.
   */
unsigned int CasioPV::CasioPV::GetNewEntry( PVDataEntry& dataEntry ){
  debugout( "BEGIN:CasioPV::GetNewEntry( PVDataEntry& )" );

  datapacket packet;
  unsigned int command;
  unsigned int dataOrder = 1;
  set<unsigned int> entrynumberlist;

  // search the lowest possible number for the new entry
  for ( ModifyList::iterator iter = m_modifiedList.begin(); iter !=m_modifiedList.end(); iter++) {
    entrynumberlist.insert( (*iter).number );
  }
  for ( set<unsigned int>::iterator iter = entrynumberlist.begin(); iter !=entrynumberlist.end(); iter++ ) {
    if ( dataOrder ==  *iter ) dataOrder++;
  }

  do {                                                                            // send new data request command
    m_protocol->SendCommandPacket( SEND_NEW_DATA_REQUEST, dataEntry.getModeCode(), dataOrder );
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve start data block
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  if ( command == START_DATA_BLOCK ) {
    while ( m_protocol->RecieveOrder() == 0x02 ) {
      m_protocol->RecieveDataPacket( packet, true );                              // recieve data
      dataEntry.setFieldData( packet );
    }

    bool checkorder = true;
    while ( !m_protocol->RecieveCommandPacket( command, checkorder ) ) {        // recieve end data block
      m_protocol->SendNAK();
      checkorder = false;
    }
    m_protocol->SendACK();

    while ( !m_protocol->RecieveCommandPacket( command ) ) {                  // recieve result command
      m_protocol->SendNAK();
    }
    m_protocol->SendACK();
  }

  debugout( "END:CasioPV::GetNewEntry( PVDataEntry& )" );
  return dataOrder;
}

/**
   * This method deletes a specified data entry on the PV
   */
void CasioPV::CasioPV::DeleteEntry( unsigned int dataCondition, unsigned int dataOrder ){
  debugout( "BEGIN:CasioPV::DeleteEntry( unsigned int, unsigned int )" );

  unsigned int command;

  do {                                                                            // send delete data command
    m_protocol->SendCommandPacket( DELETE_DATA, dataCondition, dataOrder );
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve delete data result
    m_protocol->SendNAK();
  }

  debugout( "END:CasioPV::DeleteEntry( unsigned int, unsigned int )" );
}

/**
   * This method modifies the data of a specified entry in a section
   * @param dataCondition specified through the ModeCode
   * @param dataOrder this is the number of the entry which will be deleted
   */
void CasioPV::CasioPV::ModifyEntry( PVDataEntry& dataEntry, unsigned int dataOrder ){
  debugout( "BEGIN:CasioPV::ModifyEntry( PVDataEntry&, unsigned int )" );

  unsigned int command;

  if ( !dataEntry.isSendable() ) throw CasioPVException( "CasioPV::ModifyEntry : This DataEntry is not sendable. Some fields are missing", 1005 );

  do {                                                                            // send modify data command
    m_protocol->SendCommandPacket( MODIFY_DATA, dataEntry.getModeCode(), dataOrder );
  } while (!m_protocol->RecieveACK());

  do {
    m_protocol->SendCommandPacket( START_DATA_BLOCK );                  // send start data block
  } while (!m_protocol->RecieveACK());

  for (map<unsigned int, string>::const_iterator entrydata = dataEntry.getData().begin(); entrydata != dataEntry.getData().end(); entrydata++ ){
    datapacket packet;
    packet.fieldCode = entrydata->first;
    string data = entrydata->second;

    int count = (data.size()%1024 == 0) ? (unsigned int)data.size()/1024 : (unsigned int)data.size()/1024 + 1;

    // truncate data in count packets
    for (int i = 0; i<count; i++) {
      packet.continued = ((data.size()-(1024*i)) > 1024 );
      packet.data.assign( data, (unsigned int)(1024*i), packet.continued?1024:data.size()%1024 );
      m_protocol->SendDataPacket( packet );                                     // send data
    }
  }

/*  do {
    m_protocol->SendCommandPacket( DATA_BLOCK_CHECK );            // send data block check
  } while (!m_protocol->RecieveACK());*/

  do {
    m_protocol->SendCommandPacket( END_DATA_BLOCK );
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve result command
    m_protocol->SendNAK();
  }
  m_protocol->SendACK();

  debugout( "END:CasioPV::ModifyEntry( PVDataEntry&, unsigned int )" );
}

/**
   * This method changes the optional code of the UserID. This should only be used if the PC have changed data on the PV
   */
void CasioPV::CasioPV::ChangeOptionalCode( string optionalcode ){
  debugout( "BEGIN:CasioPV::ChangeOptionalCode( string )"  );

  unsigned int command;

  m_protocol->SetOptionalCode( optionalcode );

  do {
    m_protocol->SendCommandPacket( CHANGE_USER_ID );                      // send change optional code request
  } while (!m_protocol->RecieveACK());

  while ( !m_protocol->RecieveCommandPacket( command ) ) {                    // recieve change optional code result
    m_protocol->SendNAK();
  }

  debugout( "END:CasioPV::ChangeOptionalCode( string )" );
}

/**
   * This method releases the link
   * This is phase 3 as descriped in the documentation
   */
void CasioPV::CasioPV::ReleaseLink() {
  debugout( "BEGIN:CasioPV::ReleaseLink()" );

  m_protocol->SendLinkPacket( END_COMMUNICATION );

  debugout( "END:CasioPV::ReleaseLink()" );
}

/**
   * This method returns if the PV is in the secret area.
   * @return true if the PV is in the secet area else false.
   */
bool CasioPV::CasioPV::isInSecretArea(){
  return m_protocol->isInSecretArea();
}

/**
   * Getter for the actual model code
   */
string CasioPV::CasioPV::GetModelCode(){
  return m_protocol->GetModelCode();
}

/**
   * Getter for the actual optional code
   */
string CasioPV::CasioPV::GetOptionalCode(){
  return m_protocol->GetOptionalCode();
}
