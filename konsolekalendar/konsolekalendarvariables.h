#ifndef _KONSOLEKALENDAR_VARIABLES_H_
#define _KONSOLEKALENDAR_VARIABLES_H_

/***************************************************************************
        kalendarVariables.h  -  description
           -------------------
    begin                : Sun Jan 6 2002
    copyright            : (C) 2002 by Tuukka Pasanen
    email                : illuusio@mailcity.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatetime.h>
#include <qstring.h>

class KonsoleKalendarVariables
{
  public:
    KonsoleKalendarVariables();
    ~KonsoleKalendarVariables();

    void setDate(QDateTime date);
    QDateTime getDate();
    bool isDate();

    void setStartDate(QDateTime start);
    QDateTime getStartDate();
    bool isStartDate();

    void setEndDate(QDateTime end);
    QDateTime getEndDate();
    bool isEndDate();

    void setNext(bool next);
    bool isNext();

    void setVerbose(bool verbose);
    bool isVerbose();

    void setCalendarFile(QString calendar);
    QString getCalendarFile();
      
    bool isAll();
             
    void setAll( bool all );
    bool getAll();
    
    QDate parseDate( QString string );
    QTime parseTime( QString str );

  private:
    int findNumber(const QString &str, int &pos, int &startpos);
    char findSeparator(const QString &str, int &pos, int &seppos);
    void skipWhiteSpace(const QString &str, int &pos);

    QDateTime m_date;
    bool m_bIsDate;
    QDateTime m_startDate;
    bool m_bIsStartDate;
    QDateTime m_endDate;
    QString m_calendar;
    bool m_bIsEndDate;
    bool m_bNext;
    bool m_bVerbose;
    bool m_bAll;
    int str_length;
};

#endif
