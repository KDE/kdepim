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

#include <QGraphicsProxyWidget>

#include <boost/function.hpp>

class QGraphicsView;
class QWidget;

/**
   \internal
*/
class MOBILEUI_EXPORT DeclarativeWidgetBaseHelper : public QGraphicsProxyWidget
{
  Q_OBJECT
  Q_PROPERTY( QString styleSheet READ styleSheet WRITE setStyleSheet )
public:
  QString styleSheet() const;
  void setStyleSheet( const QString &styleSheet );

protected:
    typedef boost::function<void(QGraphicsView*,QWidget*)> RegisterFunction;
    DeclarativeWidgetBaseHelper( QWidget * widget, QGraphicsItem *parent, const RegisterFunction & registerFunc );
    ~DeclarativeWidgetBaseHelper();

    QWidget * widget() const { return m_widget; }

protected:
    /* reimp */ QVariant itemChange( GraphicsItemChange change, const QVariant & value );

private:
    const RegisterFunction m_registerFunc;
    QWidget * m_widget;
};

template <typename WidgetT, typename ViewT, void (ViewT::*registerFunc)( WidgetT* )>
class DeclarativeWidgetBase : public DeclarativeWidgetBaseHelper
{
  public:
    explicit DeclarativeWidgetBase( QGraphicsItem *parent = 0 )
        : DeclarativeWidgetBaseHelper( new WidgetT, parent, &notify ) {}

    /** use this constructor if you inherit from this template to customize widget construction. */
    DeclarativeWidgetBase( WidgetT *widget, QGraphicsItem *parent )
        : DeclarativeWidgetBaseHelper( widget, parent, &notify ) {}

  protected:
    WidgetT * widget() const { return static_cast<WidgetT*>( DeclarativeWidgetBaseHelper::widget() ); }

  private:
    static void notify( QGraphicsView * v, QWidget * w ) {
        if ( ViewT * view = qobject_cast<ViewT*>( v ) )
            (view->*registerFunc)( static_cast<WidgetT*>( w ) );
    }
};

#endif
