/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <kmime/kmime_header_parsing.h>
#include <ksieve/error.h>
#include <ksieve/parser.h>
#include <ksieve/scriptbuilder.h>

#include <QDebug>
#include <cassert>
#include <limits.h>
#include <map>
#include <set>
#include <vector>

namespace KSieveExt {

class MultiScriptBuilder : public KSieve::ScriptBuilder {
    std::vector<KSieve::ScriptBuilder*> mBuilders;
public:
    MultiScriptBuilder() : KSieve::ScriptBuilder() {}
    MultiScriptBuilder( KSieve::ScriptBuilder * sb1 )
        : KSieve::ScriptBuilder(), mBuilders( 1 )
    {
        mBuilders[0] = sb1;
        assert( sb1 );
    }
    MultiScriptBuilder( KSieve::ScriptBuilder * sb1,
                        KSieve::ScriptBuilder * sb2 )
        : KSieve::ScriptBuilder(), mBuilders( 2 )
    {
        mBuilders[0] = sb1;
        mBuilders[1] = sb2;
        assert( sb1 ); assert( sb2 );
    }
    MultiScriptBuilder( KSieve::ScriptBuilder * sb1,
                        KSieve::ScriptBuilder * sb2,
                        KSieve::ScriptBuilder * sb3 )
        : KSieve::ScriptBuilder(), mBuilders( 3 )
    {
        mBuilders[0] = sb1;
        mBuilders[1] = sb2;
        mBuilders[2] = sb3;
        assert( sb1 ); assert( sb2 ); assert( sb3 );
    }
    ~MultiScriptBuilder() {}
private:
#ifdef FOREACH
#undef FOREACH
#endif
#define FOREACH for ( std::vector<KSieve::ScriptBuilder*>::const_iterator it = mBuilders.begin(), end = mBuilders.end() ; it != end ; ++it ) (*it)->
    void commandStart( const QString & identifier ) { FOREACH commandStart( identifier ); }
    void commandEnd() { FOREACH commandEnd(); }
    void testStart( const QString & identifier ) { FOREACH testStart( identifier ); }
    void testEnd() { FOREACH testEnd(); }
    void testListStart() { FOREACH testListStart(); }
    void testListEnd() { FOREACH testListEnd(); }
    void blockStart() { FOREACH blockStart(); }
    void blockEnd() { FOREACH blockEnd(); }
    void hashComment( const QString & comment ) { FOREACH hashComment( comment ); }
    void bracketComment( const QString & comment ) { FOREACH bracketComment( comment ); }
    void lineFeed() { FOREACH lineFeed(); }
    void error( const KSieve::Error & e ) { FOREACH error( e ); }
    void finished() { FOREACH finished(); }
    void taggedArgument( const QString & tag ) { FOREACH taggedArgument( tag ); }
    void stringArgument( const QString & string, bool multiline, const QString & fixme ) { FOREACH stringArgument( string, multiline, fixme ); }
    void numberArgument( unsigned long number, char quantifier ) { FOREACH numberArgument( number, quantifier ); }
    void stringListArgumentStart() { FOREACH stringListArgumentStart(); }
    void stringListEntry( const QString & string, bool multiline, const QString & fixme) { FOREACH stringListEntry( string, multiline, fixme ); }
    void stringListArgumentEnd() { FOREACH stringListArgumentEnd(); }
#undef FOREACH
};

}

namespace KSieveUi {

class GenericInformationExtractor : public KSieve::ScriptBuilder {
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
        const char * string;
        // actions:
        int if_found;
        int if_not_found;
        const char * save_tag;
    };

    const std::vector<StateNode> mNodes;
    std::map<QString,QString> mResults;
    std::set<unsigned int> mRecursionGuard;
    unsigned int mState;
    int mNestingDepth;

public:
    GenericInformationExtractor( const std::vector<StateNode> & nodes )
        : KSieve::ScriptBuilder(), mNodes( nodes ), mState( 0 ), mNestingDepth( 0 ) {}

