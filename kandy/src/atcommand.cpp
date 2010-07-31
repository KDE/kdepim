/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "atcommand.h"

#include <kdebug.h>
#include <klocale.h>

ATParameter::ATParameter()
{
  mUserInput = false;
}

ATParameter::ATParameter(const TQString &value,const TQString &name,
                         bool userInput)
{
  mName = name;
  mValue = value;
  mUserInput = userInput;
}


ATCommand::ATCommand()
{
  mHexOutput = false;

  construct();
}

ATCommand::ATCommand(const TQString &cmdString)
{
  setCmdName(i18n("New Command"));
  setCmdString(cmdString);
  mHexOutput = false;
  
  extractParameters();
  
  construct();
}

ATCommand::ATCommand(const TQString &cmdName,const TQString &cmdString,
                     bool hexOutput)
{
  setCmdName(cmdName);
  setCmdString(cmdString);
  mHexOutput = hexOutput;

  construct();
}

void ATCommand::construct()
{
  mAutoDelete = false;
  mResultFieldsList.setAutoDelete(true);
  mParameters.setAutoDelete(true);
}

ATCommand::~ATCommand()
{
//  kdDebug() << "~ATCommand: " << cmdString() << endl;
}


void ATCommand::setCmdName(const TQString &cmdName)
{
  mCmdName = cmdName;
}

TQString ATCommand::cmdName()
{
  return mCmdName;
}


void ATCommand::setCmdString(const TQString &cmdString)
{
  mCmdString = cmdString;

  mId = cmdString;
  if (mId.startsWith("at")) mId = mId.mid(2);
  else mCmdString.prepend("at");
  
//  kdDebug() << "ATCommand: Id: " << mId << endl;
}

TQString ATCommand::cmdString()
{
  return mCmdString;
}

TQString ATCommand::cmd()
{
  if (mParameters.count() > 0) {
    TQString cmd = cmdString().left(cmdString().find("=") + 1);
//    kdDebug() << "--1-cmd: " << cmd << endl;
    for(uint i=0;i<mParameters.count();++i) {
      cmd += mParameters.at(i)->value();
      if (i < mParameters.count() - 1) cmd += ",";
    }
//    kdDebug() << "--2-cmd: " << cmd << endl;
    return cmd;
  } else {
    return cmdString();
  }
}

TQString ATCommand::id()
{
  return mId;
}

void ATCommand::setHexOutput(bool hexOutput)
{
  mHexOutput = hexOutput;
}

bool ATCommand::hexOutput()
{
  return mHexOutput;
}

void ATCommand::setResultString(const TQString &resultString)
{
  mResultString = resultString;

  mResultFieldsList.clear();

  TQStringList resultFields = TQStringList::split("\n",mResultString);

  for(TQStringList::Iterator it = resultFields.begin();
      it != resultFields.end(); ++it) {
    setResultFields(*it);
  }
}


void ATCommand::setResultFields( TQString fieldsString )
{
  TQString id = mId.upper().left( mId.find( '=' ) );
  

  // Truncate the command name prepended to the output by the modem.
  if ( fieldsString.startsWith( id ) )
    fieldsString = fieldsString.mid( id.length() + 2 );

  // If modem output is enclosed by brackets, remove them, too
  if ( ( fieldsString[ 0 ] == '(' ) && ( fieldsString[ fieldsString.length() - 1 ] == ')' ) )
    fieldsString = fieldsString.mid( 1, fieldsString.length() - 2 );

  TQStringList *fields = new QStringList;
  TQStringList TmpFields = TQStringList::split( ',', fieldsString );
  TQString TmpString = "";
  

  // Assume a phonebook entry of the mobile phone has the format
  //   <familyname>, <givenname>
  // Then, the above split() call separtes this entry into 2 distinct fields
  // leading to an error in MobileGui::fillPhonebook since the number of
  // fields is != 4.
  // Hence, the fieldsString needs to be parsed a little bit. Names stored on
  // the mobile phone are quoted. Commas within a quoted are of the fieldsString
  // must not be divided into differend fields.
  for ( TQStringList::Iterator it = TmpFields.begin(); it != TmpFields.end(); it++ )
  {
    // Start of a quoted area
    if ( ( (*it)[ 0 ] == '\"' ) && ( (*it)[ (*it).length() - 1 ] != '\"' ) )
      TmpString = (*it).copy();
    else
    // End of a quoted area
    if ( ( (*it)[ 0 ] != '\"' ) && ( (*it)[ (*it).length() - 1 ] == '\"' ) )
    {
      TmpString += "," + (*it).copy();
      (*fields).append( TmpString.copy() );
      TmpString = "";
    } else
    // Not within a quoted area
    if (TmpString.isEmpty())
      (*fields).append( *it );
    else
    // Within a quoted area
      TmpString += "," + (*it).copy();
  }

  mResultFieldsList.append( fields );
}


TQString ATCommand::resultString()
{
  return mResultString;
}

TQString ATCommand::resultField(int index)
{
  if (mResultFieldsList.count() == 0) return "";

  TQStringList *resultFields = mResultFieldsList.at(0);

  TQStringList::Iterator it = resultFields->at(index);
  if (it == resultFields->end()) {
    kdDebug() << "ATCommand::resultField: index " << index << " out of range."
              << endl;
    return "";
  }

  return *it;
}


TQPtrList<TQStringList> *ATCommand::resultFields()
{
   return &mResultFieldsList;
}

void ATCommand::addParameter(ATParameter *p)
{
  mParameters.append(p);
}

void ATCommand::clearParameters()
{
  mParameters.clear();
}

TQPtrList<ATParameter> ATCommand::parameters()
{
  return mParameters;
}

void ATCommand::setParameter(int index,const TQString &value)
{
  if (mParameters.count() <= (unsigned int)index) {
    kdDebug() << "ATCommand " << cmdName() << " has no Parameter " << index
              << endl;
    return;
  }
  
  mParameters.at(index)->setValue(value);
}

void ATCommand::setParameter(int index,int value)
{
  setParameter(index,TQString::number(value));
}

TQString ATCommand::processOutput(const TQString &output)
{
  if (hexOutput()) {
    TQString hexString = output.mid(output.find('\n')+1);
    int i=0;
    TQString aChar = hexString.mid(i,2);
    TQString result;
    while(!aChar.isEmpty()) {
      int charValue = aChar.toInt(0,16);
      TQChar charEncoded(charValue);
//      result += aChar + ": " + charEncoded + "\n";
      result += charEncoded;
      i += 2;
      aChar = hexString.mid(i,2);
    }
    result += "\n";
    return result;
  } else {
    return output;
  }
}

TQString ATCommand::processOutput()
{
  return processOutput(mResultString);
}

void ATCommand::extractParameters()
{
//  kdDebug() << "Arg String: " << cmdString() << endl;
  
  int pos = cmdString().find("=");
  if (pos < 0) return;
  
  TQString paraString = cmdString().mid(pos+1);
//  kdDebug() << "Para String: " << paraString << endl;
  TQStringList paraList = TQStringList::split(",",paraString);
  
  TQStringList::ConstIterator it = paraList.begin();
  TQStringList::ConstIterator end = paraList.end();
  int argNum = 1;
  while(it != end) {
    addParameter(new ATParameter(*it,i18n("Arg %1").arg(TQString::number(argNum++)),
                                 false));
    ++it;
  }
}
