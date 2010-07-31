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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqptrlist.h>
#include <tqregexp.h>
#include <tqclipboard.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqxml.h>

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
  
    bool startElement( const TQString &, const TQString &, const TQString & qName, 
                       const TQXmlAttributes &attributes )
    {
      if ( qName == "event" ) {
        Event *event = new Event;
        TQString uid = "Qtopia" + attributes.value( "uid" );
        event->setUid( uid );

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

        TQString rtype = attributes.value( "rtype" );
        if ( !rtype.isEmpty() ) {
          TQDate startDate = event->dtStart().date();
        
          TQString freqStr = attributes.value( "rfreq" );
          int freq = freqStr.toInt();

          TQString hasEndDateStr = attributes.value( "rhasenddate" );
          bool hasEndDate = hasEndDateStr == "1";

          TQString endDateStr = attributes.value( "enddt" );
          TQDate endDate = toDateTime( endDateStr ).date();

          TQString weekDaysStr = attributes.value( "rweekdays" );
          int weekDaysNum = weekDaysStr.toInt();
          TQBitArray weekDays( 7 );
          int i;
          for( i = 1; i <= 7; ++i ) {
            weekDays.setBit( i - 1, ( 2 << i ) & weekDaysNum ); 
          }

          TQString posStr = attributes.value( "rposition" );
          int pos = posStr.toInt();

          Recurrence *r = event->recurrence();

          if ( rtype == "Daily" ) {
            r->setDaily( freq );
            if ( hasEndDate ) r->setEndDate( endDate );
          } else if ( rtype == "Weekly" ) {
            r->setWeekly( freq, weekDays );
            if ( hasEndDate ) r->setEndDate( endDate );
          } else if ( rtype == "MonthlyDate" ) {
            r->setMonthly( freq );
            if ( hasEndDate )
              r->setEndDate( endDate );
            r->addMonthlyDate( startDate.day() );
          } else if ( rtype == "MonthlyDay" ) {
            r->setMonthly( freq );
            if ( hasEndDate )
              r->setEndDate( endDate );
            TQBitArray days( 7 );
            days.fill( false );
            days.setBit( startDate.dayOfWeek() - 1 );
            r->addMonthlyPos( pos, days );
          } else if ( rtype == "Yearly" ) {
            r->setYearly( freq );
            if ( hasEndDate )
              r->setEndDate( endDate );
          }
        }

        TQString categoryList = attributes.value( "categories" );
        event->setCategories( lookupCategories( categoryList ) );

        TQString alarmStr = attributes.value( "alarm" );
        if ( !alarmStr.isEmpty() ) {
          kdDebug(5800) << "Alarm: " << alarmStr << endl;
          Alarm *alarm = new Alarm( event );
          alarm->setType( Alarm::Display );
          alarm->setEnabled( true );
          int alarmOffset = alarmStr.toInt();
          alarm->setStartOffset( alarmOffset * -60 );
          event->addAlarm( alarm );
        }

        Event *oldEvent = mCalendar->event( uid );
        if ( oldEvent ) mCalendar->deleteEvent( oldEvent );

        mCalendar->addEvent( event );
      } else if ( qName == "Task" ) {
        Todo *todo = new Todo;

        TQString uid = "Qtopia" + attributes.value( "Uid" );
        todo->setUid( uid );
        
        TQString description = attributes.value( "Description" );
        int pos = description.find( '\n' );
        if ( pos > 0 ) {
          TQString summary = description.left( pos );
          todo->setSummary( summary );
          todo->setDescription( description );
        } else {
          todo->setSummary( description );
        }
        
        int priority = attributes.value( "Priority" ).toInt();
//        if ( priority == 0 ) priority = 3;
        todo->setPriority( priority );
        
        TQString categoryList = attributes.value( "Categories" );
        todo->setCategories( lookupCategories( categoryList ) );
        
        TQString completedStr = attributes.value( "Completed" );
        if ( completedStr == "1" ) todo->setCompleted( true );
        
        TQString hasDateStr = attributes.value( "HasDate" );
        if ( hasDateStr == "1" ) {
          int year = attributes.value( "DateYear" ).toInt();
          int month = attributes.value( "DateMonth" ).toInt();
          int day = attributes.value( "DateDay" ).toInt();
          
          todo->setDtDue( TQDateTime( TQDate( year, month, day ) ) );
          todo->setHasDueDate( true );
        }
        
        Todo *oldTodo = mCalendar->todo( uid );
        if ( oldTodo ) mCalendar->deleteTodo( oldTodo );

        mCalendar->addTodo( todo );
      } else if ( qName == "Category" ) {
        TQString id = attributes.value( "id" );
        TQString name = attributes.value( "name" );
        setCategory( id, name );
      }
      
      return true;
    }

    bool warning ( const TQXmlParseException &exception )
    {
      kdDebug(5800) << "WARNING" << endl;
      printException( exception );
      return true;
    }
 
    bool error ( const TQXmlParseException &exception )
    {
      kdDebug(5800) << "ERROR" << endl;
      printException( exception );
      return false;
    }
 
    bool fatalError ( const TQXmlParseException &exception )
    {
      kdDebug(5800) << "FATALERROR" << endl;
      printException( exception );
      return false;
    }
 
    TQString errorString ()
    {
      return "QtopiaParser: Error!";
    }

  protected:
    void printException( const TQXmlParseException &exception )
    {
      kdError() << "XML Parse Error (line " << exception.lineNumber()
                << ", col " << exception.columnNumber() << "): "
                << exception.message() << "(public ID: '"
                << exception.publicId() << "' system ID: '"
                << exception.systemId() << "')" << endl;
    }

    TQDateTime toDateTime( const TQString &value )
    {
      TQDateTime dt;
      dt.setTime_t( value.toUInt() );
      
      return dt;
    }

    TQStringList lookupCategories( const TQString &categoryList )
    {
      TQStringList categoryIds = TQStringList::split( ";", categoryList );
      TQStringList categories;
      TQStringList::ConstIterator it;
      for( it = categoryIds.begin(); it != categoryIds.end(); ++it ) {
        categories.append( category( *it ) );
      }
      return categories;
    }

  private:
    Calendar *mCalendar;

    static TQString category( const TQString &id )
    {
      TQMap<TQString,TQString>::ConstIterator it = mCategoriesMap.find( id );
      if ( it == mCategoriesMap.end() ) return id;
      else return *it;
    }

    static void setCategory( const TQString &id, const TQString &name )
    {
      mCategoriesMap.insert( id, name );
    }

    static TQMap<TQString,TQString> mCategoriesMap;
};

