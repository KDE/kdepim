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

#include "messageviewitem.h"
#include <messageviewer/viewer.h>

#include <QGraphicsProxyWidget>

using namespace MessageViewer;

MessageViewItem::MessageViewItem(QDeclarativeItem* parent) : QDeclarativeItem(parent)
{
  m_viewer = new Viewer( 0 );
  KMime::Message::Ptr msg(new KMime::Message);
  msg->setContent(
    "Subject: Hello World!\n"
    "From: vkrause@kde.org\n"
    "To: volker@kdab.com\n"
    "Date: 01-01-1970\n"
    "\n"
    "Hellow World!!!\n"
  );
  msg->parse();
  m_viewer->setMessage( msg );
  m_proxy = new QGraphicsProxyWidget( this );
  m_proxy->setWidget( m_viewer );
}

MessageViewItem::~MessageViewItem()
{
  delete m_viewer;
}

void MessageViewItem::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
  QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
  m_proxy->resize( newGeometry.size() );
}

qint64 MessageViewItem::messageItemId() const
{
  return 0; // TODO
}

void MessageViewItem::setMessageItemId(qint64 id)
{
  m_viewer->setMessageItem( Akonadi::Item( id ) );
}

#include "messageviewitem.moc"
