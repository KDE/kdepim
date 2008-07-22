// THIS IS A COPY OF THE FILE FOUND IN $QTDIR/src/kernel. Needed to modify qsplitter

/****************************************************************************
** $Id$
**
** Internal header file.
**
** Created : 981027
**
** Copyright (C) 1998-99 by Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
**********************************************************************/

#ifndef QLAYOUTENGINE_P_H
#define QLAYOUTENGINE_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qlayout.cpp, qlayoutengine.cpp, qmainwindow.cpp and qsplitter.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//


#ifndef QT_H
#include "qabstractlayout.h"
#endif // QT_H

#ifndef QT_NO_LAYOUT
struct QLayoutStruct
{
    void initParameters() { minimumSize = sizeHint = 0;
    maximumSize = QWIDGETSIZE_MAX; expansive = false; empty = true; }
    void init() { stretch = 0; initParameters(); }
    //permanent storage:
    int stretch;
    //parameters:
    QCOORD sizeHint;
    QCOORD maximumSize;
    QCOORD minimumSize;
    bool expansive;
    bool empty;
    //temporary storage:
    bool done;
    //result:
    int pos;
    int size;
};


void qGeomCalc( QMemArray<QLayoutStruct> &chain, int start, int count, int pos,
		      int space, int spacer );



/*
  Modify total maximum (max) and total expansion (exp)
  when adding boxmax/boxexp.

  Expansive boxes win over non-expansive boxes.
*/
static inline void qMaxExpCalc( QCOORD & max, bool &exp,
			       QCOORD boxmax, bool boxexp )
{
    if ( exp ) {
	if ( boxexp )
	    max = qMax( max, boxmax );
    } else {
	if ( boxexp )
	    max = boxmax;
	else
	    max = qMin( max, boxmax );
    }
    exp = exp || boxexp;
}

#endif //QT_NO_LAYOUT
#endif
