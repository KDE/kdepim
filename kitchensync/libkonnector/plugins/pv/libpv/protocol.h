/***************************************************************************
                          protocol.h  -  description
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

// C++ includes
#include <string>
// project includes
#include <serial.h>
#include <datapacket.h>

/**This class manages the sending and receiving of the casio protocol
  *@author Selzer Michael
  */

// This defines are for the link packet
#define CALLING_UP                              0x0000
#define NO_RESPONSE_INTERROGATION       0x0001
#define END_COMMUNICATION                 0x0100
#define BREAK_COMMUNICATION               0x0101
#define BREAK_COMMAND_BLOCK             0x0021
#define BREAK_DATA_BLOCK                    0x0031
// This defines are for the command packet
// Negotiation Commands
#define PHASE_TRANSITION_COMMAND          0x8100
#define PROTOCOL_LEVEL_EXCHANGE         0x8110
#define USER_ID_EXCHANGE                    0x8111
#define COMMUNICATION_SPEED_SETTING     0x8112
#define PHASE_TRANSITION_COMMAND_R      0x0900
#define PROTOCOL_LEVEL_EXCHANGE_R     0x8910
#define USER_ID_EXCHANGE_R                  0x8911
#define COMMUNICATION_SPEED_SETTING_R   0x8912
// Ordinal Commands
#define ADD_DATA                              0x0010
#define MODIFY_DATA                           0x0011
#define APPEND_REGISTRATION               0x8010
#define CHANGE_USER_ID                      0x801f
#define DELETE_DATA                         0x8020
#define SEND_MODIFIED_DATA_REQUEST      0x8030
#define SEND_NEW_DATA_REQUEST           0x8031
#define DATA_SEND_REQUEST                 0x8032
#define CHECK_FOR_MODIFIED_DATA         0x8040
#define NUMBER_OF_DATA_REQUEST          0x8041
#define APPEND_REGISTRATION_R             0x8810  // used for append and add data
#define MODIFY_DATA_R                       0x8811
#define CHANGE_USER_ID_R                    0x881f
#define DELETE_DATA_R                       0x8820
#define SEND_MODIFIED_DATA_REQUEST_R    0x8830
#define SEND_NEW_DATA_REQUEST_R       0x8831
#define DATA_SEND_REQUEST_R               0x8832
#define CHECK_FOR_MODIFIED_DATA_R       0x8840
#define NUMBER_OF_DATA_REQUEST_R        0x8841
#define START_DATA_BLOCK                    0x0200
#define END_DATA_BLOCK                      0x0201
#define DATA_BLOCK_CHECK                    0x0202

#ifdef DEBUG
#define debugout( out ) { std::cout << out << std::endl; }
#else
#define debugout( out ) { /* out */ }
#endif

namespace CasioPV {

class Protocol {
  public:
    /**
       * Standard Constructor.
       */
    Protocol();

    /**
       * Constructor.
       * @param port takes a string which contains the path to the device file
       */
    Protocol( string port );

    /**
       * Constructor.
       * @param com sets a @ref Serial object which will be used for the communication
       */
    Protocol( Serial& com );

    /**
       * Destructor.
       */
    ~Protocol();

    /**
       * This method sets the speed, which will be used after phase 1 (handshak phase).
        * Supported speeds are 38400, 19200 and 9600.
       * @exception ProtocolException
       */
    void SetRequestedSpeed( int speed );

    /**
       * toggles the DTR line to wake up the PV.
       */
    void WakeUp();

    /**
       * Every packet type has its own start byte which identify it. This method reads this byte from the serial line.
       * @param checklink defaults to true.
       * If this is set to true ReceiveOrder checks received packet headers for a link packet which cancels the communication
       * and throws an exception if one is received.
       * @return received packet type
       * @exception ProtocolException
       */
    unsigned int ReceiveOrder( bool checklink = true );

    /**
       * This method waits for the acknowledge of the recently received packet.
       * The packet number is stored in m_actual_send_No.
       * @return false in case of a NAK packet and true in case of a ACK packet.
       * @exception ProtocolException
       */
    bool ReceiveACK();

