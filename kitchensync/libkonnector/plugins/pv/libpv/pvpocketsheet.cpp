/***************************************************************************
                          pvpocketsheet.cpp  -  description
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
#include "pvpocketsheet.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


CasioPV::PVPocketSheet::PVPocketSheet(unsigned int uid) {
  m_uid = uid;
  m_state = UNDEFINED;
  m_data[PS_SHEET_DATA] = "";
  m_data[PS_X_LINE_DATA] = "";
  m_data[PS_Y_LINE_DATA] = "";
  m_data[PS_CELL_DATA] = "";
}

/**
   * Destructor.
   */
CasioPV::PVPocketSheet::~PVPocketSheet(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int  CasioPV::PVPocketSheet::getModeCode() const{
  return POCKET_SHEET_PV;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVPocketSheet::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVPocketSheet::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVPocketSheet::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVPocketSheet::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVPocketSheet::getData() const{
  return m_data;
}


string CasioPV::PVPocketSheet::getSheetData()
{
  return m_data[PS_SHEET_DATA];
}

string CasioPV::PVPocketSheet::getXLineData()
{
  return m_data[PS_X_LINE_DATA];
}

string CasioPV::PVPocketSheet::getYLineData()
{
  return m_data[PS_Y_LINE_DATA];
}

string CasioPV::PVPocketSheet::getCellData()
{
  return m_data[PS_CELL_DATA];
}


void CasioPV::PVPocketSheet::setSheetData( string& value )
{
  m_data[PS_SHEET_DATA] = value;   // convert new line ???
}

void CasioPV::PVPocketSheet::setXLineData( string& value )
{
  m_data[PS_X_LINE_DATA] = value;   // convert new line ???
}

void CasioPV::PVPocketSheet::setYLineData( string& value )
{
  m_data[PS_Y_LINE_DATA] = value;   // convert new line ???
}

void CasioPV::PVPocketSheet::setCellData( string& value )
{
  m_data[PS_CELL_DATA] = Utils::ChangeReturnCodeToPV( value );     // convert new line ???
}


/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVPocketSheet::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case PS_SHEET_DATA:
    case PS_X_LINE_DATA:
    case PS_Y_LINE_DATA:
    case PS_CELL_DATA:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVPocketSheet::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4001 );
  }
}

/**
   * Checks if this instance of  pocketsheet is sendable.
   * The fields xxxxxxxxxxxxxx have to be set.
   * @return bool true if all necessary fields are filled else false.
   */
bool CasioPV::PVPocketSheet::isSendable(){
  return true;
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret false else
   */
bool CasioPV::PVPocketSheet::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVPocketSheet& pocketsheet)
{
  out << "-----   PVPocketSheet -----" << endl
       << "XLineData\t: " << pocketsheet.getXLineData() << endl
       << "YLineData\t: " << pocketsheet.getYLineData() << endl
       << "CellData\t: " << pocketsheet.getCellData() << endl
       << "SheetData\t: " << pocketsheet.getSheetData() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVPocketSheet::toXML()
{
  std::stringstream oss;
  oss << "<pocketsheet uid='" << getUid()
                               << "' state='" << getState() << "'>" << endl  
       << "<xlinedata>" << getXLineData() << "</xlinedata>" << endl
       << "<ylinedata>" << getYLineData() << "</ylinedata>" << endl
       << "<celldata>" << getCellData() << "</celldata>" << endl
       << "<sheetdata>" << getSheetData() << "</sheetdata>" << endl
      << "</pocketsheet>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVPocketSheet::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 4)
  {
    throw PVDataEntryException("PVPocketSheet::fromXML : invalid XML format", 4006);
  }
  else
  {
    setXLineData(vecElem[0]);
    setYLineData(vecElem[1]);
    setCellData(vecElem[2]);
    setSheetData(vecElem[3]);
  }
}

