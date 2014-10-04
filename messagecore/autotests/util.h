/*
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

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

#ifndef MESSAGECORE_TESTS_UTIL_H
#define MESSAGECORE_TESTS_UTIL_H

#include <gpgme++/key.h>
#include <messageviewer/viewer/objecttreeemptysource.h>

namespace MessageCore
{

namespace Test
{

/**
* setup a environment variables for tests:
* * set LC_ALL to C
* * set KDEHOME
*/
void setupEnv();

/**
* Returns list of keys used in various crypto routines
*/
std::vector<GpgME::Key> getKeys(bool smime = false);

// We can't use EmptySource, since that doesn't provide a HTML writer. Therefore, derive
// from EmptySource so we can provide our own HTML writer.
// This is only needed because ObjectTreeParser has a bug and doesn't decrypt inline PGP messages
// when there is no HTML writer, see FIXME comment in ObjectTreeParser::writeBodyString().
class TestObjectTreeSource : public MessageViewer::EmptySource
{
public:
    TestObjectTreeSource(MessageViewer::HtmlWriter *writer,
                         MessageViewer::CSSHelper *cssHelper)
        : mWriter(writer), mCSSHelper(cssHelper)
    {
    }

    virtual MessageViewer::HtmlWriter *htmlWriter()
    {
        return mWriter;
    }
    virtual MessageViewer::CSSHelper *cssHelper()
    {
        return mCSSHelper;
    }

private:
    MessageViewer::HtmlWriter *mWriter;
    MessageViewer::CSSHelper *mCSSHelper;
};

}

}

#endif
