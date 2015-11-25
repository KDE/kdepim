/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef VACATIONSCRIPTEXTRACTOR_H
#define VACATIONSCRIPTEXTRACTOR_H

#include "sieve-vacation.h"
#include "util/util.h"
#include "vacationutils.h"

#include <kmime/kmime_header_parsing.h>
#include <ksieve/error.h>
#include <ksieve/parser.h>
#include <ksieve/scriptbuilder.h>

#include "libksieve_debug.h"
#include <cassert>
#include <limits.h>
#include <map>
#include <set>
#include <vector>

namespace KSieveExt
{

class MultiScriptBuilder : public KSieve::ScriptBuilder
{
    std::vector<KSieve::ScriptBuilder *> mBuilders;
public:
    MultiScriptBuilder() : KSieve::ScriptBuilder() {}
    MultiScriptBuilder(KSieve::ScriptBuilder *sb1)
        : KSieve::ScriptBuilder(), mBuilders(1)
    {
        mBuilders[0] = sb1;
        assert(sb1);
    }
    MultiScriptBuilder(KSieve::ScriptBuilder *sb1,
                       KSieve::ScriptBuilder *sb2)
        : KSieve::ScriptBuilder(), mBuilders(2)
    {
        mBuilders[0] = sb1;
        mBuilders[1] = sb2;
        assert(sb1); assert(sb2);
    }
    MultiScriptBuilder(KSieve::ScriptBuilder *sb1,
                       KSieve::ScriptBuilder *sb2,
                       KSieve::ScriptBuilder *sb3)
        : KSieve::ScriptBuilder(), mBuilders(3)
    {
        mBuilders[0] = sb1;
        mBuilders[1] = sb2;
        mBuilders[2] = sb3;
        assert(sb1); assert(sb2); assert(sb3);
    }
    MultiScriptBuilder(KSieve::ScriptBuilder *sb1,
                       KSieve::ScriptBuilder *sb2,
                       KSieve::ScriptBuilder *sb3,
                       KSieve::ScriptBuilder *sb4)
        : KSieve::ScriptBuilder(), mBuilders(4)
    {
        mBuilders[0] = sb1;
        mBuilders[1] = sb2;
        mBuilders[2] = sb3;
        mBuilders[3] = sb4;
        assert(sb1); assert(sb2); assert(sb3); assert(sb4);
    }
    ~MultiScriptBuilder() {}
private:
#ifdef FOREACH
#undef FOREACH
#endif
#define FOREACH for (std::vector<KSieve::ScriptBuilder*>::const_iterator it = mBuilders.begin(), end = mBuilders.end() ; it != end ; ++it) (*it)->
    void commandStart(const QString &identifier, int lineNumber) Q_DECL_OVERRIDE {
        FOREACH commandStart(identifier, lineNumber);
    }
    void commandEnd(int lineNumber) Q_DECL_OVERRIDE {
        FOREACH commandEnd(lineNumber);
    }
    void testStart(const QString &identifier) Q_DECL_OVERRIDE {
        FOREACH testStart(identifier);
    }
    void testEnd() Q_DECL_OVERRIDE {
        FOREACH testEnd();
    }
    void testListStart() Q_DECL_OVERRIDE {
        FOREACH testListStart();
    }
    void testListEnd() Q_DECL_OVERRIDE {
        FOREACH testListEnd();
    }
    void blockStart(int lineNumber) Q_DECL_OVERRIDE {
        FOREACH blockStart(lineNumber);
    }
    void blockEnd(int lineNumber) Q_DECL_OVERRIDE {
        FOREACH blockEnd(lineNumber);
    }
    void hashComment(const QString &comment) Q_DECL_OVERRIDE {
        FOREACH hashComment(comment);
    }
    void bracketComment(const QString &comment) Q_DECL_OVERRIDE {
        FOREACH bracketComment(comment);
    }
    void lineFeed() Q_DECL_OVERRIDE {
        FOREACH lineFeed();
    }
    void error(const KSieve::Error &e) Q_DECL_OVERRIDE {
        FOREACH error(e);
    }
    void finished() Q_DECL_OVERRIDE {
        FOREACH finished();
    }
    void taggedArgument(const QString &tag) Q_DECL_OVERRIDE {
        FOREACH taggedArgument(tag);
    }
    void stringArgument(const QString &string, bool multiline, const QString &fixme) Q_DECL_OVERRIDE {
        FOREACH stringArgument(string, multiline, fixme);
    }
    void numberArgument(unsigned long number, char quantifier) Q_DECL_OVERRIDE {
        FOREACH numberArgument(number, quantifier);
    }
    void stringListArgumentStart() Q_DECL_OVERRIDE {
        FOREACH stringListArgumentStart();
    }
    void stringListEntry(const QString &string, bool multiline, const QString &fixme) Q_DECL_OVERRIDE {
        FOREACH stringListEntry(string, multiline, fixme);
    }
    void stringListArgumentEnd() Q_DECL_OVERRIDE {
        FOREACH stringListArgumentEnd();
    }
#undef FOREACH
};

}

