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

#include <messageviewer/interfaces/htmlwriter.h>

// Objecttreeparser needs a valid html writer othewise it doesn't parse
// inline messages, so give it one to chew on.
class TestHtmlWriter : public MessageViewer::HtmlWriter {
  public:
    explicit TestHtmlWriter() {}
    virtual ~TestHtmlWriter() {}


    virtual void begin( const QString & ) {}
    virtual void write( const QString & ) {}
    virtual void end() {}
    virtual void reset() {}
    virtual void queue( const QString & ) {}
    virtual void flush() {}
    virtual void embedPart( const QByteArray &, const QString & ) {}
    virtual void extraHead( const QString& ) {}
    
};

#endif
