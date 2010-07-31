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

#include "KDGanttXMLTools.h"
#include <tqbrush.h>
#include <tqbuffer.h>
#include <tqimage.h>
#include <zlib.h>

namespace KDGanttXML {

void createBoolNode( TQDomDocument& doc, TQDomNode& parent,
                     const TQString& elementName, bool value )
{
    TQDomElement newElement =
        doc.createElement( elementName );
    parent.appendChild( newElement );
    TQDomText elementContent =
        doc.createTextNode( value ? "true" : "false" );
    newElement.appendChild( elementContent );
}



void createSizeNode( TQDomDocument& doc, TQDomNode& parent,
                     const TQString& elementName, const TQSize& value )
{
    TQDomElement newElement =
        doc.createElement( elementName );
    parent.appendChild( newElement );
    newElement.setAttribute( "Width", value.width() );
    newElement.setAttribute( "Height", value.height() );
}


void createIntNode( TQDomDocument& doc, TQDomNode& parent,
                    const TQString& elementName, int value )
{
    TQDomElement newElement =
        doc.createElement( elementName );
    parent.appendChild( newElement );
    TQDomText elementContent =
        doc.createTextNode( TQString::number( value ) );
    newElement.appendChild( elementContent );
}


void createDoubleNode( TQDomDocument& doc, TQDomNode& parent,
                       const TQString& elementName, double value )
{
    TQDomElement newElement =
        doc.createElement( elementName );
    parent.appendChild( newElement );
    TQDomText elementContent =
        doc.createTextNode( TQString::number( value ) );
    newElement.appendChild( elementContent );
}


void createStringNode( TQDomDocument& doc, TQDomNode& parent,
                       const TQString& elementName,
                       const TQString& text )
{
    TQDomElement newElement =
        doc.createElement( elementName );
    parent.appendChild( newElement );
    TQDomText elementContent =
        doc.createTextNode( text );
    newElement.appendChild( elementContent );
}


void createColorNode( TQDomDocument& doc, TQDomNode& parent,
                      const TQString& elementName, const TQColor& color )
{
    TQDomElement colorElement = doc.createElement( elementName );
    parent.appendChild( colorElement );
    colorElement.setAttribute( "Red",
                               TQString::number( color.red() ) );
    colorElement.setAttribute( "Green",
                               TQString::number( color.green() ) );
    colorElement.setAttribute( "Blue",
                               TQString::number( color.blue() ) );
}


void createBrushNode( TQDomDocument& doc, TQDomNode& parent,
                      const TQString& elementName, const TQBrush& brush )

{
    TQDomElement brushElement = doc.createElement( elementName );
    parent.appendChild( brushElement );
    createColorNode( doc, brushElement, "Color", brush.color() );
    createStringNode( doc, brushElement, "Style",
                      KDGanttXML::brushStyleToString( brush.style() ) );
    if( brush.style() == Qt::CustomPattern && brush.pixmap() )
        createPixmapNode( doc, brushElement, "Pixmap", *brush.pixmap() );
}


void createPixmapNode( TQDomDocument& doc, TQDomNode& parent,
                       const TQString& elementName, const TQPixmap& pixmap )
{
    TQDomElement pixmapElement = doc.createElement( elementName );
    parent.appendChild( pixmapElement );

    // Convert the pixmap to an image, save that image to an in-memory
    // XPM representation and compress this representation. This
    // conforms to the file format Qt Designer uses.
    TQByteArray ba;
    TQBuffer buffer( ba );
    buffer.open( IO_WriteOnly );
    TQImageIO imgio( &buffer, "XPM" );
    TQImage image = pixmap.convertToImage();
    imgio.setImage( image );
    imgio.write();
    buffer.close();
    ulong len = ba.size() * 2;
    TQByteArray bazip( len );
    ::compress(  (uchar*) bazip.data(), &len, (uchar*) ba.data(), ba.size() );
    TQString dataString;
    static const char hexchars[] = "0123456789abcdef";
    for ( int i = 0; i < (int)len; ++i ) {
        uchar c = (uchar) bazip[i];
        dataString += hexchars[c >> 4];
        dataString += hexchars[c & 0x0f];
    }

    createStringNode( doc, pixmapElement, "Format", "XPM.GZ" );
    createIntNode( doc, pixmapElement, "Length", ba.size() );
    createStringNode( doc, pixmapElement, "Data", dataString );
}


void createRectNode( TQDomDocument& doc, TQDomNode& parent,
                     const TQString& elementName, const TQRect& rect )
{
    TQDomElement rectElement = doc.createElement( elementName );
    parent.appendChild( rectElement );
    TQDomElement xElement = doc.createElement( "X" );
    rectElement.appendChild( xElement );
    TQDomText xContent = doc.createTextNode( TQString::number( rect.x() ) );
    xElement.appendChild( xContent );
    TQDomElement yElement = doc.createElement( "Y" );
    rectElement.appendChild( yElement );
    TQDomText yContent = doc.createTextNode( TQString::number( rect.y() ) );
    yElement.appendChild( yContent );
    TQDomElement widthElement = doc.createElement( "Width" );
    rectElement.appendChild( widthElement );
    TQDomText widthContent = doc.createTextNode( TQString::number( rect.width() ) );
    widthElement.appendChild( widthContent );
    TQDomElement heightElement = doc.createElement( "Height" );
    rectElement.appendChild( heightElement );
    TQDomText heightContent = doc.createTextNode( TQString::number( rect.height() ) );
    heightElement.appendChild( heightContent );
}


void createStringListNodes( TQDomDocument& doc, TQDomNode& parent,
                            const TQString& elementName,
                            const TQStringList* list )
{
    if( !list )
        return;

    for( TQStringList::ConstIterator it = list->begin();
         it != list->end(); ++it ) {
        TQDomElement element = doc.createElement( elementName );
        parent.appendChild( element );
        TQDomText elementContent = doc.createTextNode( *it );
        element.appendChild( elementContent );
    }
}


void createFontNode( TQDomDocument& doc, TQDomNode& parent,
                     const TQString& elementName, const TQFont& font )
{
    TQDomElement fontElement = doc.createElement( elementName );
    parent.appendChild( fontElement );
    createStringNode( doc, fontElement, "Family", font.family() );
    createIntNode( doc, fontElement, "PointSize", font.pointSize() );
    createIntNode( doc, fontElement, "PixelSize", font.pixelSize() );
    createIntNode( doc, fontElement, "Weight", font.weight() );
    createBoolNode( doc, fontElement, "Italic", font.italic() );
#if QT_VERSION < 300
    // Qt 3 handles the charset internally.
    createIntNode( doc, fontElement, "CharSet", font.charSet() );
#endif
}


void createPenNode( TQDomDocument& doc, TQDomNode& parent,
                    const TQString& elementName, const TQPen& pen )
{
    TQDomElement penElement = doc.createElement( elementName );
    parent.appendChild( penElement );
    createIntNode( doc, penElement, "Width", pen.width() );
    createColorNode( doc, penElement, "Color", pen.color() );
    createStringNode( doc, penElement, "Style", penStyleToString( pen.style() ) );
}


void createDateTimeNode( TQDomDocument& doc, TQDomNode& parent,
                         const TQString& elementName,
                         const TQDateTime& datetime )
{
    TQDomElement dateTimeElement = doc.createElement( elementName );
    parent.appendChild( dateTimeElement );
    createDateNode( doc, dateTimeElement, "Date", datetime.date() );
    createTimeNode( doc, dateTimeElement, "Time", datetime.time() );
}


void createDateNode( TQDomDocument& doc, TQDomNode& parent,
                     const TQString& elementName, const TQDate& date )
{
    TQDomElement dateElement = doc.createElement( elementName );
    parent.appendChild( dateElement );
    dateElement.setAttribute( "Year", TQString::number( date.year() ) );
    dateElement.setAttribute( "Month", TQString::number( date.month() ) );
    dateElement.setAttribute( "Day", TQString::number( date.day() ) );
}


void createTimeNode( TQDomDocument& doc, TQDomNode& parent,
                      const TQString& elementName, const TQTime& time )
{
    TQDomElement timeElement = doc.createElement( elementName );
    parent.appendChild( timeElement );
    timeElement.setAttribute( "Hour",
                               TQString::number( time.hour() ) );
    timeElement.setAttribute( "Minute",
                               TQString::number( time.minute() ) );
    timeElement.setAttribute( "Second",
                               TQString::number( time.second() ) );
    timeElement.setAttribute( "Millisecond",
                               TQString::number( time.msec() ) );
}


TQString penStyleToString( Qt::PenStyle style )
{
    switch( style ) {
    case Qt::NoPen:
        return "NoPen";
    case Qt::SolidLine:
        return "SolidLine";
    case Qt::DashLine:
        return "DashLine";
    case Qt::DotLine:
        return "DotLine";
    case Qt::DashDotLine:
        return "DashDotLine";
    case Qt::DashDotDotLine:
        return "DashDotDotLine";
    default: // should not happen
        return "SolidLine";
    }
}



TQString brushStyleToString( Qt::BrushStyle style )
{
    // PENDING(kalle) Support custom patterns
    switch( style ) {
    case Qt::NoBrush:
        return "NoBrush";
    case Qt::SolidPattern:
        return "SolidPattern";
    case Qt::Dense1Pattern:
        return "Dense1Pattern";
    case Qt::Dense2Pattern:
        return "Dense2Pattern";
    case Qt::Dense3Pattern:
        return "Dense3Pattern";
    case Qt::Dense4Pattern:
        return "Dense4Pattern";
    case Qt::Dense5Pattern:
        return "Dense5Pattern";
    case Qt::Dense6Pattern:
        return "Dense6Pattern";
    case Qt::Dense7Pattern:
        return "Dense7Pattern";
    case Qt::HorPattern:
        return "HorPattern";
    case Qt::VerPattern:
        return "VerPattern";
    case Qt::CrossPattern:
        return "CrossPattern";
    case Qt::BDiagPattern:
        return "BDiagPattern";
    case Qt::FDiagPattern:
        return "FDiagPattern";
    case Qt::DiagCrossPattern:
        return "DiagCrossPattern";
    default: // should not happen (but can for a custom pattern)
        return "SolidPattern";
    }
}


bool readStringNode( const TQDomElement& element, TQString& value )
{
    value = element.text();
    return true;
}


bool readIntNode( const TQDomElement& element, int& value )
{
    bool ok = false;
    int temp = element.text().toInt( &ok );
    if( ok )
        value = temp;
    return ok;
}


bool readDoubleNode( const TQDomElement& element, double& value )
{
    bool ok = false;
    double temp = element.text().toDouble( &ok );
    if( ok )
        value = temp;
    return ok;
}


bool readBoolNode( const TQDomElement& element, bool& value )
{
    if( element.text() == "true" ) {
        value = true;
        return true;
    } else if( element.text() == "false" ) {
        value = false;
        return true;
    } else
        return false;
}


bool readColorNode( const TQDomElement& element, TQColor& value )
{
    bool ok = true;
    int red, green, blue;
    if( element.hasAttribute( "Red" ) ) {
        bool redOk = false;
        red = element.attribute( "Red" ).toInt( &redOk );
        ok = ok & redOk;
    }
    if( element.hasAttribute( "Green" ) ) {
        bool greenOk = false;
        green = element.attribute( "Green" ).toInt( &greenOk );
        ok = ok & greenOk;
    }
    if( element.hasAttribute( "Blue" ) ) {
        bool blueOk = false;
        blue = element.attribute( "Blue" ).toInt( &blueOk );
        ok = ok & blueOk;
    }

    if( ok )
        value.setRgb( red, green, blue );

    return ok;
}


bool readBrushNode( const TQDomElement& element, TQBrush& brush )
{
    bool ok = true;
    TQColor tempColor;
    Qt::BrushStyle tempStyle;
    TQPixmap tempPixmap;
    TQDomNode node = element.firstChild();
    while( !node.isNull() ) {
        TQDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            TQString tagName = element.tagName();
            if( tagName == "Color" ) {
                ok = ok & readColorNode( element, tempColor );
            } else if( tagName == "Style" ) {
		TQString value;
                ok = ok & readStringNode( element, value );
		tempStyle = stringToBrushStyle( value );
            } else if( tagName == "Pixmap" ) {
                ok = ok & readPixmapNode( element, tempPixmap );
            } else {
                qDebug( "Unknown tag in brush" );
            }
        }
        node = node.nextSibling();
    }

