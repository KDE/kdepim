/* -*- Mode: C++ -*-
   $Id: KDGanttSizingControl.h,v 1.4 2005/10/11 11:44:04 lutz Exp $
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

#ifndef KDGANTTSIZINGCONTROL_H
#define KDGANTTSIZINGCONTROL_H

#include <QWidget>

#include "kdgantt_qt3_compat.h"

class KDGanttSizingControl : public QWidget
{
    Q_OBJECT

public:
    bool isMinimized() const;

protected:
    KDGanttSizingControl( QWidget* parent = 0, Qt::WFlags f = 0 );

public slots:
    virtual void minimize( bool minimize );
    virtual void restore( bool restore );
    void changeState();

signals:
    void minimized( KDGanttSizingControl* );
    void restored( KDGanttSizingControl* );

private:
    bool _isMinimized;
};


#endif
