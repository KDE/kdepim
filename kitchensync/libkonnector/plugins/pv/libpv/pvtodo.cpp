/***************************************************************************
                          pvtodo.cpp  -  description
                             -------------------
    begin                : Don Jul 11 2002
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
#include "pvtodo.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


#include <kdebug.h>

/**
   * Constructor.
   */
CasioPV::PVTodo::PVTodo(unsigned int uid) {
  m_uid = uid;
  m_state = UNDEFINED;
  m_data[CHECK] = "";
  m_data[DUE_DATE] = "";
  m_data[ALARM_DATE] = "";
  m_data[ALARM_TIME] = "";
  m_data[CHECK_DATE] = "";
  m_data[PRIORITY] = "";
  m_data[CATEGORY] = "";
  m_data[DESCRIPTION] = "";
}

/**
   * Destructor.
   */
CasioPV::PVTodo::~PVTodo(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int   CasioPV::PVTodo::getModeCode() const{
  return TODO;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVTodo::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVTodo::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVTodo::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVTodo::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVTodo::getData() const{
  return m_data;
}


string CasioPV::PVTodo::getCheck()
{
  return m_data[CHECK];
}

string CasioPV::PVTodo::getDueDate()
{
  return Utils::ChangeDateToUnix(m_data[DUE_DATE]);
}

string CasioPV::PVTodo::getAlarmDate()
{
  return Utils::ChangeDateToUnix(m_data[ALARM_DATE]);
}

string CasioPV::PVTodo::getAlarmTime()
{
  return m_data[ALARM_TIME];
}

string CasioPV::PVTodo::getCheckDate()
{
  return Utils::ChangeDateToUnix(m_data[CHECK_DATE]);
}

string CasioPV::PVTodo::getPriority()
{
  return m_data[PRIORITY];
}

string CasioPV::PVTodo::getCategory()
{
  return m_data[CATEGORY];
}

string CasioPV::PVTodo::getDescription()
{
  return Utils::ChangeReturnCodeToUnix( m_data[DESCRIPTION] );
}


void CasioPV::PVTodo::setCheck( string& value )
{
  if ( value != "0" && value != "1" ) throw PVDataEntryException( "PVTodo::setCheck : string not 0 or 1 ", 4005 );
  m_data[CHECK] = value;
}

void CasioPV::PVTodo::setDueDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException( "PVTodo::setDueDate : string not in yyyymmdd format", 4004 );
  }
  m_data[DUE_DATE] = Utils::ChangeDateToPV(value);
}

void CasioPV::PVTodo::setAlarmDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException("PVTodo::setAlarmDate : string not in yyyymmdd format", 4004 );
  }
  
  m_data[ALARM_DATE] = Utils::ChangeDateToPV(value);
}

void CasioPV::PVTodo::setAlarmTime( string& value )
{
  m_data[ALARM_TIME] = value;
}

void CasioPV::PVTodo::setCheckDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException("PVTodo::setAlarmDate : string not in yyyymmdd format", 4004 );
  }
  m_data[CHECK_DATE] = Utils::ChangeDateToPV(value);
}

void CasioPV::PVTodo::setPriority( string& value )
{
  m_data[PRIORITY] = value;
}

void CasioPV::PVTodo::setCategory( string& value )
{
  m_data[CATEGORY] = value;
}

void CasioPV::PVTodo::setDescription( string& value )
{
  if ( value.length() > 2046 ) throw PVDataEntryException( "PVTodo::setDescription : string longer than 2046 characters", 4002 );
  m_data[DESCRIPTION] = Utils::ChangeReturnCodeToPV(value);
}


/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVTodo::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case CHECK:
    case DUE_DATE:
    case ALARM_DATE:
    case ALARM_TIME:
    case CHECK_DATE:
    case PRIORITY:
    case CATEGORY:
    case DESCRIPTION:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVTodo::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of todo is sendable.
   * The fields due date and description have to be set.
   * @return bool true if all necessary fields are filled. false else
   */
bool CasioPV::PVTodo::isSendable(){
  return ( m_data[DUE_DATE] != "" && m_data[DESCRIPTION] != "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVTodo::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVTodo& todo)
{
  out << "-----   PVTodo -----" << endl
       << "AlarmDate\t: " << todo.getAlarmDate() << endl
       << "AlarmTime\t: " << todo.getAlarmTime() << endl
       << "Category\t: " << todo.getCategory() << endl
       << "Check\t\t: " << todo.getCheck() << endl
       << "CheckDate\t: " << todo.getCheckDate() << endl
       << "DueDate\t\t: " << todo.getDueDate() << endl
       << "Priority\t: " << todo.getPriority() << endl
       << "Description\t: " << todo.getDescription() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVTodo::toXML()
{
  std::stringstream oss;
  oss << "<todo uid='" << getUid()
                        << "' state='" << getState() << "'>" << endl  
       << "<check>" << getCheck() << "</check>" << endl
       << "<duedate>" << getDueDate() << "</duedate>" << endl
       << "<alarmdate>" << getAlarmDate() << "</alarmdate>" << endl
       << "<alarmtime>" << getAlarmTime() << "</alarmtime>" << endl
       << "<checkdate>" << getCheckDate() << "</checkdate>" << endl
       << "<priority>" << getPriority() << "</priority>" << endl
       << "<category>" << getCategory() << "</category>" << endl
       << "<description>" << getDescription() << "</description>" << endl
      << "</todo>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVTodo::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 8)
  {
    throw PVDataEntryException("PVTodo::fromXML : invalid XML format", 4006);
  }
  else
  {
    setCheck(vecElem[0]);
    if (vecElem[1] != "")
    {
      setDueDate(vecElem[1]);
    }    
    if (vecElem[2] != "")
    {
      setAlarmDate(vecElem[2]);
    }
    setAlarmTime(vecElem[3]);
    if (vecElem[4] != "")
    {
      setCheckDate(vecElem[4]);
    }
    setPriority(vecElem[5]);
    setCategory(vecElem[6]);
    setDescription(vecElem[7]);
  }
}
