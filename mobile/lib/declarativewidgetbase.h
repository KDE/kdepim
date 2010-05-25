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

#ifndef DECLARATIVEWIDGETBASE_H
#define DECLARATIVEWIDGETBASE_H

#include <QtDeclarative/QDeclarativeItem>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsScene>

class QGraphicsProxyWidget;


template <typename WidgetT, typename ViewT, void (ViewT::*registerFunc)( WidgetT* )>
class DeclarativeWidgetBase  : public QDeclarativeItem
{
  public:
    explicit DeclarativeWidgetBase( QDeclarativeItem *parent = 0 ) :
      QDeclarativeItem( parent ),
      m_widget( new WidgetT ),
      m_proxy( new QGraphicsProxyWidget( this ) )
    {
      QPalette pal = m_widget->palette();
      pal.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );
      m_widget->setPalette( pal );
      m_proxy->setWidget( m_widget );
      setWidth( m_widget->width() );
      setHeight( m_widget->height() );
    }

    virtual ~DeclarativeWidgetBase() { delete m_proxy; }

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
          ViewT* view = qobject_cast<ViewT*>( scene->views().first() );
          (view->*(registerFunc))( m_widget );
        }
      }
      return QDeclarativeItem::itemChange ( change, value );
    }

  protected:
    WidgetT* m_widget;
    QGraphicsProxyWidget *m_proxy;
};

#endif
