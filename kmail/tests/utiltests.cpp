/**
 * Copyright (C)  2007 David Faure <faure@kde.org>
 * This file is subject to the GPL version 2.
 */

#include <kdebug.h>
#include <kunittest/runner.h>
#include <kunittest/module.h>
#include "utiltests.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_utilmodule, "KMail::Util Tests" );
KUNITTEST_MODULE_REGISTER_TESTER( UtilTester );

#include "util.h"
#include <mimelib/string.h>


void UtilTester::setUp()
{
  kdDebug() << "setUp" << endl;
}

void UtilTester::tearDown()
{
  kdDebug() << "tearDown" << endl;
}

static TQString makePrintable( const TQCString& str )
{
  TQString a = str;
  a = a.replace( '\r', "\\r" );
  a = a.replace( '\n', "\\n" );
  return a;
}
static TQString makePrintable( const TQByteArray& arr )
{
  TQCString str;
  KMail::Util::setFromByteArray( str, arr );
  return makePrintable( str );
}

void UtilTester::test_lf2crlf()
{
  TQCString src = "\nfoo\r\n\nbar\rblah\n\r\r\n\n\r";
  TQCString conv = KMail::Util::lf2crlf( src );
  COMPARE( makePrintable( conv ), makePrintable("\r\nfoo\r\n\r\nbar\rblah\r\n\r\r\n\r\n\r") );
  COMPARE( KMail::Util::lf2crlf( TQCString("") ), TQCString("") );

  // TQByteArray version
  TQByteArray arr;  KMail::Util::setFromQCString( arr, src );
  COMPARE( arr[arr.size()-1], '\r' );
  TQByteArray arrConv = KMail::Util::lf2crlf( arr );
  COMPARE( arrConv[arrConv.size()-1], '\r' );
  COMPARE( makePrintable( arrConv ), makePrintable("\r\nfoo\r\n\r\nbar\rblah\r\n\r\r\n\r\n\r") );
  TQByteArray empty;
  arrConv = KMail::Util::lf2crlf( empty );
  COMPARE( makePrintable( arrConv ), TQString("") );
}

void UtilTester::test_crlf2lf()
{
  TQCString src = "\r\n\r\nfoo\r\n\r\nbar\rblah\r\n\r\r\n\r\n\r";
  int len = src.length();
  COMPARE( src[len], '\0' );
  int newLen = KMail::Util::crlf2lf( src.data(), len );
  VERIFY( newLen <= len );
  TQCString cstr( src.data(), newLen + 1 );
  COMPARE( makePrintable( cstr ), makePrintable("\n\nfoo\n\nbar\rblah\n\r\n\n\r") );
}

void UtilTester::test_escapeFrom()
{
  // TODO should take a DwString, then fix kmfoldermbox.cpp:1021      msgText = escapeFrom( aMsg->asString() );

}

void UtilTester::test_append()
{
  TQCString test;
  TQCString str = "foo";
  COMPARE( (int)str.size(), 4 ); // trailing nul included
  TQByteArray s1 = KMail::Util::byteArrayFromQCStringNoDetach( str );
  COMPARE( (int)s1.size(), 3 );
  COMPARE( (int)str.size(), 3 ); // trailing nul got removed
  COMPARE( s1.data(), str.data() ); // yes, no detach
  COMPARE( s1[2], 'o' );

  TQCString bar( "bar" );
  TQByteArray s2 = KMail::Util::byteArrayFromQCStringNoDetach( bar );
  COMPARE( (int)s2.size(), 3 );

  KMail::Util::append( s1, s2 );
  COMPARE( (int)s1.size(), 6 );
  KMail::Util::setFromByteArray( test, s1 );
  COMPARE( test, TQCString( "foobar" ) );

  KMail::Util::append( s1, 0 ); // no-op
  COMPARE( (int)s1.size(), 6 );
  KMail::Util::setFromByteArray( test, s1 );
  COMPARE( test, TQCString( "foobar" ) );

  KMail::Util::append( s1, "blah" );
  COMPARE( (int)s1.size(), 10 );
  KMail::Util::setFromByteArray( test, s1 );
  COMPARE( test, TQCString( "foobarblah" ) );

  KMail::Util::append( s1, TQCString( " str" ) );
  COMPARE( (int)s1.size(), 14 );
  KMail::Util::setFromByteArray( test, s1 );
  COMPARE( test, TQCString( "foobarblah str" ) );

  TQByteArray empty;
  KMail::Util::append( empty, "a" );
  COMPARE( (int)empty.size(), 1 );
  COMPARE( empty[0], 'a' );
}

