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
  kdDebug() << "\nCreated gregorian calendar" << endl;
}

KCalendarSystemGregorian::~KCalendarSystemGregorian()
{
}

QString KCalendarSystemGregorian::getMonth(const QDate& date)
{
  kdDebug() << "Gregorian month..." << endl;
  QString q = KGlobal::locale()->monthName(date.month(),false) ;

  return q;
}

QString KCalendarSystemGregorian::getFormatDate(const QDate& date)
{
  kdDebug() << "Gregorian format date..." << endl;
  QString q = KGlobal::locale()->formatDate(date,true) ;

  return q;
}

int KCalendarSystemGregorian::getYear(const QDate& date)
{
  kdDebug() << "Gregorian year..." <<  endl;
  return date.year();
}

void KCalendarSystemGregorian::getNextMonthDate(QDate& temp)
{
  kdDebug() << "Gregorian next month date..." << endl;
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

void KCalendarSystemGregorian::getPreviousMonthDate(QDate& temp)
{
  kdDebug() << "Gregorian previous month date..." << endl;
  
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


void KCalendarSystemGregorian::getNextYearDate(QDate& temp)
{
  kdDebug() << "Gregorian next year date..." << endl;
  int day = temp.day();
  temp.setYMD(temp.year()+1, temp.month(), 1);
  if(temp.daysInMonth()<day) {
    temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
  } else {
    temp.setYMD(temp.year(), temp.month(), day);
  }
}


void KCalendarSystemGregorian::getPreviousYearDate(QDate& temp)
{
  kdDebug() << "Gregorian previous year date..." << endl;
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
  kdDebug() << "Gregorian monthsInYear" << endl;

  return 12;
}

QString KCalendarSystemGregorian::getMonthName(int month)
{
  kdDebug() << "Gregorian getMonthName" << endl;

  return KGlobal::locale()->monthName(month, false);
}


void KCalendarSystemGregorian::constructDateInMonth(QDate& date, int month)
{
  int day;
  day = date.day();

  date.setYMD(date.year(), month, 1);
  date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));

  kdDebug() << "Gregorian constructDateInMonth" << endl;
}

void KCalendarSystemGregorian::constructDateInYear(QDate& date, int year)
{
  int day;
  day = date.day();

  date.setYMD(year, date.month(), 1);
  date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));

  kdDebug() << "Gregorian constructDateInYear" << endl;
}


QDate KCalendarSystemGregorian::parseDate(QString text)
{
  kdDebug() << "Gregorian parseDate" << endl;
  return KGlobal::locale()->readDate(text);
}


QString KCalendarSystemGregorian::wDayName(int col)
{
  //kdDebug() << "Gregorian wDayName" << endl;
  return KGlobal::locale()->weekDayName(col, true);
}


int KCalendarSystemGregorian::dayOfTheWeek(const QDate& date)
{
  kdDebug() << "Gregorian dayOfTheWeek" << endl;
  QDate temp;
  temp.setYMD(date.year(), date.month(), 1);
  return temp.dayOfWeek();
}

int KCalendarSystemGregorian::numberOfDaysInMonth(const QDate& date)
{
  kdDebug() << "Gregorian daysInMonth" << endl;
  return date.daysInMonth();
}

int KCalendarSystemGregorian::numberOfDaysPrevMonth(const QDate& date)
{
  kdDebug() << "Gregorian daysinprevmonth" << endl;
  QDate temp;
  if(date.month() == 1)
  {
    temp.setYMD(date.year()-1, 12, 1);
  } else {
    temp.setYMD(date.year(), date.month()-1, 1);
  }
  return temp.daysInMonth();
}

int KCalendarSystemGregorian::getMaxValidYear()
{
  return 8000; // QDate limit
}

int KCalendarSystemGregorian::getDay(const QDate& date)
{
  return date.day();
}

int KCalendarSystemGregorian::numberOfDayInYear(const QDate& date)
{
  return date.dayOfYear();
}

void KCalendarSystemGregorian::printType()
{
  kdDebug() << "It's Gregorian!" << endl;
}


QString KCalendarSystemFactory::calTy[] = { "gregorian", "hijri" };

KCalendarSystemFactory::KCalendarSystemFactory()
{
  kdDebug() << "Created factory calendar" << endl;
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
