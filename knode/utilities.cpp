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



QCString encryptStr(const QCString& aStr)
{
  uint i,len = aStr.length();
  QCString result;

  for (i=0; i<len; i++)
  {
    unsigned char val = aStr[i] - ' ';
    val = (255-' ') - val;
    result += (val + ' ');
  }

  return result;
}



QCString decryptStr(const QCString& aStr)
{
  return encryptStr(aStr);
}



void displayInternalFileError()
{
  KMessageBox::error(knGlobals.topWidget, i18n("Unable to load/save configuration!\nWrong permissions on home directory?\nYou should close KNode now, to avoid data loss!"));
}



void displayExternalFileError()
{
  KMessageBox::error(knGlobals.topWidget, i18n("Unable to load/save file!"));
}



void displayRemoteFileError()
{
  KMessageBox::error(knGlobals.topWidget, i18n("Unable to save remote file!"));
}



void displayTempFileError()
{
  KMessageBox::error(knGlobals.topWidget, i18n("Unable to create temporary file!"));
}