    if( ok ) {
	brush.setColor( tempColor );
	brush.setStyle( tempStyle );
        if( !tempPixmap.isNull() )
            brush.setPixmap( tempPixmap );
    }

    return ok;
}


bool readPixmapNode( const TQDomElement& element, TQPixmap& pixmap )
{
    bool ok = true;
    int tempLengthi;
    TQString tempData;
    TQDomNode node = element.firstChild();
    while( !node.isNull() ) {
        TQDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            TQString tagName = element.tagName();
            if( tagName == "Format" ) {
                TQString formatName;
                ok = ok & readStringNode( element, formatName );
#ifndef NDEBUG
                if( formatName != "XPM.GZ" )
                    qDebug( "Unsupported pixmap format in XML file" );
#endif
            } else if( tagName == "Length" ) {
                ok = ok & readIntNode( element, tempLengthi );
            } else if( tagName == "Data" ) {
                ok = ok & readStringNode( element, tempData );
            } else {
                qDebug( "Unknown tag in Pixmap" );
            }
        }
        node = node.nextSibling();
    }

    if( ok ) {
	if( 0 < tempLengthi ) {
            // Decode the image file format in the same way Qt Designer does.
            char *ba = new char[ tempData.length() / 2 ];
            for ( int i = 0; i < (int)tempData.length() / 2; ++i ) {
                char h = tempData[ 2 * i ].latin1();
                char l = tempData[ 2 * i  + 1 ].latin1();
                uchar r = 0;
                if ( h <= '9' )
                    r += h - '0';
                else
                    r += h - 'a' + 10;
                r = r << 4;
                if ( l <= '9' )
                    r += l - '0';
                else
                    r += l - 'a' + 10;
                ba[ i ] = r;
            }

            if( tempLengthi < (int)tempData.length() * 5 )
                tempLengthi = tempData.length() * 5;
            unsigned long tempLength = tempLengthi;
            TQByteArray baunzip( tempLength );
            ::uncompress( (uchar*) baunzip.data(), &tempLength,
                          (uchar*) ba, tempData.length()/2 );
            TQImage image;
            image.loadFromData( (const uchar*)baunzip.data(), tempLength, "XPM" );

            if( image.isNull() )
                pixmap.resize( 0, 0 ); // This is _not_ an error, we just read a NULL pixmap!
            else
                ok = ok & pixmap.convertFromImage( image, 0 );
        } else
            pixmap.resize( 0, 0 ); // This is _not_ an error, we just read a empty pixmap!
    }

    return ok;
}


