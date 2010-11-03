/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "htmlexporter.cpp", which was part
    of the QtGui module of the Qt Toolkit. It has been customized for
    use in Blogilo, at December 2008.

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "htmlexporter.h"

#include <QString>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextBlock>
#include <QTextList>
#include <QTextTable>
#include <QTextFormat>
#include <QVarLengthArray>
#include <kdebug.h>
#include "bilbotextformat.h"

HtmlExporter::HtmlExporter()
{
    //kDebug();
}

HtmlExporter::~HtmlExporter()
{
    //kDebug();
}

QString HtmlExporter::toHtml( const QTextDocument* document )
{
    //kDebug();
    if ( document->isEmpty() ) {
        return html;
    }
    doc = document;

    //const QFont defaultFont = doc->defaultFont();
    //defaultCharFormat.setFont(defaultFont);
    emitFrame( doc->rootFrame()->begin() );
//  emitBlock(doc->rootFrame()->begin().currentBlock());
    //sanitizeHtml();
    html.replace( QRegExp( "<br[\\s]*/>" ), "<br />\n" );
    if ( html.endsWith( QLatin1String( ">\n") ) ) {
        html.remove( html.length() - 1, 1 );
    }
    return html;
}


void HtmlExporter::emitFrame( QTextFrame::Iterator frameIt )
{
//     kDebug() << "html" << html;
    if ( !frameIt.atEnd() ) {
        QTextFrame::Iterator next = frameIt;
        ++next;
        if ( next.atEnd()
                && frameIt.currentFrame() == 0
                && frameIt.parentFrame() != doc->rootFrame()
                && frameIt.currentBlock().begin().atEnd() ) {
            return;
        }
    }


    for ( QTextFrame::Iterator it = frameIt;
            !it.atEnd(); ++it ) {;
        if ( QTextFrame *f = it.currentFrame() ) {
//	    kDebug() << "Its a frame, not a block" ;
            if ( QTextTable * table = qobject_cast<QTextTable *>( f ) ) {
                emitTable( table );
            } else {
//     qDebug() << "isn't table" << endl;
                html += QLatin1String( "\n<table" );
                QTextFrameFormat format = f->frameFormat();

                if ( format.hasProperty( QTextFormat::FrameBorder ) ) {
                    emitAttribute( "border", QString::number( format.border() ) );
                }

                html += QLatin1String( " style=\"-qt-table-type: frame;" );
                emitFloatStyle( format.position(), OmitStyleTag );

                if ( format.hasProperty( QTextFormat::FrameMargin ) ) {
                    const QString margin = QString::number( format.margin() );
                    emitMargins( margin, margin, margin, margin );
                }

                html += QLatin1Char( '\"' );

                emitTextLength( "width", format.width() );
                emitTextLength( "height", format.height() );

                QBrush bg = format.background();
                if ( bg != Qt::NoBrush ) {
                    emitAttribute( "bgcolor", bg.color().name() );
                }

                html += QLatin1Char( '>' );
                html += QLatin1String( "\n<tr>\n<td style=\"border: none;\">" );
                emitFrame( f->begin() );
                html += QLatin1String( "</td></tr></table>" );
            }
        } else if ( it.currentBlock().isValid() ) {
//    qDebug()<< "is valid" << endl;
            emitBlock( it.currentBlock() );
        }
    }
}


