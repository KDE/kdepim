/***************************************************************************
                          pvcontact.cpp  -  Implementation of the
                                                   class for a contact.
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

//project includes
#include "pvcontact.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"

/**
   * Constructor.
   */
CasioPV::PVContact::PVContact(unsigned int modeCode, unsigned int uid) {
  switch (modeCode) {
    case  CONTACT_PRIVATE:
    case  CONTACT_BUSINESS:
          m_modeCode = modeCode;
          m_uid = uid;
          m_state = UNDEFINED;
          break;
    default :
          throw PVDataEntryException( "PVContact::PVContact : tried to set an unsupported ModeCode : " + modeCode, 4001 );
  }
  m_data[NAME] = "";
  m_data[HOME_NUMBER] = "";
  m_data[BUSINESS_NUMBER] = "";
  m_data[FAX_NUMBER] = "";
  m_data[BUSINESS_FAX] = "";
  m_data[MOBILE] = "";
  m_data[ADDRESS] = "";
  m_data[EMAIL] = "";
  m_data[EMPLOYER] = "";
  m_data[BUSINESS_ADDRESS] = "";
  m_data[DEPARTMENT] = "";
  m_data[POSITION] = "";
  m_data[NOTE] = "";
}

/**
   * Destructor.
   */
CasioPV::PVContact::~PVContact(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int CasioPV::PVContact::getModeCode() const
{
  return m_modeCode;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVContact::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVContact::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVContact::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVContact::getState()
{
  return m_state;
}

string CasioPV::PVContact::getName()
{
  return Utils::ChangeReturnCodeToUnix( m_data[NAME] );
}

string CasioPV::PVContact::getHomeNumber()
{
  return Utils::ChangeReturnCodeToUnix( m_data[HOME_NUMBER] );
}


string CasioPV::PVContact::getBusinessNumber()
{
  return Utils::ChangeReturnCodeToUnix( m_data[BUSINESS_NUMBER] );
}


string CasioPV::PVContact::getFaxNumber()
{
  return Utils::ChangeReturnCodeToUnix( m_data[FAX_NUMBER] );
}


string CasioPV::PVContact::getBusinessFax()
{
  return Utils::ChangeReturnCodeToUnix( m_data[BUSINESS_FAX] );
}


string CasioPV::PVContact::getMobile()
{
  return Utils::ChangeReturnCodeToUnix( m_data[MOBILE] );
}


string CasioPV::PVContact::getAddress()
{
  return Utils::ChangeReturnCodeToUnix( m_data[ADDRESS] );
}


string CasioPV::PVContact::getEmail()
{
  return Utils::ChangeReturnCodeToUnix( m_data[EMAIL] );
}


string CasioPV::PVContact::getEmployer()
{
  return Utils::ChangeReturnCodeToUnix( m_data[EMPLOYER] );
}


string CasioPV::PVContact::getBusinessAddress()
{
  return Utils::ChangeReturnCodeToUnix( m_data[BUSINESS_ADDRESS] );
}


string CasioPV::PVContact::getDepartment()
{
  return Utils::ChangeReturnCodeToUnix( m_data[DEPARTMENT] );
}


string CasioPV::PVContact::getPosition()
{
  return Utils::ChangeReturnCodeToUnix( m_data[POSITION] );
}


string CasioPV::PVContact::getNote()
{
  return Utils::ChangeReturnCodeToUnix( m_data[NOTE] );
}


void CasioPV::PVContact::setName( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setName : string longer than 2036 characters", 4002 );
  m_data[NAME] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setHomeNumber( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setHomeNumber : string longer than 2036 characters", 4002 );
  m_data[HOME_NUMBER] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setBusinessNumber( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setBusinessNumber : string longer than 2036 characters", 4002 );
  m_data[BUSINESS_NUMBER] = Utils::ChangeReturnCodeToPV( value );
}

void CasioPV::PVContact::setFaxNumber( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setFaxNumber : string longer than 2036 characters", 4002 );
  m_data[FAX_NUMBER] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setBusinessFax( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setBusinessFax : string longer than 2036 characters", 4002 );
  m_data[BUSINESS_FAX] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setMobile( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setMobile : string longer than 2036 characters", 4002 );
  m_data[MOBILE] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setAddress( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setAddress : string longer than 2036 characters", 4002 );
  m_data[ADDRESS] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setEmail( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setEmail : string longer than 2036 characters", 4002 );
  m_data[EMAIL] = Utils::ChangeReturnCodeToPV( value );
}

void CasioPV::PVContact::setEmployer( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setEmployer : string longer than 2036 characters", 4002 );
  m_data[EMPLOYER] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setBusinessAddress( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setBusinessAddress : string longer than 2036 characters", 4002 );
  m_data[BUSINESS_ADDRESS] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setDepartment( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setDepartment : string longer than 2036 characters", 4002 );
  m_data[DEPARTMENT] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setPosition( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setPosition : string longer than 2036 characters", 4002 );
  m_data[POSITION] = Utils::ChangeReturnCodeToPV( value );
}


void CasioPV::PVContact::setNote( string& value )
{
  if ( value.length() > 2036 ) throw PVDataEntryException( "PVContact::setNote : string longer than 2036 characters", 4002 );
  m_data[NOTE] = Utils::ChangeReturnCodeToPV( value );
}



/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVContact::getData() const
{
  return m_data;
}



/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVContact::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case NAME:
    case HOME_NUMBER:
    case BUSINESS_NUMBER:
    case FAX_NUMBER:
    case BUSINESS_FAX:
    case MOBILE:
    case ADDRESS:
    case EMAIL:
    case EMPLOYER:
    case BUSINESS_ADDRESS:
    case DEPARTMENT:
    case POSITION:
    case NOTE:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVContact::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of reminder is sendable.
   * The field name have to be set if this have the modecode private contact.
   * If this have the modecode for a business contacte the field employer have to be set, too.
   * @return bool true if all necessary fields are filled else false.
   */
bool CasioPV::PVContact::isSendable(){
  bool rc = false;
  if ( m_modeCode == CONTACT_PRIVATE ) {
    rc = ( m_data[NAME] != "" );
  } else {
    rc = ( m_data[NAME] != "" && m_data[EMPLOYER] != "" );
  }
  return rc;
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret else false
   */
bool CasioPV::PVContact::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVContact& contact)
{
  out << "----- PVContact -----" << endl
       << "Name\t\t: " << contact.getName() << endl
       << "HomeNumber\t: " << contact.getHomeNumber() << endl
       << "BusinessNumber\t: " << contact.getBusinessNumber() << endl
       << "FaxNumber\t: " << contact.getFaxNumber() << endl
       << "BusinessFax\t: " << contact.getBusinessFax() << endl
       << "Mobile\t\t: " << contact.getMobile() << endl
       << "Address\t\t: " << contact.getAddress() << endl
       << "Email\t\t: " << contact.getEmail() << endl
       << "Employer\t: " << contact.getEmployer() << endl
       << "BusinessAddress\t: " << contact.getBusinessAddress() << endl
       << "Department\t: " << contact.getDepartment() << endl
       << "Position\t: " << contact.getPosition() << endl
       << "Note\t\t: " << contact.getNote() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVContact::toXML()
{
  std::stringstream oss;
  oss <<  "<contact uid='" << getUid() << "' category='"
                            << Utils::getCategoryString(getModeCode())
                             << "' state='" << getState() << "'>" << endl
        << "<name>" << getName() << "</name>" << endl
        << "<homenumber>" << getHomeNumber() << "</homenumber>" << endl
        << "<businessnumber>" << getBusinessNumber() << "</businessnumber>" << endl
        << "<faxnumber>" << getFaxNumber() << "</faxnumber>" << endl
        << "<businessfax>" << getBusinessFax() << "</businessfax>" << endl
        << "<mobile>" << getMobile() << "</mobile>" << endl
        << "<address>" << getAddress() << "</address>" << endl
        << "<email>" << getEmail() << "</email>" << endl
        << "<employer>" << getEmployer() << "</employer>" << endl
        << "<businessaddress>" << getBusinessAddress() << "</businessaddress>" << endl
        << "<department>" << getDepartment() << "</department>" << endl
        << "<position>" << getPosition() << "</position>" << endl
        << "<note>" << getNote() << "</note>" << endl
       << "</contact>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVContact::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 13)
  {
    throw PVDataEntryException("PVContact::fromXML : invalid XML format", 4006);
  }
  else
  {
    setName(vecElem[0]);
    setHomeNumber(vecElem[1]);
    setBusinessNumber(vecElem[2]);
    setFaxNumber(vecElem[3]);
    setBusinessFax(vecElem[4]);
    setMobile(vecElem[5]);
    setAddress(vecElem[6]);
    setEmail(vecElem[7]);
    setEmployer(vecElem[8]);
    setBusinessAddress(vecElem[9]);
    setDepartment(vecElem[10]);
    setPosition(vecElem[11]);
    setNote(vecElem[12]);
  }
}
