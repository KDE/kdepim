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

// $Id$

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kapplication.h>

#include "calformat.h"

using namespace KCal;

CalFormat::CalFormat(Calendar *cal)
{
  mCalendar = cal;
  
  mTopWidget = 0;
  mEnableDialogs = false;
  
  mException = 0;
}

CalFormat::~CalFormat()
{
  delete mException;
}

void CalFormat::setTopwidget(QWidget *topWidget)
{
  mTopWidget = topWidget;
}

void CalFormat::showDialogs(bool enable)
{
  mEnableDialogs = enable;
}

void CalFormat::loadError(const QString &fileName) 
{
  kdDebug() << "CalFormat::loadError()" << endl;

  if (mEnableDialogs) {
    KMessageBox::sorry(mTopWidget,
                       i18n("An error has occurred loading the file:\n"
                            "%1.\n"
                            "Please verify that the file is in vCalendar "
                            "format,\n"
                            "that it exists, and it is readeable, then "
                            "try again or load another file.\n")
                            .arg(fileName),
                       i18n("KOrganizer: Error Loading Calendar"));
  }
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

QString CalFormat::createUniqueId()
{
  int hashTime = QTime::currentTime().hour() + 
                 QTime::currentTime().minute() + QTime::currentTime().second() +
                 QTime::currentTime().msec();
  QString uidStr = QString("KOrganizer-%1.%2")
                           .arg(KApplication::random())
                           .arg(hashTime);
  return uidStr;
}