void HtmlExporter::emitTable( const QTextTable *table )
{
    //qDebug() << "emitTable" << html;
    QTextTableFormat format = table->format();

    html += QLatin1String( "\n<table" );

    if ( format.hasProperty( QTextFormat::FrameBorder ) ) {
        emitAttribute( "border", QString::number( format.border() ) );
    }

    emitFloatStyle( format.position() );
    emitAlignment( format.alignment() );
    emitTextLength( "width", format.width() );

    if ( format.hasProperty( QTextFormat::TableCellSpacing ) ) {
        emitAttribute( "cellspacing", QString::number( format.cellSpacing() ) );
    }
    if ( format.hasProperty( QTextFormat::TableCellPadding ) ) {
        emitAttribute( "cellpadding", QString::number( format.cellPadding() ) );
    }

    QBrush bg = format.background();
    if ( bg != Qt::NoBrush ) {
        emitAttribute( "bgcolor", bg.color().name() );
    }

    html += QLatin1Char( '>' );

    const int rows = table->rows();
    const int columns = table->columns();

    QVector<QTextLength> columnWidths = format.columnWidthConstraints();
    if ( columnWidths.isEmpty() ) {
        columnWidths.resize( columns );
        columnWidths.fill( QTextLength() );
    }
//    Q_ASSERT(columnWidths.count() == columns);

    QVarLengthArray<bool> widthEmittedForColumn( columns );
    for ( int i = 0; i < columns; ++i ) {
        widthEmittedForColumn[i] = false;
    }

    const int headerRowCount = qMin( format.headerRowCount(), rows );
    if ( headerRowCount > 0 ) {
        html += QLatin1String( "<thead>" );
    }

    for ( int row = 0; row < rows; ++row ) {
        html += QLatin1String( "\n<tr>" );

        for ( int col = 0; col < columns; ++col ) {
            const QTextTableCell cell = table->cellAt( row, col );

            // for col/rowspans
            if ( cell.row() != row ) {
                continue;
            }

            if ( cell.column() != col ) {
                continue;
            }

            html += QLatin1String( "\n<td" );

            if ( !widthEmittedForColumn[col] ) {
                emitTextLength( "width", columnWidths.at( col ) );
                widthEmittedForColumn[col] = true;
            }

            if ( cell.columnSpan() > 1 ) {
                emitAttribute( "colspan", QString::number( cell.columnSpan() ) );
            }

            if ( cell.rowSpan() > 1 ) {
                emitAttribute( "rowspan", QString::number( cell.rowSpan() ) );
            }

            const QTextCharFormat cellFormat = cell.format();
            QBrush bg = cellFormat.background();
            if ( bg != Qt::NoBrush ) {
                emitAttribute( "bgcolor", bg.color().name() );
            }

            html += QLatin1Char( '>' );

            emitFrame( cell.begin() );

            html += QLatin1String( "</td>" );
        }

        html += QLatin1String( "</tr>" );
        if ( headerRowCount > 0 && row == headerRowCount - 1 ) {
            html += QLatin1String( "</thead>" );
        }
    }

    html += QLatin1String( "</table>" );
}


void HtmlExporter::emitAttribute( const char *attribute, const QString &value )
{
//     kDebug() << "html" << html;
    html += QLatin1Char( ' ' );
    html += QLatin1String( attribute );
    html += QLatin1String( "=\"" );
    html += value;
    html += QLatin1Char( '"' );
}

QList<HtmlExporter::tag> HtmlExporter::emitCharFormatStyle( const QTextCharFormat 
                         &charFormat, const QTextBlockFormat &blockFormat  )
{
//     kDebug() << "html" << html;

    QList<HtmlExporter::tag> tags;
    bool attributesEmitted = false;
    QLatin1String styleTag( "<span style=\"" );

    const QString family = charFormat.fontFamily();

    //if (!family.isEmpty() && family != defaultCharFormat.fontFamily()) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.hasProperty( BilboTextFormat::HasCodeStyle ) && 
         charFormat.boolProperty( BilboTextFormat::HasCodeStyle ) ) {
        tags << code;

    } else if ( !blockFormat.nonBreakableLines() && !family.isEmpty() && family != mDefaultCharFormat.fontFamily() ) {
//         if ( family.right( 7 ) == "courier" ) {
//             tags << code;
//         } else {
            if ( ! attributesEmitted ) {
                html += styleTag;
            }
            html += QLatin1String( " font-family:'" );
            html += family;
            html += QLatin1String( "';" );
            attributesEmitted = true;
//         }
    }


//     if (format.hasProperty(QTextFormat::FontPointSize)
//             && format.fontPointSize() != defaultCharFormat.fontPointSize()) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.hasProperty( QTextFormat::FontPointSize )
         && charFormat.fontPointSize() != mDefaultCharFormat.fontPointSize() ) {
        if ( ! attributesEmitted ) {
            html += styleTag;
        }
        html += QLatin1String( " font-size:" );
        html += QString::number( charFormat.fontPointSize() );
        html += QLatin1String( "pt;" );
        attributesEmitted = true;
    } else if ( charFormat.hasProperty( QTextFormat::FontSizeAdjustment ) && 
               !( blockFormat.hasProperty( BilboTextFormat::HtmlHeading ) &&
                  blockFormat.intProperty( BilboTextFormat::HtmlHeading ) ) ) {

        ///To use <h1-5> tags for font size
//         const int idx = format.intProperty(QTextFormat::FontSizeAdjustment) + 1;
//
//         switch (idx) {
//         case 0: tags << h5; break;
//             //case 1: tags << h4; break; //NOTE h4 will be the normal text!
//         case 2: tags << h3; break;
//         case 3: tags << h2; break;
//         case 4: tags << h1; break;
//         }

        ///To use <span> tags for font size
        static const char * const sizeNames[] = {
            "small", "medium", "large", "x-large", "xx-large"
        };
        const char *name = 0;
        const int idx = charFormat.intProperty( QTextFormat::FontSizeAdjustment ) + 1;
        if ( idx == 1 ) {
            // assume default to not bloat the html too much
        } else if ( idx >= 0 && idx <= 4 ) {
            name = sizeNames[idx];
        }
        if ( name ) {
            if ( ! attributesEmitted ) {
                html += styleTag;
            }
            html += QLatin1String( " font-size:" );
            html += QLatin1String( name );
            html += QLatin1Char( ';' );
            attributesEmitted = true;
        }
    }


