
#include <../kmime_headers.h>
#include <../kmime_header_parsing.h>

#include <kinstance.h>

#include <qfile.h>
#include <qcstring.h>
//#include <qstring.h>

//#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <cassert>

#define _GNU_SOURCE
#include <getopt.h>

using namespace KMime::HeaderParsing;
using namespace std;

static const char * tokenTypes[] = {
  "encoded-word",
  "atom",
  "token",
  "quoted-string",
  "domain-literal",
  "comment",
  "phrase",
  "dot-atom",
  "domain",
  "obs-route",
  "addr-spec",
  "angle-addr",
  "mailbox",
  "group",
  "address",
  "address-list",
  "parameter",
  "raw-parameter-list",
  "parameter-list",
  "time",
  "date-time"
};
static const int tokenTypesLen = sizeof tokenTypes / sizeof *tokenTypes;

void usage( const char * msg=0 ) {
  if ( msg && *msg )
    cerr << msg << endl;
  cerr <<
    "usage: test_kmime_header_parsing "
    "(--token <tokentype>|--headerfield <fieldtype>|--header)\n"
    "\n"
    "  --token <tokentype>       interpret input as <tokentype> and output\n"
    "  (-t)                      in parsed form. Currently defined values of\n"
    "                            <tokentype> are:" << endl;
  for ( int i = 0 ; i < tokenTypesLen ; ++i )
    cerr << "                               " << tokenTypes[i] << endl;
  cerr << "\n"
    "  --headerfield <fieldtype> interpret input as header field <fieldtype>\n"
    "  (-f)                      and output in parsed form.\n"
    "\n"
    "  --header                  parse an RFC2822 header. Iterates over all\n"
    "  (-h)                      header fields and outputs them in parsed form." << endl;
  exit(1);
}

ostream & operator<<( ostream & stream, const QString & str ) {
  return stream << str.utf8().data();
}

