/***************************************************************************
                          pvschedule.cpp  -  description
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

//project includes
#include "pvschedule.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


/**
   * Constructor.
   */
CasioPV::PVSchedule::PVSchedule(unsigned int uid) {
  m_uid = uid;
  m_state = UNDEFINED;
  m_data[DATE] = "";
  m_data[START_TIME] = "";
  m_data[END_TIME] = "";
  m_data[ALARM_TIME] = "";
  m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
CasioPV::PVSchedule::~PVSchedule(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int CasioPV::PVSchedule::getModeCode() const{
  return SCHEDULE;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void  CasioPV::PVSchedule::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVSchedule::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVSchedule::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVSchedule::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVSchedule::getData() const{
  return m_data;
}



string CasioPV::PVSchedule::getDate()
{
  return Utils::ChangeDateToUnix(m_data[DATE]);
}

string CasioPV::PVSchedule::getStartTime()
{
  return m_data[START_TIME];
}

string CasioPV::PVSchedule::getEndTime()
{
  return m_data[END_TIME];
}

string CasioPV::PVSchedule::getAlarmTime()
{
  return m_data[ALARM_TIME];
}

string CasioPV::PVSchedule::getDescription()
{
  return Utils::ChangeReturnCodeToUnix( m_data[DESCRIPTION] );
}


void CasioPV::PVSchedule::setDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException( "PVSchedule::setDate : string not in yyyymmdd format", 4004 );
  }
  m_data[DATE] = Utils::ChangeDateToPV(value);
}

void CasioPV::PVSchedule::setStartTime( string& value )
{
  m_data[START_TIME] = value;
}

void CasioPV::PVSchedule::setEndTime( string& value )
{
  m_data[END_TIME] = value;
}

void CasioPV::PVSchedule::setAlarmTime( string& value )
{
  m_data[ALARM_TIME] = value;
}

void CasioPV::PVSchedule::setDescription( string& value )
{
  if ( value.length() > 2046 ) throw PVDataEntryException( "PVSchedule::setDescription : string longer than 2046 characters", 4002 );
  m_data[DESCRIPTION] = Utils::ChangeReturnCodeToPV( value );
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVSchedule::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case DATE:
    case START_TIME:
    case END_TIME:
    case ALARM_TIME:
    case DESCRIPTION:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVSchedule::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of schedule is sendable.
   * The fields date, start time and description have to be set.
   * @return bool true if all necessary fields are filled else false.
   */
bool CasioPV::PVSchedule::isSendable(){
  return ( m_data[DATE] != "" && m_data[START_TIME] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVSchedule::isSecret() {
  return m_isSecret;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVSchedule& schedule)
{
  out << "-----   PVSchedule -----" << endl
       << "Date\t\t: " << schedule.getDate() << endl
       << "AlarmTime\t: " << schedule.getAlarmTime() << endl
       << "StartTime\t: " << schedule.getStartTime() << endl
       << "EndTime\t\t: " << schedule.getEndTime() << endl
       << "Description\t: " << schedule.getDescription() << endl;
  return out;
}

/**
   * Convert the PVContact Entry to an XML string
   */
string CasioPV::PVSchedule::toXML()
{
  std::stringstream oss;
  oss << "<event uid='" << getUid() << "' category='"
                            << Utils::getCategoryString(getModeCode())
                            << "' state='" << getState() << "'>" << endl  
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
void CasioPV::PVSchedule::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 5)
  {
    throw PVDataEntryException("PVSchedule::fromXML : invalid XML format", 4006);
  }
  else
  {
    if (vecElem[0] != "")
    {  
      setDate(vecElem[0]);
    }
    setStartTime(vecElem[1]);
    setEndTime(vecElem[2]);
    setAlarmTime(vecElem[3]);    
    setDescription(vecElem[4]);
  }
}