//    if (format.fontWeight() > defaultCharFormat.fontWeight()) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.fontWeight() > mDefaultCharFormat.fontWeight() && 
        !( blockFormat.hasProperty( BilboTextFormat::HtmlHeading ) &&
           blockFormat.intProperty( BilboTextFormat::HtmlHeading ) ) ) {
        tags << strong;
        /*if (! attributesEmitted ) html += styleTag;
        html += QLatin1String(" font-weight:");
        html += QString::number(format.fontWeight() * 8);
        html += QLatin1Char(';');
        attributesEmitted = true;*/
    }

//    if (format.fontItalic() != defaultCharFormat.fontItalic()) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.fontItalic() != mDefaultCharFormat.fontItalic() ) {
        tags << em;
        /*
        if (! attributesEmitted ) html += styleTag;
        html += QLatin1String(" font-style:");
        html += (format.fontItalic() ? QLatin1String("italic") : QLatin1String("normal"));
        html += QLatin1Char(';');
        attributesEmitted = true;*/
    }

//    if (format.fontUnderline() != defaultCharFormat.fontUnderline()) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.fontUnderline() != mDefaultCharFormat.fontUnderline() ) {
        tags << u;
    }


//    if (format.fontStrikeOut() != defaultCharFormat.fontStrikeOut()) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.fontStrikeOut() != mDefaultCharFormat.fontStrikeOut() ) {
        tags << s;
    }

//    if (format.fontOverline() != defaultCharFormat.fontOverline()) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.fontOverline() != mDefaultCharFormat.fontOverline() ) {
        if ( charFormat.fontOverline() ) {
            if ( ! attributesEmitted ) {
                html += styleTag;
            }
            html += QLatin1String( " text-decoration: overline;" );
            attributesEmitted = true;
        }
    }

    /*
    bool hasDecoration = false;
    bool atLeastOneDecorationSet = false;
    QLatin1String decorationTag(" text-decoration:");

    if (format.fontUnderline() != defaultCharFormat.fontUnderline() ||
    format.fontOverline() != defaultCharFormat.fontOverline() ||
    format.fontStrikeOut() != defaultCharFormat.fontStrikeOut() )
    {
      if (! attributesEmitted ) html += styleTag;
      html += decorationTag;
    }

    if (format.fontUnderline() != defaultCharFormat.fontUnderline()) {
        hasDecoration = true;
        if (format.fontUnderline()) {
            html += QLatin1String(" underline");
            atLeastOneDecorationSet = true;
        }
    }

    if (format.fontOverline() != defaultCharFormat.fontOverline()) {
        hasDecoration = true;
        if (format.fontOverline()) {
            html += QLatin1String(" overline");
            atLeastOneDecorationSet = true;
        }
    }

    if (format.fontStrikeOut() != defaultCharFormat.fontStrikeOut()) {
        hasDecoration = true;
        if (format.fontStrikeOut()) {
            html += QLatin1String(" line-through");
            atLeastOneDecorationSet = true;
        }
    }

    if (hasDecoration) {
        if (!atLeastOneDecorationSet)
            html += QLatin1String("none");
        html += QLatin1Char(';');
        attributesEmitted = true;
    }*/
    /* else {
        html.chop(qstrlen(decorationTag.latin1()));
    }*/

//     QBrush linkColor = KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::LinkText);

//    if ( format.foreground() != defaultCharFormat.foreground() &&
//            format.foreground().style() != Qt::NoBrush) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.foreground() != mDefaultCharFormat.foreground() &&
            charFormat.foreground().style() != Qt::NoBrush && !charFormat.isAnchor() ) {
        //         if ( format.foreground() != linkColor ) {
//    if ( format.anchorHref().isNull() ) {
        if ( ! attributesEmitted ) {
            html += styleTag;
            attributesEmitted = true;
        }
//    } else {
//     html += QLatin1String(" style=\"");
//    }
        html += QLatin1String( " color:" );
        html += charFormat.foreground().color().name();
        html += QLatin1Char( ';' );
//    if ( !format.anchorHref().isNull() ) {
//     html += QLatin1String("\"");
//    }
//    attributesEmitted = true;
        //         }
    }

