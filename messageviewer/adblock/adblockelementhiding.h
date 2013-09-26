/* ============================================================
*
* This file is a part of the rekonq project
* Copyright (c) 2013 Montel Laurent <montel@kde.org>
* based on code from rekonq
* Copyright (C) 2013 by Paul Rohrbach <p.b.r at gmx dot net>
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

#ifndef ADBLOCKELEMENTHIDING_H
#define ADBLOCKELEMENTHIDING_H

#include <QStringList>
#include <QMultiHash>
#include <QWebElement>

namespace MessageViewer {
class AdBlockElementHiding
{
public:
    AdBlockElementHiding();

    bool addRule(const QString &rule);
    void apply(QWebElement &document, const QString &domain) const;

    void clear();

private:
    void applyStringRule(QWebElement &document, const QString &rule) const;
    QStringList generateSubdomainList(const QString &domain) const;

    QStringList m_GenericRules;
    QMultiHash<QString, QString> m_DomainSpecificRules;
    QMultiHash<QString, QString> m_DomainSpecificRulesWhitelist;
};
}

#endif // ADBLOCKELEMENTHIDING_H
