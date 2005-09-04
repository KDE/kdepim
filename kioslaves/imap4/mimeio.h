/***************************************************************************
                          mimeio.h  -  description
                             -------------------
    begin                : Wed Oct 25 2000
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

#ifndef MIMEIO_H
#define MIMEIO_H

#include <q3cstring.h>
#include <qfile.h>

/**
  *@author Sven Carstens
  */

class mimeIO
{
public:
  mimeIO ();
  virtual ~ mimeIO ();

  virtual int outputLine (const Q3CString &, int len = -1);
  virtual int outputMimeLine (const Q3CString &);
  virtual int inputLine (Q3CString &);
  virtual int outputChar (char);
  virtual int inputChar (char &);

  void setCRLF (const char *);

protected:
    Q3CString theCRLF;
    int crlfLen;
};

class mimeIOQFile:public mimeIO
{
public:
  mimeIOQFile (const QString &);
    virtual ~ mimeIOQFile ();
  virtual int outputLine (const Q3CString &, int len = -1);
  virtual int inputLine (Q3CString &);

protected:
    QFile myFile;
};

class mimeIOQString:public mimeIO
{
public:
  mimeIOQString ();
  virtual ~ mimeIOQString ();
  virtual int outputLine (const Q3CString &, int len = -1);
  virtual int inputLine (Q3CString &);
  const QString& getString () const
  {
    return theString;
  }
  void setString (const QString & _str)
  {
    theString = _str;
  }

protected:
  QString theString;
};

#endif
