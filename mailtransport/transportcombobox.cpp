/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "transportcombobox.h"
#include "transportmanager.h"

#include <kdebug.h>
#include <klocale.h>

#include <qlineedit.h>

using namespace KPIM;

TransportComboBox::TransportComboBox(QWidget * parent) :
    KComboBox( parent )
{
  fillComboBox();
  connect( TransportManager::self(), SIGNAL(transportsChanged()), SLOT(fillComboBox()) );
}

int TransportComboBox::currentTransportId() const
{
  if ( currentIndex() == 0 )
    return 0;
  if( currentIndex() > 0 && currentIndex() < mTransports.count() )
    return mTransports.at( currentIndex() );
  return -1;
}

void TransportComboBox::setCurrentTransport(int transportId)
{
  int i = mTransports.indexOf( transportId );
  if ( i >= 0 && i < count() )
    setCurrentIndex( i );
}

bool TransportComboBox::isAdHocTransport() const
{
  return currentTransportId() == -1;
}

void TransportComboBox::fillComboBox()
{
  int oldTransport = currentTransportId();
  QString oldText;
  if ( lineEdit() )
    oldText = lineEdit()->text();
  clear();
  mTransports.clear();

  if ( !TransportManager::self()->isEmpty() ) {
    QString defName = TransportManager::self()->defaultTransportName();
    if ( defName.isEmpty() )
      addItem( i18n( "Default" ) );
    else
      addItem( i18n( "Default (%1)", defName ) );
    addItems( TransportManager::self()->transportNames() );
    mTransports << 0;
    mTransports << TransportManager::self()->transportIds();
  }

  setCurrentTransport( oldTransport );
  if ( lineEdit() )
    lineEdit()->setText( oldText );
}

#include "transportcombobox.moc"
