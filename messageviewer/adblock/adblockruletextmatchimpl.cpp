/* ============================================================
*
* This file is a part of the rekonq project
* Copyright (c) 2013, 2014 Montel Laurent <montel.org>
* based on code from rekonq
* Copyright (C) 2010-2011 by Benjamin Poulain <ikipou at gmail dot com>
* Copyright (C) 2010-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "adblockruletextmatchimpl.h"

// Qt Includes
#include <QNetworkRequest>

using namespace MessageViewer;
AdBlockRuleTextMatchImpl::AdBlockRuleTextMatchImpl(const QString &filter)
    : AdBlockRuleImpl(filter)
{
    Q_ASSERT(AdBlockRuleTextMatchImpl::isTextMatchFilter(filter));

    m_textToMatch = filter.toLower();
    m_textToMatch.remove(QLatin1Char('*'));
}


bool AdBlockRuleTextMatchImpl::match(const QNetworkRequest &request, const QString &encodedUrl, const QString &encodedUrlLowerCase) const
{
    Q_UNUSED(request);
    Q_UNUSED(encodedUrl);
    if (m_textToMatch.isEmpty())
        return false;
    
    // Case sensitive compare is faster, but would be incorrect with encodedUrl since
    // we do want case insensitive.
    // What we do is work on a lowercase version of m_textToMatch, and compare to the lowercase
    // version of encodedUrl.
    return encodedUrlLowerCase.contains(m_textToMatch, Qt::CaseSensitive);
}


bool AdBlockRuleTextMatchImpl::isTextMatchFilter(const QString &filter)
{
    // We don't deal with options just yet
    if (filter.contains(QLatin1Char('$')))
        return false;

    // We don't deal with element matching
    if (filter.contains(QLatin1String("##")))
        return false;

    // We don't deal with the begin-end matching
    if (filter.startsWith(QLatin1Char('|')) || filter.endsWith(QLatin1Char('|')))
        return false;

    // We only handle * at the beginning or the end
    int starPosition = filter.indexOf(QLatin1Char('*'));
    while (starPosition >= 0)
    {
        if (starPosition != 0 && starPosition != (filter.length() - 1))
            return false;
        starPosition = filter.indexOf(QLatin1Char('*'), starPosition + 1);
    }
    return true;
}


QString AdBlockRuleTextMatchImpl::ruleString() const
{
    return m_textToMatch;
}


QString AdBlockRuleTextMatchImpl::ruleType() const
{
    return QLatin1String("AdBlockRuleTextMatchImpl");
}