    /**
       * This method sends the acknowledge for the recently received packet.
       * The packet number is stored in m_actual_receive_No.
       */
    void SendACK();

    /**
       * This method sends the NOT acknowledge for the currently received packet.
       * The packet number is stored in m_actual_receive_No.
       * This may happens if a packet is corrupt.
       * @exception ProtocolException
       */
    void SendNAK();

    /**
       * This method waits for a link packet.
       * @param order_received defaults to false. If this is set to true the packet type byte must be checked with @ref ReceiveOrder .
       * This is used in @ref ReceiveOrder to check for link packets which cancels the communication.
       * @return unsigned int with the received link command type.
       * @exception ProtocolException
       */
    unsigned int ReceiveLinkPacket( bool order_received = false );

    /**
       * This method sends a link packet.
       * @param type takes an int which contains the type of the link packet.
       * Look in this header file to see correlatively predefined values.
       */
    void SendLinkPacket( int type );

    /**
       * This method waits for a command packet.
       * @param order_received defaults to false. If this is set to true the packet type byte must be checked with @ref ReceiveOrder .
       * @return false in the case that the checksum is not correct and true else.
       * @exception ProtocolException
       */
    bool ReceiveCommandPacket( unsigned int& commandtype, bool order_received = false );

    /**
       * This method sends a command packet.
       * @param type takes a int which contains the type of the command packet
       * @param DataCondition specifies the ModeCode of the data entry.
       * @param DataOrder specifies the nummer of the data entry.
       * Look in this header file to see correlatively predefined values.
       */
    void SendCommandPacket( unsigned int type, unsigned int DataCondition = 0, unsigned int DataOrder = 0 );

    /**
       * This method receives a datapacket and stores it in a @ref datapacket struct.
       * @param packet contains the data, the field code and the continued bit
       * @param order_received defaults to false. If this is set to true the packet type byte must be checked with @ref ReceiveOrder .
       * @return false in the case that the checksum is not correct and true else.
       * @exception ProtocolException
       */
    bool ReceiveDataPacket( datapacket& packet, bool order_received = false );

    /**
       * Sends a @ref datapacket to the PV.
       * @param packet contains the data, the field code and the continued bit
       */
    void SendDataPacket( datapacket& packet );

    /**
       * This method sets the speed of the serial connection to the value which was given by @ref SetRequestedSpeed
       * and waits one second as requested in the docs.
       */
    void EndPhase1();

    /**
       * This method returns the actual optional code of the UserID.
       * @return optional code as a string.
       */
    string GetOptionalCode();

    /**
       * This method sets the pvpin code which will be sended by @ref ReceiveLinkPacket if PVPIN is installed and enabled.
       * Make sure that this is a number which is given as a string.
       */
    void SetPVPIN( string& pvpin );

    /**
       * This method changes the optional code of the UserID. With SendCommandPacket(CHANGE_USER_ID)  it can be updated on the PV.
       */
    void SetOptionalCode( string& optionalcode );

    /**
       * This method returns the model code of the actual connected PV. Can only be used after phase 1.
       * @return model code as a string.
       */
    string GetModelCode();

    /**
       * This method returns if the PV is in the secret area.
       * @return true if the PV is in the secet area else false.
       */
    bool isInSecretArea();

    /**
       * This method returns the com object.
       * @return a reference to the private serial object.
       */
    Serial& GetSerial();

  private:
    unsigned int m_speed;
    unsigned int m_actual_receive_No;
    unsigned int m_actual_send_No;
    unsigned int m_receivedNAK;
    unsigned int m_sendNAK;
    string m_userid;
    string m_pvpin;
    /**
       * This is the first part of the user id (4 bytes).
       * Acceptable codes are in a range of 0x20-0xFE. 0xFF means a null code.
       */
    string m_modelcode;
    /**
       * This is the second part of the user id (12 bytes).
       * Acceptable codes are in a range of 0x20-0xFE. 0xFF means a null code.
       */
    string m_username;
    /**
       * This is the third part of the user id (24 bytes).
       * Acceptable codes are in a range of 0x20-0xFE. 0xFF means a null code.
       */
    string m_optionalcode;
    bool m_inSecretArea;
    Serial m_com;

