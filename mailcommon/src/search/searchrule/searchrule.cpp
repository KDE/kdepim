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
#include "searchrule.h"
#include "searchrule/searchrulenumerical.h"
#include "searchrule/searchruledate.h"
#include "searchrule/searchrulestring.h"
#include "searchrule/searchrulestatus.h"
#include "mailcommon_debug.h"

#include <KMime/KMimeMessage>

#include <KConfigGroup>

#include <QDataStream>

#include <algorithm>
#include <boost/bind.hpp>

using namespace MailCommon;

static const char *const funcConfigNames[] = {
    "contains", "contains-not",
    "equals", "not-equal",
    "regexp", "not-regexp",
    "greater", "less-or-equal", "less", "greater-or-equal",
    "is-in-addressbook", "is-not-in-addressbook",
    "is-in-category", "is-not-in-category",
    "has-attachment", "has-no-attachment",
    "start-with", "not-start-with",
    "end-with", "not-end-with"
};

static const int numFuncConfigNames =
    sizeof funcConfigNames / sizeof * funcConfigNames;

//==================================================
//
// class SearchRule (was: KMFilterRule)
//
//==================================================

SearchRule::SearchRule(const QByteArray &field, Function func, const QString &contents)
    : mField(field),
      mFunction(func),
      mContents(contents)
{
}

SearchRule::SearchRule(const SearchRule &other)
    : mField(other.mField),
      mFunction(other.mFunction),
      mContents(other.mContents)
{
}

const SearchRule &SearchRule::operator=(const SearchRule &other)
{
    if (this == &other) {
        return *this;
    }

    mField = other.mField;
    mFunction = other.mFunction;
    mContents = other.mContents;

    return *this;
}

SearchRule::Ptr SearchRule::createInstance(const QByteArray &field,
        Function func,
        const QString &contents)
{
    SearchRule::Ptr ret;
    if (field == "<status>") {
        ret = SearchRule::Ptr(new SearchRuleStatus(field, func, contents));
    } else if (field == "<age in days>" || field == "<size>") {
        ret = SearchRule::Ptr(new SearchRuleNumerical(field, func, contents));
    } else if (field == "<date>") {
        ret = SearchRule::Ptr(new SearchRuleDate(field, func, contents));
    } else {
        ret = SearchRule::Ptr(new SearchRuleString(field, func, contents));
    }

    return ret;
}

SearchRule::Ptr SearchRule::createInstance(const QByteArray &field,
        const char *func,
        const QString &contents)
{
    return (createInstance(field, configValueToFunc(func), contents));
}

SearchRule::Ptr SearchRule::createInstance(const SearchRule &other)
{
    return (createInstance(other.field(), other.function(), other.contents()));
}

SearchRule::Ptr SearchRule::createInstanceFromConfig(const KConfigGroup &config, int aIdx)
{
    const char cIdx = char(int('A') + aIdx);

    static const QString field = QStringLiteral("field");
    static const QString func = QStringLiteral("func");
    static const QString contents = QStringLiteral("contents");

    const QByteArray &field2 = config.readEntry(field + cIdx, QString()).toLatin1();
    Function func2 = configValueToFunc(config.readEntry(func + cIdx, QString()).toLatin1());
    const QString &contents2 = config.readEntry(contents + cIdx, QString());

    if (field2 == "<To or Cc>") {   // backwards compat
        return SearchRule::createInstance("<recipients>", func2, contents2);
    } else {
        return SearchRule::createInstance(field2, func2, contents2);
    }
}

SearchRule::Ptr SearchRule::createInstance(QDataStream &s)
{
    QByteArray field;
    s >> field;
    QString function;
    s >> function;
    Function func = configValueToFunc(function.toUtf8());
    QString contents;
    s >> contents;
    return createInstance(field, func, contents);
}

SearchRule::~SearchRule()
{
}

SearchRule::Function SearchRule::configValueToFunc(const char *str)
{
    if (!str) {
        return FuncNone;
    }

    for (int i = 0; i < numFuncConfigNames; ++i) {
        if (qstricmp(funcConfigNames[i], str) == 0) {
            return (Function)i;
        }
    }

    return FuncNone;
}

QString SearchRule::functionToString(Function function)
{
    if (function != FuncNone) {
        return funcConfigNames[int(function)];
    } else {
        return "invalid";
    }
}

void SearchRule::writeConfig(KConfigGroup &config, int aIdx) const
{
    const char cIdx = char('A' + aIdx);
    static const QString field = QStringLiteral("field");
    static const QString func = QStringLiteral("func");
    static const QString contents = QStringLiteral("contents");

    config.writeEntry(field + cIdx, QString(mField));
    config.writeEntry(func + cIdx, functionToString(mFunction));
    config.writeEntry(contents + cIdx, mContents);
}

