/* -*- Mode: C++ -*-
   $Id: KDGanttSizingControl.cpp,v 1.4 2005/10/11 11:44:04 lutz Exp $
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


#include "KDGanttSizingControl.h"

/*!
  \class KDGanttSizingControl KDGanttSizingControl.h
  This class is a common-base class for all sizing controls in this
  library.

  It provides common signals and slots for minimizing and restoring
  child widgets.

  This class cannot be instantiated by itself, use one of the
  subclasses instead.
*/

/*!
  Constructs an empty KDGanttSizing Control.

  \param parent the parent widget. This parameter is passed to the
  base class.
  \param name the internal widget name. This parameter is passed to
  the base class.

*/

KDGanttSizingControl::KDGanttSizingControl( QWidget* parent, Qt::WFlags f )
    :QWidget( parent, f ), _isMinimized( false )
{
}


/*!
  Restores or minimizes the child widget. \a minimize() does exactly the
  opposite to this method.

  \param restore true to restore, false to minimize
  \sa minimize()
*/

void KDGanttSizingControl::restore( bool restore )
{
    _isMinimized = !restore;
    if ( restore )
        emit restored( this );
    else
        emit minimized( this );
}




/*!
  Restores or minimizes the child widget. \a restore() does exactly the
  opposite to this method.

  \param minimize true to minimize, false to restore
  \sa restore()

*/

void KDGanttSizingControl::minimize( bool minimize )
{
    _isMinimized = minimize;
    if ( minimize )
        emit minimized( this );
    else
        emit restored( this );
}


/*!
  Returns whether the widget is minimized.
*/

bool KDGanttSizingControl::isMinimized() const
{
    return _isMinimized;
}

/*!
  Change state from either minimized to restored or visa versa.
*/

void KDGanttSizingControl::changeState()
{
    restore(_isMinimized);
}

/*!
  \fn void KDGanttSizingControl::minimized(  KDGanttSizingControl* )

  This signal is emitted when the user hides a controlled widget. The
  KDGanttSizingControl pointer given as parameter is a pointer to the widget
  itself. Normally the sender should not know the receiver, but in this
  case the receiver is likely the widget containing the KDGanttSizingControl,
  and when the KDGanttSizingControl widget is minimized/restored it might want
  to change stretching for the widget. See the example
  test/semisizingcontrol
*/


/*!
  \fn void KDGanttSizingControl::restored(  KDGanttSizingControl* )

  This signal is emitted when the user unhides a controlled widget. The
  KDGanttSizingControl pointer given as parameter is a pointer to the widget
  itself. Normally the sender should not know the receiver, but in this
  case the receiver is likely the widget containing the KDGanttSizingControl,
  and when the KDGanttSizingControl widget is minimized/restored it might want
  to change stretching for the widget. See the example
  test/semisizingcontrol
*/

#include "KDGanttSizingControl.moc"
