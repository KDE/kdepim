/***************************************************************************
                           utils.c  -  description
                             -------------------
    begin                : Thu Oct 16 2002
    copyright            : (C) 2002 by Erni Maurus
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// project includes
#include "ModeCode.h"

#include "utils.h"

using namespace std;

using namespace CasioPV;

/**
   * This method changes the return code 0x0D as it is recieved from the PV to
   * 0x0A as it is used by Unix.
   * @param data Data to be changed
   * @return string Changed data (return code in Unix format)
   */
string Utils::ChangeReturnCodeToUnix(string& data)
{
  string tmp = "";
  for ( unsigned int i = 0; i<data.length(); i++ ) {
    if ( data[i] == 0x0D ) tmp += 0x0A;
    else tmp += data[i];
  }
  return tmp;
}

/**
   * This method changes the return code 0x0A as it is used by Unix to 0x0D
   * as it is used by the PV.
   * @param data Data to be changed
   * @return string Changed data (return code in PV format)
   */
string Utils::ChangeReturnCodeToPV(string& data)
{
  string tmp = "";
  for ( unsigned int i = 0; i<data.length(); i++ ) {
    if ( data[i] == 0x0A ) tmp += 0x0D;
    else tmp += data[i];
  }
  return tmp;
}

/**
   * This method changes a date in the PV format ("yyyymmdd__") in the date
   * format normally used by PC's ("yyyymmdd").
   * @param date Date to be changed
   * @return string Changed date (date in Unix format)
   */
string Utils::ChangeDateToUnix(string& date)
{
  return (date.substr(0, 8));
}

/**
   * This method changes a date in the "yyyymmdd" format in the date format
   * used in the PV ("yyyymmdd__").
   * @param date Date to be changed
   * @return string Changed date (date in PV format)
   */
string Utils::ChangeDateToPV(string& date)
{
  return (date + "  ");
}

/**
   * Convert the XML string to a vector of elements.
   * @param strXML The XML string to be converted
   * @return vector<string> The elements in a vector
   */
vector<string> Utils::getElements(string& strXML)
{
  vector<string> vecElem;
  string line;

  vecElem.clear();

  string::size_type first_pos = 0;
  string::size_type second_pos = 0;
  string::size_type last_pos = 0;

  // Now let's find the end position of the last entry
  last_pos = strXML.find_last_of('>') - 1;
  last_pos = strXML.find_last_of('>', last_pos);
  // Check if last tag is empty (e.g. <note/> )
  if (strXML[last_pos-1] != '/')
  {
    last_pos = strXML.find_last_of('<', last_pos);
  }

  while (second_pos != last_pos)
  {
    first_pos = strXML.find('<', second_pos + 1) + 1;
    second_pos = strXML.find('>', first_pos);
    // Check if tag is empty
    if (strXML[second_pos-1] == '/')
    {
      vecElem.push_back("");  // add an empty string to vector
    }
    else
    {
      // If tag isn't empty, get element
      first_pos = second_pos + 1;
      second_pos = strXML.find('<', first_pos);
      vecElem.push_back(strXML.substr(first_pos, second_pos - first_pos));
    }
  }
  return vecElem;
}

/**
   * Checks if the date has a valid format.
   * @param strDate The date represented as a string
   * @return bool True if valid, else false
   */
bool Utils::checkDate(string& strDate)
{
  int year = ((int)strDate[0]-0x30)*1000 + ((int)strDate[1]-0x30)*100 + ((int)strDate[2]-0x30)*10 + ((int)strDate[3]-0x30),
     month = ((int)strDate[4]-0x30)*10 + ((int)strDate[5]-0x30),
     day = ((int)strDate[6]-0x30)*10 + ((int)strDate[7]-0x30);

  if ((strDate.length() != 8) || (month == 0) || (month > 12) || (day == 0) || (day > 31) ||
        (((month ==4) || (month == 6) || (month == 9) || (month == 11) || (month == 10) || (month == 12)) && (day > 30)) ||
        ((month == 2) && ((day > 29) || (((year%4 != 0) || ((year%100 == 0)) && (year%400 != 0)) && (day > 28)))))
  {
    return false;  // Date invalid
  }
  return true;
}

