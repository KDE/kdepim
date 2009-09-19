/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   It is a modified version of "qtextdocumentfragment.cpp", which is     *
 *   part of the QtGui module of the Qt Toolkit. it has been modified for  *
 *   use in Bilbo Blogger, at December 2008.                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "bilbotexthtmlimporter.h"
#include "qtextlist.h"
#include "kdebug.h"
#include "bilbotextformat.h"
// #include <QTextLength>

BilboTextHtmlImporter::BilboTextHtmlImporter( QTextDocument *_doc, const QString &_html, const QTextDocument *resourceProvider )
        : indent( 0 ), setNamedAnchorInNextOutput( false ), doc( _doc ), containsCompleteDoc( false )
{
    cursor = QTextCursor( doc );

    QString html = _html;
    const int startFragmentPos = html.indexOf( QLatin1String( "<!--StartFragment-->" ) );
    if ( startFragmentPos != -1 ) {
        const int endFragmentPos = html.indexOf( QLatin1String( "<!--EndFragment-->" ) );
        if ( startFragmentPos < endFragmentPos ) {
            html = html.mid( startFragmentPos, endFragmentPos - startFragmentPos );
        } else {
            html = html.mid( startFragmentPos );
        }

        html.prepend( QLatin1String( "<meta name=\"qrichtext\" content=\"1\" />" ) );
    }

    parse( html, resourceProvider ? resourceProvider : doc );
//    dumpHtml();
}

