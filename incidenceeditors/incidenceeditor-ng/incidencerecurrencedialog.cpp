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

#include "incidencerecurrencedialog.h"
#include "incidencerecurrenceeditor.h"

#include <QtCore/QDebug>
#include <QtGui/QVBoxLayout>

using namespace IncidenceEditorsNG;

struct IncidenceRecurrenceDialog::Private
{
  IncidenceRecurrenceEditor *mEditor;

  Private();
};

IncidenceRecurrenceDialog::Private::Private()
  : mEditor ( new IncidenceRecurrenceEditor )
{ }

IncidenceRecurrenceDialog::IncidenceRecurrenceDialog( QWidget *parent  )
  : KDialog( parent )
  , d( new IncidenceRecurrenceDialog::Private )
{
  QWidget *widget = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( widget );
  layout->addWidget( d->mEditor );
  layout->addItem( new QSpacerItem( 0,0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding ) );
  setMainWidget( widget );
  setButtons( KDialog::Ok | KDialog::Cancel );
}

IncidenceRecurrenceDialog::~IncidenceRecurrenceDialog()
{
  delete d;
}

void IncidenceRecurrenceDialog::load( const KCal::Recurrence &rec, const QDateTime &from, const QDateTime &to )
{
  d->mEditor->loadPreset( rec, from, to );
}

void IncidenceRecurrenceDialog::IncidenceRecurrenceDialog::save( KCal::Recurrence *rec )
{
  d->mEditor->savePreset( rec );
}

void IncidenceRecurrenceDialog::setDefaults( const QDateTime &from, const QDateTime &to )
{
  d->mEditor->setDefaults( from, to );
}