//    if (format.background() != defaultCharFormat.background()
//            && format.background().style() != Qt::NoBrush) {
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( !( charFormat.hasProperty( BilboTextFormat::HasCodeStyle ) && 
         charFormat.boolProperty( BilboTextFormat::HasCodeStyle ) ) ) {
        if ( charFormat.background() != mDefaultCharFormat.background()
                && charFormat.background().style() != Qt::NoBrush ) {
            if ( ! attributesEmitted ) {
                html += styleTag;
            }
            html += QLatin1String( " background-color:" );
            html += charFormat.background().color().name();
            html += QLatin1Char( ';' );
            attributesEmitted = true;
        }
    }
//    if (format.verticalAlignment() != defaultCharFormat.verticalAlignment()) { //TODO
    // NOTE the above line replaced with the bottom line to use default charFormat, which can be set from outside.
    if ( charFormat.verticalAlignment() != mDefaultCharFormat.verticalAlignment() ) { //TODO
        if ( ! attributesEmitted ) {
            html += styleTag;
        }
        html += QLatin1String( " vertical-align:" );

        QTextCharFormat::VerticalAlignment valign = charFormat.verticalAlignment();
        if ( valign == QTextCharFormat::AlignSubScript ) {
            html += QLatin1String( "sub" );
        } else if ( valign == QTextCharFormat::AlignSuperScript ) {
            html += QLatin1String( "super" );
        }

        html += QLatin1Char( ';' );
        attributesEmitted = true;
    }

    //Append close span Tag
    if ( attributesEmitted ) {
        html += QLatin1String( "\">" );
        tags.prepend( span );
    }

//     kDebug() << "html=>" << html << tags;
    return tags;
}

void HtmlExporter::emitTextLength( const char *attribute, const QTextLength &length )
{
//     kDebug() << "html" << html;
    if ( length.type() == QTextLength::VariableLength ) { // default
        return;
    }

    html += QLatin1Char( ' ' );
    html += QLatin1String( attribute );
    html += QLatin1String( "=\"" );
    html += QString::number( length.rawValue() );

    if ( length.type() == QTextLength::PercentageLength ) {
        html += QLatin1String( "%\"" );
    } else {
        html += QLatin1String( "\"" );
    }
}

void HtmlExporter::emitAlignment( Qt::Alignment align )
{
    //qDebug() << "emitAlignment" << html;
//    if (align & Qt::AlignLeft) {
//  if (align & mDefaultBlockFormat.alignment()) {
//         return;
//  }
    if ( align & Qt::AlignLeft ) {
        html += QLatin1String( " align=\"left\"" );
    } else if ( align & Qt::AlignRight ) {
        html += QLatin1String( " align=\"right\"" );
    } else if ( align & Qt::AlignHCenter ) {
        html += QLatin1String( " align=\"center\"" );
    } else if ( align & Qt::AlignJustify ) {
        html += QLatin1String( " align=\"justify\"" );
    }
    //qDebug() << "emitAlignment" << html;
}

void HtmlExporter::emitFloatStyle( QTextFrameFormat::Position pos, StyleMode mode )
{
//     kDebug() << "html" << html;
    if ( pos == QTextFrameFormat::InFlow ) {
        return;
    }

    if ( mode == EmitStyleTag ) {
        html += QLatin1String( " style=\"float:" );
    } else {
        html += QLatin1String( " float:" );
    }

    if ( pos == QTextFrameFormat::FloatLeft ) {
        html += QLatin1String( " left;" );
    } else if ( pos == QTextFrameFormat::FloatRight ) {
        html += QLatin1String( " right;" );
    } else {
//        Q_ASSERT_X(0, "HtmlExporter::emitFloatStyle()", "pos should be a valid enum type");
    }

    if ( mode == EmitStyleTag ) {
        html += QLatin1Char( '\"' );
    }
}

void HtmlExporter::emitMargins( const QString &top, const QString &bottom, const QString &left, const QString &right )
{
//     kDebug() << "html" << html;
    html += QLatin1String( " margin-top:" );
    html += top;
    html += QLatin1String( "px;" );

    html += QLatin1String( " margin-bottom:" );
    html += bottom;
    html += QLatin1String( "px;" );

    html += QLatin1String( " margin-left:" );
    html += left;
    html += QLatin1String( "px;" );

    html += QLatin1String( " margin-right:" );
    html += right;
    html += QLatin1String( "px;" );
}

