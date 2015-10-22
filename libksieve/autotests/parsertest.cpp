/*  -*- c++ -*-
    tests/parsertest.cpp

    This file is part of the testsuite of KSieve,
    the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    KSieve is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KSieve is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#include <config-libksieve.h> // SIZEOF_UNSIGNED_LONG
#include <../src/ksieve/parser.h>
using KSieve::Parser;

#include <../src/ksieve/error.h>
#include <../src/ksieve/scriptbuilder.h>

#include <QString>

#include <iostream>
#include <cstdlib>

using std::cout;
using std::cerr;
using std::endl;

#include <cassert>

enum BuilderMethod {
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
    StringListArgumentEnd,
    HashComment,
    BracketComment,
    Error,
    Finished
};

static const unsigned int MAX_RESPONSES = 100;

struct TestCase {
    const char *name;
    const char *script;
    struct Response {
        BuilderMethod method;
        const char *string;
        bool boolean;
    } responses[MAX_RESPONSES];
} testCases[] = {

    //
    // single commands:
    //

    {
        "Null script",
        Q_NULLPTR,
        { { Finished, Q_NULLPTR, false } }
    },

    {
        "Empty script",
        "",
        { { Finished, Q_NULLPTR, false } }
    },

    {
        "WS-only script",
        " \t\n\r\n",
        { { Finished, Q_NULLPTR, false } }
    },

    {
        "Bare hash comment",
        "#comment",
        {   { HashComment, "comment", false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "Bare bracket comment",
        "/*comment*/",
        {   { BracketComment, "comment", false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "Bare command",
        "command;",
        {   { CommandStart, "command", false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "Bare command - missing semicolon",
        "command",
        {   { CommandStart, "command", false },
            { Error, "MissingSemicolonOrBlock", false }
        }
    },

    {
        "surrounded by bracket comments",
        "/*comment*/command/*comment*/;/*comment*/",
        {   { BracketComment, "comment", false },
            { CommandStart, "command", false },
            { BracketComment, "comment", false },
            { CommandEnd, Q_NULLPTR, false },
            { BracketComment, "comment", false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "surrounded by hash comments",
        "#comment\ncommand#comment\n;#comment",
        {   { HashComment, "comment", false },
            { CommandStart, "command", false },
            { HashComment, "comment", false },
            { CommandEnd, Q_NULLPTR, false },
            { HashComment, "comment", false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single tagged argument",
        "command :tag;",
        {   { CommandStart, "command", false },
            { TaggedArgument, "tag", false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single tagged argument - missing semicolon",
        "command :tag",
        {   { CommandStart, "command", false },
            { TaggedArgument, "tag", false },
            { Error, "MissingSemicolonOrBlock", false }
        }
    },

    {
        "single string argument - quoted string",
        "command \"string\";",
        {   { CommandStart, "command", false },
            { StringArgument, "string", false /*quoted*/ },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single string argument - multi-line string",
        "command text:\nstring\n.\n;",
        {   { CommandStart, "command", false },
            { StringArgument, "string", true /*multiline*/ },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single number argument - 100",
        "command 100;",
        {   { CommandStart, "command", false },
            { NumberArgument, "100 ", false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single number argument - 100k",
        "command 100k;",
        {   { CommandStart, "command", false },
            { NumberArgument, "102400k", false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single number argument - 100M",
        "command 100M;",
        {   { CommandStart, "command", false },
            { NumberArgument, "104857600M", false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single number argument - 2G",
        "command 2G;",
        {   { CommandStart, "command", false },
            { NumberArgument, "2147483648G", false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

#if SIZEOF_UNSIGNED_LONG == 8
#  define ULONG_MAX_STRING "18446744073709551615"
#  define ULONG_MAXP1_STRING "18446744073709551616"
#elif SIZEOF_UNSIGNED_LONG == 4
#  define ULONG_MAX_STRING "4294967295"
#  define ULONG_MAXP1_STRING "4G"
#else
#  error sizeof( unsigned long ) != 4 && sizeof( unsigned long ) != 8 ???
#endif

    {
        "single number argument - ULONG_MAX + 1",
        "command " ULONG_MAXP1_STRING ";",
        {   { CommandStart, "command", false },
            { Error, "NumberOutOfRange", false }
        }
    },

    {
        "single number argument - ULONG_MAX",
        "command " ULONG_MAX_STRING ";",
        {   { CommandStart, "command", false },
            { NumberArgument, ULONG_MAX_STRING " ", false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single one-element string list argument - quoted string",
        "command [\"string\"];",
        {   { CommandStart, "command", false },
            { StringListArgumentStart, Q_NULLPTR, false },
            { StringListEntry, "string", false /*quoted*/ },
            { StringListArgumentEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single one-element string list argument - multi-line string",
        "command [text:\nstring\n.\n];",
        {   { CommandStart, "command", false },
            { StringListArgumentStart, Q_NULLPTR, false },
            { StringListEntry, "string", true /*multiline*/ },
            { StringListArgumentEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single two-element string list argument - quoted strings",
        "command [\"string\",\"string\"];",
        {   { CommandStart, "command", false },
            { StringListArgumentStart, Q_NULLPTR, false },
            { StringListEntry, "string", false /*quoted*/ },
            { StringListEntry, "string", false /*quoted*/ },
            { StringListArgumentEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single two-element string list argument - multi-line strings",
        "command [text:\nstring\n.\n,text:\nstring\n.\n];",
        {   { CommandStart, "command", false },
            { StringListArgumentStart, Q_NULLPTR, false },
            { StringListEntry, "string", true /*multiline*/ },
            { StringListEntry, "string", true /*multiline*/ },
            { StringListArgumentEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single two-element string list argument - quoted + multi-line strings",
        "command [\"string\",text:\nstring\n.\n];",
        {   { CommandStart, "command", false },
            { StringListArgumentStart, Q_NULLPTR, false },
            { StringListEntry, "string", false /*quoted*/ },
            { StringListEntry, "string", true /*multiline*/ },
            { StringListArgumentEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single two-element string list argument - multi-line + quoted strings",
        "command [text:\nstring\n.\n,\"string\"];",
        {   { CommandStart, "command", false },
            { StringListArgumentStart, Q_NULLPTR, false },
            { StringListEntry, "string", true /*multiline*/ },
            { StringListEntry, "string", false /*quoted*/ },
            { StringListArgumentEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "single bare test argument",
        "command test;",
        {   { CommandStart, "command", false },
            { TestStart, "test", false },
            { TestEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "one-element test list argument",
        "command(test);",
        {   { CommandStart, "command", false },
            { TestListStart, Q_NULLPTR, false },
            { TestStart, "test", false },
            { TestEnd, Q_NULLPTR, false },
            { TestListEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "two-element test list argument",
        "command(test,test);",
        {   { CommandStart, "command", false },
            { TestListStart, Q_NULLPTR, false },
            { TestStart, "test", false },
            { TestEnd, Q_NULLPTR, false },
            { TestStart, "test", false },
            { TestEnd, Q_NULLPTR, false },
            { TestListEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "zero-element block",
        "command{}",
        {   { CommandStart, "command", false },
            { BlockStart, Q_NULLPTR, false },
            { BlockEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "one-element block",
        "command{command;}",
        {   { CommandStart, "command", false },
            { BlockStart, Q_NULLPTR, false },
            { CommandStart, "command", false },
            { CommandEnd, Q_NULLPTR, false },
            { BlockEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "two-element block",
        "command{command;command;}",
        {   { CommandStart, "command", false },
            { BlockStart, Q_NULLPTR, false },
            { CommandStart, "command", false },
            { CommandEnd, Q_NULLPTR, false },
            { CommandStart, "command", false },
            { CommandEnd, Q_NULLPTR, false },
            { BlockEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

    {
        "command with a test with a test with a test",
        "command test test test;",
        {   { CommandStart, "command", false },
            { TestStart, "test", false },
            { TestStart, "test", false },
            { TestStart, "test", false },
            { TestEnd, Q_NULLPTR, false },
            { TestEnd, Q_NULLPTR, false },
            { TestEnd, Q_NULLPTR, false },
            { CommandEnd, Q_NULLPTR, false },
            { Finished, Q_NULLPTR, false }
        }
    },

};

static const int numTestCases = sizeof testCases / sizeof * testCases;

// Prints out the parse tree in XML-like format. For visual inspection
// (manual tests).
class PrintingScriptBuilder : public KSieve::ScriptBuilder
{
public:
    PrintingScriptBuilder()
        : KSieve::ScriptBuilder(), indent(0)
    {
        write("<script type=\"application/sieve\">");
        ++indent;
    }
    virtual ~PrintingScriptBuilder() {}

    void taggedArgument(const QString &tag) Q_DECL_OVERRIDE {
        write("tag", tag);
    }
    void stringArgument(const QString &string, bool multiLine, const QString & /*fixme*/) Q_DECL_OVERRIDE {
        write(multiLine ? "string type=\"multiline\"" : "string type=\"quoted\"", string);
    }
    void numberArgument(unsigned long number, char quantifier) Q_DECL_OVERRIDE {
        const QString txt = "number" + (quantifier ? QStringLiteral(" quantifier=\"%1\"").arg(quantifier) : QString());
        write(txt.toLatin1(), QString::number(number));
    }
    void commandStart(const QString &identifier, int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        write("<command>");
        ++indent;
        write("identifier", identifier);
    }
    void commandEnd(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        --indent;
        write("</command>");
    }
    void testStart(const QString &identifier) Q_DECL_OVERRIDE {
        write("<test>");
        ++indent;
        write("identifier", identifier);
    }
    void testEnd() Q_DECL_OVERRIDE {
        --indent;
        write("</test>");
    }
    void testListStart() Q_DECL_OVERRIDE {
        write("<testlist>");
        ++indent;
    }
    void testListEnd() Q_DECL_OVERRIDE {
        --indent;
        write("</testlist>");
    }
    void blockStart(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        write("<block>");
        ++indent;
    }
    void blockEnd(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        --indent;
        write("</block>");
    }
    void stringListArgumentStart() Q_DECL_OVERRIDE {
        write("<stringlist>");
        ++indent;
    }
    void stringListArgumentEnd() Q_DECL_OVERRIDE {
        --indent;
        write("</stringlist>");
    }
    void stringListEntry(const QString &string, bool multiline, const QString &hashComment) Q_DECL_OVERRIDE {
        stringArgument(string, multiline, hashComment);
    }
    void hashComment(const QString &comment) Q_DECL_OVERRIDE {
        write("comment type=\"hash\"", comment);
    }
    void bracketComment(const QString &comment) Q_DECL_OVERRIDE {
        write("comment type=\"bracket\"", comment);
    }

    void lineFeed() Q_DECL_OVERRIDE {
        write("<crlf/>");
    }

    void error(const KSieve::Error &error) Q_DECL_OVERRIDE {
        indent = 0;
        write(("Error: " + error.asString()).toLatin1());
    }
    void finished() Q_DECL_OVERRIDE {
        --indent;
        write("</script>");
    }
private:
    int indent;
    void write(const char *msg)
    {
        for (int i = 2 * indent; i > 0; --i) {
            cout << " ";
        }
        cout << msg << endl;
    }
    void write(const QByteArray &key, const QString &value)
    {
        if (value.isEmpty()) {
            write(QByteArray(QByteArray("<") + key + QByteArray("/>")));
            return;
        }
        write(QByteArray(QByteArray("<") + key + QByteArray(">")));
        ++indent;
        write(value.toUtf8().data());
        --indent;
        write(QByteArray(QByteArray("</") + key + QByteArray(">")));
    }
};

// verifes that methods get called with expected arguments (and in
// expected sequence) as specified by the TestCase. For automated
// tests.
class VerifyingScriptBuilder : public KSieve::ScriptBuilder
{
public:
    VerifyingScriptBuilder(const TestCase &testCase)
        : KSieve::ScriptBuilder(),
          mNextResponse(0), mTestCase(testCase), mOk(true)
    {
    }
    virtual ~VerifyingScriptBuilder() {}

    bool ok() const
    {
        return mOk;
    }

    void taggedArgument(const QString &tag) Q_DECL_OVERRIDE {
        checkIs(TaggedArgument);
        checkEquals(tag);
        ++mNextResponse;
    }
    void stringArgument(const QString &string, bool multiline, const QString & /*fixme*/) Q_DECL_OVERRIDE {
        checkIs(StringArgument);
        checkEquals(string);
        checkEquals(multiline);
        ++mNextResponse;
    }
    void numberArgument(unsigned long number, char quantifier) Q_DECL_OVERRIDE {
        checkIs(NumberArgument);
        checkEquals(QString::number(number) + (quantifier ? quantifier : ' '));
        ++mNextResponse;
    }
    void commandStart(const QString &identifier, int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        checkIs(CommandStart);
        checkEquals(identifier);
        ++mNextResponse;
    }
    void commandEnd(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        checkIs(CommandEnd);
        ++mNextResponse;
    }
    void testStart(const QString &identifier) Q_DECL_OVERRIDE {
        checkIs(TestStart);
        checkEquals(identifier);
        ++mNextResponse;
    }
    void testEnd() Q_DECL_OVERRIDE {
        checkIs(TestEnd);
        ++mNextResponse;
    }
    void testListStart() Q_DECL_OVERRIDE {
        checkIs(TestListStart);
        ++mNextResponse;
    }
    void testListEnd() Q_DECL_OVERRIDE {
        checkIs(TestListEnd);
        ++mNextResponse;
    }
    void blockStart(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        checkIs(BlockStart);
        ++mNextResponse;
    }
    void blockEnd(int lineNumber) Q_DECL_OVERRIDE {
        Q_UNUSED(lineNumber);
        checkIs(BlockEnd);
        ++mNextResponse;
    }
    void stringListArgumentStart() Q_DECL_OVERRIDE {
        checkIs(StringListArgumentStart);
        ++mNextResponse;
    }
    void stringListEntry(const QString &string, bool multiLine, const QString & /*fixme*/) Q_DECL_OVERRIDE {
        checkIs(StringListEntry);
        checkEquals(string);
        checkEquals(multiLine);
        ++mNextResponse;
    }
    void stringListArgumentEnd() Q_DECL_OVERRIDE {
        checkIs(StringListArgumentEnd);
        ++mNextResponse;
    }
    void hashComment(const QString &comment) Q_DECL_OVERRIDE {
        checkIs(HashComment);
        checkEquals(comment);
        ++mNextResponse;
    }
    void bracketComment(const QString &comment) Q_DECL_OVERRIDE {
        checkIs(BracketComment);
        checkEquals(comment);
        ++mNextResponse;
    }
    void lineFeed() Q_DECL_OVERRIDE {
        // FIXME
    }
    void error(const KSieve::Error &error) Q_DECL_OVERRIDE {
        checkIs(Error);
        checkEquals(QString(KSieve::Error::typeToString(error.type())));
        ++mNextResponse;
    }
    void finished() Q_DECL_OVERRIDE {
        checkIs(Finished);
        //++mNextResponse (no!)
    }

private:
    const TestCase::Response &currentResponse() const
    {
        assert(mNextResponse <= MAX_RESPONSES);
        return mTestCase.responses[mNextResponse];
    }

    void checkIs(BuilderMethod m)
    {
        if (currentResponse().method != m) {
            cerr << " expected method " << (int)currentResponse().method
                 << ", got " << (int)m;
            mOk = false;
        }
    }

    void checkEquals(const QString &s)
    {
        if (s != QString::fromUtf8(currentResponse().string)) {
            cerr << " expected string arg \""
                 << (currentResponse().string ? currentResponse().string : "<null>")
                 << "\", got \"" << (s.isNull() ? "<null>" : s.toUtf8().data()) << "\"";
            mOk = false;
        }
    }
    void checkEquals(bool b)
    {
        if (b != currentResponse().boolean) {
            cerr << " expected boolean arg <" << currentResponse().boolean
                 << ">, got <" << b << ">";
            mOk = false;
        }
    }

    unsigned int mNextResponse;
    const TestCase &mTestCase;
    bool mOk;
};

int main(int argc, char *argv[])
{

    if (argc == 2) {   // manual test

        const char *scursor = argv[1];
        const char *const send = argv[1] + qstrlen(argv[1]);

        Parser parser(scursor, send);
        PrintingScriptBuilder psb;
        parser.setScriptBuilder(&psb);
        if (parser.parse()) {
            cout << "ok" << endl;
        } else {
            cout << "bad" << endl;
        }

    } else if (argc == 1) {   // automated test
        bool success = true;
        for (int i = 0; i < numTestCases; ++i) {
            const TestCase &t = testCases[i];
            cerr << t.name << ":";
            VerifyingScriptBuilder v(t);
            Parser p(t.script, t.script + qstrlen(t.script));
            p.setScriptBuilder(&v);
            const bool ok = p.parse();
            if (v.ok())
                if (ok) {
                    cerr << " ok";
                } else {
                    cerr << " xfail";
                }
            else {
                success = false;
            }
            cerr << endl;
        }
        if (!success) {
            exit(1);
        }

    } else { // usage error
        cerr << "usage: parsertest [ <string> ]" << endl;
        exit(1);
    }

    return 0;
}
