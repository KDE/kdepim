/*
    utilities.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef UTIL
#define UTIL

class QWidget;
class QString;
class QSize;

void saveWindowSize(const QString &name, const QSize &s);
void restoreWindowSize(const QString &name, QWidget *d, const QSize &defaultSize);

const QString encryptStr(const QString& aStr);
const QString decryptStr(const QString& aStr);
QString rot13(const QString &s);

void displayInternalFileError(QWidget *w=0);   // use this for all internal files
void displayExternalFileError(QWidget *w=0);   // use this for all external files
void displayRemoteFileError(QWidget *w=0);     // use this for remote files
void displayTempFileError(QWidget *w=0);       // use this for error on temporary files

#endif
