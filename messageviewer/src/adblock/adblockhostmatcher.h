/* ============================================================
*
* This file is a part of the rekonq project
* Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>
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

#ifndef ADBLOCKHOSTMATCHER_H
#define ADBLOCKHOSTMATCHER_H

#include <QSet>
#include <QString>

namespace MessageViewer
{
class AdBlockHostMatcher
{
public:
    // Try to add an adblock filter to this host matcher.
    // If the filter is not an hostname, the filter is not added
    // and the method return false;
    bool tryAddFilter(const QString &filter);

    bool match(const QString &host) const;

    void clear();

private:
    QSet<QString> m_hostList;
};
}

#endif // ADBLOCKHOSTMATCHER_H