    /**
       * This method converts the int value in the corresponding hex value so that it could be send to the PV.
       * @return converted value hex->int
       */
    inline unsigned int HexToInt(unsigned char value){
      value -= 48;
      if ( value >= 17 ) value -= 7;
      return value;
    }

    /**
       * This method converts the received byte from the PV, which is in hex format, in the corresponding int value.
       * @return converted value int->hex
       */
    inline unsigned char IntToHex(unsigned int value){
      value += 48;
      if ( value >= 58 ) value += 7;
      return value;
    }

     /**
        * This method sends an unsigned int value in count bytes.
        * @return the checksum for the sended bytes.
        */
    inline unsigned int SendXBytes( unsigned int value, int count ){
      unsigned int sendbyte = 0;
      unsigned int checksum = 0;
      string sendstring;

      for ( int i = 0; i<count; i++ ) {
        sendbyte = IntToHex(value%16); value /=16;
        sendstring = (char)sendbyte + sendstring;
        checksum += sendbyte;
      }
      m_com.WriteString( sendstring );
      return checksum;
    }

     /**
        * This method sends a string in count bytes.
        * @return the checksum for the sended bytes.
        */
    inline unsigned int SendXBytes( string& value ){
      unsigned int checksum = 0;

      for ( unsigned int i = 0; i<value.size(); i++ ) {
        checksum += value[i];
      }
      m_com.WriteString( value );
      return checksum;
    }

     /**
        * This method receives an unsigned int with count bytes.
        * @return the checksum for the received bytes.
        */
    inline unsigned int ReceiveXBytes( unsigned int& value, unsigned int count ){
      unsigned int checksum = 0;
      string tmp = "";
      value = 0;

      tmp = m_com.ReadString( count );
      for ( unsigned int i = 0; i<tmp.size(); i++ ) {
        value *= 16; value += HexToInt( tmp[i] );
        checksum += tmp[i];
      }
      return checksum;
    }

     /**
        * This method receives a string with count bytes.
        * @return the checksum for the received bytes.
        */
    inline unsigned int ReceiveXBytes( string& value, unsigned int count ){
      unsigned int checksum = 0;

      value = m_com.ReadString( count );
      for ( unsigned int i = 0; i<count; i++ ) {
        checksum += value[i];
      }
      return checksum;
    }

     /**
        * This method sends the checksum in the hex format
        */
    inline void SendChecksumFor( unsigned int& checksum ){
      checksum--;
      string sendstring;
      sendstring = IntToHex( ((unsigned int)checksum/16 ^ 0xff)%16 ); sendstring += IntToHex( ((checksum%16) ^ 0xff)%16 );
      m_com.WriteString( sendstring );    // Checksum
    }


     /**
        * This method receives the actual count of the packet and store it in m_actual_receive_No.
        * @return the packet number as an unsigned int.
        */
    inline unsigned int ReceivePacketNo(){
      string receiveNo = m_com.ReadString( 2 );
      debugout( "PacketNo = " << dec << HexToInt( receiveNo[0] )*16 + HexToInt( receiveNo[1] ) );
      return HexToInt( receiveNo[0] )*16 + HexToInt( receiveNo[1] );
    }

     /**
        * This method sends the actual count of m_actual_send_No.
        */
    inline void SendPacketNo( unsigned int& value ){
      string sendstring;
      sendstring = IntToHex( ((int)value/16) ); sendstring += IntToHex( value%16 );
      m_com.WriteString( sendstring );
      debugout( "PacketNo = " << hex << IntToHex((int)value/16) << IntToHex(value%16) << dec );
    }

};

}; // namespace CasioPV

#endif
