/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "attendeeeditor.h"

#include <KDebug>

#include <QList>

using namespace IncidenceEditorsNG;

AttendeeEditor::AttendeeEditor( QWidget* parent )
  : MultiplyingLineEditor( new AttendeeLineFactory( parent ), parent )
{
  connect( this, SIGNAL( lineAdded( KPIM::MultiplyingLine* ) ), SLOT( slotLineAdded( KPIM::MultiplyingLine* ) ) );
  connect( this, SIGNAL( lineDeleted( int ) ), SLOT( slotLineDeleted( int ) ) );

  addData();
}

void AttendeeEditor::slotLineAdded( KPIM::MultiplyingLine* line )
{
  kDebug() << "line added";
  AttendeeLine* att = qobject_cast<AttendeeLine*>( line );
  if( !att )
    return;
  connect( att, SIGNAL( changed() ), SLOT( slotCalculateTotal() ) );
}

void AttendeeEditor::slotLineDeleted( int /*pos*/ )
{
  kDebug() << "line deleted";
}

void AttendeeEditor::slotCalculateTotal()
{
  int empty = 0;

  foreach( KPIM::MultiplyingLine *line, lines() ) {
    AttendeeLine* att = qobject_cast< AttendeeLine* >( line );
    if( att &&  att->isEmpty() ) {
        ++empty;
    }
  }

  // We always want at least one empty line
  if ( empty == 0 )
    addData();
}

AttendeeData::List AttendeeEditor::attendees() const
{
  QList<KPIM::MultiplyingLineData::Ptr> dataList = allData();
  AttendeeData::List attList;
  foreach( KPIM::MultiplyingLineData::Ptr datum, dataList ) {
    AttendeeData::Ptr att = qSharedPointerDynamicCast<AttendeeData>( datum );
    if( !att )
      continue;
    attList << att;
  }
  return attList;
}




