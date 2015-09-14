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

#include "searchruledate.h"

#include <QDateTime>

#include "filter/filterlog.h"
using MailCommon::FilterLog;

#include <KMime/KMimeMessage>
#include <KLocalizedString>
using namespace MailCommon;

SearchRuleDate::SearchRuleDate(const QByteArray &field,
                               Function func,
                               const QString &contents)
    : SearchRule(field, func, contents)
{
}

QString SearchRuleDate::informationAboutNotValidRules() const
{
    return i18n("Date is not valid.");
}

bool SearchRuleDate::isEmpty() const
{
    return !QDate::fromString(contents(), Qt::ISODate).isValid();
}

bool SearchRuleDate::matches(const Akonadi::Item &item) const
{
    const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

    const QDate msgDate = msg->date()->dateTime().date();
    const QDate dateValue = QDate::fromString(contents(), Qt::ISODate);
    bool rc = matchesInternal(dateValue, msgDate);
    if (FilterLog::instance()->isLogging()) {
        QString msg = (rc ? "<font color=#00FF00>1 = </font>"
                       : "<font color=#FF0000>0 = </font>");
        msg += FilterLog::recode(asString());
        msg += " ( <i>" + contents() + "</i> )"; //TODO change with locale?
        FilterLog::instance()->add(msg, FilterLog::RuleResult);
    }
    return rc;
}

bool SearchRuleDate::matchesInternal(const QDate &dateValue,
                                     const QDate &msgDate) const
{
    switch (function()) {
    case SearchRule::FuncEquals:
        return (dateValue == msgDate);

    case SearchRule::FuncNotEqual:
        return (dateValue != msgDate);

    case FuncIsGreater:
        return (msgDate > dateValue);

    case FuncIsLessOrEqual:
        return (msgDate <= dateValue);

    case FuncIsLess:
        return (msgDate < dateValue);

    case FuncIsGreaterOrEqual:
        return (msgDate >= dateValue);

    default:
        ;
    }
    return false;
}

SearchRule::RequiredPart SearchRuleDate::requiredPart() const
{
    return SearchRule::Envelope;
}

void SearchRuleDate::addQueryTerms(Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError) const
{
    using namespace Akonadi;
    emptyIsNotAnError = false;

    const QDate date = QDate::fromString(contents(), Qt::ISODate);
    EmailSearchTerm term(EmailSearchTerm::HeaderOnlyDate, date, akonadiComparator());
    term.setIsNegated(isNegated());
    groupTerm.addSubTerm(term);
}

