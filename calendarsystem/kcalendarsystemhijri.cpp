// Derived hijri kde calendar class
// Just a schema. It must be a plugin
// It depends on hdate routines
// Carlos Moro  <cfmoro@correo.uniovi.es>
// GNU-GPL v.2

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "hconv.h"

#include "kcalendarsystemhijri.h"

KCalendarSystemHijri::KCalendarSystemHijri()
{
  kdDebug(5400) << "\nCreated hijri calendar" << endl;
}

KCalendarSystemHijri::~KCalendarSystemHijri()
{
}

QString KCalendarSystemHijri::monthName(const QDate& date, bool shortName)
{
  kdDebug(5400) << "Arabic month..." <<  endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd;
  sd = hdate(date.year(), date.month(), date.day());
  if ( shortName )
  	return QString(hmnameshort[sd->mon-1]);
  else 
  	return QString(hmname[sd->mon-1]);
}

int KCalendarSystemHijri::year(const QDate& date)
{
  kdDebug(5400) << "Arabic year..." <<  endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd;
  sd = hdate(date.year(), date.month(), date.day());
  if (sd->year>0)
    return (sd->year);
  else
    return (-sd->year);
}

QString KCalendarSystemHijri::formatDate(const QDate& date)
{
  kdDebug(5400) << "Arabic format date..." << endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd;
  sd = hdate(date.year(), date.month(), date.day());
  return QString::number(sd->day) + QString("/") +
    QString::number(sd->mon) + QString("/")+
    QString::number(sd->year);
}

void KCalendarSystemHijri::nextMonthDate(QDate& date)
{
  // firstly, get hijri date from gregorian one
  SDATE  *sd, *gd;
  sd = hdate(date.year(), date.month(), date.day());
  // hijri + 1 month
  int day = sd->day;
  if(sd->mon==12) {
    //setYMD(temp.year()+1, 1, 1);
    sd->year+=1;
    sd->mon=1;
    sd->day=1;
  } else {
    //temp.setYMD(temp.year(), temp.month()+1, 1);
    sd->mon+=1;
  }
  if(hndays(sd->mon, sd->year) < day) {
    //temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
    sd->day = hndays(sd->mon, sd->year);
  } else {
    //temp.setYMD(temp.year(), temp.month(), day);
    sd->day = day;
  }
  // get back gregorian date from new hijri
  gd = gdate( sd -> year, sd -> mon, sd -> day);
  date.setYMD(gd->year, gd->mon, gd->day);
  kdDebug(5400) << "Arabic next month date..." << endl;
}

void KCalendarSystemHijri::previousMonthDate(QDate& date)
{
  kdDebug(5400) << "Arabic previous month date..." << endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd, *gd;
  sd = hdate(date.year(), date.month(), date.day());
  // hijri - 1 month
  int day = sd->day;
  if(sd->mon==1) {
    //temp.setYMD(temp.year()-1, 1, 1);
    sd->year-=1;
    sd->mon = 12;
    sd->day = 1;
  } else {
    //temp.setYMD(temp.year(), temp.month()-1, 1);
    sd->mon-=1;
    //sd->day = 1;
  }
  if(hndays(sd->mon, sd->year)<day) {
    //temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
    sd->day = hndays(sd->mon, sd->year);
  } else {
    //temp.setYMD(temp.year(), temp.month(), day);
    sd->day = day;
  }
  // get back gregorian date from new hijri
  gd = gdate( sd -> year, sd -> mon, sd -> day);
  date.setYMD(gd->year, gd->mon, gd->day);
}

void KCalendarSystemHijri::nextYearDate(QDate& date)
{
  kdDebug(5400) << "Arabic next year date..." << endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd, *gd;
  sd = hdate(date.year(), date.month(), date.day());
  // hijri + 1 year
  int day = sd->day;
  //temp.setYMD(temp.year()+1, temp.month(), 1);
  sd->year+=1;
  sd->day=1;
  if(hndays(sd->mon, sd->year)<day) {
    //temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
  } else {
    //temp.setYMD(temp.year(), temp.month(), day);
    sd->day = day;
  }
  // get back gregorian date from new hijri
  gd = gdate( sd -> year, sd -> mon, sd -> day);
  date.setYMD(gd->year, gd->mon, gd->day);
}

void KCalendarSystemHijri::previousYearDate(QDate& date)
{
  kdDebug(5400) << "Arabic previous year date..." << endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd, *gd;
  sd = hdate(date.year(), date.month(), date.day());
  // hijri - 1 year
  int day = sd->day;
  //temp.setYMD(temp.year()-1, temp.month(), 1);
  sd->year-=1;
  sd->day=1;
  if(hndays(sd->mon, sd->year)<day) {
    //temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
  } else {
    //temp.setYMD(temp.year(), temp.month(), day);
    sd->day = day;
  }
  // get back gregorian date from new hijri
  gd = gdate( sd -> year, sd -> mon, sd -> day);
  date.setYMD(gd->year, gd->mon, gd->day);
}

int KCalendarSystemHijri::monthsInYear( int )
{
  kdDebug(5400) << "Arabic monthsInYear" << endl;
  return 12;
}

QString KCalendarSystemHijri::monthName(int month)
{
  kdDebug(5400) << "Arabic getMonthName " << hmname[month-1] <<endl;
  //return KGlobal::locale()->monthName(month, false);
  return QString(hmname[month-1]);
}

