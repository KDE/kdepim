/***************************************************************************
                          pvmail.cpp  -  description
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
#include "pvmail.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


/**
   * Constructor.
   */
CasioPV::PVMail::PVMail( unsigned int modeCode, unsigned int uid ) {
  switch ( modeCode ) {
    case  MAIL_PROVIDER_1_RE:
    case  MAIL_PROVIDER_2_RE:
    case  MAIL_PROVIDER_3_RE:
    case  MAIL_PROVIDER_1_SE:
    case  MAIL_PROVIDER_2_SE:
    case  MAIL_PROVIDER_3_SE:
    case  MAIL_PROVIDER_1_SET:
    case  MAIL_PROVIDER_2_SET:
    case  MAIL_PROVIDER_3_SET:
    case  SMS_RECEIVE:
    case  SMS_SEND:
    case  SMS_SETTING:
          m_modeCode = modeCode;
          m_uid = uid;
          m_state = UNDEFINED;
          break;
    default :
          throw PVDataEntryException( "PVMail::PVMail : tried to set an unsupported ModeCode : " + modeCode, 4001 );
  }
  m_continued = false;
  m_data[MAIL_DATA] = "";
}

/**
   * Destructor.
   */
CasioPV::PVMail::~PVMail(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int CasioPV::PVMail::getModeCode() const{
  return m_modeCode;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVMail::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVMail::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVMail::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVMail::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVMail::getData() const{
  return m_data;
}

/**
   * Getter for mail data.
   * @return Return the mail data in binary format with up to 32KB.
   */
string CasioPV::PVMail::getMailData()
{
  return m_data[MAIL_DATA];
}

/**
   * Setter for mail data.
   * @param value sets the mail data in binary format with up to 32KB.
   * @exception PVDataEntryException
   */
void CasioPV::PVMail::setMailData( string& value )
{
  if ( value.length() > 32768 ) throw PVDataEntryException( "PVMail::setMailData : string longer than 32768 characters", 4002 );
  m_data[MAIL_DATA] = value;
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVMail::setFieldData( datapacket& packet )
{
  if ( packet.fieldCode == MAIL_DATA ) {
    if ( m_continued ) m_data[MAIL_DATA] += packet.data;
    else m_data[MAIL_DATA] = packet.data;
    m_continued =  packet.continued;
  } else {
    throw PVDataEntryException( "PVMail::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of sms is sendable.
   * The field mail data have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool CasioPV::PVMail::isSendable(){
  return ( m_data[MAIL_DATA] !=  "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVMail::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVMail& mail)
{
  out << "-----   PVMail -----" << endl
       << "Mail Data : " << mail.getMailData() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVMail::toXML()
{
  std::stringstream oss;
  oss << "<mail uid='" << getUid() << "' category='"
                        << Utils::getCategoryString(getModeCode())
                         << "' state='" << getState() << "'>" << endl                        
       << "<data>" << getMailData() << "</data>" << endl
      << "</mail>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVMail::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 1)
  {
    throw PVDataEntryException("PVMail::fromXML : invalid XML format", 4006);
  }
  else
  {
    setMailData(vecElem[0]);
  }
}
