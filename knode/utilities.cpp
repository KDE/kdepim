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

#include <qsize.h>
#include <qwidget.h>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include "utilities.h"


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


void saveWindowSize(const QString &name, const QSize &s)
{
  KConfig *c=KGlobal::config();	
	c->setGroup("WINDOW_SIZES");
	c->writeEntry(name, s);	
}


void restoreWindowSize(const QString &name, QWidget *d, const QSize &defaultSize)
{
  KConfig *c=KGlobal::config();
	c->setGroup("WINDOW_SIZES");
	
	QSize s=c->readSizeEntry(name,&defaultSize);
	
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



void displayInternalFileError()
{
	KMessageBox::error(0, i18n("Unable to load/save configuration!\nWrong permissions on home directory?\nYou should close KNode now, to avoid data loss!"));
}



void displayExternalFileError()
{
	KMessageBox::error(0, i18n("Unable to load/save file!"));
}



void displayRemoteFileError()
{
	KMessageBox::error(0, i18n("Unable to save remote file!"));
}



void displayTempFileError()
{
	KMessageBox::error(0, i18n("Unable to create temporary file!"));
}

