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

#include "searchrulestatus.h"
#include "filter/filterlog.h"
using MailCommon::FilterLog;

#include <KMime/KMimeMessage>

using namespace MailCommon;

struct _statusNames {
    const char *name;
    Akonadi::MessageStatus status;
};

static struct _statusNames statusNames[] = {
    { "Important", Akonadi::MessageStatus::statusImportant() },
    { "Unread", Akonadi::MessageStatus::statusUnread() },
    { "Read", Akonadi::MessageStatus::statusRead() },
    { "Deleted", Akonadi::MessageStatus::statusDeleted() },
    { "Replied", Akonadi::MessageStatus::statusReplied() },
    { "Forwarded", Akonadi::MessageStatus::statusForwarded() },
    { "Queued", Akonadi::MessageStatus::statusQueued() },
    { "Sent", Akonadi::MessageStatus::statusSent() },
    { "Watched", Akonadi::MessageStatus::statusWatched() },
    { "Ignored", Akonadi::MessageStatus::statusIgnored() },
    { "Action Item", Akonadi::MessageStatus::statusToAct() },
    { "Spam", Akonadi::MessageStatus::statusSpam() },
    { "Ham", Akonadi::MessageStatus::statusHam() },
    { "Has Attachment", Akonadi::MessageStatus::statusHasAttachment() }
};

static const int numStatusNames =
    sizeof statusNames / sizeof(struct _statusNames);

QString englishNameForStatus(const Akonadi::MessageStatus &status)
{
    for (int i = 0; i < numStatusNames; ++i) {
        if (statusNames[i].status == status) {
            return statusNames[i].name;
        }
    }
    return QString();
}

SearchRuleStatus::SearchRuleStatus(const QByteArray &field,
                                   Function func, const QString &aContents)
    : SearchRule(field, func, aContents)
{
    // the values are always in english, both from the conf file as well as
    // the patternedit gui
    mStatus = statusFromEnglishName(aContents);
}

SearchRuleStatus::SearchRuleStatus(Akonadi::MessageStatus status, Function func)
    : SearchRule("<status>", func, englishNameForStatus(status))
{
    mStatus = status;
}

Akonadi::MessageStatus SearchRuleStatus::statusFromEnglishName(const QString &aStatusString)
{
    for (int i = 0; i < numStatusNames; ++i) {
        if (!aStatusString.compare(statusNames[i].name)) {
            return statusNames[i].status;
        }
    }
    Akonadi::MessageStatus unknown;
    return unknown;
}

QString SearchRuleStatus::informationAboutNotValidRules() const
{
    //TODO
    return QStringLiteral("");
}

bool SearchRuleStatus::isEmpty() const
{
    return field().trimmed().isEmpty() || contents().isEmpty();
}

bool SearchRuleStatus::matches(const Akonadi::Item &item) const
{
    Akonadi::MessageStatus status;
    status.setStatusFromFlags(item.flags());
    bool rc = false;
    switch (function()) {
    case FuncEquals: // fallthrough. So that "<status> 'is' 'read'" works
    case FuncContains:
        if (status & mStatus) {
            rc = true;
        }
        break;
    case FuncNotEqual: // fallthrough. So that "<status> 'is not' 'read'" works
    case FuncContainsNot:
        if (!(status & mStatus)) {
            rc = true;
        }
        break;
    // FIXME what about the remaining funcs, how can they make sense for
    // stati?
    default:
        break;
    }
    if (FilterLog::instance()->isLogging()) {
        QString msg = (rc ? "<font color=#00FF00>1 = </font>" : "<font color=#FF0000>0 = </font>");
        msg += FilterLog::recode(asString());
        FilterLog::instance()->add(msg, FilterLog::RuleResult);
    }
    return rc;
}

SearchRule::RequiredPart SearchRuleStatus::requiredPart() const
{
    return SearchRule::Envelope;
}

void SearchRuleStatus::addQueryTerms(Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError) const
{
    using namespace Akonadi;
    emptyIsNotAnError = true;
    //TODO double check that isRead also works
    if (!mStatus.statusFlags().isEmpty()) {
        EmailSearchTerm term(EmailSearchTerm::MessageStatus, mStatus.statusFlags().toList().first(), akonadiComparator());
        term.setIsNegated(isNegated());
        groupTerm.addSubTerm(term);
    } else {
        //Special case Unread
        Akonadi::MessageStatus status;
        status.setRead(true);
        EmailSearchTerm term(EmailSearchTerm::MessageStatus, status.statusFlags().toList().first(), akonadiComparator());
        term.setIsNegated(!isNegated());
        groupTerm.addSubTerm(term);
    }
}