void HtmlExporter::emitFragment( const QTextFragment &fragment, const QTextBlockFormat &blockFormat )
{
//     kDebug() << "html" << html;
    const QTextCharFormat format = fragment.charFormat();

    bool closeAnchor = false;
    bool anchorIsOpen = false;

    if ( format.isAnchor() ) {
//         const QStringList names = format.anchorNames();
//         if (!names.isEmpty()) {
//             html += QLatin1String("<a name=\"");
//             html += names.at(0);
//             html += QLatin1String("\" ");
//    anchorIsOpen = true;
//         }
        const QString href = format.anchorHref();
        if ( !href.isEmpty() ) {
//    if (!anchorIsOpen) {
//     html += QLatin1String("<a ");
//     anchorIsOpen = true;
//    }
            html += QLatin1String( "<a href=\"" );
            html += href;
            html += QLatin1String( "\"" );
            anchorIsOpen = true;
//             closeAnchor = true;
//    html += QLatin1String("\"");
        }
        if ( format.hasProperty( BilboTextFormat::AnchorTitle ) ) {
            const QString title = format.stringProperty( BilboTextFormat::AnchorTitle );
            if ( !title.isEmpty() ) {
                html += QLatin1String( " title=\"" );
                html += title;
                html += QLatin1String( "\"" );
            }
        }
        if ( format.hasProperty( BilboTextFormat::AnchorTarget ) ) {
            const QString target = format.stringProperty( BilboTextFormat::AnchorTarget );
            if ( !target.isEmpty() ) {
                html += QLatin1String( " target=\"" );
                html += target;
                html += QLatin1String( "\"" );
            }
        }
        if ( anchorIsOpen ) {
            html += QLatin1String( ">" );
            closeAnchor = true;
        }
    }

    QList<tag> tags = emitCharFormatStyle( format, blockFormat );
//  if ( !format.anchorHref().isNull() ) {
//   html += QLatin1String(">");
//   closeAnchor = true;
//  }
//     kDebug() << "tags count" << tags.count() << endl;
    for ( int i = 0; i < tags.count(); ++i ) {
        switch ( tags.at( i ) ) {
            case span:
                break; //Jump
//             case h1:
//                 html += QLatin1String( "<h1>" );
//                 break;
//             case h2:
//                 html += QLatin1String( "<h2>" );
//                 break;
//             case h3:
//                 html += QLatin1String( "<h3>" );
//                 break;
//             case h4:
//                 html += QLatin1String( "<h4>" );
//                 break;
//             case h5:
//                 html += QLatin1String( "<h5>" );
//                 break;
            case strong:
                html += QLatin1String( "<strong>" );
                break;
            case em:
                html += QLatin1String( "<em>" );
                break;
            case s:
                html += QLatin1String( "<s>" );
                break;
            case u:
                if ( !closeAnchor )
                    html += QLatin1String( "<u>" );
                break;
            case code:
                html += QLatin1String( "<code>" );
                break;
            case sub:
                html += QLatin1String( "<sub>" );
                break;
            case sup:
                html += QLatin1String( "<sup>" );
                break;
        }
    }

    /*     QLatin1String styleTag("<span style=\"");
         html += styleTag;
     
         const bool attributesEmitted = emitCharFormatStyle(format);
        if (attributesEmitted)
            html += QLatin1String("\">");
        else
            html.chop(qstrlen(styleTag.latin1()));
    */
    QString txt = fragment.text();
//     kDebug() << txt ;
    if ( txt.count() == 1 && txt.at( 0 ) == QChar::ObjectReplacementCharacter ) {
        if ( format.isImageFormat() ) {
            QTextImageFormat imgFmt = format.toImageFormat();

            html += QLatin1String( "<img" );

            if ( imgFmt.hasProperty( QTextFormat::ImageName ) ) {
                emitAttribute( "src", imgFmt.name() );
            }

            if ( imgFmt.hasProperty( BilboTextFormat::ImageTitle ) ) {
                const QString title = imgFmt.stringProperty( BilboTextFormat::ImageTitle );
                if ( !title.isEmpty() ) {
                    emitAttribute( "title", imgFmt.stringProperty( BilboTextFormat::ImageTitle ) );
                }
            }

            if ( imgFmt.hasProperty( BilboTextFormat::ImageAlternateText ) ) {
                const QString alternate = imgFmt.stringProperty( BilboTextFormat::ImageAlternateText );
                if ( !alternate.isEmpty() ) {
                    emitAttribute( "alt", imgFmt.stringProperty( BilboTextFormat::ImageAlternateText ) );
                }
            }

            if ( imgFmt.hasProperty( QTextFormat::ImageWidth ) ) {
                emitAttribute( "width", QString::number( imgFmt.width() ) );
            }

            if ( imgFmt.hasProperty( QTextFormat::ImageHeight ) ) {
                emitAttribute( "height", QString::number( imgFmt.height() ) );
            }

            if ( QTextFrame *imageFrame = qobject_cast<QTextFrame *>( doc->objectForFormat( imgFmt ) ) ) {
                emitFloatStyle( imageFrame->frameFormat().position() );
            }

            html += QLatin1String( " />" );
        }
    } else {
//         Q_ASSERT(!txt.contains(QChar::ObjectReplacementCharacter));

        txt = Qt::escape( txt );

        // split for [\n{LineSeparator}]
        QString forcedLineBreakRegExp = QString::fromLatin1( "[\\na]" );
        forcedLineBreakRegExp[3] = QChar::LineSeparator;

        const QStringList lines = txt.split( QRegExp( forcedLineBreakRegExp ) );
        for ( int i = 0; i < lines.count(); ++i ) {
            if ( i > 0 ) {
		if ( blockFormat.nonBreakableLines() )
		{
		    html += QLatin1String( "\n" );
		} else {
		    html += QLatin1String( "<br />" ); // space on purpose for compatibility with Netscape, Lynx & Co.
		}
	    }
            //and to convert LineSeparators to <br /> tags.
            html += lines.at( i );
        }
    }

//     kDebug() << html ;

    //Close Tags
    //if (!closeAnchor)
    for ( int i = tags.count(); i > 0; --i ) {
        switch ( tags.at( i - 1 ) ) {
            case span:
                html += QLatin1String( "</span>" );
                break; //Jump
//             case h1:
//                 html += QLatin1String( "</h1>" );
//                 break;
//             case h2:
//                 html += QLatin1String( "</h2>" );
//                 break;
//             case h3:
//                 html += QLatin1String( "</h3>" );
//                 break;
//             case h4:
//                 html += QLatin1String( "</h4>" );
//                 break;
//             case h5:
//                 html += QLatin1String( "</h5>" );
//                 break;
            case strong:
                html += QLatin1String( "</strong>" );
                break;
            case em:
                html += QLatin1String( "</em>" );
                break;
            case s:
                html += QLatin1String( "</s>" );
                break;
            case u:
                if ( !closeAnchor )
                    html += QLatin1String( "</u>" );
                break;
            case code:
                html += QLatin1String( "</code>" );
                break;
            case sub:
                html += QLatin1String( "</sub>" );
                break;
            case sup:
                html += QLatin1String( "</sup>" );
                break;
        }
    }
    /*    if (attributesEmitted)
            html += QLatin1String("</span>");
    */
    if ( closeAnchor ) {
        html += QLatin1String( "</a>" );
    }
//     kDebug() << "html=>" << html;
}

