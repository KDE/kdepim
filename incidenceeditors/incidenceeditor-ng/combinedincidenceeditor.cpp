/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "combinedincidenceeditor.h"

using namespace IncidenceEditorsNG;

/// public methods

CombinedIncidenceEditor::CombinedIncidenceEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mDirtyEditorCount( 0 )
{ }

void CombinedIncidenceEditor::combine( IncidenceEditor *other )
{
  Q_ASSERT( other );
  mCombinedEditors.append( other );
  connect( other, SIGNAL(dirtyStatusChanged(bool)),
           SLOT(handleDirtyStatusChange(bool)) );
}

bool CombinedIncidenceEditor::isDirty() const
{
  return mDirtyEditorCount > 0;
}

bool CombinedIncidenceEditor::isValid()
{
  foreach ( IncidenceEditor *editor, mCombinedEditors  )
    if ( !editor->isValid() )
      return false;

  return true;
}

void CombinedIncidenceEditor::handleDirtyStatusChange( bool isDirty )
{
  const int prevDirtyCount = mDirtyEditorCount;

  Q_ASSERT( mDirtyEditorCount >= 0 );

  if ( isDirty )
    ++mDirtyEditorCount;
  else
    --mDirtyEditorCount;

  Q_ASSERT( mDirtyEditorCount >= 0 );

  if ( prevDirtyCount == 0 )
    emit dirtyStatusChanged( true );
  if ( mDirtyEditorCount == 0 )
    emit dirtyStatusChanged( false );
}

void CombinedIncidenceEditor::load( KCal::Incidence::ConstPtr incidence )
{
  foreach ( IncidenceEditor *editor, mCombinedEditors  ) {
    // load() may fire dirtyStatusChanged(), reset mDirtyEditorCount to make sure
    // we don't end up with an invalid dirty count.
    editor->blockSignals( true );
    editor->load( incidence );
    editor->blockSignals( false );
    Q_ASSERT( !editor->isDirty() );
  }

  mWasDirty = false;
  mDirtyEditorCount = 0;
  emit dirtyStatusChanged( false );
}

void CombinedIncidenceEditor::save( KCal::Incidence::Ptr incidence )
{
  foreach ( IncidenceEditor *editor, mCombinedEditors  )
    editor->save( incidence );
}

#include "moc_combinedincidenceeditor.cpp"
