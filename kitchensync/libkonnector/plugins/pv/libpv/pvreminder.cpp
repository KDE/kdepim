/***************************************************************************
                          pvreminder.cpp  -  description
                             -------------------
    begin                : Mon Jul 22 2002
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
#include "pvreminder.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


/**
   * Constructor.
   */
CasioPV::PVReminder::PVReminder(unsigned int uid) {
  m_uid = uid;
  m_state = UNDEFINED;
  m_data[TYPE] = "";
  m_data[DATE] = "";
  m_data[START_TIME] = "";
  m_data[END_TIME] = "";
  m_data[ALARM_TIME] = "";
  m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
CasioPV::PVReminder::~PVReminder(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int  CasioPV::PVReminder::getModeCode() const{
  return SCHEDULE_REMINDER;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVReminder::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVReminder::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVReminder::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVReminder::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVReminder::getData() const{
  return m_data;
}



string CasioPV::PVReminder::getType()
{
  return m_data[TYPE];
}

string CasioPV::PVReminder::getDate()
{
  return Utils::ChangeDateToUnix(m_data[DATE]);
}

string CasioPV::PVReminder::getStartTime()
{
  return m_data[START_TIME];
}

string CasioPV::PVReminder::getEndTime()
{
  return m_data[END_TIME];
}

string CasioPV::PVReminder::getAlarmTime()
{
  return m_data[ALARM_TIME];
}

string CasioPV::PVReminder::getDescription()
{
  return Utils::ChangeReturnCodeToUnix( m_data[DESCRIPTION] );
}


void CasioPV::PVReminder::setType( string& value )
{
  m_data[TYPE] = value;
}

void CasioPV::PVReminder::setDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException( "PVReminder::setDate : string not in yyyymmdd format", 4004 );
  }
  m_data[DATE] = Utils::ChangeDateToPV(value);
}

void CasioPV::PVReminder::setStartTime( string& value )
{
  m_data[START_TIME] = value;
}

void CasioPV::PVReminder::setEndTime( string& value )
{
  m_data[END_TIME] = value;
}

void CasioPV::PVReminder::setAlarmTime( string& value )
{
  m_data[ALARM_TIME] = value;
}

void CasioPV::PVReminder::setDescription( string& value )
{
  if ( value.length() > 2046 ) throw PVDataEntryException( "PVReminder::setDescription : string longer than 2046 characters", 4002 );
  m_data[DESCRIPTION] = Utils::ChangeReturnCodeToPV( value );
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVReminder::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case TYPE:
    case DATE:
    case START_TIME:
    case END_TIME:
    case ALARM_TIME:
    case DESCRIPTION:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVReminder::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of reminder is sendable.
   * The fields date, start time and description have to be set.
   * @return bool true if all necessary fields are filled else false.
   */
bool CasioPV::PVReminder::isSendable(){
  return ( m_data[DATE] != "" && m_data[START_TIME] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVReminder::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVReminder& reminder)
{
  out << "-----   PVReminder -----" << endl
       << "Type\t\t: " << reminder.getType() << endl
       << "Date\t\t: " << reminder.getDate() << endl
       << "AlarmTime\t: " << reminder.getAlarmTime() << endl
       << "StartTime\t: " << reminder.getStartTime() << endl
       << "EndTime\t\t: " << reminder.getEndTime() << endl
       << "Description\t: " << reminder.getDescription() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVReminder::toXML()
{
  std::stringstream oss;
  oss << "<event uid='" << getUid() << "' category='"
                            << Utils::getCategoryString(getModeCode())
                            << "' state='" << getState() << "'>" << endl  
       << "<type>" << getType() << "</type>" << endl
       << "<date>" << getDate() << "</date>" << endl
       << "<starttime>" << getStartTime() << "</starttime>" << endl
       << "<endtime>" << getEndTime() << "</endtime>" << endl
       << "<alarmtime>" << getAlarmTime() << "</alarmtime>" << endl
       << "<description>" << getDescription() << "</description>" << endl
      << "</event>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVReminder::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 6)
  {
    throw PVDataEntryException("PVReminder::fromXML : invalid XML format", 4006);
  }
  else
  {
    setType(vecElem[0]);
    if (vecElem[1] != "")
    {    
      setDate(vecElem[1]);
    }
    setStartTime(vecElem[2]);
    setEndTime(vecElem[3]);
    setAlarmTime(vecElem[4]);    
    setDescription(vecElem[5]);
  }
}


