/***************************************************************************
                     utilities.cpp - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kmessagebox.h>

#include "utilities.h"
#include "knode.h"
#include "knstringsplitter.h"
#include <ctype.h>
#include <qdir.h>


/*
bool stripCRLF(char *str)
{
	int pos=strlen(str)-1;
	while(str[pos]!='\n') pos--;
	//if(str[pos]=='\n') {
		str[pos--]='\0';
		if(str[pos]=='\r') str[pos]='\0';
		return true;
	//}
	//else return false;	
}



bool stripCRLF(QCString &str)
{
	int pos=str.length()-1;
	
	while(str[pos]==' ') pos--;
	
	if(str[pos]=='\n') {
		if(str[pos-1]=='\r') str.truncate(pos-1);
		else str.truncate(pos);
		return true;
	}
	else return false;
	
}



void removeQuots(QCString &str)
{
	int idx=0, pos1=0, pos2=0;
	char firstChar, lastChar;
		
	do {
		pos1=idx;
		firstChar=str[idx++];
	} while(firstChar==' ');
		
	idx=str.length();
		
	do {
		lastChar=str[--idx];
		pos2=idx-1;
	} while(lastChar==' ');
		
	if(firstChar=='"' && lastChar=='"') {
		str.remove(pos1,1);
		str.remove(pos2,1);
	}
	
}
*/


void saveDialogSize(const QString &name, const QSize &s)
{
	KConfig *c=CONF();
	c->setGroup("DIALOGS");
	c->writeEntry(name, s);
	
}


void setDialogSize(const QString &name, QWidget *d)
{
	KConfig *c=CONF();
	c->setGroup("DIALOGS");
	
	QSize s=c->readSizeEntry(name);
	
	if(s.isValid()) d->resize(s);	
}



QString encryptStr(const QString& aStr)
{
  unsigned int i, val;
  unsigned int len = aStr.length();
  QString result;

  for (i=0; i<len; i++)
  {
    // FIXME probably doesn't work like expected with non-ascii strings
    unsigned char val = aStr[i].latin1() - ' ';
    val = (255-' ') - val;
    result += QChar(val + ' ');
  }

  return result;
}



QString decryptStr(const QString& aStr)
{
  return encryptStr(aStr);
}



void snyimpl()
{
	KMessageBox::information(0,i18n("Sorry, this feature is not yet implemented :-("));
}


void displayExternalFileError()
{
	KMessageBox::error(0, i18n("Unable to load/save file!"));
}


void displayInternalFileError()
{
	KMessageBox::error(0, i18n("Unable to load/save configuration!\nWrong permissions on home directory?\n!\nYou should close this application now,\nto avoid data loss!"));
}



