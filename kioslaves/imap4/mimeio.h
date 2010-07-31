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

#include <tqcstring.h>
#include <tqfile.h>

/**
  *@author Sven Carstens
  */

class mimeIO
{
public:
  mimeIO ();
  virtual ~ mimeIO ();

  virtual int outputLine (const TQCString &, int len = -1);
  virtual int outputMimeLine (const TQCString &);
  virtual int inputLine (TQCString &);
  virtual int outputChar (char);
  virtual int inputChar (char &);

  void setCRLF (const char *);

protected:
    TQCString theCRLF;
    int crlfLen;
};

class mimeIOQFile:public mimeIO
{
public:
  mimeIOQFile (const TQString &);
    virtual ~ mimeIOQFile ();
  virtual int outputLine (const TQCString &, int len = -1);
  virtual int inputLine (TQCString &);

protected:
    TQFile myFile;
};

class mimeIOQString:public mimeIO
{
public:
  mimeIOQString ();
  virtual ~ mimeIOQString ();
  virtual int outputLine (const TQCString &, int len = -1);
  virtual int inputLine (TQCString &);
  const TQString& getString () const
  {
    return theString;
  }
  void setString (const TQString & _str)
  {
    theString = _str;
  }

protected:
  TQString theString;
};

#endif
