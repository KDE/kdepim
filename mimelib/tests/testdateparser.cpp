/*
  This file is part of the KDE PIM project.

  Copyright (C) 2009 Martin Koller <kollix@aon.at>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>
#include "testdateparser.h"

#include <mimelib/datetime.h>

#include <QList>
#include <QByteArray>

//---------------------------------------------------------------------

QTEST_KDEMAIN( TestDateParser, NoGUI )

//---------------------------------------------------------------------

void TestDateParser::testParsing()
{
  QList<QByteArray> dates;

  // first of a pair is string to parse (local time),
  // second is the same in ISO-8601 format (YYYY-MM-DDTHH:MM:SS) UTC
  // non-RFC2822 dates
  dates << "Mon Dec 15 11:04:05 PST 2003"      << "2003-12-15T19:04:05";
  dates << "Tue, Feb 04, 2003 00:01:20 +0000"  << "2003-02-04T00:01:20";
  dates << "Tue, Feb 04, 2003 10:01:20 -0220"  << "2003-02-04T12:21:20";
  dates << "Fri, 18 Sep 2009 04:44:55 -0400"   << "2009-09-18T08:44:55";
  dates << "Tue Feb 04, 2003 00:01:20 +0000"   << "2003-02-04T00:01:20";
  dates << "Fri Oct 14 09:21:49 CEST 2005"     << "2005-10-14T07:21:49";
  dates << "Tue Mar 23 18:00:02 2004"          << "2004-03-23T18:00:02";

  QByteArray date;
  for (int i = 0; i < dates.count(); i += 2)
  {
    DwDateTime dwt(dates[i].constData());
    dwt.Parse();

    QDateTime test;
    test.setTime_t(dwt.AsUnixTime());

    QDateTime iso;
    iso = QDateTime::fromString(dates[i+1], Qt::ISODate);
    iso.setTimeSpec(Qt::UTC);
    //qDebug() << test << iso;
    QVERIFY2(test == iso, dates[i]);
  }
}

//---------------------------------------------------------------------
