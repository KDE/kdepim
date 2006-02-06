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

#include <QDateTime>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include <kdebug.h>

extern "C" {
  #include <ical.h>
}

#include "icalformat.h"
#include "icalformatimpl.h"
#include "icaltimezones.h"


using namespace KCal;


// Convert an ical time to QDateTime, preserving the UTC indicator
static QDateTime toQDateTime(const icaltimetype &t)
{
  // QDate can only handle Gregorian dates, which started to be introduced
  // on 1752-09-14.
  return QDateTime(QDate((t.year >= 1753 ? t.year : 1753), t.month, t.day),
                   QTime(t.hour, t.minute, t.second),
                   (t.is_utc ? Qt::UTC : Qt::LocalTime));
}


namespace KCal {


/******************************************************************************/

ICalTimezone::ICalTimezone(ICalTimezoneSource *source, const QString &name, ICalTimezoneData *data)
  : KTimezone(source, name)
{
  setData(data);
}

ICalTimezone::ICalTimezone(const ICalTimezone &tz)
  : KTimezone(tz)
{
}

ICalTimezone::~ICalTimezone()
{
}

ICalTimezone &ICalTimezone::operator=(const ICalTimezone &tz)
{
  KTimezone::operator=(tz);
  return *this;
}

QString ICalTimezone::city() const
{
  const ICalTimezoneData *dat = static_cast<const ICalTimezoneData*>(data());
  return dat ? dat->location : QString();
}

QByteArray ICalTimezone::url() const
{
  const ICalTimezoneData *dat = static_cast<const ICalTimezoneData*>(data());
  return dat ? dat->url : QByteArray();
}

QDateTime ICalTimezone::lastModified() const
{
  const ICalTimezoneData *dat = static_cast<const ICalTimezoneData*>(data());
  return dat ? dat->lastModified : QDateTime();
}

int ICalTimezone::offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset) const
{
  if (secondOffset)
    *secondOffset = 0;
  const ICalTimezoneData *dat = static_cast<const ICalTimezoneData*>(data());
  if (!zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime  ||  !dat)
    return 0;
  QDateTime start;
  const ICalTimezoneData::Phase *phase = dat->phase(zoneDateTime, &start);
  if (!phase)
    return 0;
  if (secondOffset) {
    // Need to check whether the specified local time occurs twice
    // (if it's around the time of a clock shift).
    *secondOffset = phase->offset;
    if (start != dat->phases[0]->start) {   // no information before start of first phase
      int offsetDiff = phase->prevOffset - phase->offset;
      if (offsetDiff > 0) {
        // The last clock change was backwards, so the local time could occur twice
        QDateTime utc = zoneDateTime;   // convert local time to UTC
        utc.setTimeSpec(Qt::UTC);
        utc = utc.addSecs(-phase->offset);
        int sinceStart = start.secsTo(utc);    // how long since start of phase?
        if (sinceStart < offsetDiff)
          return phase->prevOffset;
      }
    }
  }
  return phase->offset;
}

int ICalTimezone::offsetAtUTC(const QDateTime &utcDateTime) const
{
  const ICalTimezoneData *dat = static_cast<const ICalTimezoneData*>(data());
  if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC  ||  !dat)
    return 0;
  const ICalTimezoneData::Phase *phase = dat->phase(utcDateTime);
  if (!phase)
    return 0;
  return phase->offset;
}

int ICalTimezone::offset(time_t t) const
{
  return offsetAtUTC(fromTime_t(t));
}

bool ICalTimezone::isDstAtUTC(const QDateTime &utcDateTime) const
{
  const ICalTimezoneData *dat = static_cast<const ICalTimezoneData*>(data());
  if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC  ||  !dat)
    return false;
  const ICalTimezoneData::Phase *phase = dat->phase(utcDateTime);
  return phase ? phase->isDst : false;
}

bool ICalTimezone::isDst(time_t t) const
{
  return isDstAtUTC(fromTime_t(t));
}


/******************************************************************************/

class ICalTimezoneDataPrivate
{
public:
    QList<int> utcOffsets;
};


ICalTimezoneData::ICalTimezoneData()
  : d(new ICalTimezoneDataPrivate())
{
}

ICalTimezoneData::~ICalTimezoneData()
{
  for (int i = 0, end = phases.count();  i < end;  ++i)
    delete phases[i];
  delete d;
}

ICalTimezoneData::Phase::Phase(const ICalTimezoneData::Phase &p)
  : tznameIndex(p.tznameIndex),
    start(p.start),
    offset(p.offset),
    prevOffset(p.prevOffset),
    comment(p.comment),
    recur(p.recur ? new Recurrence(*p.recur) : 0),
    isDst(p.isDst)
{
}

