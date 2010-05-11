/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "incidencegeneraleditor.h"

#include <KCal/Incidence>

#include "ui_incidencegeneral.h"

using namespace KCal;

IncidenceGeneralEditor::IncidenceGeneralEditor( QWidget *parent )
  : QWidget( parent )
  , mUi( new Ui::IncidenceGeneral )
{
  mUi->setupUi( this );
  mUi->mAlarmBell->setPixmap( SmallIcon( "task-reminder" ) );
  mUi->mRecurrenceEditButton->setIcon(
    KIconLoader::global()->loadIcon(
      "task-recurring", KIconLoader::Desktop, KIconLoader::SizeSmall ) );
  mUi->mSecrecyCombo->addItems( Incidence::secrecyList() );
}

EventGeneralEditor::EventGeneralEditor( QWidget *parent )
  : IncidenceGeneralEditor( parent )
{
  mUi->mTodoSpecifics->setVisible( false );
}

TodoGeneralEditor::TodoGeneralEditor( QWidget *parent )
  : IncidenceGeneralEditor( parent )
{ }
