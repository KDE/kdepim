/***************************************************************************
                          pvexpense.cpp  -  description
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
#include "pvexpense.h"
#include "ModeCode.h"
#include "FieldCode.h"
#include "pvdataentryexception.h"
#include "utils.h"


/**
   * Constructor.
   */
CasioPV::PVExpense::PVExpense(unsigned int uid) {
  m_uid = uid;
  m_state = UNDEFINED;
  m_data[EX_DATE] = "";
  m_data[EX_PAYMENT_TYPE] = "";
  m_data[EX_AMOUNT] = "";
  m_data[EX_EXPENSE_TYPE] = "";
  m_data[EX_NOTE] = "";
}

/**
   * Destructor.
   */
CasioPV::PVExpense::~PVExpense(){
}

/**
   * Getter for the mode code.
   * @return The mode code of the data entry.
   */
unsigned int CasioPV::PVExpense::getModeCode() const{
  return EXPENSE_PV;
}

/**
   * Setter for the uid.
   * @param uid The uid of the data entry.
   */
void CasioPV::PVExpense::setUid(unsigned int uid)
{
  m_uid = uid;
}

/**
   * Getter for the uid.
   * @return The uid of the data entry.
   */
unsigned int CasioPV::PVExpense::getUid() const
{
  return m_uid;
}

/**
   * Setter for the state of an entry.
   * @param state The state of the entry
   */
void CasioPV::PVExpense::setState(unsigned int state)
{
  m_state = state;
}
   
/**
   * Getter for the state of an entry.
   * @return The state of the entry
   */
unsigned int CasioPV::PVExpense::getState()
{
  return m_state;
}

/**
   * Getter for the data.
   * @return Return all of the data.
   */
const map<unsigned int, string>& CasioPV::PVExpense::getData() const{
  return m_data;
}


/**
   * Getter for date.
   * @return Return the date in a string with the format yyyymmdd
   */
string CasioPV::PVExpense::getDate()
{
  return m_data[EX_DATE];
}

/**
   * Getter for payment type.
   * @return Return the payment type in a string with max of 14 characters
   */
string CasioPV::PVExpense::getPaymentType()
{
  return m_data[EX_PAYMENT_TYPE];
}

/**
   * Getter for amount.
   * @return Return the amount in a string with max value of +/- 99999999
   * and max two digits of under decimal point
   */
string CasioPV::PVExpense::getAmount()
{
  return m_data[EX_AMOUNT];
}

/**
   * Getter for expence type.
   * @return Return the expence type in a string with max of 14 characters
   */
string CasioPV::PVExpense::getExpenseType()
{
  return m_data[EX_EXPENSE_TYPE];
}

/**
   * Getter for note.
   * @return Return the note in a string with up to 2008 characters
   */
string CasioPV::PVExpense::getNote()
{
  return m_data[EX_NOTE];
}


/**
   * Setter for date.
   * @param value sets the date in a string with the format yyyymmdd
   */
void CasioPV::PVExpense::setDate( string& value )
{
  if (!Utils::checkDate(value))
  {
    throw PVDataEntryException( "PVExpense::setDate : string not in yyyymmdd format", 4004 );
  }
  m_data[EX_DATE] = value;
}

/**
   * Setter for payment type.
   * @param value sets the payment type in a string with max of 14 characters
   * @exception PVDataEntryException
   */
void CasioPV::PVExpense::setPaymentType( string& value )
{
  if ( value.length() > 14 ) throw PVDataEntryException( "PVExpense::setPaymentType : string longer than 14 characters", 4002 );
  m_data[EX_PAYMENT_TYPE] = value;
}

void CasioPV::PVExpense::setAmount( string& value )
{
  m_data[EX_AMOUNT] = value;
}

void CasioPV::PVExpense::setExpenseType( string& value )
{
  if ( value.length() > 14 ) throw PVDataEntryException( "PVExpense::setExpenseType : string longer than 14 characters", 4002 );
  m_data[EX_EXPENSE_TYPE] = value;
}

void CasioPV::PVExpense::setNote( string& value )
{
  if ( value.length() > 2008 ) throw PVDataEntryException( "PVExpense::setNote : string longer than 2008 characters", 4002 );
  m_data[EX_NOTE] = value;
}

/**
   * Setter for the data of a field.
   * @param packet data packet for the field
   * @exception PVDataEntryException
   */
void CasioPV::PVExpense::setFieldData( datapacket& packet )
{
  switch( packet.fieldCode )
  {
    case EX_DATE:
    case EX_PAYMENT_TYPE:
    case EX_AMOUNT:
    case EX_EXPENSE_TYPE:
    case EX_NOTE:
      m_data[packet.fieldCode] = packet.data;
      break;
    default:
      throw PVDataEntryException( "PVExpense::setFieldData : received unsupported fieldCode : " + packet.fieldCode, 4003 );
  }
}

/**
   * Checks if this instance of expense is sendable.
   * The fields date and amount have to be set.
   * @return bool true if all necessary fields are filled else false.
   */
bool CasioPV::PVExpense::isSendable(){
  return ( m_data[EX_DATE] != "" && m_data[EX_AMOUNT] != "" );
}

/**
   * This method returns if the data entry is a secret area entry
   * @return bool true if secret else false
   */
bool CasioPV::PVExpense::isSecret() {
  return m_isSecret;
}

/**
   * stream the content
   */
std::ostream& CasioPV::operator<< (std::ostream& out, CasioPV::PVExpense expense)
{
  out << "-----   PVExpense -----" << endl
       << "Date\t\t: " << expense.getDate() << endl
       << "Amount\t\t: " << expense.getAmount() << endl
       << "ExpenseType\t: " << expense.getExpenseType() << endl
       << "PaymentType\t: " << expense.getPaymentType() << endl
       << "Note\t\t: " << expense.getNote() << endl;
  return out;
}

/**
   * Convert the Entry to an XML string
   * @return string The entry as XML string
   */
string CasioPV::PVExpense::toXML()
{
  std::stringstream oss;
  oss << "<expense uid='" << getUid()
                           << "' state='" << getState() << "'>" << endl
       << "<date>" << getDate() << "</date>" << endl
       << "<amount>" << getAmount() << "</amount>" << endl
       << "<expensetype>" << getExpenseType() << "</expensetype>" << endl
       << "<paymenttype>" << getPaymentType() << "</paymenttype>" << endl
       << "<note>" << getNote() << "</note>" << endl
      << "</expense>" << endl;
  return oss.str();
}

/**
   * Convert the XML string to an Entry
   * @param strXML The XML string to be converted
   */
void CasioPV::PVExpense::fromXML(string strXML)
{
  vector<string> vecElem;
  vecElem = Utils::getElements(strXML);
  unsigned int size = vecElem.size();
  if (size != 5)
  {
    throw PVDataEntryException("PVExpense::fromXML : invalid XML format", 4006);
  }
  else
  {
    setDate(vecElem[0]);
    setAmount(vecElem[1]);
    setExpenseType(vecElem[2]);
    setPaymentType(vecElem[3]);
    setNote(vecElem[4]);
  }
}
