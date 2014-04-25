/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "contactgroupviewitem.h"

#include <AkonadiCore/item.h>

using namespace Akonadi;
using namespace Akonadi::Contact;

ExtendedContactGroupViewer::ExtendedContactGroupViewer( QWidget *parent )
  : ContactGroupViewer( parent )
{
}

void ExtendedContactGroupViewer::itemRemoved()
{
  emit contactGroupRemoved();
}


ContactGroupViewItem::ContactGroupViewItem(QDeclarativeItem* parent)
  : DeclarativeAkonadiItem( parent )
{
  m_viewer = new ExtendedContactGroupViewer( 0 );
  connect( m_viewer, SIGNAL(contactGroupRemoved()), SIGNAL(contactGroupRemoved()) );

  setWidget( m_viewer );
}

ContactGroupViewItem::~ContactGroupViewItem()
{
  delete m_viewer;
}

qint64 ContactGroupViewItem::itemId() const
{
  return m_viewer->contactGroup().id();
}

void ContactGroupViewItem::setItemId(qint64 id)
{
  if ( itemId() != id )
    m_viewer->setContactGroup( Akonadi::Item( id ) );
}