bool readPenNode( const TQDomElement& element, TQPen& pen )
{
    bool ok = true;
    int tempWidth;
    TQColor tempColor;
    Qt::PenStyle tempStyle;
    TQDomNode node = element.firstChild();
    while( !node.isNull() ) {
        TQDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            TQString tagName = element.tagName();
            if( tagName == "Width" ) {
                ok = ok & readIntNode( element, tempWidth );
            } else if( tagName == "Color" ) {
                ok = ok & readColorNode( element, tempColor );
            } else if( tagName == "Style" ) {
		TQString value;
                ok = ok & readStringNode( element, value );
		tempStyle = stringToPenStyle( value );
            } else {
                qDebug( "Unknown tag in brush" );
            }
        }
        node = node.nextSibling();
    }

    if( ok ) {
        pen.setWidth( tempWidth );
	pen.setColor( tempColor );
	pen.setStyle( tempStyle );
    }

    return ok;
}

bool readFontNode( const TQDomElement& element, TQFont& font )
{
    bool ok = true;
    TQString family;
    int pointSize, pixelSize, weight;
    bool italic;
    int charSet;
    TQDomNode node = element.firstChild();
    while( !node.isNull() ) {
        TQDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            TQString tagName = element.tagName();
            if( tagName == "Family" ) {
                ok = ok & readStringNode( element, family );
            } else if( tagName == "PointSize" ) {
                ok = ok & readIntNode( element, pointSize );
            } else if( tagName == "PixelSize" ) {
                ok = ok & readIntNode( element, pixelSize );
            } else if( tagName == "Weight" ) {
                ok = ok & readIntNode( element, weight );
            } else if( tagName == "Italic" ) {
                ok = ok & readBoolNode( element, italic );
            } else if( tagName == "CharSet" ) {
                ok = ok & readIntNode( element, charSet );
            } else {
                qDebug( "Unknown tag in color map" );
            }
        }
        node = node.nextSibling();
    }

    if( ok ) {
        font.setFamily( family );
	if ( pointSize > 0 ) font.setPointSize( pointSize );
	if ( pixelSize > 0 ) font.setPixelSize( pixelSize );
        font.setWeight( weight );
        font.setItalic( italic );
#if QT_VERSION < 300
        // Qt 3 handles charsets internally.
        font.setCharSet( (TQFont::CharSet)charSet );
#endif
    }

    return ok;
}