    const std::map<QString,QString> & results() const { return mResults; }

private:
    void process( BuilderMethod method, const QString & string=QString() ) {
        doProcess( method, string );
        mRecursionGuard.clear();
    }
    void doProcess( BuilderMethod method, const QString & string ) {
        mRecursionGuard.insert( mState );
        bool found = true;
        const StateNode & expected = mNodes[mState];
        if ( expected.depth != -1 && mNestingDepth != expected.depth )
            found = false;
        if ( expected.method != Any && method != expected.method )
            found = false;
        if ( const char * str = expected.string )
            if ( string.toLower() != QString::fromUtf8( str ).toLower() )
                found = false;
        qDebug() << ( found ?"found:" :"not found:" )
                 << mState << "->"
                 << ( found ? expected.if_found : expected.if_not_found );
        mState = found ? expected.if_found : expected.if_not_found ;
        assert( mState < mNodes.size() );
        if ( found )
            if ( const char * save_tag = expected.save_tag )
                mResults[QString::fromLatin1(save_tag)] = string;
        if ( !found && !mRecursionGuard.count( mState ) ) {
            doProcess( method, string );
        }
    }
    void commandStart( const QString & identifier ) { qDebug() ; process( CommandStart, identifier ); }
    void commandEnd() { qDebug() ; process( CommandEnd ); }
    void testStart( const QString & identifier ) { qDebug() ; process( TestStart, identifier ); }
    void testEnd() { qDebug() ; process( TestEnd ); }
    void testListStart() { qDebug() ; process( TestListStart ); }
    void testListEnd() { qDebug() ; process( TestListEnd ); }
    void blockStart() { qDebug() ; process( BlockStart ); ++mNestingDepth; }
    void blockEnd() { qDebug() ; --mNestingDepth; process( BlockEnd ); }
    void hashComment( const QString & ) { qDebug() ; }
    void bracketComment( const QString & ) { qDebug() ; }
    void lineFeed() { qDebug() ; }
    void error( const KSieve::Error & ) {
        qDebug() ;
        mState = 0;
    }
    void finished() { qDebug() ; }

    void taggedArgument( const QString & tag ) { qDebug() ; process( TaggedArgument, tag ); }
    void stringArgument( const QString & string, bool, const QString & ) { qDebug() ; process( StringArgument, string ); }
    void numberArgument( unsigned long number, char ) { qDebug(); process( NumberArgument, QString::number( number ) ); }
    void stringListArgumentStart() { qDebug() ; process( StringListArgumentStart ); }
    void stringListEntry( const QString & string, bool, const QString & ) { qDebug() ; process( StringListEntry, string ); }
    void stringListArgumentEnd() { qDebug() ; process( StringListArgumentEnd ); }
};

typedef GenericInformationExtractor GIE;
static const GenericInformationExtractor::StateNode spamNodes[] = {
    { 0, GIE::CommandStart, "if",  1, 0, 0 },              // 0
    { 0,   GIE::TestStart, "header", 2, 0, 0 },            // 1
    { 0,     GIE::TaggedArgument, "contains", 3, 0, 0 },   // 2

    // accept both string and string-list:
    { 0,     GIE::StringArgument, "x-spam-flag", 9, 4, "x-spam-flag" },    // 3
    { 0,     GIE::StringListArgumentStart, 0, 5, 0, 0 },                   // 4
    { 0,       GIE::StringListEntry, "x-spam-flag", 6, 7, "x-spam-flag" }, // 5
    { 0,       GIE::StringListEntry, 0, 6, 8, 0 },                         // 6
    { 0,     GIE::StringListArgumentEnd, 0, 0, 5, 0 },                     // 7
    { 0,     GIE::StringListArgumentEnd, 0, 9, 0, 0 },                     // 8

    // accept both string and string-list:
    { 0,     GIE::StringArgument, "yes", 15, 10, "spam-flag-yes" },    // 9
    { 0,     GIE::StringListArgumentStart, 0, 11, 0, 0 },              // 10
    { 0,       GIE::StringListEntry, "yes", 12, 13, "spam-flag-yes" }, // 11
    { 0,       GIE::StringListEntry, 0, 12, 14, 0 },                   // 12
    { 0,     GIE::StringListArgumentEnd, 0, 0, 11, 0 },                // 13
    { 0,     GIE::StringListArgumentEnd, 0, 15, 0, 0 },                // 14

    { 0,   GIE::TestEnd, 0, 16, 0, 0 }, // 15

    // block of command, find "stop", take nested if's into account:
    { 0,   GIE::BlockStart, 0, 17, 0, 0 },                // 16
    { 1,     GIE::CommandStart, "stop", 20, 19, "stop" }, // 17
    { -1,    GIE::Any, 0, 17, 0, 0 },                     // 18
    { 0,   GIE::BlockEnd, 0, 0, 18, 0 },                  // 19

    { -1, GIE::Any, 0, 20, 20, 0 }, // 20 end state
};
static const unsigned int numSpamNodes = sizeof spamNodes / sizeof *spamNodes ;

class SpamDataExtractor : public GenericInformationExtractor {
public:
    SpamDataExtractor()
        : GenericInformationExtractor( std::vector<StateNode>( spamNodes, spamNodes + numSpamNodes ) )
    {

    }

