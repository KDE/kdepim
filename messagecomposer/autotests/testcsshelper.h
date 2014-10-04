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

#ifndef TEST_CSS_HELPER_H
#define TEST_CSS_HELPER_H

#include <messageviewer/viewer/csshelper.h>

// Objecttreeparser needs a valid css helper othewise it crashes
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

#endif
