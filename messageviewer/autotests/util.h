/*
  Copyright (c) 2010 Thomas McGuire <thomas.mcguire@kdab.com>

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
#include "interfaces/htmlwriter.h"
#include "viewer/csshelper.h"

#include <KMime/Message>

class TestHtmlWriter : public MessageViewer::HtmlWriter
{
public:
    explicit TestHtmlWriter() {}
    virtual ~TestHtmlWriter() {}

    virtual void begin(const QString &) {}
    virtual void write(const QString &) {}
    virtual void end() {}
    virtual void reset() {}
    virtual void queue(const QString &) {}
    virtual void flush() {}
    virtual void embedPart(const QByteArray &, const QString &) {}
    virtual void extraHead(const QString &) {}
};

class TestCSSHelper : public MessageViewer::CSSHelper
{
public:
    TestCSSHelper() : MessageViewer::CSSHelper(0) {}
    virtual ~TestCSSHelper() {}

    QString nonQuotedFontTag() const
    {
        return QString::fromLatin1("<");
    }

    QString quoteFontTag(int) const
    {
        return QString::fromLatin1("<");
    }
};

KMime::Message::Ptr readAndParseMail(const QString &mailFile);
