/***************************************************************************
                          protocol.cpp  -  description
                             -------------------
    begin                : Sat Feb 23 2002
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

// C++ includes
#include <iostream>

// C includes
#include <unistd.h>
// project includes
#include "protocol.h"
#include "serialexception.h"
#include "protocolexception.h"

#define SOH   0x01      // Command packet
#define STX   0x02      // Data packet
#define ACK   0x06      // ACK: Positive response packet
#define NAK   0x15      // NAK: Negative response packet
#define CAN   0x18      // Link packet


using namespace std;
/**
   * Standard Constructor.
   */
CasioPV::Protocol::Protocol()
{
  debugout( "BEGIN:Protocol::Protocol()" );

  m_speed = 9600;
  m_actual_send_No = 0;
  m_actual_recieve_No = 0;
  string m_modelcode = "0000";            // may be there are better pre-defines
  string m_username = "000000000000";
  string m_optionalcode = "000000000000000000000000";
  m_userid = "";
  m_pvpin = "";
  m_recievedNAK = 0;
  m_sendNAK = 0;
  m_inSecretArea = false;

  debugout( "END:Protocol::Protocol( string )" );
}

/**
   * Constructor.
   * @param port gives a string which contains the path to the device file
   */
CasioPV::Protocol::Protocol( string port ){
  debugout( "BEGIN:Protocol::Protocol( string )" );

  m_speed = 9600;
  m_actual_send_No = 0;
  m_actual_recieve_No = 0;
  string m_modelcode = "0000";            // may be there are better pre-defines
  string m_username = "000000000000";
  string m_optionalcode = "000000000000000000000000";
  m_userid = "";
  m_pvpin = "";
  m_recievedNAK = 0;
  m_sendNAK = 0;
  m_inSecretArea = false;
  try {
    m_com.OpenPort( port );
  } catch (SerialException e) {
    cerr << "ERROR: " << e.getMessage() << endl;
    throw ProtocolException( "Protocol::Protocol : could not open port", 2001 );
  }

  debugout( "END:Protocol::Protocol( string )" );
}

/**
   * Constructor.
   * @param com sets a @ref Serial object which will be used for the communication
   */
CasioPV::Protocol::Protocol( Serial& com ){
  debugout( "BEGIN:Protocol::Protocol( Serial& )" );

  m_speed = 9600;
  m_actual_send_No = 0;
  m_actual_recieve_No = 0;
  string m_modelcode = "0000";            // may be there are better pre-defines
  string m_username = "000000000000";
  string m_optionalcode = "000000000000000000000000";
  m_userid = "";
  m_pvpin = "";
  m_recievedNAK = 0;
  m_sendNAK = 0;
  m_inSecretArea = false;
  m_com = com;
  m_com.SetInputSpeed( m_speed );
  m_com.SetOutputSpeed( m_speed );

  debugout( "END:Protocol::Protocol( Serial& )" );
}

/**
   * Destructor.
   */
CasioPV::Protocol::~Protocol(){
  debugout( "BEGIN:Protocol::~Protocol()" );
  try {
    m_com.ClosePort();
  } catch (SerialException e) {
    cerr << "ERROR: " << e.getMessage() << endl;
    throw ProtocolException( "Protocol::Protocol : could not close port", 2001 );
  }
  debugout( "END:Protocol::~Protocol()" );
}

 /**
    * This method sets the speed, which will be used after phase 1 (handshak phase).
    * Supported speeds are 38400, 19200 and 9600.
    * @exception ProtocolException
    */
void CasioPV::Protocol::SetRequestedSpeed( int speed ){
  debugout( "BEGIN:Protocol::SetRequestedSpeed( int speed )" );

  if ( ( speed == 38400 ) || ( speed == 19200 ) || ( speed == 9600 ) ) {
    m_speed = speed;
  } else {
    cerr << "ERROR: speed NOT supported!!!!!" << endl;
    throw ProtocolException( "Protocol::SetRequestedSpeed : speed NOT supported", 2003 );
  }
  // set the I/O speed to default until phase 1 is over
  m_com.SetInputSpeed( 9600 );
  m_com.SetOutputSpeed( 9600 );
  // set back to 0
  m_actual_recieve_No = m_actual_send_No = 0;

  debugout( "END:Protocol::SetRequestedSpeed( int speed )" );
}

/**
   * toggles the DTR line to wake up the PV.
   */
void CasioPV::Protocol::WakeUp(){
  debugout( "BEGIN:Protocol::WakeUp()" );

  int count = 0;
  do {
    m_com.ClearDTR();                     // DTR OFF
    usleep( 30000 );
    m_com.SetDTR();                       // DTR ON
    usleep( 30000 );
    if ( count == 20 )  throw ProtocolException( "Protocol::WakeUp : PV didn't wake up", 2002 );
    count++;
  } while ( !m_com.CheckForNextByte() );

  debugout( "END:Protocol::WakeUp()" );
}