void BilboTextHtmlImporter::import()
{
//     kDebug();
    cursor.beginEditBlock();
    bool hasBlock = true;
    bool forceBlockMerging = false;
    for ( int i = 0; i < count(); ++i ) {
        const BilboTextHtmlParserNode *node = &at( i );
        /*
         * process each node in three stages:
        * 1) check if the hierarchy changed and we therefore passed the
        *    equivalent of a closing tag -> we may need to finish off
        *    some structures like tables
        *
        * 2) check if the current node is a special node like a
        *    <table>, <ul> or <img> tag that requires special processing
        *
        * 3) if the node should result in a QTextBlock create one and
        *    finally insert text that may be attached to the node
        */

        /* emit 'closing' table blocks or adjust current indent level
        * if we
        *  1) are beyond the first node
        *  2) the current node not being a child of the previous node
        *      means there was a tag closing in the input html
        */
        if ( i > 0 && ( node->parent != i - 1 ) ) {
            const bool blockTagClosed = closeTag( i );
            if ( hasBlock && blockTagClosed ) {
                hasBlock = false;
            }

            // make sure there's a block for 'Blah' after <ul><li>foo</ul>Blah
            if ( blockTagClosed && !hasBlock && !node->isBlock && !node->text.isEmpty() && node->displayMode != BilboTextHtmlElement::DisplayNone ) {

                QTextBlockFormat block = node->blockFormat();
                block.setIndent( indent );

                appendBlock( block, node->charFormat() );

                hasBlock = true;
            }
        }

        if ( node->displayMode == BilboTextHtmlElement::DisplayNone ) {
            if ( node->id == Html_title ) {
                doc->setMetaInformation( QTextDocument::DocumentTitle, node->text );
            }
            // ignore explicitly 'invisible' elements
            continue;
        } else if ( node->id == Html_body ) {
            containsCompleteDoc = true;
            if ( node->background.style() != Qt::NoBrush ) {
                QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
                fmt.setBackground( node->background );
                doc->rootFrame()->setFrameFormat( fmt );
                const_cast<BilboTextHtmlParserNode *>( node )->background = QBrush();
            }
        } else if ( node->isListStart ) {

            QTextListFormat::Style style = node->listStyle;

            if ( node->id == Html_ul && !node->hasOwnListStyle && node->parent ) {
                const BilboTextHtmlParserNode *n = &at( node->parent );
                while ( n ) {
                    if ( n->id == Html_ul ) {
/// me       style = nextListStyle(node->listStyle);
                        /// my code instead of above:
                        if ( node->listStyle == QTextListFormat::ListDisc ) {
                            style = QTextListFormat::ListCircle;
                        } else if ( node->listStyle == QTextListFormat::ListCircle ) {
                            style = QTextListFormat::ListSquare;
                        } else {
                            style = node->listStyle;
                        }
                        ///
                    }
                    if ( n->parent )
                        n = &at( n->parent );
                    else
                        n = 0;
                }
            }

            QTextListFormat listFmt;
            listFmt.setStyle( style );

            ++indent;
            if ( node->hasCssListIndent ) {
                listFmt.setIndent( node->cssListIndent );
            } else {
                listFmt.setIndent( indent );
            }

            List l;
            l.format = listFmt;
            lists.append( l );

            if ( node->text.isEmpty() ) {
                continue;
            }
        } else if ( node->id == Html_table ) {
            Table t = scanTable( i );
            tables.append( t );
            hasBlock = false;
            continue;
        } else if ( node->id == Html_tr && !tables.isEmpty() ) {
            continue;
        } else if ( node->id == Html_img ) {
            QTextImageFormat fmt;
            fmt.setName( node->imageName );

            QTextCharFormat nodeFmt = node->charFormat();
            if ( nodeFmt.hasProperty( QTextFormat::IsAnchor ) )
                fmt.setAnchor( nodeFmt.isAnchor() );
            if ( nodeFmt.hasProperty( QTextFormat::AnchorHref ) )
                fmt.setAnchorHref( nodeFmt.anchorHref() );
            if ( nodeFmt.hasProperty( QTextFormat::AnchorName ) )
                fmt.setAnchorName( nodeFmt.anchorName() );
            /// my code
            if ( nodeFmt.hasProperty( BilboTextFormat::AnchorTitle ) )
                fmt.setProperty( BilboTextFormat::AnchorTitle, nodeFmt.property( BilboTextFormat::AnchorTitle ) );
            if ( nodeFmt.hasProperty( BilboTextFormat::AnchorTarget ) )
                fmt.setProperty( BilboTextFormat::AnchorTarget, nodeFmt.property( BilboTextFormat::AnchorTarget ) );

            if ( !node->imageTitle.isEmpty() )
                fmt.setProperty( BilboTextFormat::ImageTitle, QVariant( node->imageTitle ) );
            if ( !node->imageAlternateText.isEmpty() )
                fmt.setProperty( BilboTextFormat::ImageAlternateText,
                                 QVariant( node->imageAlternateText ) );
            if ( node->imageWidth >= 0 )
                fmt.setWidth( node->imageWidth );
            if ( node->imageHeight >= 0 )
                fmt.setHeight( node->imageHeight );

            cursor.insertImage( fmt, QTextFrameFormat::Position( node->cssFloat ) );
            hasBlock = false;
            continue;
        } else if ( node->id == Html_hr ) {
            QTextBlockFormat blockFormat;
            blockFormat.setProperty( QTextFormat::BlockTrailingHorizontalRulerWidth, node->width );
            appendBlock( blockFormat );
            hasBlock = false;
            continue;
        } else if ( node->id == Html_comment_split ) {  ///my code

            QTextBlockFormat blockFormat;
            blockFormat.setProperty( BilboTextFormat::IsHtmlTagSign, true );
            blockFormat.setProperty( QTextFormat::BlockTrailingHorizontalRulerWidth, 
             QTextLength( QTextLength::PercentageLength, 80 ) );

            appendBlock( blockFormat );
            hasBlock = false;
            continue;   ///my code
        }

        if ( node->isBlock ) {
            QTextBlockFormat block;
            QTextCharFormat charFmt;

            if ( node->isTableCell && !tables.isEmpty() ) {
                Table &t = tables.last();
                if ( !t.isTextFrame ) {
                    const QTextTableCell cell = t.currentCell.cell();
                    if ( cell.isValid() )
                        cursor.setPosition( cell.firstPosition() );
                }
                hasBlock = true;

                if ( node->background.style() != Qt::NoBrush ) {
                    charFmt.setBackground( node->background );
                    cursor.mergeBlockCharFormat( charFmt );
                }
            }

            if ( hasBlock ) {
                block = cursor.blockFormat();
                charFmt = cursor.blockCharFormat();
            }

            // collapse
            block.setTopMargin( qMax( block.topMargin(), ( qreal )topMargin( i ) ) );

            int bottomMargin = this->bottomMargin( i );

            // for list items we may want to collapse with the bottom margin of the
            // list.
            if ( node->isListItem ) {
                if ( node->parent && at( node->parent ).isListStart ) {
                    const int listId = node->parent;
                    const BilboTextHtmlParserNode *list = &at( listId );
                    if ( list->children.last() == i /* == index of node */ )
                        bottomMargin = qMax( bottomMargin, this->bottomMargin( listId ) );
                }
            }

            block.setBottomMargin( bottomMargin );

            block.setLeftMargin( leftMargin( i ) );
            block.setRightMargin( rightMargin( i ) );

            if ( !node->isListItem && indent != 0 && ( lists.isEmpty() || !hasBlock || !lists.last().list || lists.last().list->itemNumber( cursor.block() ) == -1 ) ) {
                block.setIndent( indent );
            }

            block.merge( node->blockFormat() );
            charFmt.merge( node->charFormat() );

            // ####################
//                block.setFloatPosition(node->cssFloat);

            if ( node->wsm == BilboTextHtmlParserNode::WhiteSpacePre )
                block.setNonBreakableLines( true );

            if ( node->background.style() != Qt::NoBrush && !node->isTableCell )
                block.setBackground( node->background );

            if ( hasBlock && ( !node->isEmptyParagraph || forceBlockMerging ) ) {
                if ( cursor.position() == 0 ) {
                    containsCompleteDoc = true;
                }
                cursor.setBlockFormat( block );
                cursor.setBlockCharFormat( charFmt );
            } else {
                if ( i == 1 && cursor.position() == 0 && node->isEmptyParagraph ) {
                    containsCompleteDoc = true;
                    //kDebug() << "contains complete doc";
                    cursor.setBlockFormat( block );
                    cursor.setBlockCharFormat( charFmt );
                } else {
                    appendBlock( block, charFmt );
                }
            }

            if ( node->isListItem && !lists.isEmpty() ) {
                List &l = lists.last();
                if ( l.list ) {
                    l.list->add( cursor.block() );
                } else {
                    l.list = cursor.createList( l.format );
                }
            }

            forceBlockMerging = false;
            if ( node->id == Html_body || node->id == Html_html )
                forceBlockMerging = true;

            if ( node->isEmptyParagraph ) {
                hasBlock = false;
                continue;
            }

            hasBlock = true;
        }

        if ( node->isAnchor && !node->anchorName.isEmpty() ) {
            setNamedAnchorInNextOutput = true;
            namedAnchor = node->anchorName;
        }
        if ( node->text.isEmpty() )
            continue;
        hasBlock = false;

        QTextCharFormat format = node->charFormat();
        QString text = node->text;
        if ( setNamedAnchorInNextOutput ) {
            QTextCharFormat fmt = format;
            fmt.setAnchor( true );
            fmt.setAnchorName( namedAnchor );
            cursor.insertText( QString( text.at( 0 ) ), fmt );

            text.remove( 0, 1 );
            format.setAnchor( false );
            format.setAnchorName( QString() );

            setNamedAnchorInNextOutput = false;
        }

        int textStart = 0;
        for ( int i = 0; i < text.length(); ++i ) {
            QChar ch = text.at( i );

            const int textEnd = i;

            if ( ch == QLatin1Char( '\n' ) || ch == QChar::ParagraphSeparator ) {

                if ( textEnd > textStart )
                    cursor.insertText( text.mid( textStart, textEnd - textStart ), format );

                textStart = i + 1;

                QTextBlockFormat fmt = cursor.blockFormat();

                if ( fmt.hasProperty( QTextFormat::BlockBottomMargin ) ) {
                    QTextBlockFormat tmp = fmt;
                    tmp.clearProperty( QTextFormat::BlockBottomMargin );
                    cursor.setBlockFormat( tmp );
                }

                fmt.clearProperty( QTextFormat::BlockTopMargin );
                cursor.insertBlock( fmt );
            }
        }
        if ( textStart < text.length() )
            cursor.insertText( text.mid( textStart, text.length() - textStart ), format );
    }

    cursor.endEditBlock();
}

