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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

ATParameter::ATParameter(const QString &value,const QString &name,
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

ATCommand::ATCommand(const QString &cmdString)
{
  setCmdName(i18n("New Command"));
  setCmdString(cmdString);
  mHexOutput = false;
  
  extractParameters();
  
  construct();
}

ATCommand::ATCommand(const QString &cmdName,const QString &cmdString,
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


void ATCommand::setCmdName(const QString &cmdName)
{
  mCmdName = cmdName;
}

QString ATCommand::cmdName()
{
  return mCmdName;
}


void ATCommand::setCmdString(const QString &cmdString)
{
  mCmdString = cmdString;

  mId = cmdString;
  if (mId.startsWith("at")) mId = mId.mid(2);
  else mCmdString.prepend("at");
  
//  kdDebug() << "ATCommand: Id: " << mId << endl;
}

QString ATCommand::cmdString()
{
  return mCmdString;
}

QString ATCommand::cmd()
{
  if (mParameters.count() > 0) {
    QString cmd = cmdString().left(cmdString().find("=") + 1);
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

QString ATCommand::id()
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

void ATCommand::setResultString(const QString &resultString)
{
  mResultString = resultString;

  mResultFieldsList.clear();

  QStringList resultFields = QStringList::split("\n",mResultString);

  for(QStringList::Iterator it = resultFields.begin();
      it != resultFields.end(); ++it) {
    setResultFields(*it);
  }
}


void ATCommand::setResultFields( QString fieldsString )
{
  QString id = mId.upper().left( mId.find( '=' ) );
  

  // Truncate the command name prepended to the output by the modem.
  if ( fieldsString.startsWith( id ) )
    fieldsString = fieldsString.mid( id.length() + 2 );

  // If modem output is enclosed by brackets, remove them, too
  if ( ( fieldsString[ 0 ] == '(' ) && ( fieldsString[ fieldsString.length() - 1 ] == ')' ) )
    fieldsString = fieldsString.mid( 1, fieldsString.length() - 2 );

  QStringList *fields = new QStringList;
  QStringList TmpFields = QStringList::split( ',', fieldsString );
  QString TmpString = "";
  

  // Assume a phonebook entry of the mobile phone has the format
  //   <familyname>, <givenname>
  // Then, the above split() call separtes this entry into 2 distinct fields
  // leading to an error in MobileGui::fillPhonebook since the number of
  // fields is != 4.
  // Hence, the fieldsString needs to be parsed a little bit. Names stored on
  // the mobile phone are quoted. Commas within a quoted are of the fieldsString
  // must not be divided into differend fields.
  for ( QStringList::Iterator it = TmpFields.begin(); it != TmpFields.end(); it++ )
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
    if (TmpString == "")
      (*fields).append( *it );
    else
    // Within a quoted area
      TmpString += "," + (*it).copy();
  }

  mResultFieldsList.append( fields );
}


QString ATCommand::resultString()
{
  return mResultString;
}

QString ATCommand::resultField(int index)
{
  if (mResultFieldsList.count() == 0) return "";

  QStringList *resultFields = mResultFieldsList.at(0);

  QStringList::Iterator it = resultFields->at(index);
  if (it == resultFields->end()) {
    kdDebug() << "ATCommand::resultField: index " << index << " out of range."
              << endl;
    return "";
  }

  return *it;
}


QPtrList<QStringList> *ATCommand::resultFields()
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

QPtrList<ATParameter> ATCommand::parameters()
{
  return mParameters;
}

void ATCommand::setParameter(int index,const QString &value)
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
  setParameter(index,QString::number(value));
}

QString ATCommand::processOutput(const QString &output)
{
  if (hexOutput()) {
    QString hexString = output.mid(output.find('\n')+1);
    int i=0;
    QString aChar = hexString.mid(i,2);
    QString result;
    while(!aChar.isEmpty()) {
      int charValue = aChar.toInt(0,16);
      QChar charEncoded(charValue);
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

QString ATCommand::processOutput()
{
  return processOutput(mResultString);
}

void ATCommand::extractParameters()
{
//  kdDebug() << "Arg String: " << cmdString() << endl;
  
  int pos = cmdString().find("=");
  if (pos < 0) return;
  
  QString paraString = cmdString().mid(pos+1);
//  kdDebug() << "Para String: " << paraString << endl;
  QStringList paraList = QStringList::split(",",paraString);
  
  QStringList::ConstIterator it = paraList.begin();
  QStringList::ConstIterator end = paraList.end();
  int argNum = 1;
  while(it != end) {
    addParameter(new ATParameter(*it,i18n("Arg %1").arg(QString::number(argNum++)),
                                 false));
    ++it;
  }
}