ICalTimezoneData::Phase::~Phase()
{
  delete recur;
}

KTimezoneData *ICalTimezoneData::clone()
{
  ICalTimezoneData *newData = new ICalTimezoneData();
  newData->location     = location;
  newData->url          = url;
  newData->lastModified = lastModified;
  newData->tznames      = tznames;
  for (int i = 0, end = phases.count();  i < end;  ++i)
    newData->phases.append(new Phase(*phases[i]));
  newData->d->utcOffsets = d->utcOffsets;
  return newData;
}

QList<QByteArray> ICalTimezoneData::abbreviations() const
{
  return tznames;
}

QByteArray ICalTimezoneData::abbreviation(const QDateTime &utcDateTime) const
{
  if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)
    return QByteArray();
  const Phase *ph = phase(utcDateTime);
  return ph ? tznames[ph->tznameIndex[0]] : QByteArray();  // return the first abbreviation
}

QList<int> ICalTimezoneData::UTCOffsets() const
{
  if (d->utcOffsets.isEmpty()) {
    for (int i = 0, end = phases.count();  i < end;  ++i) {
      int offset = phases[i]->offset;
      if (d->utcOffsets.indexOf(offset) < 0)
          d->utcOffsets.append(offset);
    }
    qSort(d->utcOffsets);
  }
  return d->utcOffsets;
}

const ICalTimezoneData::Phase *ICalTimezoneData::phase(const QDateTime &dt, QDateTime *start) const
{
  int result = -1;
  QDateTime latest, previous;

  /* Find the phase with the latest start date at or before dt,
   * and the previous latest start date in any other phase.
   */
  for (int i = 0, end = phases.count();  i < end;  ++i) {
    QDateTime t = phases[i]->previousStart(dt);
    if (!t.isNull()) {
      if (latest.isNull() || t > latest) {
        previous = latest;
        latest = t;
        result = i;
      }
      else if (previous.isNull() || t > previous)
        previous = t;
    }
  }

  if (start) {
    /*
     * Find the actual start of the phase with the latest start date.
     * If other phase(s) didn't occur in the year beforehand, the phase
     * could in effect have started at a previous recurrence.
     *
     * For example, if the RRULEs and RDATEs expanded to the following:
     *    1 October 1969   Standard
     *    1 April 1970     Daylight
     *    1 October 1970   Standard
     *    1 October 1971   Standard
     *    1 April 1972     Daylight
     * Here, there was no daylight savings time in 1971. So the start of
     * the phase in operation on 1 January 1972 would actually be
     * 1 October 1970, not 1 October 1971.
     */
    if (!previous.isNull())
      latest = phases[result]->nextStart(previous);
    else if (!latest.isNull())
      latest = phases[result]->start;
    *start = latest;
  }

  return (result >= 0) ? phases[result] : 0;
}

/**
 * Find the nearest UTC start time of this phase strictly after a given UTC or local time.
 */
QDateTime ICalTimezoneData::Phase::nextStart(const QDateTime &dt) const
{
  QDateTime utc = dt;
  if (dt.timeSpec() == Qt::LocalTime) {
    // Find the equivalent UTC time in this time zone phase
    utc.setTimeSpec(Qt::UTC);
    utc = utc.addSecs(-offset);
  }

  if (!recur  ||  utc < start)
    return (utc < start) ? start : QDateTime();

  return recur->getNextDateTime(utc);
}

/**
 * Find the nearest UTC start time of this phase at or before a given UTC or local time.
 */
QDateTime ICalTimezoneData::Phase::previousStart(const QDateTime &dt) const
{
  QDateTime utc = dt;
  if (dt.timeSpec() == Qt::LocalTime) {
    // Find the equivalent UTC time in this time zone phase
    utc.setTimeSpec(Qt::UTC);
    utc = utc.addSecs(-offset);
  }

  if (!recur  ||  utc <= start)
    return (utc >= start) ? start : QDateTime();

  return recur->getPreviousDateTime(utc.addSecs(1));
}


/******************************************************************************/

class ICalTimezoneSourcePrivate
{
  public:
    static ICalTimezoneData::Phase *parsePhase(icalcomponent*, ICalTimezoneData*, bool daylight);
};


ICalTimezoneSource::ICalTimezoneSource()
{
}

ICalTimezoneSource::~ICalTimezoneSource()
{
}

bool ICalTimezoneSource::parse(const QString &fileName, KTimezones &zones)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
    return false;
  QTextStream ts(&file);
  ts.setCodec( "ISO 8859-1" );
  QByteArray text = ts.readAll().trimmed().toLatin1();
  file.close();

  bool result = false;
  icalcomponent *calendar = icalcomponent_new_from_string(text.data());
  if (calendar) {
    if (icalcomponent_isa(calendar) == ICAL_VCALENDAR_COMPONENT)
      result = parse(calendar, zones);
    icalcomponent_free(calendar);
  }
  return result;
}

