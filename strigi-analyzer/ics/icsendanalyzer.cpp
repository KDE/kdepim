/* This file is part of the KDE project
 * Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 * Copyright (C) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
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

#include <kcalcore/icalformat.h>
#include <kcalcore/memorycalendar.h>
#include <kcalcore/todo.h>
#include <kcalcore/vcalformat.h>

#include <strigi/analysisresult.h>
#include <strigi/fieldtypes.h>
#include <strigi/streamendanalyzer.h>

using namespace KCalCore;

IcsEndAnalyzer::IcsEndAnalyzer( const IcsEndAnalyzerFactory *factory )
  : m_factory( factory )
{
}

const char* IcsEndAnalyzer::name() const
{
  return "IcsEndAnalyzer";
}

bool IcsEndAnalyzer::checkHeader( const char* header, qint32 headersize ) const
{
  const char* magic = "BEGIN:VCALENDAR";
  const qint32 magicLength = strlen( magic );

  return headersize >= magicLength && !strncmp( magic, header, magicLength );
}

/*
 * I chose to use libkcal instead of reading the calendar manually. It's easier to
 * maintain this way.
 */
STRIGI_ENDANALYZER_RETVAL IcsEndAnalyzer::analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream )
{
  MemoryCalendar::Ptr calendar( new MemoryCalendar( QString::fromLatin1( "UTC" ) ) );

  const char* data = 0;

  //FIXME: large calendars will exhaust memory; incremental loading would be nice
  const qint32 size = stream->read( data, stream->size(), stream->size() );
  if ( size <= 0 ) {
    return Strigi::Error;
  }

  ICalFormat ical;
  if ( !ical.fromRawString( calendar, QByteArray::fromRawData( data, size ) ) ) {
    VCalFormat vcal;
    if ( !vcal.fromRawString( calendar, data ) ) {
      return Strigi::Error;
    }
  }

  index.addValue( m_factory->productIdField, calendar->productId().toUtf8().data() );
  index.addValue( m_factory->eventsField, static_cast<quint32>( calendar->events().count() ) );
  index.addValue( m_factory->journalsField, static_cast<quint32>( calendar->journals().count() ) );
  Todo::List todos = calendar->todos();

  // count completed and overdue
  int completed = 0;
  int overdue = 0;
  foreach ( const Todo::Ptr todo, todos ) {
    if ( todo->isCompleted() ) {
      ++completed;
    } else if ( todo->hasDueDate() && todo->dtDue().date() < QDate::currentDate() ) {
      ++overdue;
    }
  }

  index.addValue( m_factory->todosField, static_cast<quint32>( todos.count() ) );
  index.addValue( m_factory->todosCompletedField, static_cast<quint32>( completed ) );
  index.addValue( m_factory->todosOverdueField, static_cast<quint32>( overdue ) );

  calendar->close();

  return Strigi::Ok;
}

void IcsEndAnalyzerFactory::registerFields( Strigi::FieldRegister& reg )
{
  productIdField = reg.registerField( "Product Id", Strigi::FieldRegister::stringType, 1, 0 );
  eventsField = reg.registerField( "Events", Strigi::FieldRegister::integerType, 1, 0 );
  journalsField = reg.registerField( "Journals", Strigi::FieldRegister::integerType, 1, 0 );
  todosField = reg.registerField( "Todos", Strigi::FieldRegister::integerType, 1, 0 );
  todosCompletedField = reg.registerField( "Todos Completed", Strigi::FieldRegister::integerType, 1, 0 );
  todosOverdueField = reg.registerField( "Todos Overdue", Strigi::FieldRegister::integerType, 1, 0 );
}

const char* IcsEndAnalyzerFactory::name() const
{
  return "IcsEndAnalyzer";
}

Strigi::StreamEndAnalyzer* IcsEndAnalyzerFactory::newInstance() const
{
  return new IcsEndAnalyzer( this );
}

IcsFactoryFactory::IcsFactoryFactory()
  : componentData( "IcsFactoryFactory" )
{
}

std::list<Strigi::StreamEndAnalyzerFactory*> IcsFactoryFactory::streamEndAnalyzerFactories() const
{
  if ( !componentData.isValid() )
    qFatal( "KComponentData is not valid." );

  std::list<Strigi::StreamEndAnalyzerFactory*> factories;
  factories.push_back( new IcsEndAnalyzerFactory );

  return factories;
}