void HtmlExporter::emitBlockAttributes( const QTextBlock &block )
{
//     kDebug() << "html" << html;
    QTextBlockFormat format = block.blockFormat();

    if (format.hasProperty( QTextFormat::LayoutDirection ) ) {
    Qt::LayoutDirection dir = format.layoutDirection();
//  if (dir == Qt::LeftToRight) {
//   mDefaultBlockFormat.setAlignment(Qt::AlignLeft);
//  } else {
//   mDefaultBlockFormat.setAlignment(Qt::AlignRight);
//  }

//     if ( dir != mDefaultBlockFormat.layoutDirection() ) {
        // assume default to not bloat the html too much
        if ( dir == Qt::LeftToRight ) {
            html += QLatin1String( " dir=\"ltr\"" );
//    mDefaultBlockFormat.setAlignment(Qt::AlignLeft);
        } else {
            html += QLatin1String( " dir=\"rtl\"" );
//    mDefaultBlockFormat.setAlignment(Qt::AlignRight);
        }
    }

    if ( format.hasProperty( QTextFormat::BlockAlignment ) ) {
        emitAlignment( format.alignment() );
    }

    bool attributesEmitted = false;
    QLatin1String style( " style=\"" );
    //html += style;

//     if (block.begin().atEnd()) {
//         html += QLatin1String("-qt-paragraph-type:empty;");
//     }

    if ( format.hasProperty( QTextBlockFormat::FrameMargin ) ) {
        if ( !attributesEmitted ) {
            html += style;
            attributesEmitted = true;
        }
        emitMargins( QString::number( format.topMargin() ),
                     QString::number( format.bottomMargin() ),
                     QString::number( format.leftMargin() ),
                     QString::number( format.rightMargin() ) );
    }

    if ( format.hasProperty( QTextBlockFormat::BlockIndent ) ) {
//  if (format.indent() == 0) {
        if ( format.indent() == mDefaultBlockFormat.indent() ) {
            // assume default not to bloat the html too much
        } else {
            if ( !attributesEmitted ) {
                html += style;
                attributesEmitted = true;
            }
            html += QLatin1String( " -qt-block-indent:" );
            html += QString::number( format.indent() );
            html += QLatin1Char( ';' );
        }
    }

    if ( format.hasProperty( QTextBlockFormat::TextIndent ) ) {
//  if (format.textIndent() == 0) {
        if ( format.textIndent() == mDefaultBlockFormat.textIndent() ) {
            // assume default not to bloat the html too much
        } else {
            if ( !attributesEmitted ) {
                html += style;
                attributesEmitted = true;
            }
            html += QLatin1String( " text-indent:" );
            html += QString::number( format.textIndent() );
            html += QLatin1String( "px;" );
        }
    }

    //QTextCharFormat diff = formatDifference(defaultCharFormat, block.charFormat()).toCharFormat();
    //if (!diff.properties().isEmpty())
    //emitCharFormatStyle(diff);

    if ( attributesEmitted ) {
        html += QLatin1Char( '"' );
    }

//     QBrush bg = format.background();
//     if (bg != Qt::NoBrush)
//         emitAttribute("bgcolor", bg.color().name());
}

