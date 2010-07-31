/* -*- Mode: C++ -*-
   $Id$
*/

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

#ifndef KDGANTTMINIMIZESPLITTER_H
#define KDGANTTMINIMIZESPLITTER_H

#ifndef QT_H
#include "tqframe.h"
#include "tqvaluelist.h"
#endif // QT_H

#ifndef QT_NO_SPLITTER

class QSplitterData;
class QSplitterLayoutStruct;

class KDGanttMinimizeSplitter : public QFrame
{
    Q_OBJECT
    Q_ENUMS( Direction )
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY( Direction minimizeDirection READ minimizeDirection WRITE setMinimizeDirection )

public:
    enum ResizeMode { Stretch, KeepSize, FollowSizeHint };
    enum Direction { Left, Right, Up, Down };

    KDGanttMinimizeSplitter( TQWidget* parent=0, const char* name=0 );
    KDGanttMinimizeSplitter( Orientation, TQWidget* parent=0, const char* name=0 );
    ~KDGanttMinimizeSplitter();

    virtual void setOrientation( Orientation );
    Orientation orientation() const { return orient; }

    void setMinimizeDirection( Direction );
    Direction minimizeDirection() const;

#if QT_VERSION >= 300
    virtual void setResizeMode( TQWidget *w, ResizeMode );
    virtual void setOpaqueResize( bool = TRUE );
    bool opaqueResize() const;

    void moveToFirst( TQWidget * );
    void moveToLast( TQWidget * );

    void refresh() { recalc( TRUE ); }
    virtual TQSize sizeHint() const;
    virtual TQSize minimumSizeHint() const;

    TQValueList<int> sizes() const;
    void setSizes( TQValueList<int> );

    void expandPos( int id, int* min, int* max );
protected:
    void childEvent( TQChildEvent * );

    bool event( TQEvent * );
    void resizeEvent( TQResizeEvent * );

    int idAfter( TQWidget* ) const;

    void moveSplitter( QCOORD pos, int id );
    virtual void drawSplitter( TQPainter*, QCOORD x, QCOORD y,
			       QCOORD w, QCOORD h );
    void styleChange( TQStyle& );
    int adjustPos( int , int );
    virtual void setRubberband( int );
    void getRange( int id, int*, int* );

private:
    void init();
    void recalc( bool update = FALSE );
    void doResize();
    void storeSizes();
    void processChildEvents();
    QSplitterLayoutStruct *addWidget( TQWidget*, bool first = FALSE );
    void recalcId();
    void moveBefore( int pos, int id, bool upLeft );
    void moveAfter( int pos, int id, bool upLeft );
    void setG( TQWidget *w, int p, int s, bool isSplitter = FALSE );

    QCOORD pick( const TQPoint &p ) const
    { return orient == Horizontal ? p.x() : p.y(); }
    QCOORD pick( const TQSize &s ) const
    { return orient == Horizontal ? s.width() : s.height(); }

    QCOORD trans( const TQPoint &p ) const
    { return orient == Vertical ? p.x() : p.y(); }
    QCOORD trans( const TQSize &s ) const
    { return orient == Vertical ? s.width() : s.height(); }

    QSplitterData *data;
#endif

private:
    Orientation orient;
    Direction _direction;
#ifndef DOXYGEN_SKIP_INTERNAL
    friend class KDGanttSplitterHandle;
#endif
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    KDGanttMinimizeSplitter( const KDGanttMinimizeSplitter & );
    KDGanttMinimizeSplitter& operator=( const KDGanttMinimizeSplitter & );
#endif
};

#ifndef DOXYGEN_SKIP_INTERNAL
// This class was continued from a verbatim copy of the
// QSplitterHandle pertaining to the Qt Enterprise License and the
// GPL. It has only been renamed to KDGanttSplitterHandler in order to
// avoid a symbol clash on some platforms.
class KDGanttSplitterHandle : public QWidget
{
    Q_OBJECT
#if QT_VERSION >= 300
public:
    KDGanttSplitterHandle( Qt::Orientation o,
		       KDGanttMinimizeSplitter *parent, const char* name=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    bool opaque() const { return s->opaqueResize(); }

    TQSize sizeHint() const;

    int id() const { return myId; } // data->list.at(id())->wid == this
    void setId( int i ) { myId = i; }

protected:
    TQValueList<TQPointArray> buttonRegions();
    void paintEvent( TQPaintEvent * );
    void mouseMoveEvent( TQMouseEvent * );
    void mousePressEvent( TQMouseEvent * );
    void mouseReleaseEvent( TQMouseEvent * );
    int onButton( const TQPoint& p );
    void updateCursor( const TQPoint& p );

private:
    Qt::Orientation orient;
    bool opaq;
    int myId;

    KDGanttMinimizeSplitter *s;
    int _activeButton;
    bool _collapsed;
    int _origPos;
#endif
};
#endif

#endif // QT_NO_SPLITTER

#endif // KDGANTTMINIMIZESPLITTER_H