int main( int argc, char * argv[] ) {
  if ( argc == 1 || argc > 3 ) usage();
  //
  // process options:
  //
  enum { None, Token, HeaderField, Header } action = None;
  const char * argument = 0;
  bool withCRLF = false;
  while( true ) {
    int option_index = 0;
    static const struct option long_options[] = {
      // actions:
      { "token", 1, 0, 't' },
      { "headerfield", 1, 0, 'f' },
      { "header", 0, 0, 'h' },
      { "crlf", 0, 0, 'c' },
      { 0, 0, 0, 0 }
    };

    int c = getopt_long( argc, argv, "cf:ht:", long_options, &option_index );
    if ( c == -1 ) break;

    switch ( c ) {
    case 'c': // --crlf
      withCRLF = true;
      break;
    case 't': // --token <tokentype>
      action = Token;
      argument = optarg;
      break;
    case 'f': // --headerfield <headertype>
      usage( "--headerfield is not yet implemented!" );
      break;
    case 'h': // --header
      usage( "--header is not yet implemented!" );
      break;
    default:
      usage( "unknown option encountered!" );
    }
  }

  if ( optind < argc ) usage( "non-option argument encountered!" );

  assert( action == Token );

  int index;
  for ( index = 0 ; index < tokenTypesLen ; ++index )
    if ( !qstricmp( tokenTypes[index], argument ) ) break;

  if ( index >= tokenTypesLen ) usage( "unknown token type" );

  KInstance instance( "test_kmime_header_parsing" );

  QFile stdIn;
  stdIn.open( IO_ReadOnly, stdin );
  const QByteArray indata = stdIn.readAll();
  stdIn.close();
  QByteArray::ConstIterator iit = indata.begin();
  const QByteArray::ConstIterator iend = indata.end();

  switch ( index ) {
  case 0:
    { // encoded-word 
      QString result;
      QCString language;
      // must have checked for initial '=' already:
      bool ok = indata.size() >= 1 && *iit++ == '=' &&
	parseEncodedWord( iit, iend, result, language );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl
	   << "language:\n" << language.data() << endl;
    }
    break;
  case 1:
    { // atom
      QString result = "with 8bit: ";
      bool ok = parseAtom( iit, iend, result, true );
      
      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;

      result = "without 8bit: ";
#ifdef COMPILE_FAIL
      ok = parseAtom( indata.begin(), iend, result, false );
#else
      iit = indata.begin();
      ok = parseAtom( iit, iend, result, false );
#endif

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 2:
    { // token
      QString result = "with 8bit: ";
      bool ok = parseToken( iit, iend, result, true );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;

      result = "without 8bit: ";
#ifdef COMPILE_FAIL
      ok = parseToken( indata.begin(), iend, result, false );
#else
      iit = indata.begin();
      ok = parseToken( iit, iend, result, false );
#endif

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 3:
    { // quoted-string
      QString result;
      // must have checked for initial '"' already:
      bool ok = *iit++ == '"' &&
	parseGenericQuotedString( iit, iend, result, withCRLF, '"', '"' );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 4:
    { // domain-literal
      QString result;
      // must have checked for initial '[' already:
      bool ok = *iit++ == '[' &&
	parseGenericQuotedString( iit, iend, result, withCRLF, '[', ']' );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 5:
    { // comment
      QString result;
      // must have checked for initial '(' already:
      bool ok = *iit++ == '(' &&
	parseComment( iit, iend, result, withCRLF, true );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 6:
    { // phrase
      QString result;
      bool ok = parsePhrase( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 7:
    { // dot-atom
      QString result;
      bool ok = parseDotAtom( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 8:
    { // domain
      QString result;
      bool ok = parseDomain( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result:\n" << result << endl;
    }
    break;
  case 9:
    { // obs-route
      QStringList result;
      bool ok = parseObsRoute( iit, iend, result, withCRLF, true /*save*/ );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result: " << result.count() << " domains:" << endl;
      for ( QStringList::ConstIterator it = result.begin() ;
	    it != result.end() ; ++it )
	cout << (*it) << endl;
    }
    break;
  case 10:
    { // addr-spec
      KMime::Types::AddrSpec result;
      bool ok = parseAddrSpec( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.localPart:\n" << result.localPart << endl
	   << "result.domain:\n" << result.domain << endl;
    }
    break;
  case 11:
    { // angle-addr
      KMime::Types::AddrSpec result;
      bool ok = parseAngleAddr( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.localPart:\n" << result.localPart << endl
	   << "result.domain:\n" << result.domain << endl;
    }
    break;
  case 12:
    { // mailbox
      KMime::Types::Mailbox result;
      bool ok = parseMailbox( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.displayName:\n" << result.displayName << endl
	   << "result.addrSpec.localPart:\n" << result.addrSpec.localPart << endl
	   << "result.addrSpec.domain:\n" << result.addrSpec.domain << endl;
    }
    break;
  case 13:
    { // group
      KMime::Types::Address result;
      bool ok = parseGroup( iit, iend, result, withCRLF );
      
      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.displayName:\n" << result.displayName << endl;
      int i = 0;
      for ( QValueList<KMime::Types::Mailbox>::ConstIterator
	      it = result.mailboxList.begin();
	    it != result.mailboxList.end() ; ++it, ++i )
	cout << "result.mailboxList[" << i << "].displayName:\n"
	     << (*it).displayName << endl
	     << "result.mailboxList[" << i << "].addrSpec.localPart:\n"
	     << (*it).addrSpec.localPart << endl
	     << "result.mailboxList[" << i << "].addrSpec.domain:\n"
	     << (*it).addrSpec.domain << endl;
    }
    break;
  case 14:
    { // address
      KMime::Types::Address result;
      bool ok = parseAddress( iit, iend, result, withCRLF );
      
      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.displayName:\n" << endl;
      int i = 0;
      for ( QValueList<KMime::Types::Mailbox>::ConstIterator
	      it = result.mailboxList.begin();
	    it != result.mailboxList.end() ; ++it, ++i )
	cout << "result.mailboxList[" << i << "].displayName:\n"
	     << (*it).displayName << endl
	     << "result.mailboxList[" << i << "].addrSpec.localPart:\n"
	     << (*it).addrSpec.localPart << endl
	     << "result.mailboxList[" << i << "].addrSpec.domain:\n"
	     << (*it).addrSpec.domain << endl;
    }
    break;
  case 15:
    { // address-list
      QValueList<KMime::Types::Address> result;
      bool ok = parseAddressList( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl;
      int j = 0;
      for ( QValueList<KMime::Types::Address>::ConstIterator
	      jt = result.begin() ; jt != result.end() ; ++jt, ++j ) {
	cout << "result[" << j << "].displayName:\n"
	     << (*jt).displayName << endl;
	int i = 0;
	for ( QValueList<KMime::Types::Mailbox>::ConstIterator
		it = (*jt).mailboxList.begin();
	      it != (*jt).mailboxList.end() ; ++it, ++i )
	  cout << "result[" << j << "].mailboxList[" << i << "].displayName:\n"
	       << (*it).displayName << endl
	       << "result[" << j << "].mailboxList[" << i << "].addrSpec.localPart:\n"
	       << (*it).addrSpec.localPart << endl
	       << "result[" << j << "].mailboxList[" << i << "].addrSpec.domain:\n"
	       << (*it).addrSpec.domain << endl;
      }
    }
    break;
  case 16:
    { // parameter
      QPair<QString,KMime::Types::QStringOrQPair> result;
      bool ok = parseParameter( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.first (attribute):\n" << result.first << endl
	   << "result.second.qstring (value):\n" << result.second.qstring << endl
	   << "result.second.qpair (value):\n"
	   << QCString( result.second.qpair.first,
			result.second.qpair.second+1 ).data() << endl;
    }
    break;
  case 17:
    { // raw-parameter-list
      QMap<QString,KMime::Types::QStringOrQPair> result;
      bool ok = parseRawParameterList( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result: " << result.count() << " raw parameters:" << endl;
      int i = 0;
      for ( QMap<QString,KMime::Types::QStringOrQPair>::ConstIterator
	      it = result.begin() ; it != result.end() ; ++it, ++i )
	cout << "result[" << i << "].key() (attribute):\n"
	     << it.key() << endl
	     << "result[" << i << "].data().qstring (value):\n"
	     << it.data().qstring << endl
	     << "result[" << i << "].data().qpair (value):\n"
	     << QCString( it.data().qpair.first,
			  it.data().qpair.second+1 ).data() << endl;
    }
    break;
  case 18:
    { // parameter-list
      QMap<QString,QString> result;
      bool ok = parseParameterList( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result: " << result.count() << " parameters:" << endl;
      int i = 0;
      for ( QMap<QString,QString>::Iterator it = result.begin() ;
	    it != result.end() ; ++it, ++i )
	cout << "result[" << i << "].key() (attribute):\n"
	     << it.key() << endl
	     << "result[" << i << "].data() (value):\n"
	     << it.data() << endl;
    }
    break;
  case 19:
    { // time
      int hour, mins, secs;
      long int secsEastOfGMT;
      bool timeZoneKnown = true;

      bool ok = parseTime( iit, iend, hour, mins, secs,
			   secsEastOfGMT, timeZoneKnown, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.hour: " << hour << endl
	   << "result.mins: " << mins << endl
	   << "result.secs: " << secs << endl
	   << "result.secsEastOfGMT: " << secsEastOfGMT << endl
	   << "result.timeZoneKnown: " << timeZoneKnown << endl;
    }
    break;
  case 20:
    { // date-time
      KMime::Types::DateTime result;
      bool ok =  parseDateTime( iit, iend, result, withCRLF );

      cout << ( ok ? "OK" : "BAD" ) << endl
	   << "result.time (in local timezone): " << ctime( &(result.time) )
	   << "result.secsEastOfGMT: " << result.secsEastOfGMT
	   << " (" << result.secsEastOfGMT/60 << "mins)" << endl
	   << "result.timeZoneKnown: " << result.timeZoneKnown << endl;
    }
    break;
  default:
    assert( 0 );
  }
}
