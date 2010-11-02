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

#include "mobileui_export.h"

#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsScene>
#include <QtDeclarative/QDeclarativeItem>

/**
   \internal
*/
class MOBILEUI_EXPORT DeclarativeWidgetBaseHelper : public QGraphicsProxyWidget
{
protected:
    DeclarativeWidgetBaseHelper( QWidget * widget, QGraphicsItem *parent );
    ~DeclarativeWidgetBaseHelper();

    QWidget * widget() const { return m_widget; }

private:
    void init();

private:
    QWidget * m_widget;
};

template <typename WidgetT, typename ViewT, void (ViewT::*registerFunc)( WidgetT* )>
class DeclarativeWidgetBase : public DeclarativeWidgetBaseHelper
{
  public:
    explicit DeclarativeWidgetBase( QGraphicsItem *parent = 0 )
        : DeclarativeWidgetBaseHelper( new WidgetT, parent ) {}

    /** use this constructor if you inherit from this template to customize widget construction. */
    DeclarativeWidgetBase( WidgetT *widget, QGraphicsItem *parent )
        : DeclarativeWidgetBaseHelper( widget, parent ) {}

  protected:
    QVariant itemChange ( GraphicsItemChange change, const QVariant& value )
    {
      if ( change == ItemSceneHasChanged ) {
        if ( QGraphicsScene* scene = value.value<QGraphicsScene*>() )
          Q_FOREACH( QGraphicsView * v, scene->views() )
            if ( ViewT * view = qobject_cast<ViewT*>( v ) )
              (view->*registerFunc)( widget() );
      }
      return QGraphicsProxyWidget::itemChange ( change, value );
    }

  protected:
    WidgetT * widget() const { return static_cast<WidgetT*>( DeclarativeWidgetBaseHelper::widget() ); }
};

#endif
