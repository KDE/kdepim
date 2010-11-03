/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "htmlexporter.h", which was part
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
