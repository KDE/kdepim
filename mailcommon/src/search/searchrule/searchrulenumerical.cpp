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

#include "searchrulenumerical.h"

#include "filter/filterlog.h"
using MailCommon::FilterLog;

#include <QDateTime>
#include <KMime/KMimeMessage>

#include <QDataStream>
#include <QRegExp>
#include <QXmlStreamWriter>

#include <algorithm>

using namespace MailCommon;

SearchRuleNumerical::SearchRuleNumerical(const QByteArray &field,
        Function func,
        const QString &contents)
    : SearchRule(field, func, contents)
{
}

bool SearchRuleNumerical::isEmpty() const
{
    bool ok = false;
    contents().toLongLong(&ok);

    return !ok;
}

bool SearchRuleNumerical::matches(const Akonadi::Item &item) const
{
    const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

    QString msgContents;
    qint64 numericalMsgContents = 0;
    qint64 numericalValue = 0;

    if (qstricmp(field(), "<size>") == 0) {
        numericalMsgContents = item.size();
        numericalValue = contents().toLongLong();
        msgContents.setNum(numericalMsgContents);
    } else if (qstricmp(field(), "<age in days>") == 0) {
        QDateTime msgDateTime = msg->date()->dateTime();
        numericalMsgContents = msgDateTime.daysTo(QDateTime::currentDateTime());
        numericalValue = contents().toInt();
        msgContents.setNum(numericalMsgContents);
    } else {
        return false;
    }
    bool rc = matchesInternal(numericalValue, numericalMsgContents, msgContents);
    if (FilterLog::instance()->isLogging()) {
        QString msg = (rc ? "<font color=#00FF00>1 = </font>"
                       : "<font color=#FF0000>0 = </font>");
        msg += FilterLog::recode(asString());
        msg += " ( <i>" + QString::number(numericalMsgContents) + "</i> )";
        FilterLog::instance()->add(msg, FilterLog::RuleResult);
    }
    return rc;
}

SearchRule::RequiredPart SearchRuleNumerical::requiredPart() const
{
    return SearchRule::Envelope;
}

bool SearchRuleNumerical::matchesInternal(long numericalValue,
        long numericalMsgContents, const QString &msgContents) const
{
    switch (function()) {
    case SearchRule::FuncEquals:
        return (numericalValue == numericalMsgContents);

    case SearchRule::FuncNotEqual:
        return (numericalValue != numericalMsgContents);

    case SearchRule::FuncContains:
        return (msgContents.contains(contents(), Qt::CaseInsensitive));

    case SearchRule::FuncContainsNot:
        return (!msgContents.contains(contents(), Qt::CaseInsensitive));

    case SearchRule::FuncRegExp: {
        QRegExp regexp(contents(), Qt::CaseInsensitive);
        return (regexp.indexIn(msgContents) >= 0);
    }

    case SearchRule::FuncNotRegExp: {
        QRegExp regexp(contents(), Qt::CaseInsensitive);
        return (regexp.indexIn(msgContents) < 0);
    }

    case FuncIsGreater:
        return (numericalMsgContents > numericalValue);

    case FuncIsLessOrEqual:
        return (numericalMsgContents <= numericalValue);

    case FuncIsLess:
        return (numericalMsgContents < numericalValue);

    case FuncIsGreaterOrEqual:
        return (numericalMsgContents >= numericalValue);

    case FuncIsInAddressbook:  // since email-addresses are not numerical, I settle for false here
        return false;

    case FuncIsNotInAddressbook:
        return false;

    default:
        ;
    }

    return false;
}

void SearchRuleNumerical::addQueryTerms(Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError) const
{
    using namespace Akonadi;
    emptyIsNotAnError = false;
    if (qstricmp(field(), "<size>") == 0) {
        EmailSearchTerm term(EmailSearchTerm::ByteSize, contents().toInt(), akonadiComparator());
        term.setIsNegated(isNegated());
        groupTerm.addSubTerm(term);
    } else if (qstricmp(field(), "<age in days>") == 0) {
        QDate date(QDate::currentDate());
        date = date.addDays(contents().toInt());
        EmailSearchTerm term(EmailSearchTerm::HeaderOnlyDate, date, akonadiComparator());
        term.setIsNegated(isNegated());
        groupTerm.addSubTerm(term);
    }
}

QString SearchRuleNumerical::informationAboutNotValidRules() const
{
    return i18n("Content is not a number.");
}
