/***************************************************************************
                          PVContactUntitleduntitled.cpp  -  description
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

//project includes
#include "pvcontactuntitled.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


/**
   * Constructor.
   */
CasioPV::PVContactUntitled::PVContactUntitled( unsigned int modeCode, unsigned int uid ) {
  switch (modeCode) {
    case  CONTACT_UNTITLED_1:
    case  CONTACT_UNTITLED_2:
    case  CONTACT_UNTITLED_3:
// only for PV-750
    case  CONTACT_UNTITLED_4:
    case  CONTACT_UNTITLED_5:
          m_modeCode = modeCode;
          m_uid = uid;
          m_state = UNDEFINED;
          break;
    default :
          throw PVDataEntryException( "PVContactUntitled::PVContactUntitled : tried to set an unsupported ModeCode : " + modeCode, 4001 );
  }
  m_data[FIELD_1] = "";
  m_data[FIELD_2] = "";
  m_data[FIELD_3] = "";
  m_data[FIELD_4] = "";
  m_data[FIELD_5] = "";
  m_data[FIELD_6] = "";
  m_data[FIELD_7] = "";
  m_data[FIELD_8] = "";
  m_data[FIELD_9] = "";
  m_data[FIELD_10] = "";
  m_data[FIELD_11] = "";
  m_data[FIELD_12] = "";
  m_data[FIELD_13] = "";
}

/**
   * Destructor.
   */
CasioPV::PVContactUntitled::~PVContactUntitled(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int CasioPV::PVContactUntitled::getModeCode() const
{
  return m_modeCode;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVContactUntitled::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVContactUntitled::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVContactUntitled::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVContactUntitled::getState()
{
  return m_state;
}

string CasioPV::PVContactUntitled::getField1()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_1] );
}


string CasioPV::PVContactUntitled::getField2()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_2] );
}


string CasioPV::PVContactUntitled::getField3()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_3] );
}


string CasioPV::PVContactUntitled::getField4()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_4] );
}


string CasioPV::PVContactUntitled::getField5()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_5] );
}


string CasioPV::PVContactUntitled::getField6()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_6] );
}


string CasioPV::PVContactUntitled::getField7()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_7] );
}


string CasioPV::PVContactUntitled::getField8()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_8] );
}


string CasioPV::PVContactUntitled::getField9()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_9] );
}


string CasioPV::PVContactUntitled::getField10()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_10] );
}


string CasioPV::PVContactUntitled::getField11()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_11] );
}


string CasioPV::PVContactUntitled::getField12()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_12] );
}


string CasioPV::PVContactUntitled::getField13()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FIELD_13] );
}


void CasioPV::PVContactUntitled::setField1( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField1 : string longer than 2036 characters", 4002 );
  m_data[FIELD_1] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField2( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField2 : string longer than 2036 characters", 4002 );
  m_data[FIELD_2] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField3( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField3 : string longer than 2036 characters", 4002 );
  m_data[FIELD_3] = Utils::ChangeReturnCodeToPV( value );
}

void CasioPV::PVContactUntitled::setField4( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField4 : string longer than 2036 characters", 4002 );
  m_data[FIELD_4] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField5( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField5 : string longer than 2036 characters", 4002 );
  m_data[FIELD_5] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField6( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField6 : string longer than 2036 characters", 4002 );
  m_data[FIELD_6] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField7( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField7 : string longer than 2036 characters", 4002 );
  m_data[FIELD_7] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField8( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField8 : string longer than 2036 characters", 4002 );
  m_data[FIELD_8] = Utils::ChangeReturnCodeToPV( value );
}

void CasioPV::PVContactUntitled::setField9( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField9 : string longer than 2036 characters", 4002 );
  m_data[FIELD_9] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField10( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField10 : string longer than 2036 characters", 4002 );
  m_data[FIELD_10] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField11( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField11 : string longer than 2036 characters", 4002 );
  m_data[FIELD_11] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField12( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField12 : string longer than 2036 characters", 4002 );
  m_data[FIELD_12] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContactUntitled::setField13( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContactUntitled::setField13 : string longer than 2036 characters", 4002 );
  m_data[FIELD_13] = Utils::ChangeReturnCodeToPV( value );
}



/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVContactUntitled::getData() const
{
  return m_data;
}



/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVContactUntitled::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case FIELD_1:
    case FIELD_2:
    case FIELD_3:
    case FIELD_4:
    case FIELD_5:
    case FIELD_6:
    case FIELD_7:
    case FIELD_8:
    case FIELD_9:
    case FIELD_10:
    case FIELD_11:
    case FIELD_12:
    case FIELD_13:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVContactUntitled::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of reminder is sendable.
   * The field name have to be set if this have the modecode private contact.
   * If this have the modecode for a business contacte the field employer have to be set, too.
   * @return bool true if all nessecary fields are filled else false.
   */
bool CasioPV::PVContactUntitled::isSendable(){
  return ( m_data[FIELD_1] != "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret else false
   */
bool CasioPV::PVContactUntitled::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVContactUntitled& contact)
{
  out << "----- PVContactUntitled -----" << endl
       << "Field1\t\t: " << contact.getField1() << endl
       << "Field2\t\t: " << contact.getField2() << endl
       << "Field3\t\t: " << contact.getField3() << endl
       << "Field4\t\t: " << contact.getField4() << endl
       << "Field5\t\t: " << contact.getField5() << endl
       << "Field6\t\t: " << contact.getField6() << endl
       << "Field7\t\t: " << contact.getField7() << endl
       << "Field8\t\t: " << contact.getField8() << endl
       << "Field9\t\t: " << contact.getField9() << endl
       << "Field10\t\t: " << contact.getField10() << endl
       << "Field11\t\t: " << contact.getField11() << endl
       << "Field12\t\t: " << contact.getField12() << endl
       << "Field13\t\t: " << contact.getField13() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVContactUntitled::toXML()
{
  std::stringstream oss;
  oss <<  "<contact uid='" << getUid() << "' category='"
                            << Utils::getCategoryString(getModeCode())
                             << "' state='" << getState() << "'>" << endl
       << "<field1>" << getField1() << "</field1>" << endl
       << "<field2>" << getField2() << "</field2>" << endl
       << "<field3>" << getField3() << "</field3>" << endl
       << "<field4>" << getField4() << "</field4>" << endl
       << "<field5>" << getField5() << "</field5>" << endl
       << "<field6>" << getField6() << "</field6>" << endl
       << "<field7>" << getField7() << "</field7>" << endl
       << "<field8>" << getField8() << "</field8>" << endl
       << "<field9>" << getField9() << "</field9>" << endl
       << "<field10>" << getField10() << "</field10>" << endl
       << "<field11>" << getField11() << "</field11>" << endl
       << "<field12>" << getField12() << "</field12>" << endl
       << "<field13>" << getField13() << "</field13>" << endl
      << "</contact>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVContactUntitled::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 13)
  {
    throw PVDataEntryException("PVContactUntitled::fromXML : invalid XML format", 4006);
  }
  else
  {
    setField1(vecElem[0]);
    setField2(vecElem[1]);
    setField3(vecElem[2]);
    setField4(vecElem[3]);
    setField5(vecElem[4]);
    setField6(vecElem[5]);
    setField7(vecElem[6]);
    setField8(vecElem[7]);
    setField9(vecElem[8]);
    setField10(vecElem[9]);
    setField11(vecElem[10]);
    setField12(vecElem[11]);
    setField13(vecElem[12]);
  }
}
