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


#include <tqcstring.h>
#include <tqasciidict.h>

/**
  *@author Sven Carstens
  */

class mimeHdrLine
{
public:
  mimeHdrLine ();
  mimeHdrLine (mimeHdrLine *);
  mimeHdrLine (const TQCString &, const TQCString &);
   ~mimeHdrLine ();
  /** parse a Line into the class
and report characters slurped */
  int setStr (const char *);
  int appendStr (const char *);
  /** return the value */
  const TQCString& getValue ();
  /** return the label */
  const TQCString& getLabel ();
  static TQCString truncateLine (TQCString, unsigned int truncate = 80);
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
    TQCString mimeValue;
  /** contains the Label of the line
 */
  TQCString mimeLabel;
protected:                     // Protected methods
  /** parses a continuated line */
  int parseFullLine (const char *);
  int parseHalfLine (const char *);
};

#endif