    bool found() const {
        return mResults.count( QLatin1String("x-spam-flag") ) &&
                mResults.count( QLatin1String("spam-flag-yes") ) &&
                mResults.count( QLatin1String("stop") ) ;
    }
};

// to understand this table, study the output of
// libksieve/tests/parsertest
//   'if not address :domain :contains ["from"] ["mydomain.org"] { keep; stop; }'
static const GenericInformationExtractor::StateNode domainNodes[] = {
    { 0, GIE::CommandStart, "if", 1, 0, 0 },       // 0
    { 0,   GIE::TestStart, "not", 2, 0, 0, },      // 1
    { 0,     GIE::TestStart, "address", 3, 0, 0 }, // 2

    // :domain and :contains in arbitrary order:
    { 0,       GIE::TaggedArgument, "domain", 4, 5, 0 },     // 3
    { 0,       GIE::TaggedArgument, "contains", 7, 0, 0 },   // 4
    { 0,       GIE::TaggedArgument, "contains", 6, 0, 0 },   // 5
    { 0,       GIE::TaggedArgument, "domain", 7, 0, 0 },     // 6

    // accept both string and string-list:
    { 0,       GIE::StringArgument, "from", 13, 8, "from" },     // 7
    { 0,       GIE::StringListArgumentStart, 0, 9, 0, 0 },       // 8
    { 0,         GIE::StringListEntry, "from", 10, 11, "from" }, // 9
    { 0,         GIE::StringListEntry, 0, 10, 12, 0 },           // 10
    { 0,       GIE::StringListArgumentEnd, 0, 0, 9, 0 },         // 11
    { 0,       GIE::StringListArgumentEnd, 0, 13, 0, 0 },        // 12

    // string: save, string-list: save last
    { 0,       GIE::StringArgument, 0, 17, 14, "domainName" },    // 13
    { 0,       GIE::StringListArgumentStart, 0, 15, 0, 0 },       // 14
    { 0,         GIE::StringListEntry, 0, 15, 16, "domainName" }, // 15
    { 0,       GIE::StringListArgumentEnd, 0, 17, 0, 0 },         // 16

    { 0,     GIE::TestEnd, 0, 18, 0, 0 },  // 17
    { 0,   GIE::TestEnd, 0, 19, 0, 0 },    // 18

    // block of commands, find "stop", take nested if's into account:
    { 0,   GIE::BlockStart, 0, 20, 0, 0 },                 // 19
    { 1,     GIE::CommandStart, "stop", 23, 22, "stop" },  // 20
    { -1,    GIE::Any, 0, 20, 0, 0 },                      // 21
    { 0,   GIE::BlockEnd, 0, 0, 21, 0 },                   // 22

    { -1, GIE::Any, 0, 23, 23, 0 }  // 23 end state
};
static const unsigned int numDomainNodes = sizeof domainNodes / sizeof *domainNodes ;

class DomainRestrictionDataExtractor : public GenericInformationExtractor {
public:
    DomainRestrictionDataExtractor()
        : GenericInformationExtractor( std::vector<StateNode>( domainNodes, domainNodes+numDomainNodes ) )
    {

    }

    QString domainName() /*not const, since map::op[] isn't const*/ {
        return mResults.count( QLatin1String("stop") ) && mResults.count( QLatin1String("from") )
                ? mResults[QLatin1String("domainName")] : QString();
    }
};

class VacationDataExtractor : public KSieve::ScriptBuilder {
    enum Context {
        None = 0,
        // command itself:
        VacationCommand,
        // tagged args:
        Days, Addresses
    };
public:
    VacationDataExtractor();
    virtual ~VacationDataExtractor();

    int notificationInterval() const { return mNotificationInterval; }
    const QString & messageText() const { return mMessageText; }
    const QStringList & aliases() const { return mAliases; }

private:
    void commandStart( const QString & identifier );

    void commandEnd();

    void testStart( const QString & ) {}
    void testEnd() {}
    void testListStart() {}
    void testListEnd() {}
    void blockStart() {}
    void blockEnd() {}
    void hashComment( const QString & ) {}
    void bracketComment( const QString & ) {}
    void lineFeed() {}
    void error( const KSieve::Error & e );
    void finished();

    void taggedArgument( const QString & tag );

    void stringArgument( const QString & string, bool, const QString & );

    void numberArgument( unsigned long number, char );

    void stringListArgumentStart();
    void stringListEntry( const QString & string, bool, const QString & );
    void stringListArgumentEnd();

private:
    Context mContext;
    int mNotificationInterval;
    QString mMessageText;
    QStringList mAliases;

    void reset();
};

}


#endif // VACATIONSCRIPTEXTRACTOR_H
