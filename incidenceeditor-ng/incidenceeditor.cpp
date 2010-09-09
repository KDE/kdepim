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

#include "incidenceeditor-ng.h"

#include <KDebug>

using namespace IncidenceEditorNG;

IncidenceEditor::IncidenceEditor( QObject *parent )
  : QObject( parent )
  , mWasDirty( false )
{ }

IncidenceEditor::~IncidenceEditor()
{ }

void IncidenceEditor::checkDirtyStatus()
{
  if ( !mLoadedIncidence ) {
    kDebug() << "checkDirtyStatus called on an invalid incidence";
    return;
  }

  const bool dirty = isDirty();
  if ( mWasDirty != dirty ) {
    mWasDirty = dirty;
    emit dirtyStatusChanged( dirty );
  }
}

bool IncidenceEditor::isValid() const
{
  return true;
}

QString IncidenceEditor::validate()
{
  return QString();
}


KCalCore::IncidenceBase::IncidenceType IncidenceEditor::type() const
{
  if ( mLoadedIncidence) {
    return mLoadedIncidence->type();
  } else {
    return KCalCore::IncidenceBase::TypeUnknown;
  }
}

#include "moc_incidenceeditor-ng.cpp"
