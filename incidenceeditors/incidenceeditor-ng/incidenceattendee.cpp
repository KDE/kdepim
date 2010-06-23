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

#include "incidenceattendee.h"
#include "ui_eventortododesktop.h"

#include "attendeeeditor.h"

#include <KDebug>

#include <QGridLayout>

IncidenceEditorsNG::IncidenceAttendee::IncidenceAttendee( Ui::EventOrTodoDesktop* ui )
  : mUi( ui )
  , mAttendeeEditor(  new AttendeeEditor( this ) )
{
  kDebug() << "OMG!";
  QGridLayout *grid = qobject_cast< QGridLayout* >( mUi->mAttendeeTab->layout() );
  if( grid )
    grid->addWidget( mAttendeeEditor, 3, 0, 1, 3 );

   mAttendeeEditor->setCompletionMode( KGlobalSettings::self()->completionMode() );
   mAttendeeEditor->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );

  mUi->mSolveButton->setDisabled( true );
}

void IncidenceEditorsNG::IncidenceAttendee::load( KCal::Incidence::ConstPtr incidence )
{
  //TODO: implement load
}

void IncidenceEditorsNG::IncidenceAttendee::save( KCal::Incidence::Ptr incidence )
{
  //TODO: implement save
}

bool IncidenceEditorsNG::IncidenceAttendee::isDirty() const
{
  return mAttendeeEditor->isModified();
}

bool IncidenceEditorsNG::IncidenceAttendee::isValid()
{
  //TODO: implement isValid
  return true;
}