namespace KSieveUi
{

class GenericInformationExtractor : public KSieve::ScriptBuilder
{
public:
    enum BuilderMethod {
        Any,
        TaggedArgument,
        StringArgument,
        NumberArgument,
        CommandStart,
        CommandEnd,
        TestStart,
        TestEnd,
        TestListStart,
        TestListEnd,
        BlockStart,
        BlockEnd,
        StringListArgumentStart,
        StringListEntry,
        StringListArgumentEnd
    };

    struct StateNode {
        // expectation:
        int depth;
        BuilderMethod method;
        const char *string;
        // actions:
        int if_found;
        int if_not_found;
        const char *save_tag;
    };

    const std::vector<StateNode> mNodes;
    std::map<QString, QString> mResults;
    std::set<unsigned int> mRecursionGuard;
    unsigned int mState;
    int mNestingDepth;

    int mLineNumber;

public:
    GenericInformationExtractor(const std::vector<StateNode> &nodes)
        : KSieve::ScriptBuilder(), mNodes(nodes), mState(0), mNestingDepth(0), mLineNumber(0) {}

    const std::map<QString, QString> &results() const
    {
        return mResults;
    }

private:
    void process(BuilderMethod method, const QString &string = QString())
    {
        doProcess(method, string);
        mRecursionGuard.clear();
    }

    void doProcess(BuilderMethod method, const QString &string)
    {
        mRecursionGuard.insert(mState);
        bool found = true;
        const StateNode &expected = mNodes[mState];
        if (expected.depth != -1 && mNestingDepth != expected.depth) {
            found = false;
        }
        if (expected.method != Any && method != expected.method) {
            found = false;
        }
        if (const char *str = expected.string)
            if (string.toLower() != QString::fromUtf8(str).toLower()) {
                found = false;
            }
        qCDebug(LIBKSIEVE_LOG) << (found ? "found:" : "not found:")
                               << mState << "->"
                               << (found ? expected.if_found : expected.if_not_found);
        mState = found ? expected.if_found : expected.if_not_found;
        assert(mState < mNodes.size());
        if (found)
            if (const char *save_tag = expected.save_tag) {
                mResults[QString::fromLatin1(save_tag)] = string;
            }
        if (!found && !mRecursionGuard.count(mState)) {
            doProcess(method, string);
        }
    }
    void commandStart(const QString &identifier, int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber)
        qCDebug(LIBKSIEVE_LOG);
        process(CommandStart, identifier);
    }
    void commandEnd(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber)
        qCDebug(LIBKSIEVE_LOG);
        process(CommandEnd);
    }
    void testStart(const QString &identifier) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(TestStart, identifier);
    }
    void testEnd() Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(TestEnd);
    }
    void testListStart() Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(TestListStart);
    }
    void testListEnd() Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(TestListEnd);
    }
    void blockStart(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber)
        qCDebug(LIBKSIEVE_LOG);
        process(BlockStart);
        ++mNestingDepth;
    }
    void blockEnd(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber)
        qCDebug(LIBKSIEVE_LOG);
        --mNestingDepth;
        process(BlockEnd);
    }
    void hashComment(const QString &) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
    }
    void bracketComment(const QString &) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
    }
    void lineFeed() Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
    }
    void error(const KSieve::Error &) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        mState = 0;
    }
    void finished() Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
    }

    void taggedArgument(const QString &tag) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(TaggedArgument, tag);
    }
    void stringArgument(const QString &string, bool, const QString &) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(StringArgument, string);
    }
    void numberArgument(unsigned long number, char) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(NumberArgument, QString::number(number));
    }
    void stringListArgumentStart() Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(StringListArgumentStart);
    }
    void stringListEntry(const QString &string, bool, const QString &) Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(StringListEntry, string);
    }
    void stringListArgumentEnd() Q_DECL_OVERRIDE {
        qCDebug(LIBKSIEVE_LOG);
        process(StringListArgumentEnd);
    }
};

