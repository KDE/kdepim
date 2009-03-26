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

#include "icsendanalyzer.h"

#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>
#include <kcal/vcalformat.h>
#include <kcal/todo.h>

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/streamendanalyzer.h>

//#include <KDebug>

using namespace KCal;

IcsEndAnalyzer::IcsEndAnalyzer( const IcsEndAnalyzerFactory* f )
  : m_factory( f )
{
}

bool IcsEndAnalyzer::checkHeader( const char* header, qint32 headersize ) const
{
  const char* magic = "BEGIN:VCALENDAR";
  qint32 magicLength = strlen( magic );

  return headersize >= magicLength && !strncmp( magic, header, magicLength );
}

/*
I chose to use libkcal instead of reading the calendar manually. It's easier to
maintain this way.
*/
STRIGI_ENDANALYZER_RETVAL IcsEndAnalyzer::analyze( Strigi::AnalysisResult& idx, Strigi::InputStream* in )
{
  CalendarLocal cal( QString::fromLatin1( "UTC" ) );

  const char* data;
  //FIXME: large calendars will exhaust memory; incremental loading would be
  // nice
  qint32 nread = in->read( data, in->size(), in->size() );
  if ( nread <= 0 ) {
    //kDebug() <<"Reading data from input stream failed";
    return Strigi::Error;
  }

  ICalFormat ical;
  if ( !ical.fromRawString( &cal, QByteArray::fromRawData(data, nread) ) ) {
    VCalFormat vcal;
    if ( !vcal.fromRawString( &cal, data ) ) {
      //kDebug() <<"Could not load calendar";
      return Strigi::Error;
    }
  }

  idx.addValue( m_factory->field( ProductId ), cal.productId().toUtf8().data() );
  idx.addValue( m_factory->field( Events ), (quint32)cal.events().count() );
  idx.addValue( m_factory->field( Journals ), (quint32)cal.journals().count() );
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

  idx.addValue( m_factory->field( Todos ), (quint32)todos.count() );
  idx.addValue( m_factory->field( TodosCompleted ), (quint32)completed );
  idx.addValue( m_factory->field( TodosOverdue ), (quint32)overdue );

  cal.close();

  return Strigi::Ok;
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

void IcsEndAnalyzerFactory::registerFields( Strigi::FieldRegister& reg ) {
  productIdField = reg.registerField("Product Id", Strigi::FieldRegister::stringType, 1, 0 );
  eventsField = reg.registerField("Events", Strigi::FieldRegister::integerType, 1, 0 );
  journalsField = reg.registerField("Journals", Strigi::FieldRegister::integerType, 1, 0 );
  todosField = reg.registerField("Todos", Strigi::FieldRegister::integerType, 1, 0 );
  todosCompletedField = reg.registerField("Todos Completed", Strigi::FieldRegister::integerType, 1, 0 );
  todosOverdueField = reg.registerField("Todos Overdue", Strigi::FieldRegister::integerType, 1, 0 );
}