/**
   * Every packet type has its own start byte which identify it. This method reads this byte from the serial line.
   * @param checklink defaults to true.
   * If this is set to true RecieveOrder checks recieved packet headers for a link packet which cancels the communication
   * and throws an exception if one is recieved.
   * @return recieved packet type
   * @exception ProtocolException
   */
unsigned int CasioPV::Protocol::RecieveOrder( bool checklink ){
//  debugout( "BEGIN:Protocol::RecieveOrder()" );

  unsigned int order = 0;
  try {
    order = m_com.ReadByte();//1000);
  } catch ( SerialException e ) {
    if ( e.getErrorCode() == 3002 ) throw ProtocolException( "Protocol::RecieveLinkPacket : timeout", 2010 );
    if ( e.getErrorCode() == 3006 ) order = CAN;
  }
  switch ( order ) {
    case SOH  : debugout( "Command packet" );
                break;
    case STX  : debugout( "Data packet" );
                break;
    case ACK  : debugout( "ACK: Positive response packet" );
                break;
    case NAK  : debugout( "NAK: Negative response packet" );
                break;
    case CAN  : debugout( "Link packet" );
                if ( checklink ) {
                  unsigned int linktype = RecieveLinkPacket( true );
                  cerr << "Protocol::RecieveOrder : link packet : " << linktype << endl;
                  throw ProtocolException( "Protocol::RecieveOrder : link packet for break", 2011 );       //check this no response interogation others are already thrown
                }
                break;
    case 'P'    : debugout( "Looks like PVPIN is installed" );
                break;
    default : throw ProtocolException( "Protocol::RecieveOrder : unknown command : ", 2009 );// + (char)order, 2009 );
  }

//  debugout( "END:Protocol::RecieveOrder()" );
  return order;
}

/**
   * This method waits for the acknowledge of the currently recieved packet.
   * The packet number is stored in m_actual_send_No.
   * @return false in case of a NAK packet and true in case of a ACK packet.
   * @exception ProtocolException
   */
bool CasioPV::Protocol::RecieveACK(){
  debugout( "BEGIN:Protocol::RecieveACK()" );

  unsigned int ro, No;
  bool rc = true;

  ro = RecieveOrder();
  No = RecievePacketNo();

  if ( ro == NAK ) {
    cerr << "ERROR: recieved NAK" << endl;
    m_recievedNAK++;
    if ( m_recievedNAK == 5 ) throw ProtocolException( "Protocol::RecieveACK : recieved to much NAK", 2005 );
    rc = false;
  } else if ( ro == ACK ) {
    m_recievedNAK = 0;
    m_actual_send_No++;
    m_actual_send_No %= 256;
  } else {
    throw ProtocolException( "Protocol::RecieveACK : waited for ACK or NAK but recieved ", 2006 ); //+ (char)ro, 2006 );  // check could also be 2004
  }

  debugout( "END:Protocol::RecieveACK()" );
  return rc;
}

/**
   * This method sends the acknowledge of the recently recieved packet.
   * The packet number is stored in m_actual_recieve_No.
   */
void CasioPV::Protocol::SendACK() {
  debugout( "BEGIN:Protocol::SendACK()" );

  m_com.WriteByte( ACK );
  SendPacketNo( m_actual_recieve_No );
  m_sendNAK = 0;
  debugout( "Send ACK for PacketNo " << dec << m_actual_recieve_No );
  m_actual_recieve_No++;
  m_actual_recieve_No %= 256;

  debugout( "END:Protocol::SendACK()" );
}

/**
   * This method sends the NOT acknowledge for the recently recieved packet.
   * The packet number is stored in m_actual_recieve_No.
   * This may happens if packet is corrupt.
   * @exception ProtocolException
   */
void CasioPV::Protocol::SendNAK() {
  debugout( "BEGIN:Protocol::SendNAK()" );

  m_com.WriteByte( NAK );
  SendPacketNo( m_actual_recieve_No );
  m_sendNAK++;
  if ( m_sendNAK == 5 ) throw ProtocolException( "Protocol::SendNAK : sended to much NAK", 2007 );
  debugout( "Send NAK for PacketNo " << dec << m_actual_recieve_No );

  debugout( "END:Protocol::SendNAK()" );
}

/*************************************
* This part is for phase 1
* to establish the link
**************************************/

/**
   * This method waits for a link packet.
   * @param order_recieved defaults to false. If this is set to true the packet type byte must be checked with @ref RecieveOrder .
   * This is used in @ref RecieveOrder to check for link packets which cancels the communication.
   * @return unsigned int with the recieved link command type.
   * @exception ProtocolException
   */
