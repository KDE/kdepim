/***************************************************************************
                          pvmemo.cpp  -  description
                             -------------------
    begin                : Mit Jul 10 2002
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
#include "pvmemo.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


/**
   * Constructor.
   */
CasioPV::PVMemo::PVMemo( unsigned int modeCode, unsigned int uid ) {
  switch (modeCode) {
    case  MEMO_1:
    case  MEMO_2:
    case  MEMO_3:
    case  MEMO_4:
    case  MEMO_5:
          m_modeCode = modeCode;
          m_uid = uid;
          m_state = UNDEFINED;
          break;
    default :
          throw PVDataEntryException( "PVMemo::PVMemo : tried to set an unsupported ModeCode : " + modeCode, 4001 );
  }
  m_continued = false;
  m_data[MEMO] = "";
}

/**
   * Destructor.
   */
CasioPV::PVMemo::~PVMemo(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int CasioPV::PVMemo::getModeCode() const{
  return m_modeCode;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVMemo::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVMemo::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVMemo::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVMemo::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVMemo::getData() const{
  return m_data;
}

/**
   * Getter for memo.
   * @return Return the memo in a string with a maximum of 2048 characters
   */
string CasioPV::PVMemo::getMemo()
{
  return Utils::ChangeReturnCodeToUnix( m_data[MEMO] );
}


/**
   * Setter for memo.
   * @param value sets the memo in a string with a maximum of 2048 characters
   * @exception PVDataEntryException
   */
void CasioPV::PVMemo::setMemo( string& value )
{
  if ( value.length() > 2048 ) throw PVDataEntryException( "PVMemo::setMemo : string longer than 2048 characters", 4002 );
  m_data[MEMO] = Utils::ChangeReturnCodeToPV( value );
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVMemo::setFieldData( datapacket& packet )
{
  if ( packet.fieldCode == MEMO ) {
    if ( m_continued ) m_data[MEMO] += packet.data;
    else m_data[MEMO] = packet.data;
    m_continued =  packet.continued;
  } else {
    throw PVDataEntryException( "PVMemo::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of memo is sendable.
   * The field memo have to be set.
   * @return bool true if all nessecary fields are filled else false.
   */
bool CasioPV::PVMemo::isSendable(){
  return ( m_data[MEMO] !=  "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVMemo::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVMemo& memo)
{
  out << "-----   PVMemo -----" << endl
       << "Memo : " << memo.getMemo() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVMemo::toXML()
{
  std::stringstream oss;
  oss << "<memo uid='" << getUid() << "' category='"
                        << Utils::getCategoryString(getModeCode())
                         << "' state='" << getState() << "'>" << endl                        
       << "<text>" << getMemo() << "</text>" << endl
      << "</memo>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVMemo::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 1)
  {
    throw PVDataEntryException("PVMemo::fromXML : invalid XML format", 4006);
  }
  else
  {
    setMemo(vecElem[0]);
  }
}

