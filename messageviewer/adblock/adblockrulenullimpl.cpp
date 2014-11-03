/* ============================================================
*
* This file is a part of the rekonq project
* Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>
* based on code from rekonq
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "adblockrulenullimpl.h"

// Qt Includes
#include <QStringList>

using namespace MessageViewer;
AdBlockRuleNullImpl::AdBlockRuleNullImpl(const QString &filter)
    : AdBlockRuleImpl(filter)
{
}

bool AdBlockRuleNullImpl::match(const QNetworkRequest &, const QString &, const QString &) const
{
    return false;
}

bool AdBlockRuleNullImpl::isNullFilter(const QString &filter)
{
    QString parsedLine = filter;

    const int optionsNumber = parsedLine.lastIndexOf(QLatin1Char('$'));
    if (optionsNumber == 0) {
        return false;
    }

    const QStringList options(parsedLine.mid(optionsNumber + 1).split(QLatin1Char(',')));

    Q_FOREACH(const QString & option, options) {
        // NOTE:
        // I moved the check from option == QLatin1String to option.endsWith()
        // to check option && ~option. Hope it will NOT be a problem...

        // third_party: managed inside adblockrulefallbackimpl
        if (option.endsWith(QLatin1String("third-party"))) {
            return false;
        }

        // script
        if (option.endsWith(QLatin1String("script"))) {
            return true;
        }

        // image
        if (option.endsWith(QLatin1String("image"))) {
            return true;
        }

        // background
        if (option.endsWith(QLatin1String("background"))) {
            return true;
        }

        // stylesheet
        if (option.endsWith(QLatin1String("stylesheet"))) {
            return true;
        }

        // object
        if (option.endsWith(QLatin1String("object"))) {
            return true;
        }

        // xbl
        if (option.endsWith(QLatin1String("xbl"))) {
            return true;
        }

        // ping
        if (option.endsWith(QLatin1String("ping"))) {
            return true;
        }

        // xmlhttprequest
        if (option.endsWith(QLatin1String("xmlhttprequest"))) {
            return true;
        }

        // object_subrequest
        if (option.endsWith(QLatin1String("object-subrequest"))) {
            return true;
        }

        // dtd
        if (option.endsWith(QLatin1String("dtd"))) {
            return true;
        }

        // subdocument
        if (option.endsWith(QLatin1String("subdocument"))) {
            return true;
        }

        // document
        if (option.endsWith(QLatin1String("document"))) {
            return true;
        }

        // other
        if (option.endsWith(QLatin1String("other"))) {
            return true;
        }

        // collapse
        if (option.endsWith(QLatin1String("collapse"))) {
            return true;
        }
    }

    return false;
}

QString AdBlockRuleNullImpl::ruleString() const
{
    return QString();
}

QString AdBlockRuleNullImpl::ruleType() const
{
    return QLatin1String("AdBlockRuleNullImpl");
}