bool readRectNode( const TQDomElement& element, TQRect& value )
{
    bool ok = true;
    int width, height, x, y;
    TQDomNode node = element.firstChild();
    while( !node.isNull() ) {
        TQDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            TQString tagName = element.tagName();
            if( tagName == "Width" ) {
                ok = ok & readIntNode( element, width );
            } else if( tagName == "Height" ) {
                ok = ok & readIntNode( element, height );
            } else if( tagName == "X" ) {
                ok = ok & readIntNode( element, x );
            } else if( tagName == "Y" ) {
                ok = ok & readIntNode( element, y );
            } else {
                qDebug( "Unknown tag in rect" );
            }
        }
        node = node.nextSibling();
    }

    if( ok ) {
        value.setX( x );
        value.setY( y );
        value.setWidth( width );
        value.setHeight( height );
    }

    return ok;
}



bool readDateTimeNode( const TQDomElement& element, TQDateTime& datetime )
{
    bool ok = true;
    TQDate tempDate;
    TQTime tempTime;
    TQDomNode node = element.firstChild();
    while( !node.isNull() ) {
        TQDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            TQString tagName = element.tagName();
            if( tagName == "Date" ) {
                ok = ok & readDateNode( element, tempDate );
            } else if( tagName == "Time" ) {
                ok = ok & readTimeNode( element, tempTime );
            } else {
                qDebug( "Unknown tag in datetime" );
            }
        }
        node = node.nextSibling();
    }

    if( ok ) {
        datetime.setDate( tempDate );
        datetime.setTime( tempTime );
    }

    return ok;
}