// returns true if a block tag was closed
bool BilboTextHtmlImporter::closeTag( int i )
{
//     kDebug();
    const BilboTextHtmlParserNode *closedNode = &at( i - 1 );
    const int endDepth = depth( i ) - 1;
    int depth = this->depth( i - 1 );
    bool blockTagClosed = false;

    while ( depth > endDepth ) {
        if ( closedNode->id == Html_tr && !tables.isEmpty() ) {
            Table &t = tables.last();

            if ( !t.isTextFrame ) {
                ++t.currentRow;

                // for broken html with rowspans but missing tr tags
                while ( !t.currentCell.atEnd() && t.currentCell.row < t.currentRow )
                    ++t.currentCell;
            }

            blockTagClosed = true;
        } else if ( closedNode->id == Html_table && !tables.isEmpty() ) {
            tables.resize( tables.size() - 1 );

            if ( tables.isEmpty() ) {
                cursor = doc->rootFrame()->lastCursorPosition();
            } else {
                Table &t = tables.last();
                if ( t.isTextFrame )
                    cursor = t.frame->lastCursorPosition();
                else if ( !t.currentCell.atEnd() )
                    cursor = t.currentCell.cell().lastCursorPosition();
            }

            // we don't need an extra block after tables, so we don't
            // claim to have closed one for the creation of a new one
            // in import()
            blockTagClosed = false;
        } else if ( closedNode->isTableCell && !tables.isEmpty() ) {
            Table &t = tables.last();
            if ( !t.isTextFrame )
                ++tables.last().currentCell;
            blockTagClosed = true;
        } else if ( closedNode->isListStart && !lists.isEmpty() ) {
            lists.resize( lists.size() - 1 );
            --indent;
            blockTagClosed = true;
        } else if ( closedNode->id == Html_hr || closedNode->id == Html_center || closedNode->id == Html_h1 || closedNode->id == Html_h2 || closedNode->id == Html_h3 || closedNode->id == Html_h4 || closedNode->id == Html_h5 || closedNode->id == Html_h6 ) {
            blockTagClosed = true;
        } else if ( closedNode->id == Html_p || closedNode->id == Html_pre ) {
            // blockTagClosed may result in the creation of a
            // new block
            if ( !closedNode->text.isEmpty() )
                blockTagClosed = true;
        }

        closedNode = &at( closedNode->parent );
        --depth;
    }

    return blockTagClosed;
}

