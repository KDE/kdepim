/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "qtextdocumentfragment.h", which was part
    of the QtGui module of the Qt Toolkit. It has been customized for
    use in Blogilo, at December 2008.

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

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

#ifndef BILBOTEXTHTMLIMPORTER_H
#define BILBOTEXTHTMLIMPORTER_H

#include "QtGui/qtextdocument.h"
#include "QtGui/qtexttable.h"
//#include "QtCore/qatomic.h"
#include "QtCore/qlist.h"
//#include "QtCore/qmap.h"
#include "QtCore/qpointer.h"

#include "bilbohtmlparser.h"

/**
 *  Parses html text via BilboTextHtmlParser functions, then organizes returned 
 *  node list as a QTextDocument object.
 *
*/
class BilboTextHtmlImporter : public BilboTextHtmlParser
{
    struct Table;
public:
    BilboTextHtmlImporter( QTextDocument *_doc, const QString &html, const QTextDocument *resourceProvider = 0 );

    void import();

    bool containsCompleteDocument() const {
        return containsCompleteDoc;
    }

private:
    bool closeTag( int i );

    Table scanTable( int tableNodeIdx );

    void appendBlock( const QTextBlockFormat &format, QTextCharFormat charFmt = QTextCharFormat() );

    struct List {
        QTextListFormat format;
        QPointer<QTextList> list;
    };
    QVector<List> lists;
    int indent;

    // insert a named anchor the next time we emit a char format,
    // either in a block or in regular text
    bool setNamedAnchorInNextOutput;
    QString namedAnchor;

#ifdef Q_CC_SUN
    friend struct BilboTextHtmlImporter::Table;
#endif
    struct TableCellIterator {
        inline TableCellIterator( QTextTable *t = 0 ) : table( t ), row( 0 ), column( 0 ) {}

        inline TableCellIterator &operator++() {
            do {
                const QTextTableCell cell = table->cellAt( row, column );
                if ( !cell.isValid() )
                    break;
                column += cell.columnSpan();
                if ( column >= table->columns() ) {
                    column = 0;
                    ++row;
                }
            } while ( row < table->rows() && table->cellAt( row, column ).row() != row );

            return *this;
        }

        inline bool atEnd() const {
            return table == 0 || row >= table->rows();
        }

        QTextTableCell cell() const {
            return table->cellAt( row, column );
        }

        QTextTable *table;
        int row;
        int column;
    };

    struct Table {
        Table() : isTextFrame( false ), rows( 0 ), columns( 0 ), currentRow( 0 ) {}
        QPointer<QTextFrame> frame;
        bool isTextFrame;
        int rows;
        int columns;
        int currentRow; // ... for buggy html (see html_skipCell testcase)
        TableCellIterator currentCell;
    };
    QVector<Table> tables;

    QTextDocument *doc;
    QTextCursor cursor;
    bool containsCompleteDoc;
};

#endif