QString SearchRule::conditionToString(Function function)
{
    QString str;
    switch (function) {
    case FuncEquals:
        str = i18n("equal");
        break;
    case FuncNotEqual:
        str = i18n("not equal");
        break;
    case FuncIsGreater:
        str = i18n("is greater");
        break;
    case FuncIsLessOrEqual:
        str = i18n("is less or equal");
        break;
    case FuncIsLess:
        str = i18n("is less");
        break;
    case FuncIsGreaterOrEqual:
        str = i18n("is greater or equal");
        break;
    case FuncIsInAddressbook:
        str = i18n("is in addressbook");
        break;
    case FuncIsNotInAddressbook:
        str = i18n("is not in addressbook");
        break;
    case FuncIsInCategory:
        str = i18n("is in category");
        break;
    case FuncIsNotInCategory:
        str = i18n("is in category");
        break;
    case FuncHasAttachment:
        str = i18n("has an attachment");
        break;
    case FuncHasNoAttachment:
        str = i18n("has not an attachment");
        break;
    case FuncStartWith:
        str = i18n("start with");
        break;
    case FuncNotStartWith:
        str = i18n("not start with");
        break;
    case FuncEndWith:
        str = i18n("end with");
        break;
    case FuncNotEndWith:
        str = i18n("not end with");
        break;
    case FuncNone:
        str = i18n("none");
        break;
    case FuncContains:
        str = i18n("contains");
        break;
    case FuncContainsNot:
        str = i18n("not contains");
        break;
    case FuncRegExp:
        str = i18n("has regexp");
        break;
    case FuncNotRegExp:
        str = i18n("not regexp");
        break;
    }
    return str;
}

