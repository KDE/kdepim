/***************************************************************************
                          pvmultidate.cpp  -  description
                             -------------------
    begin                : Mon Jul 22 2002
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
#include "pvmultidate.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


/**
   * Constructor.
   */
CasioPV::PVMultiDate::PVMultiDate(unsigned int uid) {
  m_uid = uid;
  m_state = UNDEFINED;
  m_data[DATE] = "";
  m_data[END_DATE] = "";
  m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
CasioPV::PVMultiDate::~PVMultiDate(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int  CasioPV::PVMultiDate::getModeCode() const{
  return SCHEDULE_MULTI_DATE;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVMultiDate::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVMultiDate::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVMultiDate::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVMultiDate::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVMultiDate::getData() const{
  return m_data;
}

string CasioPV::PVMultiDate::getDate()
{
  return Utils::ChangeDateToUnix(m_data[DATE]);
}

string CasioPV::PVMultiDate::getEndDate()
{
  return Utils::ChangeDateToUnix(m_data[END_DATE]);
}

string CasioPV::PVMultiDate::getDescription()
{
  return Utils::ChangeReturnCodeToUnix( m_data[DESCRIPTION] );
}

void CasioPV::PVMultiDate::setDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException( "PVMultiDate::setDate : string not in yyyymmdd format", 4004 );
  }
  m_data[DATE] = Utils::ChangeDateToPV(value);
}

void CasioPV::PVMultiDate::setEndDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException( "PVMultiDate::setEndDate : string not in yyyymmdd format", 4004 );
  }
  m_data[END_DATE] = Utils::ChangeDateToPV(value);
}

void CasioPV::PVMultiDate::setDescription( string& value )
{
  if ( value.length() > 2046 ) throw PVDataEntryException( "PVMultiDate::setDescription : string longer than 2046 characters", 4002 );
  m_data[DESCRIPTION] = Utils::ChangeReturnCodeToPV( value );
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVMultiDate::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case DATE:
    case END_DATE:
    case DESCRIPTION:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVMultiDate::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of mulitdate is sendable.
   * The fields date and description have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool CasioPV::PVMultiDate::isSendable(){
  return ( m_data[DATE] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVMultiDate::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVMultiDate& multidate)
{
  out << "-----   PVReminder -----" << endl
       << "Date\t\t: " << multidate.getDate() << endl
       << "EndDate\t\t: " << multidate.getEndDate() << endl
       << "Description\t: " << multidate.getDescription() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVMultiDate::toXML()
{
  std::stringstream oss;
  oss << "<event uid='" << getUid() << "' category='"
                            << Utils::getCategoryString(getModeCode())
                             << "' state='" << getState() << "'>" << endl  
       << "<date>" << getDate() << "</date>" << endl
       << "<enddate>" << getEndDate() << "</enddate>" << endl
       << "<description>" << getDescription() << "</description>" << endl
      << "</event>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVMultiDate::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 3)
  {
    throw PVDataEntryException("PVMultiDate::fromXML : invalid XML format", 4006);
  }
  else
  {
    if (vecElem[0] != "")
    {
      setDate(vecElem[0]);
    }
    if (vecElem[0] != "")
    {   
      setEndDate(vecElem[1]);
    }
    setDescription(vecElem[2]);
  }
}
