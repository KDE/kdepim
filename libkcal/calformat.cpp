/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>

#include "calformat.h"

using namespace KCal;

QString CalFormat::mApplication = QString::fromLatin1("libkcal");
QString CalFormat::mProductId = QString::fromLatin1("-//K Desktop Environment//NONSGML libkcal 3.2//EN");

// An array containing the PRODID strings indexed against the calendar file format version used.
// Every time the calendar file format is changed, add an entry/entries to this list.
struct CalVersion {
  int       version;
  QString   prodId;
};
static CalVersion prodIds[] = {
  { 220, QString::fromLatin1("-//K Desktop Environment//NONSGML KOrganizer 2.2//EN") },
  { 300, QString::fromLatin1("-//K Desktop Environment//NONSGML KOrganizer 3.0//EN") },
  { 310, QString::fromLatin1("-//K Desktop Environment//NONSGML KOrganizer 3.1//EN") },
  { 320, QString::fromLatin1("-//K Desktop Environment//NONSGML KOrganizer 3.2//EN") },
  { 0 , QString() }
};


CalFormat::CalFormat()
{
  mException = 0;
}

CalFormat::~CalFormat()
{
  delete mException;
}

void CalFormat::clearException()
{
  delete mException;
  mException = 0;
}

void CalFormat::setException(ErrorFormat *exception)
{
  delete mException;
  mException = exception;
}

ErrorFormat *CalFormat::exception()
{
  return mException;
}

void CalFormat::setApplication(const QString& application, const QString& productID)
{
  mApplication = application;
  mProductId = productID;
}

QString CalFormat::createUniqueId()
{
  int hashTime = QTime::currentTime().hour() +
                 QTime::currentTime().minute() + QTime::currentTime().second() +
                 QTime::currentTime().msec();
  QString uidStr = QString("%1-%2.%3")
                           .arg(mApplication)
                           .arg(KApplication::random())
                           .arg(hashTime);
  return uidStr;
}

int CalFormat::calendarVersion(const char* prodId)
{
  for (const CalVersion* cv = prodIds;  cv->version;  ++cv) {
    if (!strcmp(prodId, cv->prodId.utf8()))
      return cv->version;
  }
  return 0;
}
