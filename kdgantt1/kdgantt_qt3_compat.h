
/****************************************************************************
 ** Copyright (C)  2001-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
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


#ifndef __QT3_COMPAT_H__
#define __QT3_COMPAT_H__

#include <QWidget>

#if QT_VERSION < 0x040000


#include <Q3PtrList>
#include <qdom.h>
#include <qvbox.h>
#include <qarray.h>
#include <Q3HButtonGroup>
#include <QListView>
#include <qcanvas.h>
#include <qdict.h>
#include <Q3ValueList>
#include <Q3CString>
#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qscrollview.h>

#include <qdockwindow.h>
#include <Q3PaintDeviceMetrics>

//for moc
typedef QListViewItem Q3ListViewItem;

#define protected public
#include <Q3Header>
#undef protected

#else

#include <Q3PtrList>
#include <QtXml/qdom.h>
#include <Q3VBox>

#include <Q3Canvas>
#include <Q3Dict>
#include <Q3ListView>
#include <Q3ValueList>
#include <Q3CString>
#include <Q3DragObject>
#include <Q3DockWindow>
#include <Q3ScrollView>
#include <Q3PopupMenu>
#include <Q3GroupBox>
#include <Q3ScrollView>
#include <Q3MemArray>
//#include <q3array.h>


#define QPtrList Q3PtrList
#define QVBox Q3VBox
#define QHBox Q3HBox
#define QCanvasText Q3CanvasText
#define QCanvasLine Q3CanvasLine
#define QCanvasPolygonalItem Q3CanvasPolygonalItem
#define QCanvasPolygon Q3CanvasPolygon
#define QCanvasEllipse Q3CanvasEllipse
#define QCanvasRectangle Q3CanvasRectangle
#define QCanvasView Q3CanvasView
#define QCanvasItem Q3CanvasItem
#define QListViewItemIterator Q3ListViewItemIterator
#define QPtrListIterator Q3PtrListIterator
#define QCanvasItemList Q3CanvasItemList
//#define 
#define QMemArray Q3MemArray

#define QListViewItem Q3ListViewItem
#define QDict Q3Dict
#define QValueList Q3ValueList
#define QCanvas Q3Canvas


#define QPointArray Q3PointArray
#define QDragObject Q3DragObject
#define QStoredDrag Q3StoredDrag
#define QDockWindow Q3DockWindow
#define QScrollView Q3ScrollView
#define QHeader Q3Header
#define QPopupMenu Q3PopupMenu
#define QGroupBox Q3GroupBox
#define QListView Q3ListView

//#define QArray Q3Array

#define protected public
#include <q3header.h>
#undef protected

#endif

#endif
