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

#include "knglobals.h"
#include "utilities.h"



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



const QString encryptStr(const QString& aStr)
{
  uint i,val,len = aStr.length();
  QCString result;

  for (i=0; i<len; i++)
  {
    val = aStr[i] - ' ';
    val = (255-' ') - val;
    result += (char)(val + ' ');
  }

  return result;
}



const QString decryptStr(const QString& aStr)
{
  return encryptStr(aStr);
}


QString rot13(const QString &s)
{
  QString r(s);

  for (int i=0; (uint)i<r.length(); i++) {
    if ( r[i] >= QChar('A') && r[i] <= QChar('M') ||
         r[i] >= QChar('a') && r[i] <= QChar('m') )
         r[i] = (char)((int)QChar(r[i]) + 13);
    else
      if  ( r[i] >= QChar('N') && r[i] <= QChar('Z') ||
            r[i] >= QChar('n') && r[i] <= QChar('z') )
        r[i] = (char)((int)QChar(r[i]) - 13);
  }  	

  return r;
}


void displayInternalFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save configuration!\nWrong permissions on home directory?\nYou should close KNode now, to avoid data loss!"));
}



void displayExternalFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save file!"));
}



void displayRemoteFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to save remote file!"));
}



void displayTempFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to create temporary file!"));
}

