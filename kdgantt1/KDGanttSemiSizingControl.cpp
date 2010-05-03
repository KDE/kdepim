/* -*- Mode: C++ -*-
   $Id: KDGanttSemiSizingControl.cpp,v 1.6 2005/10/11 13:59:02 lutz Exp $
*/

/****************************************************************************
 ** Copyright (C)  2002-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KDGantt library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** As a special exception, permission is given to link this program
 ** with any edition of Qt, and distribute the resulting executable,
 ** without including the source code for Qt in the source distribution.
 **
 **********************************************************************/


#include "KDGanttSemiSizingControl.h"
#include <QPushButton>
#include <QPainter>
#include <QBitmap>

#include <QWhatsThis>
/*!
  \class KDGanttSemiSizingControl KDGanttSemiSizingControl.h
  This class provides exactly one child widget with a button for
  minimizing and restoring. You can also specify a so-called minimize
  widget that will be shown in place of the child widget while the
  latter one is minimized. While the child widget is not minimized,
  the minimize widget will not be visible.

  If you add more than one child widget (besides the minimize widget),
  only the last one added will be visible.
*/


/*!
  Constructs an empty semi sizing control with horizontal
  orientation and the control arrow button on top of the controlled
  widget.

  \param parent the parent widget. This parameter is passed to the
  base class.
  \param name the internal widget name. This parameter is passed to
  the base class.
*/

KDGanttSemiSizingControl::KDGanttSemiSizingControl( QWidget* parent ) :
    KDGanttSizingControl( parent ), _orient( Qt::Horizontal ),
    _arrowPos( Before ), _minimizedWidget(0), _maximizedWidget(0)
{
    init();
}


/*!
  Constructs an empty semi sizing control with the specified
  orientation and the control arrow button either on top or left of
  the controlled widget (depending on the orientation).

  \param orientation the orientation of the splitter
  \param parent the parent widget. This parameter is passed to the
  base class.
  \param name the internal widget name. This parameter is passed to
  the base class.
*/

KDGanttSemiSizingControl::KDGanttSemiSizingControl( Qt::Orientation orientation,
                                                    QWidget* parent ):
    KDGanttSizingControl( parent ), _orient( orientation ),
    _arrowPos( Before ), _minimizedWidget(0), _maximizedWidget(0)
{
    init();
}


/*!
  Constructs an empty semi sizing control with the specified
  orientation and position of the control arrow button.

  \param arrowPosition specifies whether the control arrow button
  should appear before or after the controlled widget
  \param orientation the orientation of the splitter
  \param parent the parent widget. This parameter is passed to the
  base class.
  \param name the internal widget name. This parameter is passed to
  the base class.
*/

KDGanttSemiSizingControl::KDGanttSemiSizingControl( ArrowPosition arrowPosition,
                                                    Qt::Orientation orientation,
                                                    QWidget* parent ):
    KDGanttSizingControl( parent ), _orient( orientation ),
    _arrowPos( arrowPosition ), _minimizedWidget(0), _maximizedWidget(0)
{
    init();
}


/*!
  Specifies the widget that should be shown while the child widget is
  minimized. This so-called minimize widget should be a child widget
  of the KDGanttSemiSizingControl.

  \param widget the minimize widget
  \sa minimizedWidget()
*/

void KDGanttSemiSizingControl::setMinimizedWidget( QWidget* widget )
{
    _minimizedWidget = widget;
    if( _minimizedWidget ) _minimizedWidget->hide();
    setup();
}


/*!
  Returns the widget that is shown while the child widget is
  minimized.

  \return the minimize widget
  \sa setMinimizedWidget()
*/

QWidget* KDGanttSemiSizingControl::minimizedWidget() const
{
    return _minimizedWidget;
}

/*!
  Specifies the widget that should be shown while the child widget is
  maximized. This so-called maximize widget should be a child widget
  of the KDGanttSemiSizingControl.

  \param widget the minimize widget
  \sa maximizedWidget()
*/

void KDGanttSemiSizingControl::setMaximizedWidget( QWidget* widget )
{
    _maximizedWidget = widget;
    //if( _maximizedWidget ) _maximizedWidget->show();
    setup();
}

/*!
  Returns the widget that is shown while the child widget is
  maximized.

  \return the maximize widget
  \sa setMaximizedWidget()
*/

QWidget* KDGanttSemiSizingControl::maximizedWidget() const
{
    return _maximizedWidget;
}



/*!
  Sets the orientation of the simple sizing control.

  \param orientation the new orientation
  \sa orientation()
*/

void KDGanttSemiSizingControl::setOrientation( Qt::Orientation orientation )
{
    if ( _orient != orientation ) {
        _orient = orientation;
        setup();
    }
}


/*!
  Returns the orientation of the simple sizing control.
  \return the orientation
  \sa setOrientation()
*/

Qt::Orientation KDGanttSemiSizingControl::orientation() const
{
    return _orient;
}


/*!
  Returns the position of the control arrow button.

  \param arrowPosition the position of the control arrow button
  \sa arrowPosition()
*/

void KDGanttSemiSizingControl::setArrowPosition( ArrowPosition arrowPosition )
{
    if ( _arrowPos != arrowPosition ) {
        _arrowPos = arrowPosition;
        setup();
    }
}


/*!
  Returns the position of the control arrow button.

  \return the position of the control arrow button
  \sa setArrowPosition()
*/

KDGanttSemiSizingControl::ArrowPosition KDGanttSemiSizingControl::arrowPosition() const
{
    return _arrowPos;
}


/*!
  \enum KDGanttSemiSizingControl::ArrowPosition

  This enum is used for specifying whether the control arrow button
  should appear before (on top of, left of) or after (below, right of)
  the controlled widget.
*/

