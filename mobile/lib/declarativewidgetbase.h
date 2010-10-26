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

#include "stylesheetloader.h"

#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsScene>
#include <QtDeclarative/QDeclarativeItem>


template <typename WidgetT, typename ViewT, void (ViewT::*registerFunc)( WidgetT* )>
class DeclarativeWidgetBase : public QGraphicsProxyWidget
{
  public:
    explicit DeclarativeWidgetBase( QGraphicsItem *parent = 0 ) :
      QGraphicsProxyWidget( parent ),
      m_widget( new WidgetT )
    {
      init();
    }

    /** use this constructor if you inherit from this template to customize widget construction. */
    DeclarativeWidgetBase( WidgetT *widget, QGraphicsItem *parent ) :
      QGraphicsProxyWidget( parent ),
      m_widget( widget )
    {
      init();
    }

  protected:
    QVariant itemChange ( GraphicsItemChange change, const QVariant& value )
    {
      if ( change == ItemSceneHasChanged ) {
        if ( QGraphicsScene* scene = value.value<QGraphicsScene*>() )
          Q_FOREACH( QGraphicsView * v, scene->views() )
            if ( ViewT * view = qobject_cast<ViewT*>( v ) )
              (view->*registerFunc)( m_widget );
      }
      return QGraphicsProxyWidget::itemChange ( change, value );
    }

  protected:
    WidgetT* m_widget;

  private:
    void init()
    {
      Q_ASSERT( m_widget );

      QPalette pal = m_widget->palette();
      pal.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );
      m_widget->setPalette( pal );
      StyleSheetLoader::applyStyle( m_widget );
      setWidget( m_widget );
      setFocusPolicy( Qt::StrongFocus );
    }
};

#endif
