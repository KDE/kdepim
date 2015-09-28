/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef CUSTOMHEADERSTRATEGY_H
#define CUSTOMHEADERSTRATEGY_H

#include <messageviewer/headerstrategy.h>
#include <QStringList>

namespace MessageViewer
{
class CustomHeaderStrategy : public HeaderStrategy
{
    friend class HeaderStrategy;
public:
    CustomHeaderStrategy();
    ~CustomHeaderStrategy() {}

public:
    const char *name() const Q_DECL_OVERRIDE
    {
        return "custom";
    }

    void loadConfig();
    QStringList headersToDisplay() const Q_DECL_OVERRIDE
    {
        return mHeadersToDisplay;
    }
    QStringList headersToHide() const Q_DECL_OVERRIDE
    {
        return mHeadersToHide;
    }
    DefaultPolicy defaultPolicy() const Q_DECL_OVERRIDE
    {
        return mDefaultPolicy;
    }

private:
    QStringList mHeadersToDisplay;
    QStringList mHeadersToHide;
    DefaultPolicy mDefaultPolicy;
};
}

#endif // CUSTOMHEADERSTRATEGY_H