unsigned int CasioPV::Protocol::RecieveLinkPacket( bool order_recieved ){
  debugout( "BEGIN:Protocol::RecieveLinkPacket( bool order_recieved = false )" );

  unsigned int linkcommand;

  try {
    if ( !order_recieved ) {
      unsigned int ro = RecieveOrder( false );
      if ( ro == 'P' ) {                                                    // if pvpin is active send pin
        string pvpin = m_com.ReadString( 5 );
        if ( pvpin == "VPIN:" ) {
          m_com.WriteString( m_pvpin + "\r\n" );
          ro = RecieveOrder( false );
        }
      }
      if ( ro != CAN ) throw ProtocolException( "Protocol::RecieveLinkPacket : not a link packet!!", 2004 );
    }

    unsigned int response_no = RecievePacketNo();                     // packet number
    if ( !order_recieved && ( response_no != m_actual_recieve_No ) )      // in case of a break packet the packet number is the same as of the recently recieved packet
                throw ProtocolException( "Protocol::RecieveLinkPacket : Recieved wrong packet number", 2008 );

    RecieveXBytes( linkcommand, 4 );                                  // link command

    switch ( linkcommand ) {
      case CALLING_UP :                         // 0x0000
        debugout( "Calling up" );
        break;
      case NO_RESPONSE_INTERROGATION :    // 0x0001
        debugout( "No response interrogation" );
        break;
      case  END_COMMUNICATION :               // 0x0100
        debugout( "End communication" );
        break;
      case BREAK_COMMUNICATION :            // 0x0101
        debugout( "Break data block" );
        throw ProtocolException( "Protocol::RecieveOrder : link packet for break data block", 2011 );
        break;
      case BREAK_COMMAND_BLOCK :            // 0x0021
        debugout( "Break command block" );
        throw ProtocolException( "Protocol::RecieveOrder : link packet for break command block", 2011 );
        break;
      case BREAK_DATA_BLOCK :               // 0x0031
        debugout( "Break data block" );
        throw ProtocolException( "Protocol::RecieveOrder : link packet for break data block", 2011 );
        break;
      default :
        throw ProtocolException( "Protocol::RecieveLinkPacket : recieved unkown link packet !!!!!!!!", 2009);
    }
  } catch (SerialException e) {
    cerr << "ERROR: " << e.getMessage() << endl;
    throw ProtocolException( "Protocol::RecieveLinkPacket : timeout", 2010 );
  }

  debugout( "END:Protocol::RecieveLinkPacket( bool order_recieved = false )" );
  return linkcommand;
}

/**
   * This method sends a link packet.
   * @param type takes an int which contains the type of the link packet
   * look in this header file to see correlatively predefined values
   */
void CasioPV::Protocol::SendLinkPacket( int type ){
  debugout( "BEGIN:Protocol::SendLinkPacket( int )" );

  m_com.WriteByte( CAN );                                           // link packet
  SendPacketNo( m_actual_send_No );                                 // packet number
  switch ( type ) {
    case CALLING_UP :                       // 0x0000
              m_com.WriteString( "0000" );                              // calling up
              break;
    case NO_RESPONSE_INTERROGATION :  // 0x0001
              m_com.WriteString( "0001" );                              // no resonse interrogation
              break;
    case END_COMMUNICATION :              // 0x0100
              m_com.WriteString( "0100" );                              // end communication
              break;
    case BREAK_COMMUNICATION :          // 0x0101
              m_com.WriteString( "0101" );                              // break communication
              break;
    case BREAK_COMMAND_BLOCK :          // 0x0021
              m_com.WriteString( "0021" );                              // break command block
              break;
    case BREAK_DATA_BLOCK :             // 0x0031
              m_com.WriteString( "0031" );                              // break data block
              break;
  }

  debugout( "END:Protocol::SendLinkPacket( int )" );
}

/**
   * This method waits for a command packet.
   * @param order_recieved defaults to false. If this is set to true the packet type byte must be checked with @ref RecieveOrder .
   * @return false in the case that the checksum is not correct and true else.
   * @exception ProtocolException
   */
