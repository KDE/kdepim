#ifndef KCALENDARSYSTEMHIJRI_H
#define KCALENDARSYSTEMHIJRI_H

// Derived hijri calendar class
// Just a schema. It must be a plugin
// It depends on hdate routines... it may be thought a more consistent support...
// Carlos Moro . cfmoro@correo.uniovi.es
// GNU-GPL v.2

#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>

#include "kcalendarsystem.h"

class KCalendarSystemHijri : public KCalendarSystem
{
  public:
    KCalendarSystemHijri();
    virtual ~KCalendarSystemHijri();

    QString getMonth(const QDate& date);
    int getYear(const QDate& date);
    QString getFormatDate(const QDate& date);
    void getNextMonthDate(QDate& date);
    void getPreviousMonthDate(QDate& date);
    void getNextYearDate(QDate& date);
    void getPreviousYearDate(QDate& date);
    int monthsInYear(int year);
    QString getMonthName(int month);
    void constructDateInMonth(QDate& date, int month);
    void constructDateInYear(QDate& date, int year);
    QDate parseDate(QString text);
    QString wDayName(int day);
    int dayOfTheWeek(const QDate& date);
    int numberOfDaysInMonth(const QDate& date);
    int numberOfDaysPrevMonth(const QDate& date);
    int getMaxValidYear();
    int getDay(const QDate& date);
    int numberOfDayInYear(const QDate& date) ;
    void printType();

  private:
    int hndays(int year, int mon);
};

#endif