typedef GenericInformationExtractor GIE;
static const GenericInformationExtractor::StateNode spamNodes[] = {
    { 0, GIE::CommandStart, "if",  1, 0, 0 },              // 0
    { 0,   GIE::TestStart, "allof", 2, 3, 0 },             // 1
    { 0,       GIE::TestListStart, 0, 3, 0, 0 },           // 2
    { 0,   GIE::TestStart, "not", 4, 3, 0 },               // 3
    { 0,   GIE::TestStart, "header", 5, 3, 0 },            // 4
    { 0,     GIE::TaggedArgument, "contains", 6, 0, 0 },   // 5

    // accept both string and string-list:
    { 0,     GIE::StringArgument, "x-spam-flag", 12, 7, "x-spam-flag" },    // 6
    { 0,     GIE::StringListArgumentStart, 0, 8, 0, 0 },                   // 7
    { 0,       GIE::StringListEntry, "x-spam-flag", 9, 10, "x-spam-flag" }, // 8
    { 0,       GIE::StringListEntry, 0, 9, 11, 0 },                         // 9
    { 0,     GIE::StringListArgumentEnd, 0, 0, 8, 0 },                     // 10
    { 0,     GIE::StringListArgumentEnd, 0, 12, 0, 0 },                     // 11

    // accept both string and string-list:
    { 0,     GIE::StringArgument, "yes", 18, 13, "spam-flag-yes" },    // 12
    { 0,     GIE::StringListArgumentStart, 0, 14, 0, 0 },              // 13
    { 0,       GIE::StringListEntry, "yes", 15, 16, "spam-flag-yes" }, // 14
    { 0,       GIE::StringListEntry, 0, 15, 17, 0 },                   // 15
    { 0,     GIE::StringListArgumentEnd, 0, 0, 14, 0 },                // 16
    { 0,     GIE::StringListArgumentEnd, 0, 18, 0, 0 },                // 17

    { 0,   GIE::TestEnd, 0, 21, 20, 0 },     // 18
    { 0,   GIE::Any, 0, 21, 0, 0 },          // 19
    { 0,   GIE::TestListEnd, 0, 21, 19, 0 }, // 20

    // block of command, find "stop", take nested if's into account:
    { 0,   GIE::BlockStart, 0, 22, 18, 0 },                       // 21
    { 1,     GIE::CommandStart, "vacation", 24, 22, "vacation" }, // 22
    { 1,    GIE::Any, 0, 24, 0, 0 },                              // 23
    { 0,   GIE::BlockEnd, 0, 25, 23, 0 },                         // 24

    { -1, GIE::Any, 0, 25, 25, 0 },   // 25 end state
};
static const unsigned int numSpamNodes = sizeof spamNodes / sizeof * spamNodes;

class SpamDataExtractor : public GenericInformationExtractor
{
public:
    SpamDataExtractor()
        : GenericInformationExtractor(std::vector<StateNode>(spamNodes, spamNodes + numSpamNodes))
    {

    }

    bool found() const
    {
        return mResults.count(QStringLiteral("x-spam-flag")) &&
               mResults.count(QStringLiteral("spam-flag-yes")) &&
               mResults.count(QStringLiteral("vacation"));
    }
};

