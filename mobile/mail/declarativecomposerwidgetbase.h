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

#ifndef DECLARATIVECOMPOSERWIDGETBASE_H
#define DECLARATIVECOMPOSERWIDGETBASE_H

#include "composerview.h"

#include <QtDeclarative/QDeclarativeItem>
#include <QtGui/QGraphicsProxyWidget>
#include <qgraphicsscene.h>

class QGraphicsProxyWidget;

template <typename T, void (ComposerView::*registerFunc)( T* )>
class DeclarativeComposerWidgetBase : public QDeclarativeItem
{
  public:
    explicit DeclarativeComposerWidgetBase( QDeclarativeItem *parent = 0 ) :
      QDeclarativeItem( parent ),
      m_proxy( new QGraphicsProxyWidget( this ) )
    {
    }

    void geometryChanged( const QRectF &newGeometry, const QRectF &oldGeometry )
    {
      QDeclarativeItem::geometryChanged ( newGeometry, oldGeometry );
      m_proxy->resize( newGeometry.size() );
    }

  protected:
    QVariant itemChange ( GraphicsItemChange change, const QVariant& value )
    {
      if ( change == ItemSceneHasChanged ) {
        QGraphicsScene* scene = value.value<QGraphicsScene*>();
        if ( scene && !scene->views().isEmpty() ) {
          ComposerView* view = qobject_cast<ComposerView*>( scene->views().first() );
          (view->*(registerFunc))( m_widget );
        }
      }
      return QDeclarativeItem::itemChange ( change, value );
    }

  protected:
    T* m_widget;
    QGraphicsProxyWidget *m_proxy;
};

#endif
