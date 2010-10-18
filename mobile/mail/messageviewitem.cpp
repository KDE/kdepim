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

#include <math.h>

#include <QtCore/QAbstractItemModel>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <kdescendantsproxymodel_p.h>

#include <messageviewer/headerstyle.h>
#include <messageviewer/headerstrategy.h>
#include <messageviewer/mailwebview.h>
#include <messageviewer/viewer.h>
#include <messageviewer/viewer_p.h>
#include "attachmentproxymodel.h"
#include <KAction>

using namespace MessageViewer;

MessageViewItem::MessageViewItem( QDeclarativeItem* parent )
  : DeclarativeAkonadiItem( parent )
{
  m_viewer = new Viewer( 0 );
  m_viewer->setHeaderStyleAndStrategy( HeaderStyle::mobile(), HeaderStrategy::all() );
  setWidget( m_viewer );

  KDescendantsProxyModel *flatProxy = new KDescendantsProxyModel( this );
  flatProxy->setSourceModel( m_viewer->messageTreeModel() );

  m_attachmentProxy = new AttachmentProxyModel( this );
  m_attachmentProxy->setSourceModel( flatProxy );

  connect( m_viewer, SIGNAL(urlClicked(Akonadi::Item,KUrl)), SIGNAL(urlClicked(Akonadi::Item,KUrl)) );
  connect( m_viewer, SIGNAL(itemRemoved()), SIGNAL(mailRemoved()) );
}

MessageViewItem::~MessageViewItem()
{
  delete m_viewer;
}

qint64 MessageViewItem::itemId() const
{
  return m_viewer->messageItem().id();
}

void MessageViewItem::setItemId( qint64 id )
{
  m_viewer->setMessageItem( Akonadi::Item( id ) );
}

void MessageViewItem::setItem( const Akonadi::Item &item )
{
  m_viewer->setMessageItem( item );
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

QString MessageViewItem::messagePath() const
{
  return m_viewer->messagePath();
}

void MessageViewItem::setMessagePath( const QString& messagePath )
{
  m_viewer->setMessagePath( messagePath );
}

QObject* MessageViewItem::attachmentModel() const
{
  return m_attachmentProxy;
}

void MessageViewItem::scrollDown( int dist )
{
  m_viewer->slotScrollDown( dist );
}


void MessageViewItem::scrollUp( int dist )
{
  m_viewer->slotScrollUp( dist );
}

void MessageViewItem::simulateMouseClick( const QPoint &_pos )
{
  QPoint pos = _pos;
  QWidget *widget = m_viewer->d_ptr->mViewer;
  QWidget *childWidget = m_viewer->childAt( pos );

  if (childWidget) {
    widget = childWidget;
    pos = childWidget->mapFromGlobal( pos );
  }

  QMouseEvent *event = new QMouseEvent( QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QCoreApplication::postEvent(widget, event);
  event = new QMouseEvent( QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QCoreApplication::postEvent(widget, event);
}

Viewer* MessageViewItem::viewer()
{
    return m_viewer;
}

void MessageViewItem::saveAllAttachments()
{
  m_viewer->slotAttachmentSaveAll();
}

void MessageViewItem::copyAllToClipboard()
{
  m_viewer->selectAll();
  m_viewer->copySelectionToClipboard();
  m_viewer->clearSelection();
}


#include "messageviewitem.moc"