TQMap<TQString,TQString> QtopiaParser::mCategoriesMap;

QtopiaFormat::QtopiaFormat()
{
}

QtopiaFormat::~QtopiaFormat()
{
}

bool QtopiaFormat::load( Calendar *calendar, const TQString &fileName)
{
  kdDebug(5800) << "QtopiaFormat::load() " << fileName << endl;

  clearException();

  QtopiaParser handler( calendar );
  TQFile xmlFile( fileName );
  TQXmlInputSource source( xmlFile );
  TQXmlSimpleReader reader;
  reader.setContentHandler( &handler );
  return reader.parse( source );
}

bool QtopiaFormat::save( Calendar *calendar, const TQString &fileName )
{
  kdDebug(5800) << "QtopiaFormat::save(): " << fileName << endl;

  clearException();

  TQString text = toString( calendar );

  if ( text.isNull() ) return false;

  // TODO: write backup file

  TQFile file( fileName );
  if (!file.open( IO_WriteOnly ) ) {
    setException(new ErrorFormat(ErrorFormat::SaveError,
                 i18n("Could not open file '%1'").arg(fileName)));
    return false;
  }
  TQTextStream ts( &file );
  ts << text;
  file.close();

  return true;
}

bool QtopiaFormat::fromString( Calendar *, const TQString & )
{
  kdDebug(5800) << "QtopiaFormat::fromString() not yet implemented." << endl;
  return false;
}

TQString QtopiaFormat::toString( Calendar * )
{
  return TQString::null;
}
