/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qclipboard.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qxml.h>

#include <kdebug.h>
#include <klocale.h>

#include "calendar.h"
#include "calendarlocal.h"

#include "qtopiaformat.h"

using namespace KCal;

class QtopiaParser : public QXmlDefaultHandler
{
  public:
    QtopiaParser( Calendar *calendar ) : mCalendar( calendar ) {}
  
    bool startElement( const QString &, const QString &, const QString & qName, 
                       const QXmlAttributes &attributes )
    {
      if ( qName == "event" ) {
        Event *event = new Event;
        QString uid = attributes.value( "uid" );

        event->setSummary( attributes.value( "description" ) );
        event->setLocation( attributes.value( "location" ) );
        event->setDescription( attributes.value( "note" ) );
        event->setDtStart( toDateTime( attributes.value( "start" ) ) );
        event->setDtEnd( toDateTime( attributes.value( "end" ) ) );
        if ( attributes.value( "type" ) == "AllDay" ) {
          event->setFloats( true );
        } else {
          event->setFloats( false );
        }
        QString rtype = attributes.value( "rtype" );
        if ( !rtype.isEmpty() ) {
          QDate startDate = event->dtStart().date();
        
          QString freqStr = attributes.value( "rfreq" );
          int freq = freqStr.toInt();

          QString hasEndDateStr = attributes.value( "rhasenddate" );
          bool hasEndDate = hasEndDateStr == "1";

          QString endDateStr = attributes.value( "enddt" );
          QDate endDate = toDateTime( endDateStr ).date();

          QString weekDaysStr = attributes.value( "rweekdays" );
          int weekDaysNum = weekDaysStr.toInt();
          QBitArray weekDays( 7 );
          int i;
          for( i = 1; i <= 7; ++i ) {
            weekDays.setBit( i - 1, ( 2 << i ) & weekDaysNum ); 
          }

          QString posStr = attributes.value( "rposition" );
          int pos = posStr.toInt();

          Recurrence *r = event->recurrence();

          if ( rtype == "Daily" ) {
            if ( hasEndDate ) r->setDaily( freq, endDate );
            else r->setDaily( freq, -1 );
          } else if ( rtype == "Weekly" ) {
            if ( hasEndDate ) r->setWeekly( freq, weekDays, endDate );
            else r->setWeekly( freq, weekDays, -1 );
          } else if ( rtype == "MonthlyDay" ) {
            if ( hasEndDate )
              r->setMonthly( Recurrence::rMonthlyDay, freq, endDate );
            else
              r->setMonthly( Recurrence::rMonthlyDay, freq, -1 );
            r->addMonthlyDay( pos );
          } else if ( rtype == "MonthlyDate" ) {
            if ( hasEndDate )
              r->setMonthly( Recurrence::rMonthlyPos, freq, endDate );
            else
              r->setMonthly( Recurrence::rMonthlyPos, freq, -1 );
            QBitArray days( 7 );
            days.fill( false );
            days.setBit( startDate.dayOfWeek() - 1 );
            r->addMonthlyPos( startDate.day(), days );
          } else if ( rtype == "Yearly" ) {
            if ( hasEndDate )
              r->setYearly( Recurrence::rYearlyMonth, freq, endDate );
            else
              r->setYearly( Recurrence::rYearlyMonth, freq, -1 );
            r->addYearlyNum( startDate.month() );
          }
        }

        mCalendar->addEvent( event );
      }
      
      return true;
    }

    bool warning ( const QXmlParseException &exception )
    {
      kdDebug() << "WARNING" << endl;
      printException( exception );
      return true;
    }
 
    bool error ( const QXmlParseException &exception )
    {
      kdDebug() << "ERROR" << endl;
      printException( exception );
      return false;
    }
 
    bool fatalError ( const QXmlParseException &exception )
    {
      kdDebug() << "FATALERROR" << endl;
      printException( exception );
      return false;
    }
 
    QString errorString ()
    {
      return "QtopiaParser: Error!";
    }

  protected:
    void printException( const QXmlParseException &exception )
    {
      kdError() << "XML Parse Error (line " << exception.lineNumber()
                << ", col " << exception.columnNumber() << "): "
                << exception.message() << "(public ID: '"
                << exception.publicId() << "' system ID: '"
                << exception.systemId() << "')" << endl;
    }

    QDateTime toDateTime( const QString &value )
    {
      QDateTime dt;
      dt.setTime_t( value.toUInt() );
      
      return dt;
    }

  private:
    Calendar *mCalendar;
};


QtopiaFormat::QtopiaFormat()
{
}

QtopiaFormat::~QtopiaFormat()
{
}

bool QtopiaFormat::load( Calendar *calendar, const QString &fileName)
{
  kdDebug(5800) << "QtopiaFormat::load() " << fileName << endl;

  clearException();

  QtopiaParser handler( calendar );
  QFile xmlFile( fileName );
  QXmlInputSource source( &xmlFile );
  QXmlSimpleReader reader;
  reader.setContentHandler( &handler );
  return reader.parse( source );
}

bool QtopiaFormat::save( Calendar *calendar, const QString &fileName )
{
  kdDebug(5800) << "QtopiaFormat::save(): " << fileName << endl;

  clearException();

  QString text = toString( calendar );

  if ( text.isNull() ) return false;

  // TODO: write backup file

  QFile file( fileName );
  if (!file.open( IO_WriteOnly ) ) {
    setException(new ErrorFormat(ErrorFormat::SaveError,
                 i18n("Could not open file '%1'").arg(fileName)));
    return false;
  }
  QTextStream ts( &file );
  ts << text;
  file.close();

  return true;
}

bool QtopiaFormat::fromString( Calendar *, const QString & )
{
  kdDebug() << "QtopiaFormat::fromString() not yet implemented." << endl;
  return false;
}

QString QtopiaFormat::toString( Calendar * )
{
  return QString::null;
}
