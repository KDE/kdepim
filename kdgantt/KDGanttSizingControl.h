/* -*- Mode: C++ -*-
   $Id$
*/

/****************************************************************************
** Copyright (C) 2001-2003 Klar�lvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KDGantt library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KDGantt licenses may use this file in
** accordance with the KDGantt Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.klaralvdalens-datakonsult.se/?page=products for
**   information about KDGantt Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
**********************************************************************/

#ifndef KDGANTTSIZINGCONTROL_H
#define KDGANTTSIZINGCONTROL_H

#include <qwidget.h>

class KDGanttSizingControl : public QWidget
{
    Q_OBJECT

public:
    bool isMinimized() const;
    
protected:
    KDGanttSizingControl( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );

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