void HtmlExporter::emitBlock( const QTextBlock &block )
{
    // save and later restore, in case we 'change' the default format by
    // emitting block char format information

    // NOTE the bottom line is commented, to use default charFormat, which can be set from outside.
    //QTextCharFormat oldDefaultCharFormat = defaultCharFormat;
    QString blockTag;
    bool isBlockQuote = false;
    const QTextBlockFormat blockFormat = block.blockFormat();

    if ( blockFormat.hasProperty( BilboTextFormat::IsBlockQuote ) && 
         blockFormat.boolProperty( BilboTextFormat::IsBlockQuote ) ) {
        isBlockQuote = true;
    }
    QTextList *list = block.textList();
    if ( list ) {
        if ( list->itemNumber( block ) == 0 ) { // first item? emit <ul> or appropriate
//    qDebug() << "first item" << endl;
            if ( isBlockQuote ) {
                html += QLatin1String( "<blockquote>" );
            }
            const QTextListFormat format = list->format();
            const int style = format.style();
            switch ( style ) {
                case QTextListFormat::ListDecimal:
                    html += QLatin1String( "<ol" );
                    break;
                case QTextListFormat::ListDisc:
                    html += QLatin1String( "<ul" );
                    break;
                case QTextListFormat::ListCircle:
                    html += QLatin1String( "<ul type=\"circle\"" );
                    break;
                case QTextListFormat::ListSquare:
                    html += QLatin1String( "<ul type=\"square\"" );
                    break;
                case QTextListFormat::ListLowerAlpha:
                    html += QLatin1String( "<ol type=\"a\"" );
                    break;
                case QTextListFormat::ListUpperAlpha:
                    html += QLatin1String( "<ol type=\"A\"" );
                    break;
                default:
                    html += QLatin1String( "<ul" ); // ### should not happen
                    //qDebug() << html;
            }
            /*
            if (format.hasProperty(QTextFormat::ListIndent)) {
                html += QLatin1String(" style=\"-qt-list-indent: ");
                html += QString::number(format.indent());
                html += QLatin1String(";\"");
            }*/

            html += QLatin1Char( '>' );
        }
        blockTag = QLatin1String( "li" );
//         html += QLatin1String( "<li " );
    }

//     const QTextBlockFormat blockFormat = block.blockFormat();
    if ( blockFormat.hasProperty( QTextFormat::BlockTrailingHorizontalRulerWidth ) ) { 
        if ( ( blockFormat.hasProperty( BilboTextFormat::IsHtmlTagSign ) ) && 
            ( blockFormat.boolProperty( BilboTextFormat::IsHtmlTagSign ) ) ) {
            html += QLatin1String( "<!--split-->" );
            return;
        } else {
            html += QLatin1String( "<hr" );
    
            QTextLength width = blockFormat.lengthProperty( QTextFormat::BlockTrailingHorizontalRulerWidth );
            if ( width.type() != QTextLength::VariableLength ) {
                emitTextLength( "width", width );
            } else {
                html += QLatin1Char( ' ' );
            }
    
            html += QLatin1String( "/>" );
            return;
        }
    }

    const bool pre = blockFormat.nonBreakableLines();
    if ( pre ) {
//   qDebug() << "NonBreakable lines" << endl;
//         if (list) {
//             html += QLatin1Char('>');
//   }
//         html += QLatin1String( "<pre" );
//         emitBlockAttributes( block );
//         html += QLatin1Char( '>' );
        blockTag = QLatin1String( "pre" );

    } else {
        if (!list) {
            if ( isBlockQuote ) {
                html += QLatin1String( "<blockquote>" );
            }

            if ( ( blockFormat.hasProperty( BilboTextFormat::HtmlHeading ) ) && (
                blockFormat.intProperty( BilboTextFormat::HtmlHeading ) ) ) {
                const int index = blockFormat.intProperty( BilboTextFormat::HtmlHeading );
                blockTag = QLatin1Char( 'h' ) + QString::number( index );
            } else {
                //html += QLatin1String("<div");
//                 html += QLatin1String( "<p" );
                blockTag = QLatin1String( "p" );
            }
        }
    }
    if ( !blockTag.isEmpty() ) {
        html += QLatin1Char( '<' ) + blockTag;
        emitBlockAttributes( block );
        html += QLatin1Char( '>' );
    }

    QTextBlock::Iterator it = block.begin();

    for ( ; !it.atEnd(); ++it ) {
        emitFragment( it.fragment(), blockFormat );
    }

    if ( !blockTag.isEmpty() ) {
        html += QLatin1String( "</" ) + blockTag + QLatin1String( ">\n" );
    }

//     if ( pre ) {
//         html += QLatin1String( "</pre>\n" );
//     } else {
//         if ( list ) {
//             html += QLatin1String( "</li>\n" );
//         } else {
//             if ( blockFormat::boolProperty( BilboTextFormat::IsHtmlHeading ) ) {
//                 const int index = format.intProperty( QTextFormat::FontSizeAdjustment );
//                 switch ( index ) {
//                     case -2:
//                        html += QLatin1String( "</h6>" );
//                        break;
//                     case -1:
//                        html += QLatin1String( "</h5>" );
//                        break;
//                     case 0:
//                        html += QLatin1String( "</h4>" );
//                        break;
//                     case 1:
//                        html += QLatin1String( "</h3>" );
//                        break;
//                     case 2:
//                        html += QLatin1String( "<h2" );
//                        break;
//                     case 3:
//                        html += QLatin1String( "<h1" );
//                        break;
//                 }
//             } else {
//                 html += QLatin1String( "</p>\n" );
//             }
//         }
//     }
    // HACK html.replace( QRegExp("<br[\\s]*/>[\\n]*<br[\\s]*/>[\\n]*"),"<br />&nbsp;<br />" );

    if ( list ) {
        if ( list->itemNumber( block ) == list->count() - 1 ) { // last item? close list
            if ( isOrderedList( list->format().style() ) ) {
                html += QLatin1String( "</ol>\n" );
            } else {
                html += QLatin1String( "</ul>\n" );
            }
            if ( isBlockQuote ) {
                html += QLatin1String( "</blockquote>\n" );
            }
        }
    } else {
        if ( isBlockQuote ) {
            html += QLatin1String( "</blockquote>\n" );
        }
    }
    // NOTE the bottom line is commented, to use default charFormat, which can be set from outside.
    //defaultCharFormat = oldDefaultCharFormat;
}

QTextFormat HtmlExporter::formatDifference( const QTextFormat &from, const QTextFormat &to )
{
    //kDebug();

    QTextFormat diff = to;

    const QMap<int, QVariant> props = to.properties();
    for ( QMap<int, QVariant>::ConstIterator it = props.begin(), end = props.end();
            it != end; ++it )
        if ( it.value() == from.property( it.key() ) )
            diff.clearProperty( it.key() );

    return diff;
}

bool HtmlExporter::isOrderedList( int style )
{
//     kDebug() << "html" << html;
    return style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha
           || style == QTextListFormat::ListUpperAlpha;
}

void HtmlExporter::sanitizeHtml()
{
//     kDebug() << "BEFORE" << html;

    int length = 0;
    while ( html.count() != length ) {
        length = html.count();
        html.remove( QRegExp( "</[^<]+><[^<|/]+[^/]>" ) );
    }
//     kDebug() << "AFTER" << html;

}

void HtmlExporter::setDefaultCharFormat( QTextCharFormat charFormat )
{
    mDefaultCharFormat = charFormat;
}

void HtmlExporter::setDefaultBlockFormat( QTextBlockFormat blockFormat )
{
    mDefaultBlockFormat = blockFormat;
}

