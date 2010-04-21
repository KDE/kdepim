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

#include <akonadi/contact/contactgroupviewer.h>
#include <QtGui/QGraphicsProxyWidget>
#include <akonadi/item.h>


using namespace Akonadi;
using namespace Akonadi::Contact;

ContactGroupViewItem::ContactGroupViewItem(QDeclarativeItem* parent)
  : QDeclarativeItem(parent)
{
  m_viewer = new ContactGroupViewer( 0 );
  m_proxy = new QGraphicsProxyWidget( this );
  m_proxy->setWidget( m_viewer );
}

ContactGroupViewItem::~ContactGroupViewItem()
{
  delete m_viewer;
}

void ContactGroupViewItem::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
  QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
  m_proxy->resize( newGeometry.size() );
}

qint64 ContactGroupViewItem::itemId() const
{
  return m_viewer->contactGroup().id();
}

void ContactGroupViewItem::setItemId(qint64 id)
{
  m_viewer->setContactGroup( Akonadi::Item( id ) );
}

#include "contactgroupviewitem.moc"