bool ICalTimezoneSource::parse(icalcomponent *calendar, KTimezones &zones)
{
  for (icalcomponent *c = icalcomponent_get_first_component(calendar, ICAL_VTIMEZONE_COMPONENT);
       c;  c = icalcomponent_get_next_component(calendar, ICAL_VTIMEZONE_COMPONENT))
  {
    ICalTimezone *zone = parse(c);
    if (!zone)
      return false;
    if (!zones.add(zone)) {
      delete zone;
      return false;
    }
  }
  return true;
}

ICalTimezone *ICalTimezoneSource::parse(icalcomponent *vtimezone)
{
  QString name;
  QString xlocation;
  ICalTimezoneData* data = new ICalTimezoneData();

  // Read the fixed properties which can only appear once in VTIMEZONE
  icalproperty *p = icalcomponent_get_first_property(vtimezone, ICAL_ANY_PROPERTY);
  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_TZID_PROPERTY:
        name = QString::fromUtf8(icalproperty_get_tzid(p));
        break;

      case ICAL_TZURL_PROPERTY:
        data->url = icalproperty_get_tzurl(p);
        break;

      case ICAL_LOCATION_PROPERTY:
        // This isn't mentioned in RFC2445, but libical reads it ...
        data->location = QString::fromUtf8(icalproperty_get_location(p));
        break;

      case ICAL_X_PROPERTY: {   // use X-LIC-LOCATION if LOCATION is missing
        const char *xname = icalproperty_get_x_name(p);
        if (xname  &&  !strcmp(xname, "X-LIC-LOCATION"))
            xlocation = QString::fromUtf8(icalproperty_get_x(p));
        break;
      }
      case ICAL_LASTMODIFIED_PROPERTY: {
        icaltimetype t = icalproperty_get_lastmodified(p);
        if (t.is_utc) {
          data->lastModified = toQDateTime(t);
        } else {
          kDebug(5800) << "ICalTimezoneSource::parse(): LAST-MODIFIED not UTC" << endl;
        }
        break;
		}
      default:
        break;
    }
    p = icalcomponent_get_next_property(vtimezone, ICAL_ANY_PROPERTY);
  }

  if (name.isEmpty()) {
    kDebug(5800) << "ICalTimezoneSource::parse(): TZID missing" << endl;
    delete data;
    return 0;
  }
  if (data->location.isEmpty()  &&  !xlocation.isEmpty())
    data->location = xlocation;
  kDebug(5800) << "---zoneId: \"" << name << '"' << endl;

  /*
   * Iterate through all time zone rules for this VTIMEZONE,
   * and create a Phase object containing details for each one.
   */
  for (icalcomponent *c = icalcomponent_get_first_component(vtimezone, ICAL_ANY_COMPONENT);
       c;  c = icalcomponent_get_next_component(vtimezone, ICAL_ANY_COMPONENT))
  {
    ICalTimezoneData::Phase *phase = 0;
    icalcomponent_kind kind = icalcomponent_isa(c);
    switch (kind) {

      case ICAL_XSTANDARD_COMPONENT:
        kDebug(5800) << "---standard phase: found" << endl;
        phase = ICalTimezoneSourcePrivate::parsePhase(c, data, false);
        break;

      case ICAL_XDAYLIGHT_COMPONENT:
        kDebug(5800) << "---daylight phase: found" << endl;
        phase = ICalTimezoneSourcePrivate::parsePhase(c, data, true);
        break;

      default:
        kDebug(5800) << "ICalTimezoneSource::parse(): Unknown component: " << kind << endl;
        break;
    }
    if (phase)
      data->phases.append(phase);
  }

  if (!data->phases.isEmpty()) {
    // Sort the phases by start date
    qSort(data->phases);

    /* Adjust the start time of the earliest phase to use the
     * phase's UTC offset instead of the previous phase's (since
     * we don't have any information about the previous phase)
     */
    ICalTimezoneData::Phase *phase = data->phases[0];
    phase->start = phase->start.addSecs(phase->prevOffset - phase->offset);
  }

  return new ICalTimezone(this, name, data);
}

