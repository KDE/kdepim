#ifndef _KONSOLEKALENDARVARIABLES_H_
#define _KONSOLEKALENDARVARIABLES_H_

/***************************************************************************
        konsolekalendarvariables.h  -  description
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

#include "calendarlocal.h"
#include "event.h"

namespace KCal {

class KonsoleKalendarVariables
{
  public:
    KonsoleKalendarVariables();
    ~KonsoleKalendarVariables();

    void setDate( QDateTime date );
    QDateTime getDate();
    bool isDate();

    void setStartDate( QDateTime start );
    QDateTime getStartDate();
    bool isStartDate();

    void setEndDate( QDateTime end );
    QDateTime getEndDate();
    bool isEndDate();

    void setNext( bool next );
    bool isNext();

    void setVerbose( bool verbose );
    bool isVerbose();

    void setCalendarFile( QString calendar );
    QString getCalendarFile();

    void setImportFile( QString calendar );
    QString getImportFile();


    void setDescription( QString description );
    QString getDescription();
    bool isDescription();

    void setSummary( QString description );
    QString getSummary();
    bool isSummary();


    bool isAll();

    void setAll( bool all );
    bool getAll();

    QDate parseDate( QString string );
    QTime parseTime( QString str );

    /*
     * Set calendar file for global use
     */

   void setCalendar( CalendarLocal *calendar );

   /*
    * Get global calendar
    */

   CalendarLocal *getCalendar();

    /*
     * Set calendar file for global use
     */

   void setExportType( int export_type );

   /*
    * Get global calendar
    */

   int getExportType();


   
   //Q_ENUMS( export_types );
   
   /*
    * Our export types
    */
   enum {
     NONE,
     TEXT_NORMAL,
     TEXT_KORGANIZER,
     HTML,
     XHTML,
     XML,
     CSV,
     VCARD
   } export_types;
   

  private:
    int findNumber( const QString &str, int &pos, int &startpos );
    char findSeparator( const QString &str, int &pos, int &seppos );
    void skipWhiteSpace( const QString &str, int &pos );

    QDateTime m_date;
    bool m_bIsDate;
    QDateTime m_startDate;
    QDateTime m_endDate;
    bool m_bIsStartDate;
    QString m_calendar;
    QString m_import;
    QString m_description;
    QString m_summary;
    bool m_bSummary;
    bool m_bIsEndDate;
    bool m_bNext;
    bool m_bVerbose;
    bool m_bAll;
    bool m_bDescription;
    int str_length;
    int m_export_type;
    // We can use this from everywhere
    CalendarLocal *m_caledarLocal;
   
};

}

#endif
