/***************************************************************************
                          mimehdrline.h  -  description
                             -------------------
    begin                : Wed Oct 11 2000
    copyright            : (C) 2000 by Sven Carstens
    email                : s.carstens@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIMEHDRLINE_H
#define MIMEHDRLINE_H


#include <q3cstring.h>
#include <q3asciidict.h>

/**
  *@author Sven Carstens
  */

class mimeHdrLine
{
public:
  mimeHdrLine ();
  mimeHdrLine (mimeHdrLine *);
  mimeHdrLine (const Q3CString &, const Q3CString &);
   ~mimeHdrLine ();
  /** parse a Line into the class
and report characters slurped */
  int setStr (const char *);
  int appendStr (const char *);
  /** return the value */
  const Q3CString& getValue ();
  /** return the label */
  const Q3CString& getLabel ();
  static Q3CString truncateLine (Q3CString, unsigned int truncate = 80);
  static int parseSeparator (char, const char *);
  static int parseQuoted (char, char, const char *);
  /** skip all white space characters */
  static int skipWS (const char *);
  /** slurp one word respecting backticks */
  static int parseHalfWord (const char *);
  static int parseWord (const char *);
  static int parseAlphaNum (const char *);

protected:                     // Protected attributes
  /** contains the Value 
 */
    Q3CString mimeValue;
  /** contains the Label of the line
 */
  Q3CString mimeLabel;
protected:                     // Protected methods
  /** parses a continuated line */
  int parseFullLine (const char *);
  int parseHalfLine (const char *);
};

#endif
