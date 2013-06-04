/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include <libkdepim/addressline/addresseelineedit.h>
#include <KPIMUtils/Email>

#include <klocale.h>
#include <kpushbutton.h>
#include <kguiitem.h>

AttendeeSelector::AttendeeSelector(QWidget * parent)
  : KDialog( parent )
{
  setCaption( i18n("Select Attendees") );
  setButtons( Ok|Cancel );

  ui.setupUi( mainWidget() );

  ui.addButton->setGuiItem( KStandardGuiItem::add() );
  connect( ui.addButton, SIGNAL(clicked()), SLOT(addClicked()) );
  ui.removeButton->setGuiItem( KStandardGuiItem::remove() );
  connect( ui.removeButton, SIGNAL(clicked()), SLOT(removeClicked()) );

  ui.attendeeEdit->setClickMessage( i18n("Click to add a new attendee") );
  connect( ui.attendeeEdit, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)) );
  connect( ui.attendeeEdit, SIGNAL(returnPressed(QString)), SLOT(addClicked()) );

  connect( ui.attendeeList, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()) );
  enableButtonOk( false );
}

QStringList AttendeeSelector::attendees() const
{
  QStringList rv;
  const int numberOfAttendee( ui.attendeeList->count() );
  for ( int i = 0; i < numberOfAttendee; ++i ) {
    const QString addr = ui.attendeeList->item( i )->text();

    // Build a nice address for this attendee including the CN.
    QString tname, temail;
    KPIMUtils::extractEmailAddressAndName( addr, temail, tname );  // ignore return value
                                                                   // which is always false
    rv << temail;
  }
  return rv;
}

void AttendeeSelector::addClicked()
{
  if ( !ui.attendeeEdit->text().isEmpty() )
    ui.attendeeList->addItem( ui.attendeeEdit->text() );
  ui.attendeeEdit->clear();
  enableButtonOk( true );
}

void AttendeeSelector::removeClicked()
{
  delete ui.attendeeList->takeItem( ui.attendeeList->currentRow() );
  enableButtonOk( ( ui.attendeeList->count()>0 ) );
}

void AttendeeSelector::textChanged( const QString &text )
{
  ui.addButton->setEnabled( !text.isEmpty() );
}

void AttendeeSelector::selectionChanged()
{
  ui.removeButton->setEnabled( ui.attendeeList->currentItem() );
}

#include "attendeeselector.moc"
