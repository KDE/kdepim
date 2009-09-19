/***************************************************************************
 *   This file is a part of Bilbo Blogger.                                 * 
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   It is a modified version of "htmlexporter.h" from                     *
 *   KBlogger project. it has been modified for use in Bilbo Blogger, at   *
 *   November 2008.                                                        *
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

#ifndef HTML_EXPORTER_H
#define HTML_EXPORTER_H

#include <QTextFrame>
#include <QList>

class QTextDocument;
class QTextBlock;
class QString;
class QTextFrameFormat;
class StyleMode;
class QTextTable;
class QTextCharFormat;
class QTextLength;


class HtmlExporter
{
public:
    HtmlExporter();
    ~HtmlExporter();

    QString toHtml( const QTextDocument* document );

    void setDefaultCharFormat( QTextCharFormat charFormat );
    void setDefaultBlockFormat( QTextBlockFormat blockFormat );

private:
    enum StyleMode { EmitStyleTag, OmitStyleTag };
//     enum tag { span = 0, h1, h2, h3, h4, h5, strong, em, s, u, code, sub, sup };
    enum tag { span = 0, strong, em, s, u, code, sub, sup };

    void emitFrame( QTextFrame::Iterator frameIt );
    void emitTable( const QTextTable *table );
    void emitAttribute( const char *attribute, const QString &value );
    QList<tag>  emitCharFormatStyle( const QTextCharFormat &charFormat, 
                                     const QTextBlockFormat &blockFormat );
    void emitTextLength( const char *attribute, const QTextLength &length );
    void emitAlignment( Qt::Alignment align );
    void emitFloatStyle( QTextFrameFormat::Position pos, StyleMode mode = EmitStyleTag );
    void emitMargins( const QString &top, const QString &bottom, const QString &left, const QString &right );
    void emitFragment( const QTextFragment &fragment, const QTextBlockFormat &blockFormat);
    bool isOrderedList( int style );
    void emitBlockAttributes( const QTextBlock &block );
    void emitBlock( const QTextBlock &block );
    QTextFormat formatDifference( const QTextFormat &from, const QTextFormat &to );
    void sanitizeHtml();

    QString html;
    const QTextDocument* doc;
    //QTextCharFormat defaultCharFormat;
    QTextCharFormat mDefaultCharFormat;
    QTextBlockFormat mDefaultBlockFormat;
};

#endif