void UtilTester::test_insert()
{
  TQCString test;
  TQCString str = "foo";
  COMPARE( (int)str.size(), 4 ); // trailing nul included
  TQByteArray s1;
  KMail::Util::setFromQCString( s1, str );

  KMail::Util::insert( s1, 1, "bar" );
  COMPARE( (int)s1.size(), 6 );
  COMPARE( makePrintable(s1), TQString( "fbaroo" ) );

  KMail::Util::insert( s1, 6, "END" );
  COMPARE( (int)s1.size(), 9 );
  COMPARE( makePrintable(s1), TQString( "fbarooEND" ) );

  KMail::Util::insert( s1, 0, "BEGIN" );
  COMPARE( (int)s1.size(), 14 );
  COMPARE( makePrintable(s1), TQString( "BEGINfbarooEND" ) );
}

void UtilTester::test_DwStringConversions( const TQCString& cstr )
{
  // QCString->DwString->QCString
  COMPARE( (int)cstr.size(), 8 );
  DwString dwstr = KMail::Util::dwString( cstr );
  COMPARE( (int)dwstr.size(), 7 );
  COMPARE( dwstr[6], 'r' );
  TQCString cstr2 = KMail::Util::CString( dwstr );
  COMPARE( (int)cstr2.size(), 8 );
  COMPARE( cstr2, cstr );
  COMPARE( cstr2[6], 'r' );

  // And also QCString->QByteArray
  TQByteArray arr;
  KMail::Util::setFromQCString( arr, cstr );
  COMPARE( (int)arr.size(), 7 );
  COMPARE( arr[6], 'r' );

  KMail::Util::setFromQCString( arr, TQCString() );
  COMPARE( (int)arr.size(), 0 );

  // DwString->QByteArray
  TQByteArray ba = KMail::Util::ByteArray( dwstr );
  COMPARE( (int)ba.size(), 7 );
  COMPARE( ba[6], 'r' );

  ba = KMail::Util::ByteArray( DwString() );
  COMPARE( (int)ba.size(), 0 );
}

void UtilTester::test_DwStringConversions()
{
  TQCString cstr = "foo&bar";
  test_DwStringConversions( cstr );
  // now embed a nul. Note that cstr="foo\0bar" wouldn't work.
  cstr[3] = '\0';
  test_DwStringConversions( cstr );

  cstr = TQCString();
  DwString dwstr = KMail::Util::dwString( cstr );
  COMPARE( (int)dwstr.size(), 0 );
  VERIFY( dwstr.empty() );

  dwstr = KMail::Util::dwString( TQByteArray() );
  COMPARE( (int)dwstr.size(), 0 );
  VERIFY( dwstr.empty() );
}

void UtilTester::test_QByteArrayQCString()
{
  TQCString str = "foobar";
  COMPARE( (int)str.size(), 7 ); // trailing nul included
  TQByteArray s1 = KMail::Util::byteArrayFromQCStringNoDetach( str );
  COMPARE( (int)str.size(), 6 ); // trailing nul got removed
  COMPARE( s1.data(), str.data() ); // yes, no detach
  COMPARE( s1[5], 'r' );
  COMPARE( str[5], 'r' );

  KMail::Util::restoreQCString( str );
  COMPARE( (int)str.size(), 7 ); // trailing nul included
  COMPARE( str[5], 'r' );
  COMPARE( str[6], '\0' );

}

#include "utiltests.moc"

