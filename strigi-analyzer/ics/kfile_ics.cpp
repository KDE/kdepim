/* This file is part of the KDE project
 * Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 * Copyright (C) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that
t it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

//#include <KDebug>

#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>
#include <kcal/vcalformat.h>
#include <kcal/todo.h>

#include "kfile_ics.h"

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/streamendanalyzer.h>

using namespace KCal;

IcsEndAnalyzer::IcsEndAnalyzer( const IcsEndAnalyzerFactory* f )
  : m_factory( f )
{
}

bool IcsEndAnalyzer::checkHeader( const char* header, int32_t headersize ) const
{
  const char* magic = "BEGIN:VCALENDAR";
  int32_t magicLength = strlen( magic );

  return headersize >= magicLength && !strncmp( magic, header, magicLength );
}

/*
I chose to use libkcal instead of reading the calendar manually. It's easier to
maintain this way.
*/
char IcsEndAnalyzer::analyze( Strigi::AnalysisResult& idx, jstreams::InputStream* in )
{
  CalendarLocal cal( QString::fromLatin1( "UTC" ) );

  const char* data;
  //FIXME: large calendars will exhaust memory; incremental loading would be nice
  if ( in->read( data, 1, in->getSize() ) < 0 ) {
    //kDebug() << "Reading data from input stream failed" << endl;
    return jstreams::Error;
  }

  ICalFormat ical;
  if ( !ical.fromRawString( &cal, data ) ) {
    VCalFormat vcal;
    if ( !vcal.fromRawString( &cal, data ) ) {
      //kDebug() << "Could not load calendar" << endl;
      return jstreams::Error;
    }
  }

  idx.setField( m_factory->field( ProductId ), cal.productId().toUtf8().data() );
  idx.setField( m_factory->field( Events ), cal.events().count() );
  idx.setField( m_factory->field( Journals ), cal.journals().count() );
  Todo::List todos = cal.todos();

  // count completed and overdue
  int completed = 0;
  int overdue = 0;
  foreach ( const Todo* todo, todos ) {
    if ( todo->isCompleted() ) {
      ++completed;
    } else if ( todo->hasDueDate() && todo->dtDue().date() < QDate::currentDate() ) {
      ++overdue;
    }
  }

  idx.setField( m_factory->field( Todos ), todos.count() );
  idx.setField( m_factory->field( TodosCompleted ), completed );
  idx.setField( m_factory->field( TodosOverdue ), overdue );

  cal.close();

  return jstreams::Ok;
}

const Strigi::RegisteredField* IcsEndAnalyzerFactory::field( IcsEndAnalyzer::Field f ) const
{
  switch ( f ) {
    case IcsEndAnalyzer::ProductId:
      return productIdField;
      break;
    case IcsEndAnalyzer::Events:
      return eventsField;
      break;
    case IcsEndAnalyzer::Journals:
      return journalsField;
      break;
    case IcsEndAnalyzer::Todos:
      return todosField;
      break;
    case IcsEndAnalyzer::TodosCompleted:
      return todosCompletedField;
      break;
    case IcsEndAnalyzer::TodosOverdue:
    default:
      return todosOverdueField;
      break;
  }
}

void IcsEndAnalyzerFactory::registerFields( Strigi::FieldRegister& reg )
{
  // these cnstr's aren't pretty
  static const cnstr product = "Product Id";
  static const cnstr events = "Events";
  static const cnstr journals = "Journals";
  static const cnstr todos = "Todos";
  static const cnstr todoscompleted = "Todos Completed";
  static const cnstr todosoverdue = "Todos Overdue";
  productIdField = reg.registerField( product, Strigi::FieldRegister::stringType, 1, 0 );
  eventsField = reg.registerField( events, Strigi::FieldRegister::integerType, 1, 0 );
  journalsField = reg.registerField( journals, Strigi::FieldRegister::integerType, 1, 0 );
  todosField = reg.registerField( todos, Strigi::FieldRegister::integerType, 1, 0 );
  todosCompletedField = reg.registerField( todoscompleted, Strigi::FieldRegister::integerType, 1, 0 );
  todosOverdueField = reg.registerField( todosoverdue, Strigi::FieldRegister::integerType, 1, 0 );
}

