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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
#include <config.h>
#include <ksieve/parser.h>
using KSieve::Parser;

#include <ksieve/error.h>
#include <ksieve/scriptbuilder.h>

#include <qcstring.h> // qstrlen
#include <qstring.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

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
  const char * name;
  const char * script;
  struct Response {
    BuilderMethod method;
    const char * string;
    bool boolean;
  } responses[MAX_RESPONSES];
} testCases[] = {

  //
  // single commands:
  //

  { "Null script",
    0,
    { { Finished, 0, false } }
  },

  { "Empty script",
    "",
    { { Finished, 0, false } }
  },

  { "WS-only script",
    " \t\n\r\n",
    { { Finished, 0, false } }
  },

  { "Bare hash comment",
    "#comment",
    { { HashComment, "comment", false },
      { Finished, 0, false } }
  },

  { "Bare bracket comment",
    "/*comment*/",
    { { BracketComment, "comment", false },
      { Finished, 0, false } }
  },

  { "Bare command",
    "command;",
    { { CommandStart, "command", false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "Bare command - missing semicolon",
    "command",
    { { CommandStart, "command", false },
      { Error, "MissingSemicolonOrBlock", false } }
  },

  { "surrounded by bracket comments",
    "/*comment*/command/*comment*/;/*comment*/",
    { { BracketComment, "comment", false },
      { CommandStart, "command", false },
      { BracketComment, "comment", false },
      { CommandEnd, 0, false },
      { BracketComment, "comment", false },
      { Finished, 0, false } }
  },

  { "surrounded by hash comments",
    "#comment\ncommand#comment\n;#comment",
    { { HashComment, "comment", false },
      { CommandStart, "command", false },
      { HashComment, "comment", false },
      { CommandEnd, 0, false },
      { HashComment, "comment", false },
      { Finished, 0, false } }
  },

  { "single tagged argument",
    "command :tag;",
    { { CommandStart, "command", false },
      { TaggedArgument, "tag", false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single tagged argument - missing semicolon",
    "command :tag",
    { { CommandStart, "command", false },
      { TaggedArgument, "tag", false },
      { Error, "MissingSemicolonOrBlock", false } }
  },

  { "single string argument - quoted string",
    "command \"string\";",
    { { CommandStart, "command", false },
      { StringArgument, "string", false /*quoted*/ },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single string argument - multi-line string",
    "command text:\nstring\n.\n;",
    { { CommandStart, "command", false },
      { StringArgument, "string", true /*multiline*/ },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single number argument - 100",
    "command 100;",
    { { CommandStart, "command", false },
      { NumberArgument, "100 ", false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single number argument - 100k",
    "command 100k;",
    { { CommandStart, "command", false },
      { NumberArgument, "102400k", false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single number argument - 100M",
    "command 100M;",
    { { CommandStart, "command", false },
      { NumberArgument, "104857600M", false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single number argument - 2G",
    "command 2G;",
    { { CommandStart, "command", false },
      { NumberArgument, "2147483648G", false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

#if SIZEOF_UNSIGNED_LONG == 8
#  define ULONG_MAX_STRING "9223372036854775807"
#  define ULONG_MAXP1_STRING "9223372036854775808"
#elif SIZEOF_UNSIGNED_LONG == 4
#  define ULONG_MAX_STRING "4294967295"
#  define ULONG_MAXP1_STRING "4G"
#else
#  error sizeof( unsigned long ) != 4 && sizeof( unsigned long ) != 8 ???
#endif

  { "single number argument - ULONG_MAX + 1",
    "command " ULONG_MAXP1_STRING ";",
    { { CommandStart, "command", false },
      { Error, "NumberOutOfRange", false } }
  },

  { "single number argument - ULONG_MAX",
    "command " ULONG_MAX_STRING ";",
    { { CommandStart, "command", false },
      { NumberArgument, ULONG_MAX_STRING " ", false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single one-element string list argument - quoted string",
    "command [\"string\"];",
    { { CommandStart, "command", false },
      { StringListArgumentStart, 0, false },
      { StringListEntry, "string", false /*quoted*/ },
      { StringListArgumentEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single one-element string list argument - multi-line string",
    "command [text:\nstring\n.\n];",
    { { CommandStart, "command", false },
      { StringListArgumentStart, 0, false },
      { StringListEntry, "string", true /*multiline*/ },
      { StringListArgumentEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single two-element string list argument - quoted strings",
    "command [\"string\",\"string\"];",
    { { CommandStart, "command", false },
      { StringListArgumentStart, 0, false },
      { StringListEntry, "string", false /*quoted*/ },
      { StringListEntry, "string", false /*quoted*/ },
      { StringListArgumentEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single two-element string list argument - multi-line strings",
    "command [text:\nstring\n.\n,text:\nstring\n.\n];",
    { { CommandStart, "command", false },
      { StringListArgumentStart, 0, false },
      { StringListEntry, "string", true /*multiline*/ },
      { StringListEntry, "string", true /*multiline*/ },
      { StringListArgumentEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single two-element string list argument - quoted + multi-line strings",
    "command [\"string\",text:\nstring\n.\n];",
    { { CommandStart, "command", false },
      { StringListArgumentStart, 0, false },
      { StringListEntry, "string", false /*quoted*/ },
      { StringListEntry, "string", true /*multiline*/ },
      { StringListArgumentEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single two-element string list argument - multi-line + quoted strings",
    "command [text:\nstring\n.\n,\"string\"];",
    { { CommandStart, "command", false },
      { StringListArgumentStart, 0, false },
      { StringListEntry, "string", true /*multiline*/ },
      { StringListEntry, "string", false /*quoted*/ },
      { StringListArgumentEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "single bare test argument",
    "command test;",
    { { CommandStart, "command", false },
      { TestStart, "test", false },
      { TestEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "one-element test list argument",
    "command(test);",
    { { CommandStart, "command", false },
      { TestListStart, 0, false },
      { TestStart, "test", false },
      { TestEnd, 0, false },
      { TestListEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "two-element test list argument",
    "command(test,test);",
    { { CommandStart, "command", false },
      { TestListStart, 0, false },
      { TestStart, "test", false },
      { TestEnd, 0, false },
      { TestStart, "test", false },
      { TestEnd, 0, false },
      { TestListEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "zero-element block",
    "command{}",
    { { CommandStart, "command", false },
      { BlockStart, 0, false },
      { BlockEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "one-element block",
    "command{command;}",
    { { CommandStart, "command", false },
      { BlockStart, 0, false },
      { CommandStart, "command", false },
      { CommandEnd, 0, false },
      { BlockEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "two-element block",
    "command{command;command;}",
    { { CommandStart, "command", false },
      { BlockStart, 0, false },
      { CommandStart, "command", false },
      { CommandEnd, 0, false },
      { CommandStart, "command", false },
      { CommandEnd, 0, false },
      { BlockEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

  { "command with a test with a test with a test",
    "command test test test;",
    { { CommandStart, "command", false },
      { TestStart, "test", false },
      { TestStart, "test", false },
      { TestStart, "test", false },
      { TestEnd, 0, false },
      { TestEnd, 0, false },
      { TestEnd, 0, false },
      { CommandEnd, 0, false },
      { Finished, 0, false } }
  },

};

static const int numTestCases = sizeof testCases / sizeof *testCases ;

// Prints out the parse tree in XML-like format. For visual inspection
// (manual tests).
class PrintingScriptBuilder : public KSieve::ScriptBuilder {
public:
  PrintingScriptBuilder()
    : KSieve::ScriptBuilder(), indent( 0 )
  {
    write( "<script type=\"application/sieve\">" );
    ++indent;
  }
  virtual ~PrintingScriptBuilder() {}

  void taggedArgument( const QString & tag ) {
    write( "tag", tag );
  }
  void stringArgument( const QString & string, bool multiLine, const QString & /*fixme*/ ) {
    write( multiLine ? "string type=\"multiline\"" : "string type=\"quoted\"", string );
  }
  void numberArgument( unsigned long number, char quantifier ) {
    const QString txt = "number" + ( quantifier ? QString(" quantifier=\"%1\"").arg( quantifier ) : QString::null ) ;
    write( txt.latin1(), QString::number( number ) );
  }
  void commandStart( const QString & identifier ) {
    write( "<command>" );
    ++indent;
    write( "identifier", identifier );
  }
  void commandEnd() {
    --indent;
    write( "</command>" );
  }
  void testStart( const QString & identifier ) {
    write( "<test>" );
    ++indent;
    write( "identifier", identifier );
  }
  void testEnd() {
    --indent;
    write( "</test>" );
  }
  void testListStart() {
    write( "<testlist>" );
    ++indent;
  }
  void testListEnd() {
    --indent;
    write( "</testlist>" );
  }
  void blockStart() {
    write( "<block>" );
    ++indent;
  }
  void blockEnd() {
    --indent;
    write( "</block>" );
  }
  void stringListArgumentStart() {
    write( "<stringlist>" );
    ++indent;
  }
  void stringListArgumentEnd() {
    --indent;
    write( "</stringlist>" );
  }
  void stringListEntry( const QString & string, bool multiline, const QString & hashComment ) {
    stringArgument( string, multiline, hashComment );
  }
  void hashComment( const QString & comment ) {
    write( "comment type=\"hash\"", comment );
  }
  void bracketComment( const QString & comment ) {
    write( "comment type=\"bracket\"", comment );
  }

  void lineFeed() {
    write( "<crlf/>" );
  }

  void error( const KSieve::Error & error ) {
    indent = 0;
    write( ("Error: " + error.asString()).latin1() );
  }
  void finished() {
    --indent;
    write( "</script>" );
  }
private:
  int indent;
  void write( const char * msg ) {
    for ( int i = 2*indent ; i > 0 ; --i )
      cout << " ";
    cout << msg << endl;
  }
  void write( const QCString & key, const QString & value ) {
    if ( value.isEmpty() ) {
      write( "<" + key + "/>" );
      return;
    }
    write( "<" + key + ">" );
    ++indent;
    write( value.utf8().data() );
    --indent;
    write( "</" + key + ">" );
  }
};


// verifes that methods get called with expected arguments (and in
// expected sequence) as specified by the TestCase. For automated
// tests.
class VerifyingScriptBuilder : public KSieve::ScriptBuilder {
public:
  VerifyingScriptBuilder( const TestCase & testCase )
    : KSieve::ScriptBuilder(),
      mNextResponse( 0 ), mTestCase( testCase ), mOk( true )
  {
  }
  virtual ~VerifyingScriptBuilder() {}

  bool ok() const { return mOk; }

  void taggedArgument( const QString & tag ) {
    checkIs( TaggedArgument );
    checkEquals( tag );
    ++mNextResponse;
  }
  void stringArgument( const QString & string, bool multiline, const QString & /*fixme*/ ) {
    checkIs( StringArgument );
    checkEquals( string );
    checkEquals( multiline );
    ++mNextResponse;
  }
  void numberArgument( unsigned long number, char quantifier ) {
    checkIs( NumberArgument );
    checkEquals( QString::number( number ) + ( quantifier ? quantifier : ' ' ) );
    ++mNextResponse;
  }
  void commandStart( const QString & identifier ) {
    checkIs( CommandStart );
    checkEquals( identifier );
    ++mNextResponse;
  }
  void commandEnd() {
    checkIs( CommandEnd );
    ++mNextResponse;
  }
  void testStart( const QString & identifier ) {
    checkIs( TestStart );
    checkEquals( identifier );
    ++mNextResponse;
  }
  void testEnd() {
    checkIs( TestEnd );
    ++mNextResponse;
  }
  void testListStart() {
    checkIs( TestListStart );
    ++mNextResponse;
  }
  void testListEnd() {
    checkIs( TestListEnd );
    ++mNextResponse;
  }
  void blockStart() {
    checkIs( BlockStart );
    ++mNextResponse;
  }
  void blockEnd() {
    checkIs( BlockEnd );
    ++mNextResponse;
  }
  void stringListArgumentStart() {
    checkIs( StringListArgumentStart );
    ++mNextResponse;
  }
  void stringListEntry( const QString & string, bool multiLine, const QString & /*fixme*/ ) {
    checkIs( StringListEntry );
    checkEquals( string );
    checkEquals( multiLine );
    ++mNextResponse;
  }
  void stringListArgumentEnd() {
    checkIs( StringListArgumentEnd );
    ++mNextResponse;
  }
  void hashComment( const QString & comment ) {
    checkIs( HashComment );
    checkEquals( comment );
    ++mNextResponse;
  }
  void bracketComment( const QString & comment ) {
    checkIs( BracketComment );
    checkEquals( comment );
    ++mNextResponse;
  }
  void lineFeed() {
    // FIXME
  }
  void error( const KSieve::Error & error ) {
    checkIs( Error );
    checkEquals( QString( KSieve::Error::typeToString( error.type() ) ) );
    ++mNextResponse;
  }
  void finished() {
    checkIs( Finished );
    //++mNextResponse (no!)
  }

private:
  const TestCase::Response & currentResponse() const {
    assert( mNextResponse <= MAX_RESPONSES );
    return mTestCase.responses[mNextResponse];
  }

  void checkIs( BuilderMethod m ) {
    if ( currentResponse().method != m ) {
      cerr << " expected method " << (int)currentResponse().method
	   << ", got " << (int)m;
      mOk = false;
    }
  }

  void checkEquals( const QString & s ) {
    if ( s != QString::fromUtf8( currentResponse().string ) ) {
      cerr << " expected string arg \""
	   << ( currentResponse().string ? currentResponse().string : "<null>" )
	   << "\", got \"" << ( s.isNull() ? "<null>" : s.utf8().data() ) << "\"";
      mOk = false;
    }
  }
  void checkEquals( bool b ) {
    if ( b != currentResponse().boolean ) {
      cerr << " expected boolean arg <" << currentResponse().boolean
	   << ">, got <" << b << ">";
      mOk = false;
    }
  }

  unsigned int mNextResponse;
  const TestCase & mTestCase;
  bool mOk;
};


int main( int argc, char * argv[]  ) {

  if ( argc == 2 ) { // manual test

    const char * scursor = argv[1];
    const char * const send = argv[1] + qstrlen( argv[1] );

    Parser parser( scursor, send );
    PrintingScriptBuilder psb;
    parser.setScriptBuilder( &psb );
    if ( parser.parse() )
      cout << "ok" << endl;
    else
      cout << "bad" << endl;


  } else if ( argc == 1 ) { // automated test
    bool success = true;
    for ( int i = 0 ; i < numTestCases ; ++i ) {
      const TestCase & t = testCases[i];
      cerr << t.name << ":";
      VerifyingScriptBuilder v( t );
      Parser p( t.script, t.script + qstrlen( t.script ) );
      p.setScriptBuilder( &v );
      const bool ok = p.parse();
      if ( v.ok() )
	if ( ok )
	  cerr << " ok";
	else 
	  cerr << " xfail";
      else
	success = false;
      cerr << endl;
    }
    if ( !success )
      exit( 1 );

  } else { // usage error
    cerr << "usage: parsertest [ <string> ]" << endl;
    exit( 1 );
  }

  return 0;
}
