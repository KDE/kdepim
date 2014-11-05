/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef MOBILEHEADERSTYLE_H
#define MOBILEHEADERSTYLE_H
#include "header/headerstyle.h"

namespace MessageViewer
{
class MobileHeaderStyle : public HeaderStyle
{
    friend class HeaderStyle;
protected:
    MobileHeaderStyle() : HeaderStyle() {}
    ~MobileHeaderStyle() {}

public:
    const char *name() const
    {
        return "mobile";
    }

    QString format(KMime::Message *message) const Q_DECL_OVERRIDE;
};

class MobileExtendedHeaderStyle : public HeaderStyle
{
    friend class HeaderStyle;
protected:
    MobileExtendedHeaderStyle() : HeaderStyle() {}
    ~MobileExtendedHeaderStyle() {}

public:
    const char *name() const
    {
        return "mobileExtended";
    }

    QString format(KMime::Message *message) const Q_DECL_OVERRIDE;
};
}

#endif // MOBILEHEADERSTYLE_H
