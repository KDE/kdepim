/*
    This file is part of libkcal.

    Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <stdlib.h>

#include <qtest_kde.h>
#include <QDateTime>
#include <QTextStream>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <ktempfile.h>

extern "C" {
  #include <libical/ical.h>
}
#include "icaltimezones.h"
#include "testicaltimezones.moc"

using namespace KCal;

static icalcomponent *loadCALENDAR(const char *vcal);
static icalcomponent *loadVTIMEZONE(const char *vtz);

static QDateTime start        (QDate(1967,10,29), QTime(7,0,0), Qt::UTC);
static QDateTime daylight87   (QDate(1987,4,5),   QTime(7,0,0), Qt::UTC);
static QDateTime standardOct87(QDate(1987,10,25), QTime(6,0,0), Qt::UTC);
static QDateTime daylight88   (QDate(1988,4,3),   QTime(7,0,0), Qt::UTC);
static QDateTime daylight97   (QDate(1997,4,6),   QTime(7,0,0), Qt::UTC);
static QDateTime standardOct97(QDate(1997,10,26), QTime(6,0,0), Qt::UTC);
static QDateTime spring98     (QDate(1998,5,5),   QTime(7,0,0), Qt::UTC);
static QDateTime standardOct98(QDate(1998,10,25), QTime(6,0,0), Qt::UTC);
static QDateTime daylight99   (QDate(1998,4,25),  QTime(7,0,0), Qt::UTC);
static QDateTime standardOct99(QDate(1999,10,31), QTime(6,0,0), Qt::UTC);
static QDateTime daylight00   (QDate(2000,4,30),  QTime(7,0,0), Qt::UTC);
static QDateTime spring01     (QDate(2001,5,1),   QTime(7,0,0), Qt::UTC);

// First daylight savings time has an end date, takes a break for a year,
// and is then replaced by another
static const char *VTZ_Western =
  "BEGIN:VTIMEZONE\n"
  "TZID:Test-Dummy-Western\n"
  "LAST-MODIFIED:19870101T000000Z\n"
  "TZURL:http://tz.reference.net/dummies/western\n"
  "LOCATION:Zedland/Tryburgh\n"
  "X-LIC-LOCATION:Wyland/Tryburgh\n"
  "BEGIN:STANDARD\n"
  "DTSTART:19671029T020000\n"
  "RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10\n"
  "TZOFFSETFROM:-0400\n"
  "TZOFFSETTO:-0500\n"
  "TZNAME:WST\n"
  "END:STANDARD\n"
  "BEGIN:DAYLIGHT\n"
  "DTSTART:19870405T020000\n"
  "RRULE:FREQ=YEARLY;BYDAY=1SU;BYMONTH=4;UNTIL=19970405T070000Z\n"
  "TZOFFSETFROM:-0500\n"
  "TZOFFSETTO:-0400\n"
  "TZNAME:WDT1\n"
  "END:DAYLIGHT\n"
  "BEGIN:DAYLIGHT\n"
  "DTSTART:19990425T020000\n"
  "RDATE:20000430T020000\n"
  "TZOFFSETFROM:-0500\n"
  "TZOFFSETTO:-0400\n"
  "TZNAME:WDT2\n"
  "END:DAYLIGHT\n"
  "END:VTIMEZONE\n";

// Standard time only
static const char *VTZ_other =
  "BEGIN:VTIMEZONE\n"
  "TZID:Test-Dummy-Other\n"
  "TZURL:http://tz.reference.net/dummies/other\n"
  "X-LIC-LOCATION:Wyland/Tryburgh\n"
  "BEGIN:STANDARD\n"
  "DTSTART:19500101T000000\n"
  "RDATE:19500101T000000\n"
  "TZOFFSETFROM:0000\n"
  "TZOFFSETTO:0300\n"
  "TZNAME:OST\n"
  "END:STANDARD\n"
  "END:VTIMEZONE\n";

// CALENDAR component header and footer
static const char *calendarHeader =
"BEGIN:VCALENDAR\n"
"PRODID:-//Libkcal//NONSGML ICalTimeZonesTest//EN\n"
"VERSION:2.0\n";
static const char *calendarFooter =
"END:CALENDAR\n";


int main(int argc, char *argv[]) 
{
    setenv("LC_ALL", "C", 1);
    KAboutData aboutData("qttest", "qttest", "version");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication::disableAutoDcopRegistration();
    KApplication app(false);
    ICalTimeZonesTest tc;
    return QtTest::exec(&tc, argc, argv);
}


///////////////////////////
// ICalTimeZoneSource tests
///////////////////////////

void ICalTimeZonesTest::parse()
{
    // Create the full CALENDAR text and write it to a temporary file
    QByteArray text = calendarHeader;
    text += VTZ_Western;
    text += VTZ_other;
    text += calendarFooter;
    KTempFile tmpFile;
    tmpFile.setAutoDelete(true);
    QString path = tmpFile.name();
    *tmpFile.textStream() << text.data();
    tmpFile.close();

    // Parse the file, the CALENDAR text string and the individual VTIMEZONE strings,
    // and check that ICalTimeZone instances with the same names are created in each case.
    ICalTimeZoneSource src;
    KTimeZones timezones1;
    QVERIFY(src.parse(path, timezones1));

    icalcomponent *calendar = loadCALENDAR(text);
    QVERIFY(calendar);
    KTimeZones timezones2;
    QVERIFY(src.parse(calendar, timezones2));

    for (icalcomponent *ctz = icalcomponent_get_first_component(calendar, ICAL_VTIMEZONE_COMPONENT);
         ctz;  ctz = icalcomponent_get_next_component(calendar, ICAL_VTIMEZONE_COMPONENT))
    {
        ICalTimeZone *tz = src.parse(ctz);
        QVERIFY(tz);
        QVERIFY(timezones1.zone(tz->name()));
        QVERIFY(timezones2.zone(tz->name()));
        delete tz;
    }
    icalcomponent_free(calendar);
}


/////////////////////
// ICalTimeZone tests
/////////////////////

void ICalTimeZonesTest::general()
{
    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    QCOMPARE(tz->name(), QString::fromLatin1("Test-Dummy-Western"));
    QCOMPARE(tz->url(), QByteArray("http://tz.reference.net/dummies/western"));
    QCOMPARE(tz->city(), QString::fromLatin1("Zedland/Tryburgh"));
    QCOMPARE(tz->lastModified(), QDateTime(QDate(1987,1,1), QTime(0,0,0), Qt::UTC));

    ICalTimeZone tz1(*tz);
    QCOMPARE(tz1.name(), tz->name());
    QCOMPARE(tz1.url(), tz->url());
    QCOMPARE(tz1.city(), tz->city());
    QCOMPARE(tz1.lastModified(), tz->lastModified());

    vtimezone = loadVTIMEZONE(VTZ_other);
    QVERIFY(vtimezone);
    ICalTimeZone *tz2 = src.parse(vtimezone);
    QVERIFY(tz2);
    icalcomponent_free(vtimezone);

    QCOMPARE(tz2->name(), QString::fromLatin1("Test-Dummy-Other"));
    QCOMPARE(tz2->url(), QByteArray("http://tz.reference.net/dummies/other"));
    QCOMPARE(tz2->city(), QString::fromLatin1("Wyland/Tryburgh"));
    QVERIFY(tz2->lastModified().isNull());

    tz1 = *tz2;
    QCOMPARE(tz1.name(), tz2->name());
    QCOMPARE(tz1.url(), tz2->url());
    QCOMPARE(tz1.city(), tz2->city());
    QCOMPARE(tz1.lastModified(), tz2->lastModified());

    delete tz;
    delete tz2;
}

void ICalTimeZonesTest::offsetAtUTC()
{
    QDateTime local(QDate(2000,6,30), QTime(7,0,0), Qt::LocalTime);

    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    QCOMPARE(tz->offsetAtUTC(start.addSecs(-1)), 0);
    QCOMPARE(tz->offsetAtUTC(start), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight87.addSecs(-1)), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight87), -4*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct87.addSecs(-1)), -4*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct87), -5*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct87.addDays(1)), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight88.addSecs(-1)), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight88), -4*3600);
    QCOMPARE(tz->offsetAtUTC(daylight97.addSecs(-1)), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight97), -4*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct97.addSecs(-1)), -4*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct97), -5*3600);
    QCOMPARE(tz->offsetAtUTC(spring98), -5*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct98.addSecs(-1)), -5*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct98), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight99.addSecs(-1)), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight99), -4*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct99.addSecs(-1)), -4*3600);
    QCOMPARE(tz->offsetAtUTC(standardOct99), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight00.addSecs(-1)), -5*3600);
    QCOMPARE(tz->offsetAtUTC(daylight00), -4*3600);
    QCOMPARE(tz->offsetAtUTC(spring01), -5*3600);
    QCOMPARE(tz->offsetAtUTC(local), 0);

    // Check that copy constructor copies phases correctly
    ICalTimeZone tz1(*tz);
    QCOMPARE(tz1.offsetAtUTC(start.addSecs(-1)), 0);
    QCOMPARE(tz1.offsetAtUTC(start), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight87.addSecs(-1)), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight87), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct87.addSecs(-1)), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct87), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct87.addDays(1)), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight88.addSecs(-1)), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight88), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight97.addSecs(-1)), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight97), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct97.addSecs(-1)), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct97), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(spring98), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct98.addSecs(-1)), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct98), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight99.addSecs(-1)), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight99), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct99.addSecs(-1)), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(standardOct99), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight00.addSecs(-1)), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(daylight00), -4*3600);
    QCOMPARE(tz1.offsetAtUTC(spring01), -5*3600);
    QCOMPARE(tz1.offsetAtUTC(local), 0);

    delete tz;
}

void ICalTimeZonesTest::offset()
{
    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    QCOMPARE(tz->offset((time_t)start.addSecs(-1).toTime_t()), 0);
    QCOMPARE(tz->offset((time_t)start.toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight87.addSecs(-1).toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight87.toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)standardOct87.addSecs(-1).toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)standardOct87.toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)standardOct87.addDays(1).toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight88.addSecs(-1).toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight88.toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)daylight97.addSecs(-1).toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight97.toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)standardOct97.addSecs(-1).toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)standardOct97.toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)spring98.toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)standardOct98.addSecs(-1).toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)standardOct98.toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight99.addSecs(-1).toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight99.toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)standardOct99.addSecs(-1).toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)standardOct99.toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight00.addSecs(-1).toTime_t()), -5*3600);
    QCOMPARE(tz->offset((time_t)daylight00.toTime_t()), -4*3600);
    QCOMPARE(tz->offset((time_t)spring01.toTime_t()), -5*3600);

    delete tz;
}

void ICalTimeZonesTest::offsetAtZoneTime()
{
    int offset2;
    QDateTime l_start(QDate(1967,10,29), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_daylight87(QDate(1987,4,5), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_standardOct87(QDate(1987,10,25), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_daylight88(QDate(1988,4,3), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_daylight97(QDate(1997,4,6), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_standardOct97(QDate(1997,10,26), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_spring98(QDate(1998,5,5), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_standardOct98(QDate(1998,10,25), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_daylight99(QDate(1998,4,25), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_standardOct99(QDate(1999,10,31), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_daylight00(QDate(2000,4,30), QTime(2,0,0), Qt::LocalTime);
    QDateTime l_spring01(QDate(2001,5,1),   QTime(2,0,0), Qt::LocalTime);

    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    // Standard time: start of definitions
    QCOMPARE(tz->offsetAtZoneTime(l_start.addSecs(-1), &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(tz->offsetAtZoneTime(l_start, &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_start.addSecs(3599), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_start.addSecs(3600), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    // Change to daylight savings time
    QCOMPARE(tz->offsetAtZoneTime(l_daylight87.addSecs(-1), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight87, &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight87.addSecs(3599), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight87.addSecs(3600), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    // Change to standard time
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct87.addSecs(-3601), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct87.addSecs(-3600), &offset2), -4*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct87.addSecs(-1), &offset2), -4*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct87, &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct87.addSecs(3599), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct87.addSecs(3600), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    // Change to daylight savings time
    QCOMPARE(tz->offsetAtZoneTime(l_daylight88.addSecs(-1), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight88, &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight88.addSecs(3599), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight88.addSecs(3600), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    // Change to daylight savings time
    QCOMPARE(tz->offsetAtZoneTime(l_daylight97.addSecs(-1), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight97, &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight97.addSecs(3599), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight97.addSecs(3600), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    // Change to standard time
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct97.addSecs(-3601), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct97.addSecs(-3600), &offset2), -4*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct97.addSecs(-1), &offset2), -4*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct97, &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct97.addSecs(3599), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct97.addSecs(3600), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    // In standard time (no daylight savings this year)
    QCOMPARE(tz->offsetAtZoneTime(l_spring98, &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    // Remain in standard time (no daylight savings this year)
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct98.addSecs(-3601), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct98.addSecs(-1), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct98, &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct98.addSecs(3599), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct98.addSecs(3600), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    // Change to daylight savings time
    QCOMPARE(tz->offsetAtZoneTime(l_daylight99.addSecs(-1), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight99, &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight99.addSecs(3599), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight99.addSecs(3600), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    // Change to standard time
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct99.addSecs(-3601), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct99.addSecs(-3600), &offset2), -4*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct99.addSecs(-1), &offset2), -4*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct99, &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct99.addSecs(3599), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_standardOct99.addSecs(3600), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    // Change to daylight savings time
    QCOMPARE(tz->offsetAtZoneTime(l_daylight00.addSecs(-1), &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight00, &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight00.addSecs(3599), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    QCOMPARE(tz->offsetAtZoneTime(l_daylight00.addSecs(3600), &offset2), -4*3600);
    QCOMPARE(offset2, -4*3600);
    // In standard time (no daylight savings this year)
    QCOMPARE(tz->offsetAtZoneTime(l_spring01, &offset2), -5*3600);
    QCOMPARE(offset2, -5*3600);
    // UTC time
    QCOMPARE(tz->offsetAtZoneTime(daylight99.addSecs(-1), &offset2), 0);
    QCOMPARE(offset2, 0);

    delete tz;
}

void ICalTimeZonesTest::abbreviation()
{
    QDateTime local(QDate(2000,6,30), QTime(7,0,0), Qt::LocalTime);

    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    QCOMPARE(tz->abbreviation(start), QByteArray("WST"));
    QCOMPARE(tz->abbreviation(daylight87), QByteArray("WDT1"));
    QCOMPARE(tz->abbreviation(spring98), QByteArray("WST"));
    QCOMPARE(tz->abbreviation(daylight99), QByteArray("WDT2"));
    QCOMPARE(tz->abbreviation(standardOct99), QByteArray("WST"));
    QCOMPARE(tz->abbreviation(spring01), QByteArray("WST"));
    QVERIFY(tz->abbreviation(local).isEmpty());

    QList<QByteArray> abbrs = tz->abbreviations();
    QCOMPARE(abbrs.count(), 3);
    QVERIFY(abbrs.indexOf(QByteArray("WST")) >= 0);
    QVERIFY(abbrs.indexOf(QByteArray("WDT1")) >= 0);
    QVERIFY(abbrs.indexOf(QByteArray("WDT2")) >= 0);

    delete tz;
}

void ICalTimeZonesTest::isDstAtUTC()
{
    QDateTime local(QDate(2000,6,30), QTime(7,0,0), Qt::LocalTime);

    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    QVERIFY(!tz->isDstAtUTC(start.addSecs(-1)));
    QVERIFY(!tz->isDstAtUTC(start));
    QVERIFY(!tz->isDstAtUTC(daylight87.addSecs(-1)));
    QVERIFY(tz->isDstAtUTC(daylight87));
    QVERIFY(tz->isDstAtUTC(standardOct87.addSecs(-1)));
    QVERIFY(!tz->isDstAtUTC(standardOct87));
    QVERIFY(!tz->isDstAtUTC(standardOct87.addDays(1)));
    QVERIFY(!tz->isDstAtUTC(daylight88.addSecs(-1)));
    QVERIFY(tz->isDstAtUTC(daylight88));
    QVERIFY(!tz->isDstAtUTC(daylight97.addSecs(-1)));
    QVERIFY(tz->isDstAtUTC(daylight97));
    QVERIFY(tz->isDstAtUTC(standardOct97.addSecs(-1)));
    QVERIFY(!tz->isDstAtUTC(standardOct97));
    QVERIFY(!tz->isDstAtUTC(spring98));
    QVERIFY(!tz->isDstAtUTC(standardOct98.addSecs(-1)));
    QVERIFY(!tz->isDstAtUTC(standardOct98));
    QVERIFY(!tz->isDstAtUTC(daylight99.addSecs(-1)));
    QVERIFY(tz->isDstAtUTC(daylight99));
    QVERIFY(tz->isDstAtUTC(standardOct99.addSecs(-1)));
    QVERIFY(!tz->isDstAtUTC(standardOct99));
    QVERIFY(!tz->isDstAtUTC(daylight00.addSecs(-1)));
    QVERIFY(tz->isDstAtUTC(daylight00));
    QVERIFY(!tz->isDstAtUTC(spring01));
    QVERIFY(!tz->isDstAtUTC(local));

    delete tz;
}

void ICalTimeZonesTest::isDst()
{
    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    QVERIFY(!tz->isDst((time_t)start.addSecs(-1).toTime_t()));
    QVERIFY(!tz->isDst((time_t)start.toTime_t()));
    QVERIFY(!tz->isDst((time_t)daylight87.addSecs(-1).toTime_t()));
    QVERIFY(tz->isDst((time_t)daylight87.toTime_t()));
    QVERIFY(tz->isDst((time_t)standardOct87.addSecs(-1).toTime_t()));
    QVERIFY(!tz->isDst((time_t)standardOct87.toTime_t()));
    QVERIFY(!tz->isDst((time_t)standardOct87.addDays(1).toTime_t()));
    QVERIFY(!tz->isDst((time_t)daylight88.addSecs(-1).toTime_t()));
    QVERIFY(tz->isDst((time_t)daylight88.toTime_t()));
    QVERIFY(!tz->isDst((time_t)daylight97.addSecs(-1).toTime_t()));
    QVERIFY(tz->isDst((time_t)daylight97.toTime_t()));
    QVERIFY(tz->isDst((time_t)standardOct97.addSecs(-1).toTime_t()));
    QVERIFY(!tz->isDst((time_t)standardOct97.toTime_t()));
    QVERIFY(!tz->isDst((time_t)spring98.toTime_t()));
    QVERIFY(!tz->isDst((time_t)standardOct98.addSecs(-1).toTime_t()));
    QVERIFY(!tz->isDst((time_t)standardOct98.toTime_t()));
    QVERIFY(!tz->isDst((time_t)daylight99.addSecs(-1).toTime_t()));
    QVERIFY(tz->isDst((time_t)daylight99.toTime_t()));
    QVERIFY(tz->isDst((time_t)standardOct99.addSecs(-1).toTime_t()));
    QVERIFY(!tz->isDst((time_t)standardOct99.toTime_t()));
    QVERIFY(!tz->isDst((time_t)daylight00.addSecs(-1).toTime_t()));
    QVERIFY(tz->isDst((time_t)daylight00.toTime_t()));
    QVERIFY(!tz->isDst((time_t)spring01.toTime_t()));

    delete tz;
}

void ICalTimeZonesTest::UTCOffsets()
{
    icalcomponent *vtimezone = loadVTIMEZONE(VTZ_Western);
    QVERIFY(vtimezone);
    ICalTimeZoneSource src;
    ICalTimeZone *tz = src.parse(vtimezone);
    QVERIFY(tz);
    icalcomponent_free(vtimezone);

    vtimezone = loadVTIMEZONE(VTZ_other);
    QVERIFY(vtimezone);
    ICalTimeZone *tz2 = src.parse(vtimezone);
    QVERIFY(tz2);
    icalcomponent_free(vtimezone);

    QList<int> offsets = tz->UTCOffsets();
    QCOMPARE(offsets.count(), 2);
    QCOMPARE(offsets[0], -5*3600);
    QCOMPARE(offsets[1], -4*3600);

    offsets = tz2->UTCOffsets();
    QCOMPARE(offsets.count(), 1);
    QCOMPARE(offsets[0], 3*3600);

    delete tz;
    delete tz2;
}



icalcomponent *loadCALENDAR(const char *vcal)
{
  icalcomponent *calendar = icalcomponent_new_from_string(const_cast<char*>(vcal));
  if (calendar) {
    if (icalcomponent_isa(calendar) == ICAL_VCALENDAR_COMPONENT)
      return calendar;
    icalcomponent_free(calendar);
  }
  return 0;
}

icalcomponent *loadVTIMEZONE(const char *vtz)
{
  icalcomponent *vtimezone = icalcomponent_new_from_string(const_cast<char*>(vtz));
  if (vtimezone) {
    if (icalcomponent_isa(vtimezone) == ICAL_VTIMEZONE_COMPONENT)
      return vtimezone;
    icalcomponent_free(vtimezone);
  }
  return 0;
}