bool readDateNode( const TQDomElement& element, TQDate& value )
{
    bool ok = true;
    int year, month, day;
    if( element.hasAttribute( "Year" ) ) {
        bool yearOk = false;
        year = element.attribute( "Year" ).toInt( &yearOk );
        ok = ok & yearOk;
    }
    if( element.hasAttribute( "Month" ) ) {
        bool monthOk = false;
        month = element.attribute( "Month" ).toInt( &monthOk );
        ok = ok & monthOk;
    }
    if( element.hasAttribute( "Day" ) ) {
        bool dayOk = false;
        day = element.attribute( "Day" ).toInt( &dayOk );
        ok = ok & dayOk;
    }

    if( ok )
        value.setYMD( year, month, day );

    return ok;
}



bool readTimeNode( const TQDomElement& element, TQTime& value )
{
    bool ok = true;
    int hour, minute, second, msec;
    if( element.hasAttribute( "Hour" ) ) {
        bool hourOk = false;
        hour = element.attribute( "Hour" ).toInt( &hourOk );
        ok = ok & hourOk;
    }
    if( element.hasAttribute( "Minute" ) ) {
        bool minuteOk = false;
        minute = element.attribute( "Minute" ).toInt( &minuteOk );
        ok = ok & minuteOk;
    }
    if( element.hasAttribute( "Second" ) ) {
        bool secondOk = false;
        second = element.attribute( "Second" ).toInt( &secondOk );
        ok = ok & secondOk;
    }
    if( element.hasAttribute( "Millisecond" ) ) {
        bool msecOk = false;
        msec = element.attribute( "Millisecond" ).toInt( &msecOk );
        ok = ok & msecOk;
    }

    if( ok )
        value.setHMS( hour, minute, second, msec );

    return ok;
}



Qt::PenStyle stringToPenStyle( const TQString& style )
{
    if( style == "NoPen" )
        return Qt::NoPen;
    else if( style == "SolidLine" )
        return Qt::SolidLine;
    else if( style == "DashLine" )
        return Qt::DashLine;
    else if( style == "DotLine" )
        return Qt::DotLine;
    else if( style == "DashDotLine" )
        return Qt::DashDotLine;
    else if( style == "DashDotDotLine" )
        return Qt::DashDotDotLine;
    else // should not happen
        return Qt::SolidLine;
}


Qt::BrushStyle stringToBrushStyle( const TQString& style )
{
    // PENDING(kalle) Support custom patterns
    if( style == "NoBrush" )
        return Qt::NoBrush;
    else if( style == "SolidPattern" )
        return Qt::SolidPattern;
    else if( style == "Dense1Pattern" )
        return Qt::Dense1Pattern;
    else if( style == "Dense2Pattern" )
        return Qt::Dense2Pattern;
    else if( style == "Dense3Pattern" )
        return Qt::Dense3Pattern;
    else if( style == "Dense4Pattern" )
        return Qt::Dense4Pattern;
    else if( style == "Dense5Pattern" )
        return Qt::Dense5Pattern;
    else if( style == "Dense6Pattern" )
        return Qt::Dense6Pattern;
    else if( style == "Dense7Pattern" )
        return Qt::Dense7Pattern;
    else if( style == "HorPattern" )
        return Qt::HorPattern;
    else if( style == "VerPattern" )
        return Qt::VerPattern;
    else if( style == "CrossPattern" )
        return Qt::CrossPattern;
    else if( style == "BDiagPattern" )
        return Qt::BDiagPattern;
    else if( style == "FDiagPattern" )
        return Qt::FDiagPattern;
    else if( style == "DiagCrossPattern" )
        return Qt::DiagCrossPattern;
    else // should not happen (but can with custom patterns)
        return Qt::SolidPattern;
}

}
