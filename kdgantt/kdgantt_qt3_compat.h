
/****************************************************************************
 ** Copyright (C)  2001-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
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
 ** See http://www.klaralvdalens-datakonsult.se/Public/products/ for
 **   information about KDGantt Commercial License Agreements.
 **
 ** Contact info@klaralvdalens-datakonsult.se if any conditions of this
 ** licensing are not clear to you.
 **
 ** As a special exception, permission is given to link this program
 ** with any edition of Qt, and distribute the resulting executable,
 ** without including the source code for Qt in the source distribution.
 **
 **********************************************************************/


#ifndef __QT3_COMPAT_H__
#define __QT3_COMPAT_H__

#include <qwidget.h>

#if QT_VERSION < 0x040000


#include <qptrlist.h>
#include <qdom.h>
#include <qvbox.h>  
#include <qarray.h> 
#include <qhbuttongroup.h>
#include <qlistview.h>
#include <qcanvas.h>
#include <qdict.h>
#include "qvaluelist.h"
#include <qcstring.h>
#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qscrollview.h>
#include <qtooltip.h>
#include <qdockwindow.h>
#include <qpaintdevicemetrics.h>

//for moc
typedef QListViewItem Q3ListViewItem;

#define protected public
#include <qheader.h>
#undef protected

#else

#include <q3ptrlist.h>
#include <QtXml/qdom.h>
#include <q3vbox.h>

#include <q3canvas.h>
#include <q3dict.h>
#include <q3listview.h>
#include "q3valuelist.h"
#include <q3cstring.h>
#include <q3dragobject.h>
#include <q3dockwindow.h>
#include <q3scrollview.h>
#include <q3popupmenu.h>
#include <q3groupbox.h>
#include <q3scrollview.h>
#include <q3memarray.h> 
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
