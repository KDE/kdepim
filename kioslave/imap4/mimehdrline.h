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


#include <QByteArray>

/**
  *@author Sven Carstens
  */

class mimeHdrLine
{
public:
  mimeHdrLine ();
  mimeHdrLine (mimeHdrLine *);
  mimeHdrLine (const QByteArray &, const QByteArray &);
   ~mimeHdrLine ();
  /** parse a Line into the class
and report characters slurped */
  int setStr (const char *);
  int appendStr (const char *);
  /** return the value */
  const QByteArray& getValue ();
  /** return the label */
  const QByteArray& getLabel ();
  static QByteArray truncateLine (QByteArray, unsigned int truncate = 80);
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
    QByteArray mimeValue;
  /** contains the Label of the line
 */
  QByteArray mimeLabel;
protected:                     // Protected methods
  /** parses a continuated line */
  int parseFullLine (const char *);
  int parseHalfLine (const char *);
};

#endif
