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
#include <kdebug.h>
#include <kapplication.h>

#include "calformat.h"

using namespace KCal;

QString CalFormat::mApplication = QString::fromLatin1("libkcal");
QString CalFormat::mProductId = QString::fromLatin1("-//K Desktop Environment//NONSGML libkcal 3.0//EN");


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
