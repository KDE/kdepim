/***************************************************************************
                          mimeio.cc  -  description
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

#include <iostream>
using namespace std;

#include "mimeio.h"

mimeIO::mimeIO ()
{
  theCRLF = "\r\n";
  crlfLen = 2;
}

mimeIO::~mimeIO ()
{
}

int mimeIO::inputLine (QByteArray & aLine)
{
  char input;

  aLine = QByteArray();
  while (inputChar (input))
  {
    aLine += input;
    if (input == '\n')
      break;
  }
//  cout << aLine.length() << " - " << aLine;
  return aLine.length ();
}

int mimeIO::outputLine (const QByteArray & aLine, int len)
{
  int i;

  if (len == -1) {
    len = aLine.length();
  }
  int start = len;
  for (i = 0; i < start; i++)
    if (!outputChar (aLine[i]))
      break;
  return i;
}

int mimeIO::outputMimeLine (const QByteArray & inLine)
{
  int retVal = 0;
  QByteArray aLine = inLine;
  int len = inLine.length();

  int theLF = aLine.lastIndexOf ('\n');
  if (theLF == len - 1 && theLF != -1)
  {
    //we have a trailing LF, now check for CR
    if (aLine[theLF - 1] == '\r')
      theLF--;
    //truncate the line
    aLine.truncate(theLF);
    len = theLF;
    theLF = -1;
  }
  //now truncate the line
  {
    int start, end, offset;
    start = 0;
    end = aLine.indexOf ('\n', start);
    while (end >= 0)
    {
      offset = 1;
      if (end && aLine[end - 1] == '\r')
      {
        offset++;
        end--;
      }
      outputLine (aLine.mid (start, end - start) + theCRLF, end - start + crlfLen);
      start = end + offset;
      end = aLine.indexOf ('\n', start);
    }
    outputLine (aLine.mid (start, len - start) + theCRLF, len - start + crlfLen);
  }
  return retVal;
}

int mimeIO::inputChar (char &aChar)
{
  if (cin.eof ())
  {
//    cout << "EOF" << endl;
    return 0;
  }
  cin.get (aChar);
  return 1;
}

int mimeIO::outputChar (char aChar)
{
  cout << aChar;
  return 1;
}

// void mimeIO::setCRLF (const char *aCRLF)
// {
//   theCRLF = aCRLF;
//   crlfLen = strlen(aCRLF);
// }

mimeIOQFile::mimeIOQFile (const QString & aName):
mimeIO (),
myFile (aName)
{
  myFile.open (QIODevice::ReadOnly);
}

mimeIOQFile::~mimeIOQFile ()
{
  myFile.close ();
}

int mimeIOQFile::outputLine (const QByteArray &, int)
{
  return 0;
}

int mimeIOQFile::inputLine (QByteArray & data)
{
  data.resize( 1024 );
  myFile.readLine (data.data(), 1024);

  return data.length ();
}

mimeIOQString::mimeIOQString ()
{
}

mimeIOQString::~mimeIOQString ()
{
}

int mimeIOQString::outputLine (const QByteArray & _str, int len)
{
  if (len == -1) {
    len = _str.length();
  }
  theString += _str;
  return len;
}

int mimeIOQString::inputLine (QByteArray & _str)
{
  if (theString.isEmpty ())
    return 0;

  int i = theString.indexOf ('\n');

  if (i == -1)
    return 0;
  _str = theString.left (i + 1).toLatin1 ();
  theString = theString.right (theString.length () - i - 1);
  return _str.length ();
}
