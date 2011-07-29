/*
    Copyright (c) 2010 Anselmo Lacerda S. de Melo <anselmolsm@gmail.com>
    Copyright (c) 2010 Artur Duque de Souza <asouza@kde.org>

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

#include "composerautoresizer.h"

#include <QtGui/QGraphicsProxyWidget>

ComposerAutoResizer::ComposerAutoResizer( QWidget *parent )
  : QObject( parent ),
    mComposer( qobject_cast<QTextEdit*>( parent ) ),
    mEdit( qobject_cast<QFrame*>( parent ) ),
    mFlickable( 0 )
{
  Q_ASSERT( mComposer );

  // detect when the text changes
  connect( parent, SIGNAL(textChanged()), this, SLOT(textEditChanged()) );
  connect( parent, SIGNAL(cursorPositionChanged()), this, SLOT(textEditChanged()) );

  // get the original minimum size of the widget
  mMinimumHeight = mEdit->size().height();
}

QDeclarativeItem *ComposerAutoResizer::findFlickable( QGraphicsItem *parent ) const
{
  // looks for a QML Flickable Item based on the name of the class
  // It's not optimal but it's the only way as
  // QDeclarativeFlickable is not public
  const QString flickableClassName( "QDeclarativeFlickable" );
  while ( parent ) {
    QDeclarativeItem *item = qobject_cast<QDeclarativeItem*>( parent );
    if ( item ) {
      if ( !flickableClassName.compare( item->metaObject()->className() ) ) {
        return item;
        break;
      }
    }
    parent = parent->parentItem();
  }

  return 0;
}

void ComposerAutoResizer::textEditChanged()
{
  QTextDocument *document = mComposer->document();
  const QRect cursor = mComposer->cursorRect();
  const QSize size = document->size().toSize();
  const QRect frameRect = mEdit->frameRect();
  const QRect contentsRect = mEdit->contentsRect();

  // sets the size of the widget dynamically
  mEdit->setMinimumHeight( qMax( mMinimumHeight, size.height() + (frameRect.height() - contentsRect.height()) ) );
  mEdit->setMaximumHeight( qMax( mMinimumHeight, size.height() + (frameRect.height() - contentsRect.height()) ) );

  const QGraphicsProxyWidget *proxy = mEdit->graphicsProxyWidget();
  QGraphicsItem *proxyItem = proxy->parentItem();

  // position of the widget
  const QPointF pos = proxy->pos();

  // make sure the cursor is visible so the user doesn't loose track of the kb focus
  if ( mFlickable || (mFlickable = findFlickable( proxyItem )) ) {
    const int dy = cursor.center().y();
    const int y = pos.y() + dy - mMinimumHeight;
    if ( y >= 0 ) {
      mFlickable->setProperty( "contentY", y );
    } else {
      mFlickable->setProperty( "contentY", 0 );
    }
  }
}

#include "composerautoresizer.moc"
