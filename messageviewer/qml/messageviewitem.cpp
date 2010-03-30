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

#include <math.h>

#include <qabstractitemmodel.h>
#include <QGraphicsProxyWidget>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <kdescendantsproxymodel_p.h>

using namespace MessageViewer;

static double sDirectionThreshHold = 8.5; /// Threshold in pixels

MessageViewItem::MessageViewItem(QDeclarativeItem* parent)
  : QDeclarativeItem(parent)
  , m_mousePressed( false )
  , m_dx( 0 )
  , m_dy( 0 )
{
  m_viewer = new Viewer( 0 );
  m_proxy = new QGraphicsProxyWidget( this );
  m_proxy->setWidget( m_viewer );
  m_proxy->installEventFilter( this );
  m_attachmentProxy = new KDescendantsProxyModel( this );
  m_attachmentProxy->setSourceModel( m_viewer->messageTreeModel() );
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

MessageViewItem::Direction MessageViewItem::direction() const
{
  const double length = sqrt( ( m_dx ^ 2 )  + ( m_dy ^ 2 ) );
  if (length < sDirectionThreshHold )
    return Unknown;

  bool horizontal = false;
  if ( m_dx != 0 ) {
    // We Use an X shape to determine the direction of the move.
    // tan(45) == 1; && tan(-45) == -1; Same for 135 and 225 degrees
    double angle = m_dy / m_dx;
    horizontal = angle > -1 && angle <= 1;
  }

  Direction dir;
  if ( horizontal ) {
    dir = m_dx > 0 ? Right : Left;
  } else {
    dir = m_dy > 0 ? Up : Down;
  }

  return dir;
}

bool MessageViewItem::eventFilter( QObject *obj, QEvent *ev )
{
  if ( ev->type() == QEvent::GraphicsSceneMousePress ) {
    QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>( ev );
    if ( mev->button() == Qt::LeftButton ) {
      m_mousePressed = true;
      return true;
    }
  } else if ( ev->type() == QEvent::GraphicsSceneMouseRelease ) {
    QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>( ev );
    if ( mev->button() == Qt::LeftButton ) {
      Direction dir = direction();
      if ( dir == Left ) {
        emit previousMessageRequest();
      } else if ( dir == Right ) {
        emit nextMessageRequest();
      }

      m_mousePressed = false;
      m_dx = 0;
      m_dy = 0;
      return true;
    }
  } else if ( QEvent::GraphicsSceneMouseMove && m_mousePressed ) {
    QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>( ev );
    m_dx += mev->pos().x() - mev->lastPos().x(); // Moving to right gives positive values
    m_dy += mev->pos().y() - mev->lastPos().y(); // Moving up gives positive values
    Direction dir = direction();
    if ( dir == Up ) {
      for ( int i = 0; i < m_dy; ++i )
        m_viewer->slotScrollUp();
    } else if ( dir == Down ) {
      for ( int i = 0; i < -m_dy; ++i )
        m_viewer->slotScrollDown();
    }
    return true;
  }

  return QObject::eventFilter( obj, ev );
}

qint64 MessageViewItem::messageItemId() const
{
  return 0; // TODO
}

void MessageViewItem::setMessageItemId(qint64 id)
{
  m_viewer->setMessageItem( Akonadi::Item( id ) );
}

QString MessageViewItem::splashMessage() const
{
  return QString(); // TODO
}

void MessageViewItem::setSplashMessage(const QString& message)
{
  if ( message.isEmpty() )
    m_viewer->enableMessageDisplay();
  else
    m_viewer->displaySplashPage( message );
}

QObject* MessageViewItem::messageTreeModel() const
{
  return m_attachmentProxy;
}

#include "messageviewitem.moc"
