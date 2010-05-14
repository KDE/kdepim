/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "combinedincidenceeditor.h"

using namespace IncidenceEditorsNG;

/// public methods

bool CombinedIncidenceEditor::isDirty() const
{
  return mDirtyEditorCount > 0;
}

/// protected methods

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

void CombinedIncidenceEditor::handleDirtyStatusChange( bool isDirty )
{
  Q_ASSERT( mDirtyEditorCount >= 0 );

  if ( isDirty )
    ++mDirtyEditorCount;
  else
    --mDirtyEditorCount;

  Q_ASSERT( mDirtyEditorCount >= 0 );
}

void CombinedIncidenceEditor::load( KCal::Incidence::ConstPtr incidence )
{
  foreach ( IncidenceEditor *editor, mCombinedEditors  )
    editor->load( incidence );
}

void CombinedIncidenceEditor::save( KCal::Incidence::Ptr incidence )
{
  foreach ( IncidenceEditor *editor, mCombinedEditors  )
    editor->save( incidence );
}
