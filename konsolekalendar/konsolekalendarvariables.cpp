/***************************************************************************
        konsolekalendarvariables.cpp  -  description
           -------------------
    begin                : Sun Jan 6 2002
    copyright            : (C) 2002-2003 by Tuukka Pasanen
    copyright            : (C) 2003 by Allen Winter
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

#include <klocale.h>
#include <kdebug.h>

#include <qdatetime.h>
#include <qstring.h>

#include <stdlib.h>
#include <iostream>
#include <stdio.h>

#include "konsolekalendarvariables.h"

using namespace KCal;
using namespace std;

KonsoleKalendarVariables::KonsoleKalendarVariables()
{
  m_bIsStartDateTime = false;
  m_bIsEndDateTime = false;
  m_bNext = false;
  m_bVerbose = false;
  m_bDryRun = false;
  m_bDescription = false;
  m_description = "Default description";
  m_bSummary = false;
  m_summary = "Default summary";
  m_bFloating = true;
}

KonsoleKalendarVariables::~KonsoleKalendarVariables()
{
}

void KonsoleKalendarVariables::setStartDateTime(QDateTime start)
{
  m_bIsStartDateTime = true;
  m_startDateTime = start;
}

QDateTime KonsoleKalendarVariables::getStartDateTime()
{
  return m_startDateTime;
}
  
bool KonsoleKalendarVariables::isStartDateTime()
{
  return m_bIsStartDateTime;
}

void KonsoleKalendarVariables::setEndDateTime(QDateTime end)
{
  m_bIsEndDateTime = true;
  m_endDateTime = end;
}

QDateTime KonsoleKalendarVariables::getEndDateTime()
{
  return m_endDateTime;
}

bool KonsoleKalendarVariables::isEndDateTime()
{
  return m_bIsEndDateTime;
}

void KonsoleKalendarVariables::setNext(bool next)
{
  m_bNext = next;
}

bool KonsoleKalendarVariables::isNext()
{
  return m_bNext;
}

void KonsoleKalendarVariables::setVerbose(bool verbose)
{
  m_bVerbose = verbose;
}

bool KonsoleKalendarVariables::isVerbose()
{
  return m_bVerbose;
}

void KonsoleKalendarVariables::setDryRun(bool dryrun)
{
  m_bDryRun = dryrun;
}

bool KonsoleKalendarVariables::isDryRun()
{
  return m_bDryRun;
}

void KonsoleKalendarVariables::setCalendarFile(QString calendar)
{
  m_calendar = calendar;
}

QString KonsoleKalendarVariables::getCalendarFile()
{
  return m_calendar;
}

void KonsoleKalendarVariables::setImportFile(QString calendar)
{
  m_import = calendar;
}

QString KonsoleKalendarVariables::getImportFile()
{
  return m_import;
}


void KonsoleKalendarVariables::setCalendar( CalendarLocal *calendar )
{
	m_caledarLocal = calendar;
}

CalendarLocal *KonsoleKalendarVariables::getCalendar()
{
	return m_caledarLocal;
}


void KonsoleKalendarVariables::setExportType( int export_type )
{
  m_export_type = export_type;
}

int KonsoleKalendarVariables::getExportType()
{
  return m_export_type;
}


bool KonsoleKalendarVariables::isAll()
{
  return m_bAll;
}

void KonsoleKalendarVariables::setAll( bool all)
{
  m_bAll = all;
}

bool KonsoleKalendarVariables::getAll()
{
  return m_bAll;
}

void KonsoleKalendarVariables::setDescription(QString description)
{
  m_bDescription = true;
  m_description = description;
}

QString KonsoleKalendarVariables::getDescription()
{
  return m_description;
}

bool KonsoleKalendarVariables::isDescription()
{
  return m_bDescription;
}

void KonsoleKalendarVariables::setSummary(QString summary)
{
  m_bSummary = true;
  m_summary = summary;
}

QString KonsoleKalendarVariables::getSummary()
{
  return m_summary;
}

bool KonsoleKalendarVariables::isSummary()
{
  return m_bSummary;
}

void KonsoleKalendarVariables::setFloating(bool floating)
{
  m_bFloating = floating;
}

bool KonsoleKalendarVariables::getFloating()
{
  return m_bFloating;
}

void KonsoleKalendarVariables::printSpecs(QString mode)
{
  if( mode.upper() != "VIEW" ) {
    cout << i18n("  What:  ").local8Bit() << getSummary().local8Bit() << endl;
    cout << i18n("  Begin: ").local8Bit() << getStartDateTime().toString(Qt::TextDate).local8Bit() << endl;
    cout << i18n("  End:   ").local8Bit() << getEndDateTime().toString(Qt::TextDate).local8Bit() << endl;
    if( getFloating() == true ) {
      cout << i18n("  No Time Associated with Event").local8Bit() << endl;
    }
    if( getSummary() != getDescription() ) {
      cout << i18n("  Desc:  ").local8Bit() << getDescription().local8Bit() << endl;
    }
  } else {
    cout << i18n("  Begin: ").local8Bit() << getStartDateTime().toString(Qt::TextDate).local8Bit() << endl;
    cout << i18n("  End:   ").local8Bit() << getEndDateTime().toString(Qt::TextDate).local8Bit() << endl;
  }    
}

QTime KonsoleKalendarVariables::parseTime( QString str )
{
	int strpos = 0;
	int startpos = 0;
	unsigned int hours;
	unsigned int mins;
	
	// Time is much simpler task than date.

	// TODO
	// CHECK THIS REALLY OUT!
	// User can add any times he/she like
	
	hours = findNumber( str, strpos, startpos );
	strpos ++;
	mins = findNumber( str, strpos, startpos );

	if( hours > 23 )
	{
		hours = 0;
	}
	
	if( mins > 59 )
	{	
		mins = 0;
	}
	       
	// printf("%d:%d\r\n",hours, mins);
	
	return QTime( hours, mins, 0, 0);
	
}


QDate KonsoleKalendarVariables::parseDate(QString str)
{  
  int strpos=0,   // actual position in string
  errpos=-1;    // position of first error in string, or -1 for "no error"

  bool lookNumber, done;

  int numbers[3]; // the three numbers making up a date. Order depends on inputmode
  int numstart;
  char separator = '\0';
  int actnum=0;   // the index of the next number
  int cursorpos = 0;

  int actsep=0,     // index of the next separator
  tottok=0,     // how many items/tokens have been parsed?
  seppos;       // position of last separator parsed

  int format = -1;

  /* For not having to call QString::length() frequently without knowing whether
   * that function starts to count it chars every time, we save that value
   */

  str_length=str.length();

  numbers[0]=numbers[1]=numbers[2]=-1;

  lookNumber=true;
  done=false;

  /* We parse the string until
   * - there is an error ( errpos!=-1 ), or
   * - we reach the end of the string ( strpos>=str.length() ), or
   * - we found everything that makes up a date
   */

  while( (errpos==-1) && (strpos < str_length) && (!done)) {
    if( lookNumber ) {
      // We are currently looking for a number
      if(( numbers[actnum]=findNumber(str,strpos,numstart) )==-1) {
        // but be reached the end of the string
        done=true;
      } else {
        /* if num==-2, this means that there was anything else.
         * this could mean
         * that the user deleted the number in-between some separators
         * and is just about to enter a new number
         */
        if(numbers[actnum]==-2) {
          numbers[actnum]=-1;
        }

        // since we found a number, we increase the counters
        actnum++;
        tottok++;

        /* if we found a total of three numbers, we're done.
         * if not, there should come a separator
         */
        if(actnum==3)
          done=true;
        else
          lookNumber=false;
      }
    } else {
      // We are currently looking for a separator
      switch(actsep) {
        case 0:
          // It's the first sep, so look what the user preferres
          separator = findSeparator(str,strpos,seppos);
          switch(separator) {
            case '.':
              // german format 'dd.mm.yyyy'
              format = 1;
              break;
            case '-':
              format = 2;
              break;
            case '/':
              // normal format 'mm/dd/yyyy' or 'mm-dd-yyyy'
              format = 3;
              break;
            default:
              // anything else we did not expect
              errpos=seppos;
          }
          break;

        case 1:
          // The second sep must be the same as the first (Not 1-1/2000)
          if(separator!=findSeparator(str,strpos,seppos))
            errpos=seppos;
          break;

      }

      // Increase all the counters
      actsep++;
      tottok++;

      lookNumber=true;
    };
  }


  /* We're through parsing.
   *
   * If there was no error, this could mean that
   * 1) the string ended before we found a complete date
   * 2) We found a complete date
   *
   * In the second case, there could be non-whitespace garbage at the end of the
   * string, which leads to an error.
   *
   * The test does nothing in the first case, since the string is already used up.
   */
  if(errpos==-1) {
    if(strpos<str_length) {
      skipWhiteSpace(str,strpos);
    
      if(strpos<str_length) {
        // There is garbage. ERROR!!!
        errpos=strpos;
      }
    }
  }

  // If there was an error, we can't do anymore.
  if(errpos!=-1) {
    cursorpos=errpos;
    return QDate::currentDate();
  };

  /* Now, we have anything the user gave us.
   *
   * So we can now check whether
   * - the user gave us enough
   * - he entered a real date
   * - he used a two-digit year which is not nice
   */


  // First, we sort the three numbers into day, month and year
  switch(format) {
    case 1:
      //ddescr.day=numbers[0];
      //ddescr.month=numbers[1];
      //ddescr.year=numbers[2];

      return QDate(numbers[2], numbers[1], numbers[0]);

    case 2:
    case 3:
       //ddescr.day=numbers[1];
       //ddescr.month=numbers[0];
       //ddescr.year=numbers[2];
       return QDate(numbers[2], numbers[1], numbers[0]);

    default:
      break;
  };
  return QDate::currentDate();
}

// res: Number, -1 reached end, -2 garbage
int KonsoleKalendarVariables::findNumber(const QString &str, int &pos, int &startpos)
{
  skipWhiteSpace(str,pos);

  if(pos >= str_length)
    return -1;

  startpos=pos;

  while( (pos<str_length) && (str[pos]>='0') && (str[pos]<='9') )
    pos++;

  if(startpos==pos)
    return -2;

  return str.mid(startpos,pos-startpos).toInt();
}


// res: char, 0 reached end
char KonsoleKalendarVariables::findSeparator(const QString &str, int &pos, int &seppos)
{
  skipWhiteSpace(str,pos);

  if(pos>=str_length) {
    seppos=-1;
    return 0;
  };

  seppos=pos;
  pos++;
  return str[seppos];
}

#define ISWHITESPACE(c) ( ((c)==' ') || ((c)=='\t') || ((c)=='\n') || ((c)=='\r') )

void KonsoleKalendarVariables::skipWhiteSpace(const QString &str, int &pos)
{
  while( (pos<str_length) && ISWHITESPACE(str[pos]) )
    pos++;
}
