/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef PLAINHEADERSTYLE_H
#define PLAINHEADERSTYLE_H

#include "messageviewer/headerstyle.h"
#include "messageviewer_export.h"
namespace MessageViewer
{

class MESSAGEVIEWER_EXPORT PlainHeaderStyle : public HeaderStyle
{
public:
    PlainHeaderStyle();
    ~PlainHeaderStyle() {}

public:
    const char *name() const Q_DECL_OVERRIDE;

    QString format(KMime::Message *message) const Q_DECL_OVERRIDE;

private:
    QString formatAllMessageHeaders(KMime::Message *message) const;
};
}
#endif // PLAINHEADERSTYLE_H