/**
   * Converts a category used in PV to a string
   * @param unsigned int The category used in PV
   * @return string The corresponding string. Empty string if not found.
   */
string Utils::getCategoryString(unsigned int category)
{
  switch (category)
  {
    case CONTACT_PRIVATE:
      return ("Contact Private");
      break;
    case CONTACT_BUSINESS:
      return ("Contact Business");
      break;
    case CONTACT_UNTITLED_1:
      return ("Contact Untitled 1");
      break;
    case CONTACT_UNTITLED_2:
      return ("Contact Untitled 2");
      break;
    case CONTACT_UNTITLED_3:
      return ("Contact Untitled 3");
      break;
    case CONTACT_UNTITLED_4:
      return ("Contact Untitled 4");
      break;
    case CONTACT_UNTITLED_5:
      return ("Contact Untitled 5");
      break;
    case MEMO_1:
      return ("Memo 1");
      break;
    case MEMO_2:
      return ("Memo 2");
      break;
    case MEMO_3:
      return ("Memo 3");
      break;
    case MEMO_4:
      return ("Memo 4");
      break;
    case MEMO_5:
      return ("Memo 5");
      break;
    case SCHEDULE:
      return ("Schedule");
      break;
    case SCHEDULE_MULTI_DATE:
      return ("Schedule Multi Date");
      break;
    case SCHEDULE_REMINDER:
      return ("Schedule Reminder");
      break;
    case EXPENSE_PV:
      return ("Expense PV");
      break;
    case TODO:
      return ("To Do");
      break;
    case POCKET_SHEET_PV:
      return ("Pocket Sheet PV");
      break;
    case QUICK_MEMO:
      return ("Quick Memo");
      break;
    default:
      return ("");
      break;
  }
}

/**
   * Converts a category (string type) to a category used in PV
   * @param string The category as string
   * @return unsigned int The category used in PV. 0 if not found
   */
unsigned int Utils::getCategoryPV(const string& strCategory)
{
  if (strCategory == "Contact Private")
    return CONTACT_PRIVATE;
  else if (strCategory == "Contact Business")
    return CONTACT_BUSINESS;
  else if (strCategory == "Contact Untitled 1")
    return CONTACT_UNTITLED_1;
  else if (strCategory == "Contact Untitled 2")
    return CONTACT_UNTITLED_2;
  else if (strCategory == "Contact Untitled 3")
    return CONTACT_UNTITLED_3;
  else if (strCategory == "Contact Untitled 4")
    return CONTACT_UNTITLED_4;
  else if (strCategory == "Contact Untitled 5")
    return CONTACT_UNTITLED_5;
  else if (strCategory == "Memo 1")
    return MEMO_1;
  else if (strCategory == "Memo 2")
    return MEMO_2;
  else if (strCategory == "Memo 3")
    return MEMO_3;
  else if (strCategory == "Memo 4")
    return MEMO_4;
  else if (strCategory == "Memo 5")
    return MEMO_5;
  else if (strCategory == "Schedule")
    return SCHEDULE;
  else if (strCategory == "Schedule Multi Date")
    return SCHEDULE_MULTI_DATE;
  else if (strCategory == "Schedule Reminder")
    return SCHEDULE_REMINDER;
  else if (strCategory == "Expense PV")
    return EXPENSE_PV;
  else if (strCategory == "To Do")
    return TODO;
  else if (strCategory == "Pocket Sheet PV")
    return POCKET_SHEET_PV;
  else if (strCategory == "Quick Memo")
    return QUICK_MEMO;
  else
    return 0;
}