void SearchRule::generateSieveScript(QStringList &requires, QString &code)
{
    QString contentStr = mContents;
    if (mField == "<size>") {
        QString comparaison;
        int offset = 0;
        switch (mFunction) {
        case FuncEquals:
            comparaison = QLatin1Char('"') + i18n("size equals not supported") + QLatin1Char('"');
            break;
        case FuncNotEqual:
            comparaison = QLatin1Char('"') + i18n("size not equals not supported") + QLatin1Char('"');
            break;
        case FuncIsGreater:
            comparaison = QStringLiteral(":over");
            break;
        case FuncIsLessOrEqual:
            comparaison = QStringLiteral(":under");
            offset = 1;
            break;
        case FuncIsLess:
            comparaison = QStringLiteral(":under");
            break;
        case FuncIsGreaterOrEqual:
            comparaison = QStringLiteral(":over");
            offset = -1;
            break;
        case FuncIsInAddressbook:
        case FuncIsNotInAddressbook:
        case FuncIsInCategory:
        case FuncIsNotInCategory:
        case FuncHasAttachment:
        case FuncHasNoAttachment:
        case FuncStartWith:
        case FuncNotStartWith:
        case FuncEndWith:
        case FuncNotEndWith:
        case FuncNone:
        case FuncContains:
        case FuncContainsNot:
        case FuncRegExp:
        case FuncNotRegExp:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += QStringLiteral("size %1 %2K").arg(comparaison).arg(QString::number(mContents.toInt() + offset));
    } else if (mField == "<status>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<status> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<any header>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<any header> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "contents") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<contents> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<age in days>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<age in days> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<date>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<date> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<recipients>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<recipients> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<tag>") {
        code += QLatin1Char('"') + i18n("<Tag> is not supported") + QLatin1Char('"');
    } else if (mField == "<message>") {
        //TODO ?
        code += i18n("<message> not implemented/supported");
    } else if (mField == "<body>") {
        if (!requires.contains(QStringLiteral("body"))) {
            requires << QStringLiteral("body");
        }
        QString comparaison;
        bool negative = false;
        switch (mFunction) {
        case FuncNone:
            break;
        case FuncContains:
            comparaison = QStringLiteral(":contains");
            break;
        case FuncContainsNot:
            negative = true;
            comparaison = QStringLiteral(":contains");
            break;
        case FuncEquals:
            comparaison = QStringLiteral(":is");
            break;
        case FuncNotEqual:
            comparaison = QStringLiteral(":is");
            negative = true;
            break;
        case FuncRegExp:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            break;
        case FuncNotRegExp:
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            negative = true;
            break;
        case FuncStartWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            contentStr = QLatin1Char('^') + contentStr;
            break;
        case FuncNotStartWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            contentStr = QLatin1Char('^') + contentStr;
            negative = true;
            break;
        case FuncEndWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            contentStr = contentStr + QLatin1Char('$');
            break;
        case FuncNotEndWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            contentStr = contentStr + QLatin1Char('$');
            negative = true;
            break;
        case FuncIsGreater:
        case FuncIsLessOrEqual:
        case FuncIsLess:
        case FuncIsGreaterOrEqual:
        case FuncIsInAddressbook:
        case FuncIsNotInAddressbook:
        case FuncIsInCategory:
        case FuncIsNotInCategory:
        case FuncHasAttachment:
        case FuncHasNoAttachment:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += (negative ? QStringLiteral("not ") : QString()) + QStringLiteral("body :text %1 \"%2\"").arg(comparaison).arg(contentStr);
    } else {
        QString comparaison;
        bool negative = false;
        switch (mFunction) {
        case FuncNone:
            break;
        case FuncContains:
            comparaison = QStringLiteral(":contains");
            break;
        case FuncContainsNot:
            negative = true;
            comparaison = QStringLiteral(":contains");
            break;
        case FuncEquals:
            comparaison = QStringLiteral(":is");
            break;
        case FuncNotEqual:
            comparaison = QStringLiteral(":is");
            negative = true;
            break;
        case FuncRegExp:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            break;
        case FuncNotRegExp:
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            negative = true;
            break;
        case FuncStartWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            contentStr = QLatin1Char('^') + contentStr;
            break;
        case FuncNotStartWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            contentStr = QLatin1Char('^') + contentStr;
            negative = true;
            break;
        case FuncEndWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            contentStr = contentStr + QLatin1Char('$');
            break;
        case FuncNotEndWith:
            comparaison = QStringLiteral(":regex");
            if (!requires.contains(QStringLiteral("regex"))) {
                requires << QStringLiteral("regex");
            }
            comparaison = QStringLiteral(":regex");
            contentStr = contentStr + QLatin1Char('$');
            negative = true;
            break;

        case FuncIsGreater:
        case FuncIsLessOrEqual:
        case FuncIsLess:
        case FuncIsGreaterOrEqual:
        case FuncIsInAddressbook:
        case FuncIsNotInAddressbook:
        case FuncIsInCategory:
        case FuncIsNotInCategory:
        case FuncHasAttachment:
        case FuncHasNoAttachment:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += (negative ? QStringLiteral("not ") : QString()) + QStringLiteral("header %1 \"%2\" \"%3\"").arg(comparaison).arg(QLatin1String(mField)).arg(contentStr);
    }
}

void SearchRule::setFunction(Function function)
{
    mFunction = function;
}

SearchRule::Function SearchRule::function() const
{
    return mFunction;
}

void SearchRule::setField(const QByteArray &field)
{
    mField = field;
}

QByteArray SearchRule::field() const
{
    return mField;
}

void SearchRule::setContents(const QString &contents)
{
    mContents = contents;
}

QString SearchRule::contents() const
{
    return mContents;
}

const QString SearchRule::asString() const
{
    QString result  = QLatin1String("\"") + mField + QLatin1String("\" <");
    result += functionToString(mFunction);
    result += "> \"" + mContents + "\"";

    return result;
}

Akonadi::SearchTerm::Condition SearchRule::akonadiComparator() const
{
    switch (function()) {
    case SearchRule::FuncContains:
    case SearchRule::FuncContainsNot:
        return Akonadi::SearchTerm::CondContains;

    case SearchRule::FuncEquals:
    case SearchRule::FuncNotEqual:
        return Akonadi::SearchTerm::CondEqual;

    case SearchRule::FuncIsGreater:
        return Akonadi::SearchTerm::CondGreaterThan;

    case SearchRule::FuncIsGreaterOrEqual:
        return Akonadi::SearchTerm::CondGreaterOrEqual;

    case SearchRule::FuncIsLess:
        return Akonadi::SearchTerm::CondLessThan;

    case SearchRule::FuncIsLessOrEqual:
        return Akonadi::SearchTerm::CondLessOrEqual;

    case SearchRule::FuncRegExp:
    case SearchRule::FuncNotRegExp:
        //TODO is this sufficient?
        return Akonadi::SearchTerm::CondContains;

    case SearchRule::FuncStartWith:
    case SearchRule::FuncNotStartWith:
    case SearchRule::FuncEndWith:
    case SearchRule::FuncNotEndWith:
        //TODO is this sufficient?
        return Akonadi::SearchTerm::CondContains;
    default:
        qCDebug(MAILCOMMON_LOG) << "Unhandled function type: " << function();
    }

    return Akonadi::SearchTerm::CondEqual;
}

bool SearchRule::isNegated() const
{
    bool negate = false;
    switch (function()) {
    case SearchRule::FuncContainsNot:
    case SearchRule::FuncNotEqual:
    case SearchRule::FuncNotRegExp:
    case SearchRule::FuncHasNoAttachment:
    case SearchRule::FuncIsNotInCategory:
    case SearchRule::FuncIsNotInAddressbook:
    case SearchRule::FuncNotStartWith:
    case SearchRule::FuncNotEndWith:
        negate = true;
    default:
        break;
    }
    return negate;
}

QDataStream &SearchRule::operator >>(QDataStream &s) const
{
    s << mField << functionToString(mFunction) << mContents;
    return s;
}

