/* ============================================================
*
* This file is a part of the rekonq project
* Copyright (c) 2013, 2014 Montel Laurent <montel.org>
* based on code from rekonq
* Copyright (C) 2010-2011 by Benjamin Poulain <ikipou at gmail dot com>
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */

// Self Includes
#include "adblockhostmatcher.h"


using namespace MessageViewer;
bool AdBlockHostMatcher::tryAddFilter(const QString &filter)
{
    if (filter.startsWith(QLatin1String("||")))
    {
        QString domain = filter.mid(2);

        if (!domain.endsWith(QLatin1Char('^')))
            return false;

        if (domain.contains(QLatin1Char('$')))
            return false;

        domain = domain.left(domain.size() - 1);

        if (domain.contains(QLatin1Char('/')) || domain.contains(QLatin1Char('*')) || domain.contains(QLatin1Char('^')))
            return false;

        domain = domain.toLower();
        m_hostList.insert(domain);
        return true;
    }

    if (filter.startsWith(QLatin1String("@@")))
    {
        QString domain = filter.mid(2);

        if (domain.contains(QLatin1Char('^')))
            return false;

        if (domain.contains(QLatin1Char('$')))
            return false;

        if (domain.contains(QLatin1Char('*')))
            return false;

        if (domain.contains(QLatin1Char('|')))
            return false;

        if (domain.contains(QLatin1Char('/')))
        {
            if (!domain.endsWith(QLatin1Char('/')))
                return false;
        }
        domain = domain.toLower();
        m_hostList.insert(domain);
        return true;
    }

    return false;
}

bool AdBlockHostMatcher::match(const QString &host) const
{
    return m_hostList.contains(host.toLower());
}

void AdBlockHostMatcher::clear()
{
    m_hostList.clear();
}
