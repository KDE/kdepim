#ifndef KCALENDARSYSTEMHIJRI_H
#define KCALENDARSYSTEMHIJRI_H

/**
 Derived hijri calendar class
 Just a schema. It must be a plugin
 It depends on hdate routines... it may be thought a more consistent support...
 Carlos Moro . cfmoro@correo.uniovi.es
 GNU-GPL v.2
*/

#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>

#include "kcalendarsystem.h"

/**
 Hijri calendar type implementation
*/
class KCalendarSystemHijri : public KCalendarSystem
{
  public:
    KCalendarSystemHijri();
    virtual ~KCalendarSystemHijri();

    int year(const QDate& date);
    QString formatDate(const QDate& date);
    void nextMonthDate(QDate& date);
    void previousMonthDate(QDate& date);
    void nextYearDate(QDate& date);
    void previousYearDate(QDate& date);
    int monthsInYear(int year);
    QString monthName(int month, bool shortName = false);
    QString monthName(const QDate& date, bool shortName = false);
    void constructDateInMonth(QDate& date, int month);
    void constructDateInYear(QDate& date, int year);
    QDate parseDate(const QString& text);
    QString weekDayName(int col, bool shortName = false);
    int dayOfTheWeek(const QDate& date);
    int numberOfDaysInMonth(const QDate& date);
    int numberOfDaysPrevMonth(const QDate& date);
    int maxValidYear();
    int day(const QDate& date);
    int month(const QDate& date);
    int numberOfDayInYear(const QDate& date) ;
    int weekDayOfPray();
    void printType();

  private:

/**
 Gets the number of days in a month for a given date

 @param year given year
 @param mon month number
 @return number of days in month
*/
    int hndays(int year, int mon);
};

#endif
