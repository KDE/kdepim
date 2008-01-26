/*
    Copyright (c) a2007 Volker Krause <vkrause@kde.org>

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

#include "attendeeselector.h"
#include "ui_attendeeselector.h"

#include <libkdepim/addresseelineedit.h>

#include <klocale.h>
#include <kpushbutton.h>

#include <qlayout.h>

AttendeeSelector::AttendeeSelector(QWidget * parent)
  : KDialogBase( parent, 0, true, i18n("Select Attendees"), Ok|Cancel, NoDefault, true )
{
  ui = new AttendeeSelectorWidget( this );
  setMainWidget( ui );

  QGridLayout *layout = static_cast<QGridLayout*>( ui->layout() );
  layout->setMargin( 0 );

  ui->addButton->setGuiItem( KStdGuiItem::add() );
  connect( ui->addButton, SIGNAL(clicked()), SLOT(addClicked()) );
  ui->removeButton->setGuiItem( KStdGuiItem::remove() );
  connect( ui->removeButton, SIGNAL(clicked()), SLOT(removeClicked()) );

  ui->attendeeEdit->setClickMessage( i18n("Click to add a new attendee") );
  connect( ui->attendeeEdit, SIGNAL(textChanged(const QString&)), SLOT(textChanged(const QString&)) );
  connect( ui->attendeeEdit, SIGNAL(returnPressed(const QString&)), SLOT(addClicked()) );

  connect( ui->attendeeList, SIGNAL(selectionChanged()), SLOT(selectionChanged()) );
}

QStringList AttendeeSelector::attendees() const
{
  QStringList rv;
  for ( uint i = 0; i < ui->attendeeList->count(); ++i )
    rv << ui->attendeeList->item( i )->text();
  return rv;
}

void AttendeeSelector::addClicked()
{
  if ( !ui->attendeeEdit->text().isEmpty() )
    ui->attendeeList->insertItem( ui->attendeeEdit->text() );
  ui->attendeeEdit->clear();
}

void AttendeeSelector::removeClicked()
{
  if ( ui->attendeeList->currentItem() >= 0 )
    ui->attendeeList->removeItem( ui->attendeeList->currentItem() );
}

void AttendeeSelector::textChanged( const QString &text )
{
  ui->addButton->setEnabled( !text.isEmpty() );
}

void AttendeeSelector::selectionChanged()
{
  ui->removeButton->setEnabled( ui->attendeeList->currentItem() >= 0 );
}

#include "attendeeselector.moc"
