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

#include "searchrulestring.h"
#include "filter/filterlog.h"
using MailCommon::FilterLog;

#include <Akonadi/Contact/ContactSearchJob>

#include <AkonadiCore/SearchQuery>

#include <KMime/KMimeMessage>

#include <KEmailAddress>

#include <KLocalizedString>

#include <QRegExp>

#include <algorithm>
#include <boost/bind.hpp>

using namespace MailCommon;
SearchRuleString::SearchRuleString(const QByteArray &field,
                                   Function func,
                                   const QString &contents)
    : SearchRule(field, func, contents)
{
}

SearchRuleString::SearchRuleString(const SearchRuleString &other)
    : SearchRule(other)
{
}

const SearchRuleString &SearchRuleString::operator=(const SearchRuleString &other)
{
    if (this == &other) {
        return *this;
    }

    setField(other.field());
    setFunction(other.function());
    setContents(other.contents());

    return *this;
}

SearchRuleString::~SearchRuleString()
{
}

bool SearchRuleString::isEmpty() const
{
    return field().trimmed().isEmpty() || contents().isEmpty();
}

SearchRule::RequiredPart SearchRuleString::requiredPart() const
{
    const QByteArray f = field();
    SearchRule::RequiredPart part = Header;
    if (qstricmp(f, "<recipients>") == 0 ||
            qstricmp(f, "<status>") == 0 ||
            qstricmp(f, "<tag>") == 0 ||
            qstricmp(f, "subject") == 0 ||
            qstricmp(f, "from") == 0 ||
            qstricmp(f, "sender") == 0 ||
            qstricmp(f, "reply-to") == 0 ||
            qstricmp(f, "to") == 0  ||
            qstricmp(f, "cc") == 0 ||
            qstricmp(f, "bcc") == 0 ||
            qstricmp(f, "in-reply-to") == 0 ||
            qstricmp(f, "message-id") == 0 ||
            qstricmp(f, "references") == 0) {
        // these fields are directly provided by KMime::Message, no need to fetch the whole Header part
        part = Envelope;
    } else if (qstricmp(f, "<message>") == 0 ||
               qstricmp(f, "<body>") == 0) {
        part = CompleteMessage;
    }

    return part;
}

bool SearchRuleString::matches(const Akonadi::Item &item) const
{
    if (isEmpty()) {
        return false;
    }
    const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
    Q_ASSERT(msg.data());

    if (!msg->hasHeader("From")) {
        msg->parse(); // probably not parsed yet: make sure we can access all headers
    }

    QString msgContents;
    // Show the value used to compare the rules against in the log.
    // Overwrite the value for complete messages and all headers!
    bool logContents = true;

    if (qstricmp(field(), "<message>") == 0) {
        msgContents = msg->encodedContent();
        logContents = false;
    } else if (qstricmp(field(), "<body>") == 0) {
        msgContents = msg->body();
        logContents = false;
    } else if (qstricmp(field(), "<any header>") == 0) {
        msgContents = msg->head();
        logContents = false;
    } else if (qstricmp(field(), "<recipients>") == 0) {
        // (mmutz 2001-11-05) hack to fix "<recipients> !contains foo" to
        // meet user's expectations. See FAQ entry in KDE 2.2.2's KMail
        // handbook
        if (function() == FuncEquals || function() == FuncNotEqual) {
            // do we need to treat this case specially? Ie.: What shall
            // "equality" mean for recipients.
            return
                matchesInternal(msg->to()->asUnicodeString()) ||
                matchesInternal(msg->cc()->asUnicodeString()) ||
                matchesInternal(msg->bcc()->asUnicodeString());
        }
        msgContents = msg->to()->asUnicodeString();
        msgContents += ", " + msg->cc()->asUnicodeString();
        msgContents += ", " + msg->bcc()->asUnicodeString();
    } else if (qstricmp(field(), "<tag>") == 0) {
        //port?
        //     const Nepomuk2::Resource res( item.url() );
        //     foreach ( const Nepomuk2::Tag &tag, res.tags() ) {
        //       msgContents += tag.label();
        //     }
        logContents = false;
    } else {
        // make sure to treat messages with multiple header lines for
        // the same header correctly
        msgContents = msg->headerByType(field()) ?
                      msg->headerByType(field())->asUnicodeString() :
                      "";
    }

    if (function() == FuncIsInAddressbook ||
            function() == FuncIsNotInAddressbook) {
        // I think only the "from"-field makes sense.
        msgContents = msg->headerByType(field()) ?
                      msg->headerByType(field())->asUnicodeString() :
                      "";

        if (msgContents.isEmpty()) {
            return (function() == FuncIsInAddressbook) ? false : true;
        }
    }

    // these two functions need the kmmessage therefore they don't call matchesInternal
    if (function() == FuncHasAttachment) {
        return KMime::hasAttachment(msg.data());
    } else if (function() == FuncHasNoAttachment) {
        return !KMime::hasAttachment(msg.data());
    }

    bool rc = matchesInternal(msgContents);
    if (FilterLog::instance()->isLogging()) {
        QString msg = (rc ? "<font color=#00FF00>1 = </font>" : "<font color=#FF0000>0 = </font>");
        msg += FilterLog::recode(asString());
        // only log headers bcause messages and bodies can be pretty large
        if (logContents) {
            msg += " (<i>" + FilterLog::recode(msgContents) + "</i>)";
        }
        FilterLog::instance()->add(msg, FilterLog::RuleResult);
    }
    return rc;
}

