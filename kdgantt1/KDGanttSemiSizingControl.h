/* -*- Mode: C++ -*-
   $Id: KDGanttSemiSizingControl.h,v 1.4 2005/10/11 11:44:04 lutz Exp $
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


#ifndef KDGANTTSEMISIZINGCONTROL_H
#define KDGANTTSEMISIZINGCONTROL_H

#include <QLayout>

#include "kdgantt_qt3_compat.h"

#include "KDGanttSizingControl.h"
class QPushButton;
class QBoxLayout;

class KDGanttSemiSizingControl : public KDGanttSizingControl
{
    Q_PROPERTY( ArrowPosition arrowPosition READ arrowPosition WRITE setArrowPosition )
    Q_ENUMS( ArrowPosition )
    Q_OBJECT

public:
    enum ArrowPosition { Before, After };

    KDGanttSemiSizingControl( QWidget* parent = 0 );
    KDGanttSemiSizingControl( Qt::Orientation orientation, QWidget* parent = 0 );
    KDGanttSemiSizingControl( ArrowPosition arrowPosition,
                              Qt::Orientation orientation, QWidget* parent = 0 );

    void setMinimizedWidget( QWidget* widget );
    void setMaximizedWidget( QWidget* widget );
    QWidget* minimizedWidget() const;
    QWidget* maximizedWidget() const;

    void setOrientation( Qt::Orientation orientation );
    Qt::Orientation orientation() const;

    void setArrowPosition( ArrowPosition arrowPosition );
    ArrowPosition arrowPosition() const;

public slots:
    virtual void minimize( bool minimize );
    virtual void restore( bool restore );

protected:
    void setup();
    void init();
    enum Direction {Left, Right, Up, Down };
    QPixmap pixmap( Direction );

private:
    Qt::Orientation _orient;
    ArrowPosition _arrowPos;
    QWidget* _minimizedWidget;
    QWidget* _maximizedWidget;
    QBoxLayout* _layout;
    QPushButton* _but;
};


#endif

