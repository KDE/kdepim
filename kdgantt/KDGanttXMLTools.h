/* -*- Mode: C++ -*-
   $Id$
   KDGantt - a multi-platform charting engine
*/

/****************************************************************************
 ** Copyright (C)  2002-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef __KDGANTTXMLTOOLS_H__
#define __KDGANTTXMLTOOLS_H__

#include <tqpen.h>
#include <tqdom.h>
#include <tqstring.h>
#include <tqcolor.h>
#include <tqrect.h>
#include <tqfont.h>
#include <tqstringlist.h>
#include <tqdatetime.h>

namespace KDGanttXML {
    TQString penStyleToString( Qt::PenStyle style );
    Qt::PenStyle stringToPenStyle( const TQString& style );
    TQString brushStyleToString( Qt::BrushStyle style );
    Qt::BrushStyle stringToBrushStyle( const TQString& style );

    void createBoolNode( TQDomDocument& doc, TQDomNode& parent,
                         const TQString& elementName, bool value );
    void createSizeNode( TQDomDocument& doc, TQDomNode& parent,
                         const TQString& elementName, const TQSize& value );
    void createIntNode( TQDomDocument& doc, TQDomNode& parent,
                        const TQString& elementName, int value );
    void createDoubleNode( TQDomDocument& doc, TQDomNode& parent,
                           const TQString& elementName, double value );
    void createStringNode( TQDomDocument& doc, TQDomNode& parent,
                           const TQString& elementName,
                           const TQString& text );
    void createColorNode( TQDomDocument& doc, TQDomNode& parent,
                          const TQString& elementName, const TQColor& color );
    void createBrushNode( TQDomDocument& doc, TQDomNode& parent,
                          const TQString& elementName, const TQBrush& brush );
    void createPixmapNode( TQDomDocument& doc, TQDomNode& parent,
                           const TQString& elementName, const TQPixmap& pixmap );
    void createRectNode( TQDomDocument& doc, TQDomNode& parent,
                         const TQString& elementName, const TQRect& rect );
    void createStringListNodes( TQDomDocument& doc, TQDomNode& parent,
                                const TQString& elementName,
                                const TQStringList* list );
    void createFontNode( TQDomDocument& doc, TQDomNode& parent,
                         const TQString& elementName, const TQFont& font );

    void createPenNode( TQDomDocument& doc, TQDomNode& parent,
                        const TQString& elementName, const TQPen& pen );
    void createDateTimeNode( TQDomDocument& doc, TQDomNode& parent,
                             const TQString& elementName,
                             const TQDateTime& datetime );
    void createDateNode( TQDomDocument& doc, TQDomNode& parent,
                         const TQString& elementName, const TQDate& date );
    void createTimeNode( TQDomDocument& doc, TQDomNode& parent,
                         const TQString& elementName, const TQTime& time );
    bool readIntNode( const TQDomElement& element, int& value );
    bool readStringNode( const TQDomElement& element, TQString& value );
    bool readDoubleNode( const TQDomElement& element, double& value );
    bool readBoolNode( const TQDomElement& element, bool& value );
    bool readColorNode( const TQDomElement& element, TQColor& value );
    bool readBrushNode( const TQDomElement& element, TQBrush& brush );
    bool readPixmapNode( const TQDomElement& element, TQPixmap& pixmap );
    bool readRectNode( const TQDomElement& element, TQRect& value );
    bool readFontNode( const TQDomElement& element, TQFont& font );
    bool readPenNode( const TQDomElement& element, TQPen& pen );
    bool readDateTimeNode( const TQDomElement& element, TQDateTime& datetime );
    bool readDateNode( const TQDomElement& element, TQDate& date );
    bool readTimeNode( const TQDomElement& element, TQTime& time );
}
#endif
