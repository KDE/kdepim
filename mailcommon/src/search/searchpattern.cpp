/*

  Author: Marc Mutz <mutz@kde.org>
  Copyright (C) 2012 Andras Mantia <amantia@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "searchpattern.h"
#include "searchrule/searchrulenumerical.h"
#include "searchrule/searchruledate.h"
#include "searchrule/searchrulestring.h"
#include "searchrule/searchrulestatus.h"
#include "filter/filterlog.h"
using MailCommon::FilterLog;
#include "mailcommon_debug.h"
#include <Akonadi/Contact/ContactSearchJob>

#include <SearchQuery>

#include <KMime/KMimeMessage>

#include <KEmailAddress>

#include <KConfigGroup>
#include <KLocalizedString>

#include <QDataStream>

#include <algorithm>
#include <boost/bind.hpp>

using namespace MailCommon;

//==================================================
//
// class SearchPattern
//
//==================================================

SearchPattern::SearchPattern()
    : QList<SearchRule::Ptr>()
{
    init();
}

SearchPattern::SearchPattern(const KConfigGroup &config)
    : QList<SearchRule::Ptr>()
{
    readConfig(config);
}

SearchPattern::~SearchPattern()
{
}

bool SearchPattern::matches(const Akonadi::Item &item, bool ignoreBody) const
{
    if (isEmpty()) {
        return true;
    }
    if (!item.hasPayload<KMime::Message::Ptr>()) {
        return false;
    }

    QList<SearchRule::Ptr>::const_iterator it;
    QList<SearchRule::Ptr>::const_iterator end(constEnd());
    switch (mOperator) {
    case OpAnd: // all rules must match
        for (it = constBegin(); it != end; ++it) {
            if (!((*it)->requiredPart() == SearchRule::CompleteMessage && ignoreBody)) {
                if (!(*it)->matches(item)) {
                    return false;
                }
            }
        }
        return true;

    case OpOr:  // at least one rule must match
        for (it = constBegin(); it != end; ++it) {
            if (!((*it)->requiredPart() == MailCommon::SearchRule::CompleteMessage && ignoreBody)) {
                if ((*it)->matches(item)) {
                    return true;
                }
            }
        }
        return false;

    case OpAll:
        return true;

    default:
        return false;
    }
}

SearchRule::RequiredPart SearchPattern::requiredPart() const
{
    SearchRule::RequiredPart reqPart = SearchRule::Envelope;

    if (!isEmpty()) {
        reqPart = (*std::max_element(constBegin(), constEnd(),
                                     boost::bind(&MailCommon::SearchRule::requiredPart, _1) <
                                     boost::bind(&MailCommon::SearchRule::requiredPart, _2)))->requiredPart();
    }
    return reqPart;
}

QString SearchPattern::purify(bool removeAction)
{
    QString informationAboutNotValidPattern;
    QList<SearchRule::Ptr>::iterator it = end();
    while (it != begin()) {
        --it;
        if ((*it)->isEmpty()) {
            if (removeAction) {
#ifndef NDEBUG
                qCDebug(MAILCOMMON_LOG) << "Removing" << (*it)->asString();
#endif
                if (!informationAboutNotValidPattern.isEmpty()) {
                    informationAboutNotValidPattern += QLatin1Char('\n');
                }
                informationAboutNotValidPattern += (*it)->informationAboutNotValidRules();

                erase(it);
                it = end();
            }
        }
    }

    return informationAboutNotValidPattern;
}

void SearchPattern::readConfig(const KConfigGroup &config)
{
    init();

    mName = config.readEntry("name");
    if (!config.hasKey("rules")) {
        qCDebug(MAILCOMMON_LOG) << "Found legacy config! Converting.";
        importLegacyConfig(config);
        return;
    }

    const QString op = config.readEntry("operator");
    if (op == QLatin1String("or")) {
        mOperator = OpOr;
    } else if (op == QLatin1String("and")) {
        mOperator = OpAnd;
    } else if (op == QLatin1String("all")) {
        mOperator = OpAll;
    }

    const int nRules = config.readEntry("rules", 0);

    for (int i = 0; i < nRules; ++i) {
        SearchRule::Ptr r = SearchRule::createInstanceFromConfig(config, i);
        if (!r->isEmpty()) {
            append(r);
        }
    }
}

void SearchPattern::importLegacyConfig(const KConfigGroup &config)
{
    SearchRule::Ptr rule =
        SearchRule::createInstance(
            config.readEntry("fieldA").toLatin1(),
            config.readEntry("funcA").toLatin1(),
            config.readEntry("contentsA"));

    if (rule->isEmpty()) {
        // if the first rule is invalid,
        // we really can't do much heuristics...
        return;
    }
    append(rule);

    const QString sOperator = config.readEntry("operator");
    if (sOperator == QLatin1String("ignore")) {
        return;
    }

    rule =
        SearchRule::createInstance(
            config.readEntry("fieldB").toLatin1(),
            config.readEntry("funcB").toLatin1(),
            config.readEntry("contentsB"));

    if (rule->isEmpty()) {
        return;
    }
    append(rule);

    if (sOperator == QLatin1String("or")) {
        mOperator = OpOr;
        return;
    }
    // This is the interesting case...
    if (sOperator == QLatin1String("unless")) {     // meaning "and not", ie we need to...
        // ...invert the function (e.g. "equals" <-> "doesn't equal")
        // We simply toggle the last bit (xor with 0x1)... This assumes that
        // SearchRule::Function's come in adjacent pairs of pros and cons
        SearchRule::Function func = last()->function();
        unsigned int intFunc = (unsigned int)func;
        func = SearchRule::Function(intFunc ^ 0x1);

        last()->setFunction(func);
    }

    // treat any other case as "and" (our default).
}

void SearchPattern::writeConfig(KConfigGroup &config) const
{
    config.writeEntry("name", mName);
    switch (mOperator) {
    case OpOr:
        config.writeEntry("operator", "or");
        break;
    case OpAnd:
        config.writeEntry("operator", "and");
        break;
    case OpAll:
        config.writeEntry("operator", "all");
        break;
    }

    int i = 0;
    QList<SearchRule::Ptr>::const_iterator it;
    QList<SearchRule::Ptr>::const_iterator endIt(constEnd());

    if (count() >= filterRulesMaximumSize()) {
        qCDebug(MAILCOMMON_LOG) << "Number of patterns > to filter max rules";
    }
    for (it = constBegin(); it != endIt && i < filterRulesMaximumSize(); ++i, ++it) {
        // we could do this ourselves, but we want the rules to be extensible,
        // so we give the rule it's number and let it do the rest.
        (*it)->writeConfig(config, i);
    }

    // save the total number of rules.
    config.writeEntry("rules", i);
}

int SearchPattern::filterRulesMaximumSize()
{
    return 8;
}

void SearchPattern::init()
{
    clear();
    mOperator = OpAnd;
    mName = '<' + i18nc("name used for a virgin filter", "unknown") + '>';
}

QString SearchPattern::asString() const
{
    QString result;
    switch (mOperator) {
    case OpOr:
        result = i18n("(match any of the following)");
        break;
    case OpAnd:
        result = i18n("(match all of the following)");
        break;
    case OpAll:
        result = i18n("(match all messages)");
        break;
    }

    QList<SearchRule::Ptr>::const_iterator it;
    QList<SearchRule::Ptr>::const_iterator endIt = constEnd();
    for (it = constBegin(); it != endIt; ++it) {
        result += "\n\t" + FilterLog::recode((*it)->asString());
    }

    return result;
}

SearchPattern::SparqlQueryError SearchPattern::asAkonadiQuery(Akonadi::SearchQuery &query) const
{
    query = Akonadi::SearchQuery();

    Akonadi::SearchTerm term(Akonadi::SearchTerm::RelAnd);
    if (op() == SearchPattern::OpOr) {
        term = Akonadi::SearchTerm(Akonadi::SearchTerm::RelOr);
    }

    const_iterator end(constEnd());
    bool emptyIsNotAnError = false;
    bool resultAddQuery = false;
    for (const_iterator it = constBegin(); it != end; ++it) {
        (*it)->addQueryTerms(term, emptyIsNotAnError);
        resultAddQuery &= emptyIsNotAnError;
    }

    if (term.subTerms().isEmpty()) {
        if (resultAddQuery) {
            qCDebug(MAILCOMMON_LOG) << " innergroup is Empty. Need to report bug";
            return MissingCheck;
        } else {
            return EmptyResult;
        }
    }
    query.setTerm(term);

    return NoError;
}

const SearchPattern &SearchPattern::operator=(const SearchPattern &other)
{
    if (this == &other) {
        return *this;
    }

    setOp(other.op());
    setName(other.name());

    clear(); // ###
    QList<SearchRule::Ptr>::const_iterator it;
    QList<SearchRule::Ptr>::const_iterator end(other.constEnd());
    for (it = other.constBegin(); it != end; ++it) {
        append(SearchRule::createInstance(**it));     // deep copy
    }

    return *this;
}

QByteArray SearchPattern::serialize() const
{
    QByteArray out;
    QDataStream stream(&out, QIODevice::WriteOnly);
    *this >> stream;
    return out;
}

void SearchPattern::deserialize(const QByteArray &str)
{
    QDataStream stream(str);
    *this << stream;
}

QDataStream &SearchPattern::operator>>(QDataStream &s) const
{
    switch (op()) {
    case SearchPattern::OpAnd:
        s << QStringLiteral("and");
        break;
    case SearchPattern::OpOr:
        s << QStringLiteral("or");
        break;
    case SearchPattern::OpAll:
        s << QStringLiteral("all");
        break;
    }

    Q_FOREACH (const SearchRule::Ptr rule, *this) {
        *rule >> s;
    }
    return s;
}

QDataStream &SearchPattern::operator<<(QDataStream &s)
{
    QString op;
    s >> op;
    if (op == QLatin1String("and")) {
        setOp(OpAnd);
    } else if (op == QLatin1String("or")) {
        setOp(OpOr);
    } else if (op == QLatin1String("all")) {
        setOp(OpAll);
    }

    while (!s.atEnd()) {
        SearchRule::Ptr rule = SearchRule::createInstance(s);
        append(rule);
    }
    return s;
}

void SearchPattern::generateSieveScript(QStringList &requires, QString &code)
{
    code += QLatin1String("\n#") + mName + QLatin1Char('\n');
    switch (mOperator) {
    case OpOr:
        code += QLatin1String("if anyof (");
        break;
    case OpAnd:
        code += QLatin1String("if allof (");
        break;
    case OpAll:
        code += QLatin1String("if (true) {");
        return;
    }

    QList<SearchRule::Ptr>::const_iterator it;
    QList<SearchRule::Ptr>::const_iterator endIt(constEnd());
    int i = 0;
    for (it = constBegin(); it != endIt && i < filterRulesMaximumSize(); ++i, ++it) {
        if (i != 0) {
            code += QLatin1String("\n, ");
        }
        (*it)->generateSieveScript(requires, code);
    }
}

// Needed for MSVC 2010, as it seems to not implicit cast for a pointer anymore
#ifdef _MSC_VER
namespace MailCommon
{
uint qHash(SearchRule::Ptr sr)
{
    return ::qHash(sr.get());
}
}
#endif
