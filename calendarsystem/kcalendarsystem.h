#ifndef KCALENDARSYSTEM
#define KCALENDARSYSTEM

#include <qdatetime.h>
#include <qstring.h>

/**
  CalendarSystem abstract class, default derived kde gregorian class and
  factory class. Provides support for different calendar types for kde calendar
  widget and related stuff.

  Derived classes must be created through FactoryCalendar class

  @Author: Carlos Moro  <cfmoro@correo.uniovi.es>
  @Licence: GNU-GPL v.2
*/
class KCalendarSystem
{
public:
  KCalendarSystem ()
  {
  }
  virtual ~ KCalendarSystem ()
  {
  }

    /**
     Gets specific calendar type month name for a given gregorian date

     @param date gregorian date
     @return month name 
    */
  virtual QString monthName (const QDate & date) = 0;

    /**
     Gets specific calendar type year for a given gregorian date

     @param date gregorian date
     @return year 
    */
  virtual int year (const QDate & date) = 0;

    /**
     Gets specific calendar type day/month/year formatted text date for a given gregorian date

     @param date gregorian date
     @return  formatted text date
    */
  virtual QString formatDate (const QDate & date) = 0;

    /**
     Gets specific calendar type (its gregorian equivalent) next month date for a given gregorian date

     @param date date to be +1 month updated
    */
  virtual void nextMonthDate (QDate & date) = 0;

    /**
     Gets specific calendar type (its gregorian equivalent) previous month date for a given gregorian date
     @param date date to be -1 month updated
    */
  virtual void previousMonthDate (QDate & date) = 0;

    /**
     Gets specific calendar type next year date (its gregorian equivalent) for a given gregorian date
     @param date date to be +1 year updated
    */
  virtual void nextYearDate (QDate & date) = 0;

    /**
     Gets specific calendar type (its gregorian equivalent) previous year date for a given gregorian date

     @param date date to be -1 year updated
    */
  virtual void previousYearDate (QDate & date) = 0;

    /**
     Gets specific calendar type number of month for a given year

     @param year the year year
     @return  number of months in that year
    */
  virtual int monthsInYear (int year) = 0;

    /**
     Gets specific calendar type month name

     @param month number of month in year
     @return  month name 
    */
  virtual QString monthName (int month) = 0;

    /**
     Given a month, construct a valid date in given date's year

     @param date date that indicates the year and whose value is updated
     @param  month month number which resulting date belongs to
    */
  virtual void constructDateInMonth (QDate & date, int month) = 0;

    /**
     Given a year, construct a valid date 

     @param date date whose value is updated
     @param  year year of the new date
    */
  virtual void constructDateInYear (QDate & date, int year) = 0;

    /**
     Convert a formatted day/month/year string to a valid date object

     @param text day/month/year string
     @return valid parsed date object
    */
  virtual QDate parseDate (QString text) = 0;

    /**
     Gets specific calendar type week day name

     @param col number of day in week (1 -> Monday)
     @param shortName short or complete day name
     @return day name
    */
  virtual QString weekDayName (int col, bool shortName = false) = 0;

    /**
     Gets specific calendar type number of day of week number for a given date

     @param date gregorian date
     @return day of week
    */
  virtual int dayOfTheWeek (const QDate & date) = 0;

    /**
     Gets specific calendar type number of days in month for a given date

     @param date gregorian date 
     @return number of days for month in date
    */
  virtual int numberOfDaysInMonth (const QDate & date) = 0;

    /**
     Gets specific calendar type number of days in previous month for a given date

     @param date gregorian date
     @return number of days for previous month of date
    */
  virtual int numberOfDaysPrevMonth (const QDate & date) = 0;

    /**
     Gets the maximum year value supported by specific calendar type algorithms (QDate, 8000)

     @return maximum year supported
    */
  virtual int maxValidYear () = 0;

    /**
     Gets specific calendar type day number in month for a given date

     @param date gregorian date equivalent to the specific one
     @return day number 
    */
  virtual int day (const QDate & date) = 0;

    /**
     Gets specific calendar type day number in year for a given date

     @param date gregorian date equivalent to the specific one
     @return day number 
    */
  virtual int numberOfDayInYear (const QDate & date) = 0;

    /**
     Just a small debugging test ;)
    */
  virtual void printType () = 0;
};

/**
  Default derived, gregorian calendar class
*/
class KCalendarSystemGregorian:public KCalendarSystem
{
public:
  KCalendarSystemGregorian ();
  virtual ~ KCalendarSystemGregorian ();

  QString monthName (const QDate & date);
  int year (const QDate & date);
  QString formatDate (const QDate & date);
  void nextMonthDate (QDate & date);
  void previousMonthDate (QDate & date);
  void nextYearDate (QDate & date);
  void previousYearDate (QDate & date);
  int monthsInYear (int year);
  QString monthName (int month);
  void constructDateInMonth (QDate & date, int month);
  void constructDateInYear (QDate & date, int year);
  QDate parseDate (QString text);
  QString weekDayName (int col, bool shortName);
  int dayOfTheWeek (const QDate & date);
  int numberOfDaysInMonth (const QDate & date);
  int numberOfDaysPrevMonth (const QDate & date);
  int maxValidYear ();
  int day (const QDate & date);
  int numberOfDayInYear (const QDate & date);
  void printType ();
};

/**
  Factory class for calendar types
*/
class KCalendarSystemFactory
{
public:
  KCalendarSystemFactory ();
  ~KCalendarSystemFactory ();

    /**
     Gets specific calendar type number of days in previous month for a given date

     @param calType string identification of the specific calendar type to be constructed
     @return a KCalendarSystem object
    */
  static KCalendarSystem *create (const QString & calType = "gregorian");

private:
    /**
     Supported calendar types (its names)
    */
  static QString calTy[];
};

#endif
// Best regards ;)