BilboTextHtmlImporter::Table BilboTextHtmlImporter::scanTable( int tableNodeIdx )
{
    Table table;
    table.columns = 0;

    QVector<QTextLength> columnWidths;
    QVector<int> rowSpanCellsPerRow;

    int tableHeaderRowCount = 0;
    QVector<int> rowNodes;
    rowNodes.reserve( at( tableNodeIdx ).children.count() );
    foreach( int row, at( tableNodeIdx ).children )
    switch ( at( row ).id ) {
        case Html_tr:
            rowNodes += row;
            break;
        case Html_thead:
        case Html_tbody:
        case Html_tfoot:
            foreach( int potentialRow, at( row ).children )
            if ( at( potentialRow ).id == Html_tr ) {
                rowNodes += potentialRow;
                if ( at( row ).id == Html_thead )
                    ++tableHeaderRowCount;
            }
            break;
        default:
            break;
    }

    int effectiveRow = 0;
    foreach( int row, rowNodes ) {
        int colsInRow = 0;

        foreach( int cell, at( row ).children ) {

            if ( at( cell ).isTableCell ) {
                const BilboTextHtmlParserNode &c = at( cell );
                const int currentColumn = colsInRow;
                colsInRow += c.tableCellColSpan;

                if ( c.tableCellRowSpan > 1 ) {
                    rowSpanCellsPerRow.resize( effectiveRow + c.tableCellRowSpan + 1 );
                    for ( int r = effectiveRow + 1; r < effectiveRow + c.tableCellRowSpan; ++r ) {
                        rowSpanCellsPerRow[r]++;
                    }
                }

                columnWidths.resize( qMax( columnWidths.count(), colsInRow ) );
                for ( int i = currentColumn; i < currentColumn + c.tableCellColSpan; ++i ) {
                    if ( columnWidths.at( i ).type() == QTextLength::VariableLength )  {
                        columnWidths[i] = c.width;
                    }
                }
            }
        }

        table.columns = qMax( table.columns, colsInRow + rowSpanCellsPerRow.value( effectiveRow, 0 ) );

        ++effectiveRow;
        rowSpanCellsPerRow.append( 0 );
    }
    table.rows = effectiveRow;

    if ( table.rows == 0 || table.columns == 0 )
        return table;

    QTextFrameFormat fmt;
    const BilboTextHtmlParserNode &node = at( tableNodeIdx );
    if ( node.isTextFrame ) {
        // for plain text frames we set the frame margin
        // for all of top/bottom/left/right, so in the import
        // here it doesn't matter which one we pick
        fmt.setMargin( node.uncollapsedMargin( BilboTextHtmlParser::MarginTop ) );
    } else {
        QTextTableFormat tableFmt;
        tableFmt.setCellSpacing( node.tableCellSpacing );
        tableFmt.setCellPadding( node.tableCellPadding );
        if ( node.alignment )
            tableFmt.setAlignment( node.alignment );
        tableFmt.setColumns( table.columns );
        tableFmt.setColumnWidthConstraints( columnWidths );
        tableFmt.setHeaderRowCount( tableHeaderRowCount );
        fmt = tableFmt;
    }

    fmt.setBorder( node.tableBorder );
    fmt.setWidth( node.width );
    fmt.setHeight( node.height );
    if ( node.pageBreakPolicy != QTextFormat::PageBreak_Auto )
        fmt.setPageBreakPolicy( node.pageBreakPolicy );

    if ( node.direction < 2 )
        fmt.setLayoutDirection( Qt::LayoutDirection( node.direction ) );
    if ( node.background.style() != Qt::NoBrush )
        fmt.setBackground( node.background );
    else
        fmt.clearBackground();
    fmt.setPosition( QTextFrameFormat::Position( node.cssFloat ) );

    if ( node.isTextFrame ) {
        table.frame = cursor.insertFrame( fmt );
        table.isTextFrame = true;
    } else {
        QTextTable *textTable = cursor.insertTable( table.rows, table.columns, fmt.toTableFormat() );
        table.frame = textTable;

        TableCellIterator it( textTable );
        foreach( int row, rowNodes )
        foreach( int cell, at( row ).children )
        if ( at( cell ).isTableCell ) {
            const BilboTextHtmlParserNode &c = at( cell );

            if ( c.tableCellColSpan > 1 || c.tableCellRowSpan > 1 )
                textTable->mergeCells( it.row, it.column, c.tableCellRowSpan, c.tableCellColSpan );

            ++it;
        }

        table.currentCell = TableCellIterator( textTable );
    }
    return table;
}

void BilboTextHtmlImporter::appendBlock( const QTextBlockFormat &format, QTextCharFormat charFmt )
{
//     kDebug();
    if ( setNamedAnchorInNextOutput ) {
        charFmt.setAnchor( true );
        charFmt.setAnchorName( namedAnchor );
        setNamedAnchorInNextOutput = false;
    }

    cursor.insertBlock( format, charFmt );
}

