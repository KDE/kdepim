// Gregorian calendar system implementation factory for creation of kde calendar
// systems.
// Also default gregorian and factory classes
// Carlos Moro, <cfmoro@correo.uniovi.es>
// GNU-GPL v.2

#include <qdatetime.h>
#include <qstring.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "kcalendarsystemhijri.h"

#include "kcalendarsystem.h"

KCalendarSystemGregorian::KCalendarSystemGregorian()
{
  kdDebug(5400) << "\nCreated gregorian calendar" << endl;
}

KCalendarSystemGregorian::~KCalendarSystemGregorian()
{
}

QString KCalendarSystemGregorian::monthName(const QDate& date, bool shortName)
{
  kdDebug(5400) << "Gregorian month..." << endl;
  QString q = KGlobal::locale()->monthName(date.month(), shortName) ;

  return q;
}

QString KCalendarSystemGregorian::formatDate(const QDate& date)
{
  kdDebug(5400) << "Gregorian format date..." << endl;
  QString q = KGlobal::locale()->formatDate(date,true) ;

  return q;
}

int KCalendarSystemGregorian::year(const QDate& date)
{
  kdDebug(5400) << "Gregorian year..." <<  endl;
  return date.year();
}

void KCalendarSystemGregorian::nextMonthDate(QDate& temp)
{
  kdDebug(5400) << "Gregorian next month date..." << endl;
  int day = temp.day();
  if(temp.month()==12) {
    temp.setYMD(temp.year()+1, 1, 1);
  } else {
    temp.setYMD(temp.year(), temp.month()+1, 1);
  }

  if(temp.daysInMonth()<day) {
    temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
  } else {
    temp.setYMD(temp.year(), temp.month(), day);
  }
}

void KCalendarSystemGregorian::previousMonthDate(QDate& temp)
{
  kdDebug(5400) << "Gregorian previous month date..." << endl;
  
  int day = temp.day();
  
  if(temp.month()==1) {
    temp.setYMD(temp.year()-1, 12, 1);
  } else {
    temp.setYMD(temp.year(), temp.month()-1, 1);
  }
  if(temp.daysInMonth()<day) {
    temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
  } else {
    temp.setYMD(temp.year(), temp.month(), day);
  }
}


void KCalendarSystemGregorian::nextYearDate(QDate& temp)
{
  kdDebug(5400) << "Gregorian next year date..." << endl;
  int day = temp.day();
  temp.setYMD(temp.year()+1, temp.month(), 1);
  if(temp.daysInMonth()<day) {
    temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
  } else {
    temp.setYMD(temp.year(), temp.month(), day);
  }
}


void KCalendarSystemGregorian::previousYearDate(QDate& temp)
{
  kdDebug(5400) << "Gregorian previous year date..." << endl;
  int day = temp.day();
  temp.setYMD(temp.year()-1, temp.month(), 1);
  if(temp.daysInMonth()<day) {
    temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
  } else {
    temp.setYMD(temp.year(), temp.month(), day);
  }
}


int KCalendarSystemGregorian::monthsInYear( int )
{
  kdDebug(5400) << "Gregorian monthsInYear" << endl;

  return 12;
}

QString KCalendarSystemGregorian::monthName(int month)
{
  kdDebug(5400) << "Gregorian getMonthName" << endl;

  return KGlobal::locale()->monthName(month, false);
}


void KCalendarSystemGregorian::constructDateInMonth(QDate& date, int month)
{
  int day;
  day = date.day();

  date.setYMD(date.year(), month, 1);
  date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));

  kdDebug(5400) << "Gregorian constructDateInMonth" << endl;
}

void KCalendarSystemGregorian::constructDateInYear(QDate& date, int year)
{
  int day;
  day = date.day();

  date.setYMD(year, date.month(), 1);
  date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));

  kdDebug(5400) << "Gregorian constructDateInYear" << endl;
}


QDate KCalendarSystemGregorian::parseDate(QString text)
{
  kdDebug(5400) << "Gregorian parseDate" << endl;
  return KGlobal::locale()->readDate(text);
}


QString KCalendarSystemGregorian::weekDayName(int col, bool shortName)
{
  //kdDebug(5400) << "Gregorian wDayName" << endl;
  return KGlobal::locale()->weekDayName(col, shortName);
}


int KCalendarSystemGregorian::dayOfTheWeek(const QDate& date)
{
  return date.dayOfWeek();
}

int KCalendarSystemGregorian::numberOfDaysInMonth(const QDate& date)
{
  kdDebug(5400) << "Gregorian daysInMonth" << endl;
  return date.daysInMonth();
}

int KCalendarSystemGregorian::numberOfDaysPrevMonth(const QDate& date)
{
  kdDebug(5400) << "Gregorian daysinprevmonth" << endl;
  QDate temp;
  if(date.month() == 1)
  {
    temp.setYMD(date.year()-1, 12, 1);
  } else {
    temp.setYMD(date.year(), date.month()-1, 1);
  }
  return temp.daysInMonth();
}

int KCalendarSystemGregorian::maxValidYear()
{
  return 8000; // QDate limit
}

int KCalendarSystemGregorian::day(const QDate& date)
{
  return date.day();
}

int KCalendarSystemGregorian::month(const QDate& date)
{
  return date.month();
}

int KCalendarSystemGregorian::numberOfDayInYear(const QDate& date)
{
  return date.dayOfYear();
}

int KCalendarSystemGregorian::weekDayOfPray() {
   return 7; // sunday
}

void KCalendarSystemGregorian::printType()
{
  kdDebug(5400) << "It's Gregorian!" << endl;
}


QString KCalendarSystemFactory::calTy[] = { "gregorian", "hijri" };

KCalendarSystemFactory::KCalendarSystemFactory()
{
  kdDebug(5400) << "Created factory calendar" << endl;
}

KCalendarSystemFactory::~KCalendarSystemFactory()
{
}

KCalendarSystem *KCalendarSystemFactory::create( const QString &calType )
{
  if( calType == calTy[1] )
    return  new KCalendarSystemHijri();
  else
    return  new KCalendarSystemGregorian();
}