void KCalendarSystemHijri::constructDateInMonth(QDate& date, int month)
{
  kdDebug(5400) << "Arabic constructDateInMonth" << endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd, *gd;
  sd = hdate(date.year(), date.month(), date.day());
  // construct
  int day;
  day = sd->day;

  //date.setYMD(date.year(), month, 1);
  sd->mon = month;
  sd->day = 1;
  //date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));
  sd->mon = month;
  sd->day = QMIN(day, hndays(sd->mon, sd->year));
  // get back gregorian date from new hijri
  gd = gdate( sd -> year, sd -> mon, sd -> day);
  date.setYMD(gd->year, gd->mon, gd->day);
}

void KCalendarSystemHijri::constructDateInYear(QDate& date, int year)
{
  kdDebug(5400) << "Hijri constructDateInYear" << endl;
  // firstly, get hijri date from gregorian one
  SDATE  *sd, *gd;
  sd = hdate(date.year(), date.month(), date.day());
  // construct
  int day;
  day = date.day();

  //date.setYMD(year, date.month(), 1);
  sd->year = year;
  sd->day =1;
  //date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));
  sd->year = year;
  sd->day = QMIN(day, hndays(sd->mon, sd->year));
  // get back gregorian date from new hijri
  gd = gdate( sd -> year, sd -> mon, sd -> day);
  date.setYMD(gd->year, gd->mon, gd->day);
}

QDate KCalendarSystemHijri::parseDate(QString text)
{
  kdDebug(5400) << "Hijri parseDate" << endl;
  // entered values belong to hijri date...
  kdDebug(5400) << "parseDate text " << text << endl;
  int day, month, year;
  QStringList d = QStringList::split("/", text);
  QStringList::Iterator it;
  QString a;

  it = d.at(0);
  a = *it;
  day = a.toInt();

  it = d.at(1);
  a = *it;
  month = a.toInt();

  it = d.at(2);
  a = *it;
  year = a.toInt();

  // convert to gregorian...
  SDATE *gd;
  gd = gdate(year, month, day);
  QDate enteredDate;
  enteredDate.setYMD(gd->year, gd->mon, gd->day);
  kdDebug(5400) << "DEBUG year: " << enteredDate.year() << endl;
  return enteredDate;
}

QString KCalendarSystemHijri::weekDayName(int day, bool shortName)
{
  if( shortName )
     return QString( sdow[day-1] );
  else
     return QString( dow[day-1] );
}

int KCalendarSystemHijri::dayOfTheWeek(const QDate& date)
{
  //kdDebug(5400) << "Hijri dayOfTheWeek" << endl;
  SDATE  *sd;
  // firstly, we get hijri date from actual gregorian date
  sd = hdate(date.year(), date.month(), date.day());
  if( sd->dw == 0 ) 
	return 7;
  else 
	return (sd->dw);
}

int KCalendarSystemHijri::numberOfDaysInMonth(const QDate& date)
{
  kdDebug(5400) << "Hijri daysInMonth" << endl;
  SDATE  *sd;
  // firstly, we get hijri date from actual gregorian date
  sd = hdate(date.year(), date.month(), date.day());
  return hndays(sd->mon, sd->year);
}

int KCalendarSystemHijri::hndays(int mon,int year)
{
  SDATE fd, ld;
  int nd = 666;
  fd = *gdate(year, mon, 1);
  ld = *gdate(year, mon+1, 1);
  ld = *caldate(julianday(ld.year, ld.mon, ld.day, 0.0) -1.0);
  if (fd.mon == ld.mon)
    nd = ld.day - fd.day +1;
  else
    nd = ndays(fd.mon,fd.year) - fd.day + ld.day +1;

  return nd;
}

int KCalendarSystemHijri::numberOfDaysPrevMonth(const QDate& date)
{
  kdDebug(5400) << "Hijri daysinprevmonth" << endl;
  SDATE  *sd, *gd, *sd1;
  // firstly, we get hijri date from actual gregorian date
  sd = hdate(date.year(), date.month(), date.day());

  if(sd->mon == 1) {
    //temp.setYMD(date.year()-1, 12, 1);
    gd = gdate((sd->year)-1, 12, 1);
  } else {
    //temp.setYMD(date.year(), date.month()-1, 1);
    gd = gdate(sd->year, (sd->mon)-1, 1);
  }
  sd1 = hdate(gd->year, gd->mon, gd->day);
  return hndays( sd1->mon, sd1->year);
}

// Max valid year that may be converted to QDate
int KCalendarSystemHijri::maxValidYear()
{
  QDate date(8000, 1, 1);

  SDATE  *sd;
  // firstly, we get hijri date from gregorian date
  sd = hdate(date.year(), date.month(), date.day());

  return sd->year;
}

int KCalendarSystemHijri::day(const QDate& date)
{
  SDATE  *sd;
  // firstly, we get hijri date from actual gregorian date
  sd = hdate(date.year(), date.month(), date.day());

  return sd->day;
}

int KCalendarSystemHijri::month(const QDate& date)
{
  SDATE  *sd;
  // firstly, we get hijri date from actual gregorian date
  sd = hdate(date.year(), date.month(), date.day());

  return sd->mon;
}

int KCalendarSystemHijri::numberOfDayInYear(const QDate& date)
{
  SDATE *sd;
  // firstly, we get hijri date from actual gregorian date
  sd = hdate(date.year(), date.month(), date.day());
  
  int day = 0;
  
  for(int c = 1; c < sd->mon; c++)
    day+=hndays(c, sd->year);
  day+=sd->day;
  
  return day;
}

int KCalendarSystemHijri::weekDayOfPray() {
   return 5; // friday
}
void KCalendarSystemHijri::printType()
{
  kdDebug(5400) << "It's hijri!" << endl;
}