void KDGanttSemiSizingControl::init()
{
    _but = new QPushButton( this );
    _but->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    connect( _but, SIGNAL( clicked() ), this, SLOT(changeState()) );
    _layout = 0;
    _but->setWhatsThis( tr("Click on this button to show the \nlegend at the bottom of the widget"));
    _but->setToolTip( tr("Show / hide legend"));


}

void KDGanttSemiSizingControl::setup()
{
    //-------------------------------------------------- Setup layout
    delete _layout;
    QBoxLayout* butLayout; // _layout will delete me

    if ( _orient == Qt::Horizontal || isMinimized() )
        _layout = new QHBoxLayout( this );
    else
        _layout = new QVBoxLayout( this );

    if ( _orient == Qt::Vertical && !isMinimized() ) {
        butLayout = new QHBoxLayout();
        _layout->addItem( butLayout );
    } else {
        butLayout = new QVBoxLayout();
        _layout->addItem( butLayout );
    }


    //---------------------------------------- Set the arrow on the button
    if ( !isMinimized() ) {
        _but->setIcon( QIcon(pixmap( Down )) );
    }
    else {
        if ( _arrowPos == Before ) {
            _but->setIcon( QIcon(pixmap( Right )) );
        }
        else {
            _but->setIcon( QIcon(pixmap( Left )) );
        }
    }

    //------------------------------ Setup the button at the correct possition
    if ( _arrowPos == After && _orient == Qt::Vertical && !isMinimized() ) {
        butLayout->addStretch( 1 );
        butLayout->addWidget( _but, 0, Qt::AlignLeft );
    }
    else {
        butLayout->addWidget( _but, 0, Qt::AlignRight  );
        butLayout->addStretch( 1 );
    }

    // Set widget in the correct possition
    QWidget* widget;
    /* ************************** old code ***************
       if ( isMinimized() )
       widget = _minimizedWidget;
       else
       widget = _maximizedWidget;
       if( widget ) {
       if ( _arrowPos == Before  || _orient == Vertical && !isMinimized() )
       _layout->addWidget( widget, 1 );
       else
       _layout->insertWidget( 0, widget, 1 );
	}
     ************************************************** */
    // hack for the usage in KDGantt as pop-up legend widget
    // for this purpose,
    // the _maximizedWidget must be a child of the parent of this widget

    if ( isMinimized() ) {
       widget = _minimizedWidget;
       if( widget ) {
	 if ( _arrowPos == Before  || ( _orient == Qt::Vertical && !isMinimized() ) )
	   _layout->addWidget( widget, 1 );
	 else
	   _layout->insertWidget( 0, widget, 1 );
       }
    }
    else {
      if ( _arrowPos == Before  || ( _orient == Qt::Vertical && !isMinimized() ) )
	_layout->addStretch( 1 );
      else
	_layout->insertStretch( 0, 1 );
      widget = _maximizedWidget;
      // the following is only the special case
      // arrowPos == Before  and  _orient == Vertical
      //widget->move( 0+x(), _but->height()+y());
    }
}


/*!
  Restores or minimizes the child widget. \a minimize() does exactly the
  opposite to this method.

  \param restore true to restore, false to minimize
  \sa minimize()
*/

void KDGanttSemiSizingControl::restore( bool restore )
{
    if ( ! restore ) {
        minimize( true );
    }
    else {
        if( _maximizedWidget ) _maximizedWidget->show();
        if( _minimizedWidget ) _minimizedWidget->hide();
        KDGanttSizingControl::restore( restore );
        setup();
    }
}

/*!
  Restores or minimizes the child widget. \a restore() does exactly the
  opposite to this method.

  \param minimize true to minimize, false to restore
  \sa restore()

*/

void KDGanttSemiSizingControl::minimize( bool minimize )
{
    if ( ! minimize ) {
        restore( true );
    }
    else {
        if( _minimizedWidget ) _minimizedWidget->show();
	if( _maximizedWidget ) _maximizedWidget->hide();
        KDGanttSizingControl::minimize( minimize );
        setup();
    }
}

QPixmap KDGanttSemiSizingControl::pixmap( Direction direction ) {
    int s = 10;
    QPixmap pix( s, s );
    pix.fill( Qt::blue );

    QPointArray arr;
    switch ( direction ) {
    case Up:    arr.setPoints( 3,   0, s-1,   s-1, s-1,   0, s/2   ); ;break;
    case Down:  arr.setPoints( 3,   0, 0,     s-1, 0,     s/2, s-1 ); break;
    case Left:  arr.setPoints( 3,   s-1, 0,   s-1, s-1,   0, s/2   ); break;
    case Right: arr.setPoints( 3,   0,0,      s-1, s/2,   0, s-1   ); break;
    }
#if QT_VERSION < 0x040000
    QPainter p( &pix );
    p.setPen( Qt::black );
    p.setBrush( colorGroup().button() );
    p.drawPolygon( arr );
    QBitmap bit( s, s );
    bit.fill( color0 );

    QPainter p2( &bit );
    p2.setPen( color1 );
    p2.setBrush( color1 );
    p2.drawPolygon( arr );
    pix.setMask( bit );
#else
    QPainter p( &pix );
    p.setPen( Qt::black );
    p.setBrush( palette().color( QPalette::Button) );
    p.drawPolygon( arr );
    QBitmap bit( s, s );
    bit.fill( Qt::blue );

    QPainter p2( &bit );
    p2.setPen( Qt::red );
    p2.setBrush( Qt::red );
    p2.drawPolygon( arr );
    pix.setMask( bit );
#endif
    return pix;
}

#include "KDGanttSemiSizingControl.moc"