// to understand this table, study the output of
// libksieve/tests/parsertest
//   'if not address :domain :contains ["from"] ["mydomain.org"] { keep; stop; }'
static const GenericInformationExtractor::StateNode domainNodes[] = {
    { 0, GIE::CommandStart, "if", 1, 0, 0 },       // 0
    { 0,   GIE::TestStart, "allof", 2, 3, 0 },     // 1
    { 0,       GIE::TestListStart, 0, 3, 0, 0 },   // 2
    { 0,     GIE::TestStart, "address", 4, 3, 0 }, // 3

    // :domain and :contains in arbitrary order:
    { 0,       GIE::TaggedArgument, "domain", 5, 6, 0 },     // 4
    { 0,       GIE::TaggedArgument, "contains", 8, 0, 0 },   // 5
    { 0,       GIE::TaggedArgument, "contains", 7, 0, 0 },   // 6
    { 0,       GIE::TaggedArgument, "domain", 8, 0, 0 },     // 7

    // accept both string and string-list:
    { 0,       GIE::StringArgument, "from", 14, 9, "from" },     // 8
    { 0,       GIE::StringListArgumentStart, 0, 10, 0, 0 },       // 9
    { 0,         GIE::StringListEntry, "from", 11, 12, "from" }, // 10
    { 0,         GIE::StringListEntry, 0, 11, 13, 0 },           // 11
    { 0,       GIE::StringListArgumentEnd, 0, 0, 10, 0 },         // 12
    { 0,       GIE::StringListArgumentEnd, 0, 14, 0, 0 },        // 13

    // string: save, string-list: save last
    { 0,       GIE::StringArgument, 0, 18, 15, "domainName" },    // 14
    { 0,       GIE::StringListArgumentStart, 0, 16, 0, 0 },       // 15
    { 0,         GIE::StringListEntry, 0, 16, 17, "domainName" }, // 16
    { 0,       GIE::StringListArgumentEnd, 0, 18, 0, 0 },         // 17

    { 0,   GIE::TestEnd, 0, 18, 20, 0 },    // 18
    { 0,   GIE::Any, 0, 18, 0, 0 },        // 19

    // block of commands, find "stop", take nested if's into account:
    { 0,   GIE::BlockStart, 0, 21, 19, 0 },                        // 20
    { 1,     GIE::CommandStart, "vacation", 23, 21, "vacation" },  // 21
    { 1,     GIE::Any, 0, 23, 0, 0 },                              // 22
    { 0,   GIE::BlockEnd, 0, 24, 22, 0 },                          // 23

    { -1, GIE::Any, 0, 24, 24, 0 }  // 24 end state
};
static const unsigned int numDomainNodes = sizeof domainNodes / sizeof * domainNodes;

class DomainRestrictionDataExtractor : public GenericInformationExtractor
{
public:
    DomainRestrictionDataExtractor()
        : GenericInformationExtractor(std::vector<StateNode>(domainNodes, domainNodes + numDomainNodes))
    {

    }

    QString domainName() /*not const, since map::op[] isn't const*/
    {
        return mResults.count(QStringLiteral("vacation")) && mResults.count(QStringLiteral("from"))
               ? mResults[QStringLiteral("domainName")] : QString();
    }
};

