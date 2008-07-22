

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

#include <KDGanttView.h>

#include <KDGanttViewItemDrag.h>
#include <KDGanttViewItem.h>
#include <QPixmap>

/*!
  \class KDGanttViewItemDrag KDGanttViewItemDrag.h
  \brief Drag and drop of KD Gantt items.

  This class implements drag and drop of KD Gantt items within a Gantt
  chart. It is mainly used for internal purposes, but made a part of
  the public API nevertheless, as you may want to subclass it for some
  specialized functionality.
*/


/*!
  The constructor. Creates a KDGanttViewItemDrag object and
  initializes the drag data in the form of an XML document.

  \param item the item that is dragged
  \param source the source widget
  \param name the internal object name
*/
#include <QBitmap>
KDGanttViewItemDrag::KDGanttViewItemDrag( KDGanttViewItem* item , QWidget *source,  const char * name  ) : QStoredDrag("x-application/x-KDGanttViewItemDrag", source,  name )
{
    myItem = item;

    QPixmap pix;
    if (item->pixmap() ) {
        pix = QPixmap(*(item->pixmap())) ;
    }
    else {
        KDGanttViewItem::Shape start,  middle, end;
        item->shapes( start, middle, end );
        QColor st, mi, en;
        item->colors( st, mi, en );
        pix = QPixmap(item->myGanttView->getPixmap( start, st, item->myGanttView->lvBackgroundColor(), 10 ));
    }
    int off = -pix.width()/2 -2;
#ifdef Q_WS_WIN
    if ( item->pixmap() )
#endif
    setPixmap( pix , QPoint( off, off ) );
    QDomDocument doc( "GanttView" );
    QString docstart = "<GanttView/>";
    doc.setContent( docstart );
    QDomElement itemsElement = doc.createElement( "Items" );
    doc.documentElement().appendChild( itemsElement );
    item->createNode( doc, itemsElement );
#if QT_VERSION < 0x040000
    QDataStream s( array, IO_WriteOnly );
#else
    QDataStream s( &array, IO_WriteOnly );
#endif
    s << doc.toString();
}


/*!
  Returns the encoded data of the drag object.

  \param c the format of the data
  \return the encoded data of the drag object
*/
QByteArray KDGanttViewItemDrag::encodedData( const char * c) const
{
    QString s ( c );
    if ( s == "x-application/x-KDGanttViewItemDrag" ) {
        return array;
    }
    return QByteArray();
}

/*!
  Returns the dragged item

  \return the dragged item
*/
KDGanttViewItem* KDGanttViewItemDrag::getItem()
{
    return myItem;
}


/*!
  Returns whether this drag object class can decode the data passed in \a e.

  \param e the mime source that has been dragged
  \return true if KDGanttViewItemDrag can decode the data in \a e.
*/
bool KDGanttViewItemDrag::canDecode (  const QMimeSource * e )
{
    if ( QString( e->format() ) == "x-application/x-KDGanttViewItemDrag" )
        return true;

    return false;
}


/*!
  Decodes the data passed in \a e into an XML string that is written
  into \a string.

  \param e the data to decode
  \param string the resulting XML string
  \return true if the operation succeeded
*/
bool KDGanttViewItemDrag::decode (  const QMimeSource * e , QString &  string)
{
    QByteArray arr;
    arr = e->encodedData( "x-application/x-KDGanttViewItemDrag");
    //qDebug("KDGanttViewItemDrag::decode length %d ", arr.count());
    if ( arr.count() ) {
#if QT_VERSION < 0x040000
    QDataStream s( arr, IO_ReadOnly );
#else
    QDataStream s( &arr, IO_ReadOnly );
#endif
        s >> string;
    }
    return true;
}