bool CasioPV::Protocol::RecieveCommandPacket( unsigned int& commandtype, bool order_recieved ) {
  debugout( "BEGIN:Protocol::RecieveCommandPacket( unsigned int&, bool order_recieved=false )" );

  string bytes = "";
  unsigned int NoOfBytes = 0;
  try {
    if ( !order_recieved && RecieveOrder() != SOH ) throw ProtocolException( "Protocol::RecieveCommandPacket : not a command packet!!", 2004);

    unsigned int response_no = RecievePacketNo();                       // PacketNo
    if ( response_no != m_actual_recieve_No ) throw ProtocolException( "Protocol::RecieveCommandPacket : Recieved wrong packet number!!!!!!!!", 2008 );

    unsigned int check = RecieveXBytes( NoOfBytes, 2 );                   // Number of bytes

    check += RecieveXBytes( commandtype, 4 );                         // command

    check += RecieveXBytes( bytes, NoOfBytes - 4 );                       // data

    string checksum = m_com.ReadString( 2 );                            // checksum

    check--;
    if ( (IntToHex(((int)check/16 ^ 0xff)%16) != checksum[0]) ||
       (IntToHex(((check%16) ^ 0xff)%16) != checksum[1]) ) {
          cerr << "ERROR: Checksum not correct !!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
          if ( commandtype == USER_ID_EXCHANGE_R ) return false; // rc = false; this is cause sometimes there seems to be a problem with the checksum of the userid so ignore checksum
    }// else { //if the USER_ID_EXCHANGE_R problem is solved

    switch ( commandtype ) {
      case PHASE_TRANSITION_COMMAND :             // 0x8100
        debugout( "Command waiting for result" );
        debugout( "Phase transition command: Parameter = Transit host" );
        debugout( "Transit host = " << bytes );
        break;
      case PROTOCOL_LEVEL_EXCHANGE :              // 0x8110
        debugout( "Command waiting for result" );
        debugout( "Protocol level exchange: Parameter = Level info" );
        debugout( "Level info = " << bytes );
        break;
      case USER_ID_EXCHANGE :                         // 0x8111
        debugout( "Command waiting for result" );
        debugout( "User ID exchange: Parameter = UserID" );
        debugout( "UserID = " << bytes );
        m_inSecretArea = ( bytes[3] == '1' );
        m_modelcode.assign( bytes, 0, 3 );
        m_username.assign( bytes, 4, 15 );
        m_optionalcode.assign( bytes, 16, 39 );
//        m_userid = bytes;
        break;
      case COMMUNICATION_SPEED_SETTING :          // 0x8112
        {debugout( "Command waiting for result" );
        debugout( "Communication Speed Setting: Parameter = Com. speed" );
        debugout( "Com. speed = " << bytes );
        unsigned int supported = 0;
        for ( unsigned int i = 0; i < 12; i++ ) {
          if ( bytes[i] != '0' ) {
            supported += 16^i;                     // !!!!not tested
          }
        }}
        break;
      case ADD_DATA :                                   // 0x0010
        debugout( "Command waiting for result" );
        debugout( "Add data" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case MODIFY_DATA :                                // 0x0011
        debugout( "Command waiting for result" );
        debugout( "Modify data" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case APPEND_REGISTRATION :                      // 0x8010
        debugout( "Command waiting for result" );
        debugout( "Append registration" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case CHANGE_USER_ID :                           // 0x801f
        debugout( "Command waiting for result" );
        debugout( "Change user id" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case DELETE_DATA :                                // 0x8020
        debugout( "Command waiting for result" );
        debugout( "Append registration" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case SEND_MODIFIED_DATA_REQUEST :           // 0x8030
        debugout( "Command waiting for result" );
        debugout( "Send modified data request" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case SEND_NEW_DATA_REQUEST :                // 0x8031
        debugout( "Command waiting for result" );
        debugout( "Send new data request" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case DATA_SEND_REQUEST :                      // 0x8032
        debugout( "Command waiting for result" );
        debugout( "Data send request" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case CHECK_FOR_MODIFIED_DATA :                // 0x8040
        debugout( "Command waiting for result" );
        debugout( "Check for modified data" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case NUMBER_OF_DATA_REQUEST :               // 0x8041
        debugout( "Command waiting for result" );
        debugout( "Number of data request" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case PROTOCOL_LEVEL_EXCHANGE_R :            // 0x8910
        debugout( "Result command" );
        debugout( "Protocol level exchange" );
        debugout( "Level info = " << bytes );
        break;
      case USER_ID_EXCHANGE_R :                     // 0x8911
        debugout( "Result command" );
        debugout( "User ID exchange" );
        debugout( "UserID = " << bytes );
        break;
      case COMMUNICATION_SPEED_SETTING_R :        // 0x8912
        debugout( "Result command"  );
        debugout( "Communication speed setting" );
        debugout( "Com. speed = " << bytes );
        break;
      case PHASE_TRANSITION_COMMAND_R :           // 0x0900
        debugout( "Result command" );
        debugout( "Phase transition command" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      case APPEND_REGISTRATION_R :                  // 0x8810   // should be used for append and add data, but is only used by add data
        debugout( "Result command" );
        debugout( "Append registration" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case MODIFY_DATA_R :                              // 0x8811
        debugout( "Result command" );
        debugout( "Modify data" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case CHANGE_USER_ID_R :                         // 0x881f
        debugout( "Result command" );
        debugout( "Change user id" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case DELETE_DATA_R :                              // 0x8820
        debugout( "Result command" );
        debugout( "Delete data" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case SEND_MODIFIED_DATA_REQUEST_R :       // 0x8830
        debugout( "Result command" );
        debugout( "Send modified data request" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case SEND_NEW_DATA_REQUEST_R :              // 0x8831
        debugout( "Result command" );
        debugout( "Send new data request" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case DATA_SEND_REQUEST_R :                    // 0x8832
        debugout( "Result command" );
        debugout( "Data send request" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case CHECK_FOR_MODIFIED_DATA_R :              // 0x8840
        debugout( "Result command" );
        debugout( "Check for modified data" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case NUMBER_OF_DATA_REQUEST_R :           // 0x8841
        debugout( "Result command" );
        debugout( "Number of data request" );
        if ( bytes != "0000" ) throw ProtocolException( "Protocol::RecieveCommandPacket : command was not success full. resultcode: "+bytes, 2013);
        break;
      case START_DATA_BLOCK :                       // 0x0200
        debugout( "Start data block" );
        break;
      case END_DATA_BLOCK :                           // 0x0201
        debugout( "End data block" );
        break;
      case DATA_BLOCK_CHECK :                       // 0x0202
        debugout( "Data block check" );
        cerr << "Not implemented!!!!!!!!!" << endl;
        break;
      default :
        cerr << "ERROR: Unknown Command : " << hex << commandtype << dec << endl;
        throw ProtocolException( "Protocol::RecieveCommandPacket : Unknown Command", 2004 );
    }

  } catch ( SerialException e ) {
    cerr << "ERROR: " << e.getMessage() << endl;
    throw ProtocolException( "Protocol::RecieveCommandPacket : timeout", 2010 );
  }

  debugout( "END:Protocol::RecieveCommandPacket( unsigned int&, bool order_recieved=false )" );
  return true;
}

/**
   * This method sends a command packet.
   * @param type takes a int which contains the type of the command packet
   * @param DataCondition specifies the ModeCode of the data entry
   * @param DataOrder specifies the nummer of the data entry
   * look in this header file to see correlatively predefined values
   */
void CasioPV::Protocol::SendCommandPacket( unsigned int type, unsigned int DataCondition, unsigned int DataOrder ){
  debugout( "BEGIN:Protocol::SendCommandPacket( int, int, int )" );

  m_com.WriteByte( SOH );                                           // command packet
  SendPacketNo( m_actual_send_No );                                 // PacketNo

  unsigned int checksum;

  switch ( type ) {
    // command packets
    // Negotiation Commands
    // Protocol level exchange
    case PROTOCOL_LEVEL_EXCHANGE :                            // 0x8110
              debugout( "Protocol level exchange" );
              m_com.WriteString( "0C" );                                // No of bytes
              m_com.WriteString( "8110" );                              // Protocol level exchange
              m_com.WriteString( "20001000" );                          // level for PC application
              m_com.WriteString( "40" );                                // Checksum
              break;
    // User ID exchange
    case USER_ID_EXCHANGE :                                       // 0x8111
              debugout( "User ID exchange" );
              m_com.WriteString( "2C" );                                // No of bytes
              m_com.WriteString( "8111" );                              // User ID exchange
              checksum = 0x32 + 0x43 + 0x38 + 0x31 + 0x31 + 0x31;
              checksum += SendXBytes( m_userid  );                    // Send ID String    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! send the same like in the response
              SendChecksumFor( checksum );                          // Checksum
              break;
    // Communication speed setting
    case COMMUNICATION_SPEED_SETTING :                        // 0x8112
              debugout( "Communication speed setting" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "8112" );                              // Communication speed setting
              // Possible speed settings
              // 230400,153600,115200, 76800, 57600, 38400, 28800, 19200, 14400, 9600, 7200, 4800
              // supported speed settings: 57600, 38400, 19200, 9600
              m_com.WriteString( "000021111000" );                    // Send speed
              m_com.WriteString( "8D" );                                // Checksum
              break;
    // Ordinal Commands
    // Add data
    case ADD_DATA :                                                 // 0x0010
              debugout( "Add data" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "0010" );                              // Add data
              checksum = 0x31 + 0x30 + 0x30 + 0x30 + 0x31 + 0x30;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              checksum += SendXBytes( DataOrder, 6 );               // Data order
              SendChecksumFor( checksum );
              break;
    // Modify data
    case MODIFY_DATA :                                              // 0x0011
              debugout( "Modify data" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "0011" );                              // Modify data
              checksum = 0x31 + 0x30 + 0x30 + 0x30 + 0x31 + 0x31;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              checksum += SendXBytes( DataOrder, 6 );               // Data order
              SendChecksumFor( checksum );
              break;
    // Append registration
    case APPEND_REGISTRATION :                                    // 0x8010
              debugout( "Append registration" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "8010" );                              // Append registration
              checksum = 0x31 + 0x30 + 0x38 + 0x30 + 0x31 + 0x30;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              checksum += 6 * 0x46;
              m_com.WriteString( "FFFFFF" );                          // Option (fixed value 0xFFFFFF)
              SendChecksumFor( checksum );
              break;
    // Change User ID
    case CHANGE_USER_ID :                                         // 0x801f
              debugout( "Change User ID" );
              m_com.WriteString( "1C" );                                // No of bytes
              m_com.WriteString( "801F" );                              // Change User ID
              checksum = 0x31 + 0x43 + 0x38 + 0x30 + 0x31 + 0x46;
              checksum += SendXBytes( m_optionalcode );             // Send UserID String
              SendChecksumFor( checksum );                          // Checksum
              break;
    // Delete data
    case DELETE_DATA :                                              // 0x8020
              debugout( "Delete data" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "8020" );                              // Delete data
              checksum = 0x31 + 0x30 + 0x38 + 0x30 + 0x32 + 0x30;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              checksum += SendXBytes( DataOrder, 6 );               // Data order
              SendChecksumFor( checksum );
              break;
    // Send modified data request
    case SEND_MODIFIED_DATA_REQUEST :                         // 0x8030
              debugout( "Send modified data request" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "8030" );                              // Send modified data request
              checksum = 0x31 + 0x30 + 0x38 + 0x30 + 0x33 + 0x30;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              checksum += SendXBytes( DataOrder, 6 );               // Data order
              SendChecksumFor( checksum );
              break;
    // Send new data request
    case SEND_NEW_DATA_REQUEST :                              // 0x8031
              debugout( "Send new data request" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "8031" );                              // Send new data request
              checksum = 0x31 + 0x30 + 0x38 + 0x30 + 0x33 + 0x31;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              checksum += SendXBytes( DataOrder, 6 );               // Data order
              SendChecksumFor( checksum );
              break;
    // Data send request
    case DATA_SEND_REQUEST :                                    // 0x8032
              debugout( "Data send request" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "8032" );                              // Data send request
              checksum = 0x31 + 0x30 + 0x38 + 0x30 + 0x33 + 0x32;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              checksum += SendXBytes( DataOrder, 6 );               // Data order
              SendChecksumFor( checksum );
              break;
    // Check for modified data
    case CHECK_FOR_MODIFIED_DATA :                              // 0x8040
              debugout( "Check for modified data" );
              m_com.WriteString( "0A" );                                // No of bytes
              m_com.WriteString( "8040" );                              // Check for modified data
              checksum = 0x30 + 0x41 + 0x38 + 0x30 + 0x34 + 0x30;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              SendChecksumFor( checksum );
              break;
    // Number of data request
    case NUMBER_OF_DATA_REQUEST :                             // 0x8041
              debugout( "Number of data request" );
              m_com.WriteString( "0A" );                                // No of bytes
              m_com.WriteString( "8041" );                              // Number of data request
              checksum = 0x30 + 0x41 + 0x38 + 0x30 + 0x34 + 0x31;
              checksum += SendXBytes( DataCondition, 6 );             // Data condition
              SendChecksumFor( checksum );
              break;
    // response packets
    // Negotiation commands
    // Protocol level exchange response
    case PROTOCOL_LEVEL_EXCHANGE_R :                          // 0x8910
              debugout( "Protocol level exchange response" );
              m_com.WriteString( "0C" );                                // No of bytes
              m_com.WriteString( "8910" );                              // Protocol level exchange
              m_com.WriteString( "20001000" );                          // level for PC application 20001000
              m_com.WriteString( "38" );                                // Checksum
              break;
    // User ID exchange response
    case USER_ID_EXCHANGE_R :                                   // 0x8911
              debugout( "User ID exchange response" );
              m_com.WriteString( "2C" );                                // No of bytes
              m_com.WriteString( "8911" );                              // User ID exchange
              m_com.WriteString( "À!00000000000000000000000000000000000000" );  // Send UserID String
              m_com.WriteString( "B7" );                                // Checksum
              break;
    // Communication speed setting response
    case COMMUNICATION_SPEED_SETTING_R :                      // 0x8912
              debugout( "Communication speed setting response" );
              m_com.WriteString( "10" );                                // No of bytes
              m_com.WriteString( "8912" );                              // Communication speed setting
              // supported speed settings: 57600, 38400, 19200, 9600
              for (unsigned int i = 12; i>0; i--) {                           // speed
                if ( (i == 8) && ( m_speed == 57600 ) ||
                    (i == 7) && ( m_speed == 38400 ) ||
                    (i == 5) && ( m_speed == 19200 ) ||
                    (i == 3) && ( m_speed == 9600 ) ) {
                  m_com.WriteByte(0x31);
                } else {
                  m_com.WriteByte(0x30);
                }
              }
              m_com.WriteString( "8A" );                                // Checksum
              break;
    // Phase Transition Command response
    case PHASE_TRANSITION_COMMAND_R :                         // 0x0900
              debugout( "Phase Transition Command response to 01" );
              m_com.WriteString( "08" );                                // No of bytes
              m_com.WriteString( "0900" );                              // Phase Transition Command
              m_com.WriteString( "0000" );                              // Send Phase Transition Command
              m_com.WriteString( "0F" );                                // Checksum
              break;
    // Ordinal Commands
    // Modify data response
    case MODIFY_DATA_R :                                            // 0x8811
              debugout( "Modify data response" );
              m_com.WriteString( "08" );                                // No of bytes
              m_com.WriteString( "8811" );                              // Modify data response
              m_com.WriteString( "0000" );                              // Result OK
              m_com.WriteString( "8D" );                                // Checksum
              break;
    // Append registration response
    case APPEND_REGISTRATION_R :                                // 0x8810
              debugout( "Append registration response" );
              m_com.WriteString( "08" );                                // No of bytes
              m_com.WriteString( "8810" );                              // Append registration response
              m_com.WriteString( "0000" );                              // Result OK
              m_com.WriteString( "8D" );                                // Checksum
              break;
    // Delete data response
    case DELETE_DATA_R :                                            // 0x8820
              debugout( "Delete data response" );
              m_com.WriteString( "08" );                                // No of bytes
              m_com.WriteString( "8820" );                              // Delete data response
              m_com.WriteString( "0000" );                              // Result OK
              m_com.WriteString( "06" );                                // Checksum
              break;
    // Data send request response
    case DATA_SEND_REQUEST_R :                                  // 0x8832
              debugout( "Data send request response" );
              m_com.WriteString( "08" );                                // No of bytes
              m_com.WriteString( "8832" );                              // Data send request response
              m_com.WriteString( "0000" );                              // Result OK
              m_com.WriteString( "8A" );                                // Checksum
              break;
    // Number of data request response
    case NUMBER_OF_DATA_REQUEST_R :                         // 0x8841
              debugout( "Number of data request response" );
              m_com.WriteString( "0A" );                                // No of bytes
              m_com.WriteString( "8841" );                              // Number of data request response
              m_com.WriteString( "0000" );                              // Result OK
              m_com.WriteString( "8D" );                                // Checksum
              break;
    // Sub-commands for data block
    // Start Data Block
    case START_DATA_BLOCK :                                     // 0x0200
              debugout( "Start Data Block" );
              m_com.WriteString( "04" );                                // No of bytes
              m_com.WriteString( "0200" );                              // start data block
              m_com.WriteString( "DA" );                                // Checksum
              break;
    // End Data Block
    case END_DATA_BLOCK :                                         // 0x0201
              debugout( "End Data Block" );
              m_com.WriteString( "04" );                                // No of bytes
              m_com.WriteString( "0201" );                              // end data block
              m_com.WriteString( "D9" );                                // Checksum
              break;
    // End Data Block II
    case 0x8201 :                                         // 0x8201
              debugout( "End Data Block II" );
              m_com.WriteString( "04" );                                // No of bytes
              m_com.WriteString( "8201" );                              // end data block
              m_com.WriteString( "D1" );                                // Checksum
              break;
    // Data Block Check
    case DATA_BLOCK_CHECK :                                     // 0x0202
              debugout( "Data Block Check" );
              m_com.WriteString( "04" );                                // No of bytes
              m_com.WriteString( "0202" );                              // data block check
              m_com.WriteString( "D8" );                                // Checksum
              break;
  }
  debugout( "END:Protocol::SendCommandPacket(int, int, int)" );
}

/*************************************
* This part is for phase 2
* to exchange data
**************************************/

/**
   * This method recieves a datapacket and stores it in a @ref datapacket struct.
   * @param packet contains the data, the field code and the continued bit
   * @param order_recieved defaults to false. If this is set to true the packet type byte must be checked with @ref RecieveOrder .
   * @return false in the case that the checksum is not correct and true else.
   * @exception ProtocolException
   */
bool CasioPV::Protocol::RecieveDataPacket( datapacket& packet, bool order_recieved ){
  debugout( "BEGIN:Protocol::RecieveDataPacket( datapacket&, bool order_recieved = false )" );

  bool rc = true;
  try {
    if ( !order_recieved && RecieveOrder() != STX ) throw ProtocolException( "Protocol::RecieveDataPacket :  not a data packet!!", 2004 );

    unsigned int response_no = RecievePacketNo();                     // PacketNo
    if ( response_no != m_actual_recieve_No ) throw ProtocolException( "Protocol::RecieveDataPacket :  Recieved wrong packet number", 2008 );

    unsigned int NoOfBytes = 0;
    unsigned int check = RecieveXBytes( NoOfBytes, 4 );                 // Number of bytes

    check += RecieveXBytes( packet.fieldCode, 5 );                      // field code

    unsigned char continued = m_com.ReadByte();                     // continued flag is the 6th byte of the field code
    packet.continued = ( continued == '1' );
    check += (unsigned int)continued;

    check += RecieveXBytes( packet.data, NoOfBytes - 6 );               // data

    string checksum = m_com.ReadString( 2 );                          // checksum

    debugout( "packet.fieldCode = " << hex << packet.fieldCode << dec );
    debugout( "packet.continued = " << (packet.continued ? "true" : "false") );
    debugout( "packet.data = " << packet.data );

    check--;
    if ( (IntToHex(((int)check/16 ^ 0xff)%16) != checksum[0]) ||
       (IntToHex(((check%16) ^ 0xff)%16) != checksum[1]) ) {
          cerr << "ERROR: Checksum not correct !!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
          rc = false;
    }                                                                                  // what should be done if the checksum is not correct?
    // increment of  actual_recieve_No
    m_actual_recieve_No++;
    m_actual_recieve_No %= 256;

  } catch (SerialException e) {
    cerr << "ERROR: " << e.getMessage() << endl;
    throw ProtocolException( "Protocol::RecieveDataPacket : timeout", 2000); //check define for timeout
  }

  debugout( "END:Protocol::RecieveDataPacket( datapacket&, bool order_recieved = false )" );
  return rc;
}

/**
   * Sends a @ref datapacket to the PV.
   * @param packet contains the data, the field code and the continued bit
   */
void CasioPV::Protocol::SendDataPacket( datapacket& packet ) {
  debugout( "BEGIN:Protocol::SendDataPacket( datapacket& )" );

  unsigned int checksum;

  m_com.WriteByte( STX );                                           // data packet

  SendPacketNo( m_actual_send_No );                                 // PacketNo

  checksum = SendXBytes( packet.data.size()+6, 4 );                   // Number of bytes

  checksum += SendXBytes( packet.fieldCode, 5 );                        // FieldCode

  checksum += SendXBytes( packet.continued, 1 );                      // continue flag

  checksum += SendXBytes( packet.data );                              // data

  SendChecksumFor( checksum );                                      // checksum

  // increment of  actual_send_No
  m_actual_send_No++;
  m_actual_send_No %= 256;

  debugout( "END:Protocol::SendDataPacket( datapacket& )" );
}

/**
   * This method sets the speed of the serial connection to the value which was given by @ref SetRequestedSpeed
   * and waits one second as requested in the docs.
   */
void CasioPV::Protocol::EndPhase1(){
  debugout( "BEGIN:Protocol::EndPhase1()" );

  debugout(  "Link established !! Now setting the new com speed and wait for a second" );
  m_com.SetInputSpeed( m_speed );
  m_com.SetOutputSpeed( m_speed );
  debugout( "com speed set to " << m_speed );
  // have to wait one second as requested in the docs
  sleep(1);

  debugout( "END:Protocol::EndPhase1()" );
}

/**
   * This method returns the actual optional code of the UserID.
   */
string CasioPV::Protocol::GetOptionalCode(){
  return m_optionalcode;
}

/**
   * This method sets the pvpin code which will be sended by @ref RecieveLinkPacket if PVPIN is installed and enabled.
   * Make sure that this is a number which is given as a string.
   */
void CasioPV::Protocol::SetPVPIN( string& pvpin ){
  m_pvpin = pvpin;
}

/**
   * This method changes the optional code of the UserID. With SendCommandPacket(CHANGE_USER_ID)  it can be updated on the PV.
   */
void CasioPV::Protocol::SetOptionalCode( string& optionalcode ){
  if ( optionalcode.length() != 24 ) throw ProtocolException( "Protocol::SetOptionalCode : the optional code must be 24 bytes long", 2012 );
  m_optionalcode = optionalcode;
}

/**
   * This method returns the model code of the actual connected PV. Can only be used after phase 1.
   * @return model code as a string.
   */
string CasioPV::Protocol::GetModelCode(){
  return m_modelcode;
}

/**
   * This method returns if the PV is in the secret area.
   * @return true if the PV is in the secet area else false.
   */
bool CasioPV::Protocol::isInSecretArea(){
  return m_inSecretArea;
}

/**
   * This method returns the com object.
   * @return a reference to the private serial object.
   */
CasioPV::Serial& CasioPV::Protocol::GetSerial(){
  return m_com;
}