// if not allof (currentdate :value "ge" date "YYYY-MM-DD",
//               currentfate :value "le" date "YYYY-MM-DD) { keep; stop; }
static const GenericInformationExtractor::StateNode datesNodes[] = {
    { 0, GIE::CommandStart, "if", 1, 0, 0 },            // 0
    { 0,     GIE::TestStart, "allof", 2, 0, 0 },        // 1

    // handle startDate and endDate in arbitrary order
    { 0,       GIE::TestListStart, 0, 3, 0, 0 },                 // 2
    { 0,         GIE::TestStart, "currentdate", 4, 3, 0 },          // 3
    { 0,           GIE::TaggedArgument, "value", 5, 4, 0 },         // 4
    { 0,           GIE::StringArgument, "ge", 6, 10, 0 },           // 5
    { 0,           GIE::StringArgument, "date", 7, 8, 0 },          // 6
    { 0,           GIE::StringArgument, 0, 15, 0, "startDate" },    // 7
    { 0,           GIE::StringArgument, "iso8601", 9, 0, 0 },       // 8
    { 0,           GIE::StringArgument, 0, 15, 0, "startDateTime" },// 9
    { 0,           GIE::StringArgument, "le", 11, 0, 0 },           // 10
    { 0,           GIE::StringArgument, "date", 12, 13, 0 },        // 11
    { 0,           GIE::StringArgument, 0, 15, 0, "endDate" },      // 12
    { 0,           GIE::StringArgument, "iso8601", 14, 0, 0 },      // 13
    { 0,           GIE::StringArgument, 0, 15, 0, "endDateTime" },  // 14
    { 0,         GIE::TestEnd, 0, 16, 0, 0 },                    // 15

    { 0,         GIE::TestStart, "currentdate", 17, 16, 0 },        // 16
    { 0,           GIE::TaggedArgument, "value", 18, 17, 0 },         // 17
    { 0,           GIE::StringArgument, "le", 19, 23, 0 },            // 18
    { 0,           GIE::StringArgument, "date", 20, 21, 0 },          // 19
    { 0,           GIE::StringArgument, 0, 28, 0, "endDate" },        // 20
    { 0,           GIE::StringArgument, "iso8601", 22, 0, 0 },        // 21
    { 0,           GIE::StringArgument, 0, 28, 0, "endDateTime" },    // 22
    { 0,           GIE::StringArgument, "ge", 24, 0, 0 },             // 23
    { 0,           GIE::StringArgument, "date", 25, 26, 0 },          // 24
    { 0,           GIE::StringArgument, 0, 28, 0, "startDate" },      // 25
    { 0,           GIE::StringArgument, "iso8601", 27, 0, 0 },        // 26
    { 0,           GIE::StringArgument, 0, 28, 0, "startDateTime" },  // 27
    { 0,         GIE::TestEnd, 0, 32, 0, 0 },                      // 28
    { 0,         GIE::TestStart, 0, 31, 30, 0 },        // 29
    { -1,          GIE::Any, 0, 32, 0, 0 },                      // 30
    { 0,         GIE::TestEnd, 0, 32, 30, 0 },        // 31
    { 0,      GIE::TestListEnd, 0, 33, 29, 0 },                   // 32

    { 0,   GIE::TestEnd, 0, 34, 0, 0 },                 // 33

    // block of commands, find "stop", take nested if's into account:
    { 0,   GIE::BlockStart, 0, 36, 33, 0 },                 // 34
    { -1,    GIE::Any, 0, 36, 0, 0 },                      // 35
    { 1,     GIE::CommandStart, "vacation", 38, 35, "vacation" },  // 36
    { -1,    GIE::Any, 0, 38, 0, 0 },                      // 37
    { 0,   GIE::BlockEnd, 0, 39, 37, 0 },                   // 38

    { -1, GIE::Any, 0, 39, 39, 0 }                      // 39 end state
};

static const unsigned int numDatesNodes = sizeof datesNodes / sizeof * datesNodes;

class DateExtractor : public GenericInformationExtractor
{
public:
    DateExtractor()
        : GenericInformationExtractor(std::vector<StateNode>(datesNodes, datesNodes + numDatesNodes))
    {
    }

    QDate endDate() const
    {
        if (results().count(QStringLiteral("endDateTime")) == 1) {
            return datetime(QStringLiteral("endDateTime")).date();
        } else {
            return date(QStringLiteral("endDate"));
        }
    }

    QDate startDate() const
    {
        if (results().count(QStringLiteral("startDateTime")) == 1) {
            return datetime(QStringLiteral("startDateTime")).date();
        } else {
            return date(QStringLiteral("startDate"));
        }
    }

    QTime endTime() const
    {
        return datetime(QStringLiteral("endDateTime")).time();
    }

    QTime startTime() const
    {
        return datetime(QStringLiteral("startDateTime")).time();
    }

private:
    QDate date(const QString &name) const
    {
        if (results().count(name) == 0) {
            return QDate();
        } else {
            return QDate::fromString(results().at(name), Qt::ISODate);
        }
    }

    QDateTime datetime(const QString &name) const
    {
        if (results().count(name) == 0) {
            return QDateTime();
        } else {
            return QDateTime::fromString(results().at(name), Qt::ISODate);
        }
    }
};

class VacationDataExtractor : public KSieve::ScriptBuilder
{
    enum Context {
        None = 0,
        // command itself:
        VacationCommand,
        // tagged args:
        Days, Addresses, Subject,
        VacationEnd,
        IfBlock,
        RedirectCommand
    };
public:
    VacationDataExtractor();
    virtual ~VacationDataExtractor();