ICalTimezoneData::Phase *ICalTimezoneSourcePrivate::parsePhase(icalcomponent *c, ICalTimezoneData *data, bool daylight)
{
  // Read the observance data for this standard/daylight savings phase.
  ICalTimezoneData::Phase *phase = new ICalTimezoneData::Phase;
  phase->isDst = daylight;

  bool recurs             = false;
  bool found_dtstart      = false;
  bool found_tzoffsetfrom = false;
  bool found_tzoffsetto   = false;
  icaltimetype dtstart = icaltime_null_time();

  // Now do the ical reading.
  icalproperty *p = icalcomponent_get_first_property(c, ICAL_ANY_PROPERTY);
  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_TZNAME_PROPERTY:     // abbreviated name for this time offset
      {
        // TZNAME can appear multiple times in order to provide language
        // translations of the time zone offset name.
#warning Does this cope with multiple language specifications?
        QByteArray tzname = icalproperty_get_tzname(p);
        // Outlook (2000) places "Standard Time" and "Daylight Time" in the TZNAME
        // strings, which is totally useless. So ignore those.
        if (!daylight  &&  tzname == "Standard Time"
        ||  daylight  &&  tzname == "Daylight Time")
          break;
        int i = data->tznames.indexOf(tzname);
        if (i < 0) {
          data->tznames.append(tzname);
          i = data->tznames.count() - 1;
        } 
        phase->tznameIndex.append(i);
        break;
      }
      case ICAL_DTSTART_PROPERTY:      // local time at which phase starts
        dtstart = icalproperty_get_dtstart(p);
        found_dtstart = true;
        break;

      case ICAL_TZOFFSETFROM_PROPERTY:    // UTC offset immediately before start of phase
        phase->prevOffset = icalproperty_get_tzoffsetfrom(p);
        found_tzoffsetfrom = true;
        break;

      case ICAL_TZOFFSETTO_PROPERTY:
        phase->offset = icalproperty_get_tzoffsetto(p);
        found_tzoffsetto = true;
        break;

      case ICAL_COMMENT_PROPERTY:
        phase->comment = QString::fromUtf8(icalproperty_get_comment(p));
        break;

      case ICAL_RDATE_PROPERTY:
      case ICAL_RRULE_PROPERTY:
        recurs = true;
        break;

      default:
        kDebug(5800) << "ICalTimezoneSource::readPhase(): Unknown property: " << kind << endl;
        break;
    }
    p = icalcomponent_get_next_property(c, ICAL_ANY_PROPERTY);
  }

  // Validate the phase data
  if (!found_dtstart || !found_tzoffsetfrom || !found_tzoffsetto) {
    kDebug(5800) << "ICalTimezoneSource::readPhase(): DTSTART/TZOFFSETFROM/TZOFFSETTO missing" << endl;
    delete phase;
    return 0;
  }

  if (recurs) {
    /* RDATE or RRULE is specified. There should only be one or the other, but
     * it doesn't really matter - the code can cope with both.
     * Note that we had to get DTSTART, TZOFFSETFROM, TZOFFSETTO before reading
     * recurrences.
     */
    icalproperty *p = icalcomponent_get_first_property(c, ICAL_ANY_PROPERTY);
    while (p) {
      icalproperty_kind kind = icalproperty_isa(p);
      switch (kind) {

        case ICAL_RDATE_PROPERTY:
        {
          icaltimetype t = icalproperty_get_rdate(p).time;
          if (icaltime_is_date(t)) {
            // RDATE with a DATE value inherits the (local) time from DTSTART
            t.hour    = dtstart.hour;
            t.minute  = dtstart.minute;
            t.second  = dtstart.second;
            t.is_date = 0;
            t.is_utc  = 0;    // dtstart is in local time
          }
          // RFC2445 is a bit vague about whether RDATE is in local time or UTC,
          // so we support both to be safe.
          if (!t.is_utc) {
            t.second -= phase->prevOffset;    // convert to UTC
            t.is_utc = 1;
            icaltime_normalize(t);
          }
          QDateTime dt = toQDateTime(t);
          if (!phase->recur)
            phase->recur = new Recurrence();
          phase->recur->addRDateTime(dt);
          break;
        }
        case ICAL_RRULE_PROPERTY:
        {
          RecurrenceRule *r = new RecurrenceRule();
	  ICalFormat icf;
	  ICalFormatImpl impl(&icf);
#warning Check that icalproperty_get_rrule(p).until.zone is 0 (otherwise it will not work)
          impl.readRecurrence(icalproperty_get_rrule(p), r);
          if (!phase->recur)
            phase->recur = new Recurrence();
          phase->recur->addRRule(r);
          break;
        }
        default:
          break;
      }
      p = icalcomponent_get_next_property(c, ICAL_ANY_PROPERTY);
    }
  }

  // Convert DTSTART from local time to UTC
  dtstart.second -= phase->prevOffset;
  dtstart.is_utc = 1;
  phase->start = toQDateTime(icaltime_normalize(dtstart));

  // Set the UTC start time for the recurrence
  if (phase->recur)
    phase->recur->setStartDateTime(phase->start);

  return phase;
}

}  // namespace KCal
