/***************************************************************************
                           utils.h  -  description
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

#ifndef UTILS_H
#define UTILS_H

// c++ includes
#include <string>
#include <sstream>
#include <vector>

/**This class contains helper methods. They are used by the pv data container
  *classes (derived from PVDataEntry).
  *@author Maurus Erni
  */
using std::string;
using std::vector;

namespace CasioPV {

  class Utils
  {
    public:

      /**
         * This method changes the return code 0x0D as it is received from the
         * PV to 0x0A as it is used by Unix.
         * @param data Data to be changed
         * @return string Changed data (return code in Unix format)
         */
      static string ChangeReturnCodeToUnix(string& data);

      /**
         * This method changes the return code 0x0A as it is used by Unix to
         * 0x0D as it is used by the PV.
         * @param data Data to be changed
         * @return string Changed data (return code in PV format)
         */
      static string ChangeReturnCodeToPV(string& data);

      /**
         * This method changes a date in the PV format ("yyyymmdd__") in the
         * date format normally used by PC's ("yyyymmdd").
         * @param date Date to be changed
         * @return string Changed date (date in Unix format)
         */
      static string ChangeDateToUnix(string& date);

      /**
         * This method changes a date in the "yyyymmdd" format in the date
         * format used in the PV ("yyyymmdd__").
         * @param date Date to be changed
         * @return string Changed date (date in PV format)
         */
      static string ChangeDateToPV(string& date);

      /**
         * Convert the XML string to a vector of elements.
         * @param strXML The XML string to be converted
         * @return vector<string> The elements in a vector
         */
      static vector<string> getElements(string& strXML);

      /**
         * Checks if the date has a valid format.
         * @param strDate The date represented as a string
         * @return bool true if valid, else false
         */
      static bool checkDate(string& strDate);

      /**
         * Converts a category used in PV to a string
         * @param unsigned int The category used in PV
         * @return string The corresponding string. Empty string if not found.
         */
      static string getCategoryString(unsigned int category);

      /**
         * Converts a category (string type) to a category used in PV
         * @param string The category as string
         * @return unsigned int The category used in PV. 0 if not found.
         */
      static unsigned int getCategoryPV(const string& strCategory);
  };
};  // namespace

#endif
