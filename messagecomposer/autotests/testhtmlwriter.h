/*
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
  */

#ifndef TEST_HTML_WRITER_H
#define TEST_HTML_WRITER_H

#include <messageviewer/htmlwriter.h>

// Objecttreeparser needs a valid html writer othewise it doesn't parse
// inline messages, so give it one to chew on.
class TestHtmlWriter : public MessageViewer::HtmlWriter
{
public:
    explicit TestHtmlWriter() {}
    virtual ~TestHtmlWriter() {}

    void begin(const QString &) Q_DECL_OVERRIDE {}
    void write(const QString &) Q_DECL_OVERRIDE {}
    void end() Q_DECL_OVERRIDE {}
    void reset() Q_DECL_OVERRIDE {}
    void queue(const QString &) Q_DECL_OVERRIDE {}
    void flush() Q_DECL_OVERRIDE {}
    void embedPart(const QByteArray &, const QString &) Q_DECL_OVERRIDE {}
    void extraHead(const QString &) Q_DECL_OVERRIDE {}

};

#endif
