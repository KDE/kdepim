/***************************************************************************
                          pvquickmemo.cpp  -  description
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
#include "pvquickmemo.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


CasioPV::PVQuickMemo::PVQuickMemo(unsigned int uid) {
  m_uid = uid;
  m_state = UNDEFINED;
  m_continued = false;
  m_data[QM_CATEGORY] = "";
  m_data[QM_BITMAP_DATA] = "";
}

/**
   * Destructor.
   */
CasioPV::PVQuickMemo::~PVQuickMemo(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int  CasioPV::PVQuickMemo::getModeCode() const{
  return QUICK_MEMO;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVQuickMemo::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVQuickMemo::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVQuickMemo::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVQuickMemo::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVQuickMemo::getData() const{
  return m_data;
}


string CasioPV::PVQuickMemo::getCategory()
{
  return m_data[QM_CATEGORY];
}

string CasioPV::PVQuickMemo::getBitmapData()
{
  // This is descript in the docs on page 18 (binary data)
  string tmp  = "";
  bool _1b = false;                // 0x1b is a escape code so the next byte should between 0xa0 and 0xbf
  for ( unsigned int i = 0; i < m_data[QM_BITMAP_DATA].length(); i++) {
    unsigned int  tmpint;
    if (  (int)m_data[QM_BITMAP_DATA][i] < 0 ) tmpint = 0x100 + (int)m_data[QM_BITMAP_DATA][i];
    else tmpint = (int)m_data[QM_BITMAP_DATA][i];

    if ( tmpint != 0x1b ) {
      if ( (tmpint >= 0xa0) && (tmpint < 0xbf) && !(_1b) ) {
        tmp += (char)(tmpint - 0xa0);
      } else {
        tmp += m_data[QM_BITMAP_DATA][i];
        _1b = false;
      }
    } else {
      _1b = true;
    }
  }
  return tmp;//m_data[QM_BITMAP_DATA];
}


void CasioPV::PVQuickMemo::setCategory( string& value )
{
  m_data[QM_CATEGORY] = value;
}

void CasioPV::PVQuickMemo::setBitmapData( string& value )
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!this is untested !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // This is descript in the docs on page 18 (binary data)
  string tmp  = "";
  for ( unsigned int i = 0; i < value.length(); i++) {
    unsigned int  tmpint;
    if (  (int)value[i] < 0 ) tmpint = 0x100 + (int)value[i];
    else tmpint = (int)value[i];

    if ( tmpint <= 0x1f ) {
      tmp += (char)(tmpint + 0xa0);
    } else {
      if ( (tmpint >= 0xa0) && (tmpint <= 0xbf) ) {
        tmp += 0x1b + value[i];
      } else {
        tmp += value[i];
      }
    }
  }
  m_data[QM_BITMAP_DATA] = tmp;
}


/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVQuickMemo::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case QM_CATEGORY:
    case QM_BITMAP_DATA:
      if ( m_continued ) m_data[packet.fieldCode] += packet.data;
      else m_data[packet.fieldCode] = packet.data;
      m_continued = packet.continued;
      break;
    default:
      throw PVDataEntryException( "PVQuickMemo::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of quickmemo is sendable.
   * The field bitmapdata have to be set.
   * @return bool true if all necessary fields are filled else false.
   */
bool CasioPV::PVQuickMemo::isSendable(){
  return ( m_data[QM_BITMAP_DATA] != "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVQuickMemo::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVQuickMemo& quickmemo)
{
/*  out << "-----   PVQuickMemo -----" << endl
       << "Category\t: " << quickmemo.getCategory() << endl
       << "BitmapData\t: " << quickmemo.getBitmapData() << endl
       << "Sorry but this is binary data" << endl;*/
  string tmp = quickmemo.getBitmapData();
  for (unsigned int i = 0; i < tmp.length(); i++) {
    out << tmp[i];
  }
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVQuickMemo::toXML()
{
  std::stringstream oss;
  oss << "<quickmemo uid='" << getUid()
                             << "' state='" << getState() << "'>" << endl  
       << "<data>" << getBitmapData() << "</data>" << endl
      << "</quickmemo>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVQuickMemo::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 1)
  {
    throw PVDataEntryException("PVQuickMemo::fromXML : invalid XML format", 4006);
  }
  else
  {
    setBitmapData(vecElem[0]);
  }
}
