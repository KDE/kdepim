/***************************************************************************
                     utilities.h - description
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

#ifndef UTIL
#define UTIL

class QWidget;
class QString;
class QSize;

/*
#define SIZE(w) 		w->setMinimumSize(w->sizeHint())
#define FSIZE(w) 		w->setFixedSize(w->sizeHint())
#define WIDTH(w,s)  w->setMinimumWidth(s); w->setMinimumHeight(w->sizeHint().height())
#define HEIGHT(w,s) w->setMinimumHeight(s); w->setMinimumWidth(w->sizeHint().width());

bool stripCRLF(char *str);
bool stripCRLF(QCString &str);
void removeQuots(QCString &str);*/

void saveWindowSize(const QString &name, const QSize &s);
void restoreWindowSize(const QString &name, QWidget *d, const QSize &defaultSize);

QCString encryptStr(const QCString& aStr);
QCString decryptStr(const QCString& aStr);

void displayInternalFileError();	 // use this for all internal files
void displayExternalFileError();   // use this for all external files
void displayRemoteFileError();     // use this for remote files
void displayTempFileError();       // use this for error on temporary files

#endif