    bool commandFound() const
    {
        return mContext == VacationEnd;
    }
    bool active() const
    {
        return mActive;
    }
    int notificationInterval() const
    {
        return mNotificationInterval;
    }
    const QString &messageText() const
    {
        return mMessageText;
    }
    const QStringList &aliases() const
    {
        return mAliases;
    }
    const QString &ifComment() const
    {
        return mIfComment;
    }
    VacationUtils::MailAction mailAction() const
    {
        return mMailAction;
    }
    const QString &mailActionRecipient() const
    {
        return mMailActionRecipient;
    }

    const QString &subject() const
    {
        return mSubject;
    }

    int lineStart() const
    {
        return mLineStart;
    }

    int lineEnd() const
    {
        return mLineEnd;
    }

private:
    void commandStart(const QString &identifier, int lineNumber) Q_DECL_OVERRIDE;

    void commandEnd(int lineNumber) Q_DECL_OVERRIDE;

    void testStart(const QString &) Q_DECL_OVERRIDE;
    void testEnd() Q_DECL_OVERRIDE {}
    void testListStart() Q_DECL_OVERRIDE {}
    void testListEnd() Q_DECL_OVERRIDE {}
    void blockStart(int lineNumber) Q_DECL_OVERRIDE;
    void blockEnd(int lineNumber) Q_DECL_OVERRIDE;
    void hashComment(const QString &) Q_DECL_OVERRIDE;
    void bracketComment(const QString &) Q_DECL_OVERRIDE {}
    void lineFeed() Q_DECL_OVERRIDE {}
    void error(const KSieve::Error &e) Q_DECL_OVERRIDE;
    void finished() Q_DECL_OVERRIDE;

    void taggedArgument(const QString &tag) Q_DECL_OVERRIDE;

    void stringArgument(const QString &string, bool, const QString &) Q_DECL_OVERRIDE;

    void numberArgument(unsigned long number, char) Q_DECL_OVERRIDE;

    void stringListArgumentStart() Q_DECL_OVERRIDE;
    void stringListEntry(const QString &string, bool, const QString &) Q_DECL_OVERRIDE;
    void stringListArgumentEnd() Q_DECL_OVERRIDE;

private:
    Context mContext;
    int mNotificationInterval;
    QString mMessageText;
    QString mSubject;
    QStringList mAliases;
    bool mActive;
    bool mInIfBlock;
    bool mFoundInBlock;
    int mBlockLevel;
    QString mIfComment;
    int mLineStart;
    int mLineEnd;

    VacationUtils::MailAction mMailAction;
    Context mMailActionContext;
    QString mMailActionRecipient;

    void reset();
};

class RequireExtractor : public KSieve::ScriptBuilder
{
    enum Context {
        None = 0,
        // command itself:
        RequireCommand,
        EndState
    };
public:
    RequireExtractor();
    virtual ~RequireExtractor();

    bool commandFound() const
    {
        return mContext == EndState;
    }
    const QStringList &requirements() const
    {
        return mRequirements;
    }

    int lineStart() const
    {
        return mLineStart;
    }
    int lineEnd() const
    {
        return mLineEnd;
    }

private:
    void commandStart(const QString &identifier, int lineNumber);

    void commandEnd(int lineNumber);

    void testStart(const QString &) {}
    void testEnd() {}
    void testListStart() {}
    void testListEnd() {}
    void blockStart(int lineNumber)
    {
        Q_UNUSED(lineNumber)
    }
    void blockEnd(int lineNumber)
    {
        Q_UNUSED(lineNumber)
    }
    void hashComment(const QString &) {}
    void bracketComment(const QString &) {}
    void lineFeed() {}
    void error(const KSieve::Error &e);
    void finished();

    void taggedArgument(const QString &tag)
    {
        Q_UNUSED(tag)
    }
    void numberArgument(unsigned long number, char)
    {
        Q_UNUSED(number)
    }

    void stringArgument(const QString &string, bool, const QString &);

    void stringListArgumentStart() {}
    void stringListEntry(const QString &string, bool, const QString &);
    void stringListArgumentEnd() {}

private:
    Context mContext;
    QStringList mRequirements;
    int mLineStart;
    int mLineEnd;
};
}

#endif // VACATIONSCRIPTEXTRACTOR_H