void SearchRuleString::addQueryTerms(Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError) const
{
    using namespace Akonadi;
    emptyIsNotAnError = false;
    SearchTerm termGroup(SearchTerm::RelOr);
    if (qstricmp(field(), "subject") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Subject, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "reply-to") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderReplyTo, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "<message>") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Message, contents(), akonadiComparator()));
    } else if (field() == "<body>") {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Body, contents(), akonadiComparator()));
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Attachment, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "<recipients>") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderTo, contents(), akonadiComparator()));
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderCC, contents(), akonadiComparator()));
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderBCC, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "<any header>") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Headers, contents(), akonadiComparator()));
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Subject, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "to") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderTo, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "cc") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderCC, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "bcc") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderBCC, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "from") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderFrom, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "list-id") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderListId, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "resent-from") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderResentFrom, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "x-loop") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderXLoop, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "x-mailing-list") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderXMailingList, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "x-spam-flag") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderXSpamFlag, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "organization")  == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderOrganization, contents(), akonadiComparator()));
    } else if (qstricmp(field(), "<tag>") == 0) {
        termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::MessageTag, contents(), akonadiComparator()));
    }

    // TODO complete for other headers, generic headers

    if (!termGroup.subTerms().isEmpty()) {
        termGroup.setIsNegated(isNegated());
        groupTerm.addSubTerm(termGroup);
    }
}

QString SearchRuleString::informationAboutNotValidRules() const
{
    return i18n("String is empty.");
}

// helper, does the actual comparing
bool SearchRuleString::matchesInternal(const QString &msgContents) const
{
    if (msgContents.isEmpty()) {
        return false;
    }

    switch (function()) {
    case SearchRule::FuncEquals:
        return (QString::compare(msgContents.toLower(), contents().toLower()) == 0);

    case SearchRule::FuncNotEqual:
        return (QString::compare(msgContents.toLower(), contents().toLower()) != 0);

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

    case SearchRule::FuncStartWith: {
        return msgContents.startsWith(contents());
    }

    case SearchRule::FuncNotStartWith: {
        return !msgContents.startsWith(contents());
    }

    case SearchRule::FuncEndWith: {
        return msgContents.endsWith(contents());
    }

    case SearchRule::FuncNotEndWith: {
        return !msgContents.endsWith(contents());
    }

    case FuncIsGreater:
        return (QString::compare(msgContents.toLower(), contents().toLower()) > 0);

    case FuncIsLessOrEqual:
        return (QString::compare(msgContents.toLower(), contents().toLower()) <= 0);

    case FuncIsLess:
        return (QString::compare(msgContents.toLower(), contents().toLower()) < 0);

    case FuncIsGreaterOrEqual:
        return (QString::compare(msgContents.toLower(), contents().toLower()) >= 0);

    case FuncIsInAddressbook: {
        const QStringList addressList = KEmailAddress::splitAddressList(msgContents.toLower());
        QStringList::ConstIterator end(addressList.constEnd());
        for (QStringList::ConstIterator it = addressList.constBegin(); (it != end); ++it) {
            const QString email(KEmailAddress::extractEmailAddress(*it).toLower());
            if (!email.isEmpty()) {
                Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
                job->setLimit(1);
                job->setQuery(Akonadi::ContactSearchJob::Email, email);
                job->exec();

                if (!job->contacts().isEmpty()) {
                    return true;
                }
            }
        }
        return false;
    }

    case FuncIsNotInAddressbook: {
        const QStringList addressList = KEmailAddress::splitAddressList(msgContents.toLower());
        QStringList::ConstIterator end(addressList.constEnd());

        for (QStringList::ConstIterator it = addressList.constBegin(); (it != end); ++it) {
            const QString email(KEmailAddress::extractEmailAddress(*it).toLower());
            if (!email.isEmpty()) {
                Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
                job->setLimit(1);
                job->setQuery(Akonadi::ContactSearchJob::Email, email);
                job->exec();

                if (job->contacts().isEmpty()) {
                    return true;
                }
            }
        }
        return false;
    }

    case FuncIsInCategory: {
        QString category = contents();
        const QStringList addressList =  KEmailAddress::splitAddressList(msgContents.toLower());

        QStringList::ConstIterator end(addressList.constEnd());
        for (QStringList::ConstIterator it = addressList.constBegin(); it != end; ++it) {
            const QString email(KEmailAddress::extractEmailAddress(*it).toLower());
            if (!email.isEmpty()) {
                Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
                job->setQuery(Akonadi::ContactSearchJob::Email, email);
                job->exec();

                const KContacts::Addressee::List contacts = job->contacts();

                foreach (const KContacts::Addressee &contact, contacts) {
                    if (contact.hasCategory(category)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    case FuncIsNotInCategory: {
        QString category = contents();
        const QStringList addressList =  KEmailAddress::splitAddressList(msgContents.toLower());

        QStringList::ConstIterator end(addressList.constEnd());
        for (QStringList::ConstIterator it = addressList.constBegin(); it != end; ++it) {
            const QString email(KEmailAddress::extractEmailAddress(*it).toLower());
            if (!email.isEmpty()) {
                Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
                job->setQuery(Akonadi::ContactSearchJob::Email, email);
                job->exec();

                const KContacts::Addressee::List contacts = job->contacts();

                foreach (const KContacts::Addressee &contact, contacts) {
                    if (contact.hasCategory(category)) {
                        return false;
                    }
                }
            }

        }
        return true;
    }
    default:
        ;
    }

    return false;
}
