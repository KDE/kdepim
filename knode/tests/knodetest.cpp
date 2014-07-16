/*
  Copyright 2009 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "knodetest.h"

#include <utils/locale.h>
using namespace KNode::Utilities;


#include <qtest_kde.h>
#include <KCharsets>
#include <KGlobal>
#include <KLocale>

#include <QDebug>
#include <QMap>
#include <QTest>


void KNodeTest::testUtilitiesLocale()
{
  // List of all name of encodings from KCharset and their equivalent to use in MIME headers
  QMap<QString, QString> kcharsetVsMime;
  kcharsetVsMime.insert( "Big5", "BIG5" );
  kcharsetVsMime.insert( "Big5-HKSCS", "BIG5-HKSCS" );
  kcharsetVsMime.insert( "cp 1250", "WINDOWS-1250" );
  kcharsetVsMime.insert( "cp 1251", "WINDOWS-1251" );
  kcharsetVsMime.insert( "cp 1252", "WINDOWS-1252" );
  kcharsetVsMime.insert( "cp 1253", "WINDOWS-1253" );
  kcharsetVsMime.insert( "cp 1254", "WINDOWS-1254" );
  kcharsetVsMime.insert( "cp 1255", "WINDOWS-1255" );
  kcharsetVsMime.insert( "cp 1256", "WINDOWS-1256" );
  kcharsetVsMime.insert( "cp 1257", "WINDOWS-1257" );
  kcharsetVsMime.insert( "EUC-JP", "ISO-2022-JP" ); // Special case (euc-jp and iso-2022-jp are different)
  kcharsetVsMime.insert( "EUC-KR", "EUC-KR" );
  kcharsetVsMime.insert( "GB18030", "GB18030" );
  kcharsetVsMime.insert( "GB2312", "GB2312" );
  kcharsetVsMime.insert( "GBK", "GBK" );
  kcharsetVsMime.insert( "IBM850", "IBM850" );
  kcharsetVsMime.insert( "IBM866", "IBM866" );
  kcharsetVsMime.insert( "IBM874", "IBM874" );
  kcharsetVsMime.insert( "ISO 10646-UCS-2", "UTF-8" );
  kcharsetVsMime.insert( "ISO 8859-11", "TIS-620" ); // correct ??
  kcharsetVsMime.insert( "ISO 8859-13", "ISO-8859-13" );
  kcharsetVsMime.insert( "ISO 8859-14", "ISO-8859-14" );
  kcharsetVsMime.insert( "ISO 8859-15", "ISO-8859-15" );
  kcharsetVsMime.insert( "ISO 8859-16", "ISO-8859-16" );
  kcharsetVsMime.insert( "ISO 8859-1", "ISO-8859-1" );
  kcharsetVsMime.insert( "ISO 8859-2", "ISO-8859-2" );
  kcharsetVsMime.insert( "ISO 8859-3", "ISO-8859-3" );
  kcharsetVsMime.insert( "ISO 8859-4", "ISO-8859-4" );
  kcharsetVsMime.insert( "ISO 8859-5", "ISO-8859-5" );
  kcharsetVsMime.insert( "ISO 8859-6", "ISO-8859-6" );
  kcharsetVsMime.insert( "ISO 8859-7", "ISO-8859-7" );
  kcharsetVsMime.insert( "ISO 8859-8-I", "ISO-8859-8" ); // "ISO-8859-8" and "ISO-8859-8-I" have different MIBenum actually
  kcharsetVsMime.insert( "ISO 8859-8", "ISO-8859-8" );
  kcharsetVsMime.insert( "ISO 8859-9", "ISO-8859-9" );
  kcharsetVsMime.insert( "jis7", "ISO-2022-JP" ); // we trust qtcodec for this association
  kcharsetVsMime.insert( "KOI8-R", "KOI8-R" );
  kcharsetVsMime.insert( "KOI8-U", "KOI8-U" );
  kcharsetVsMime.insert( "sjis", "SHIFT_JIS" );
  kcharsetVsMime.insert( "TIS620", "TIS-620" );
  kcharsetVsMime.insert( "TSCII", "TSCII" );
  kcharsetVsMime.insert( "ucs2", "UTF-8" ); // wondering what is the difference between "ucs2" and "ISO 10646-UCS-2" above
  kcharsetVsMime.insert( "UTF-16", "UTF-16" );
  kcharsetVsMime.insert( "utf7", "UTF-8" ); // KGlobal::charsets()->codecForName() do not returns a valid codec for "utf7" so "UTF-8" is returned.
  kcharsetVsMime.insert( "UTF-8", "UTF-8" );
  kcharsetVsMime.insert( "windows-1258", "WINDOWS-1258" );
  kcharsetVsMime.insert( "winsami2", "WINSAMI2" );


  // Check that every name from "KCharsets::availableEncodingNames()" will be taken into account
  QStringList kcharsetNames = kcharsetVsMime.keys();
  foreach( const QString &encName, KGlobal::charsets()->availableEncodingNames() ) {
    QVERIFY( kcharsetNames.contains( encName ) );
  }

  // Check that the convertion in Locale::toMimeCharset() is correct for these
  foreach ( const QString &encName, KGlobal::charsets()->availableEncodingNames() ) {
    qDebug() << "Current encoding:" << encName;
    QCOMPARE( Locale::toMimeCharset( encName ), kcharsetVsMime.value( encName ) );
  }

  // Do not see bug #163524 again (empty charset)
  QVERIFY( !Locale::toMimeCharset( "" ).isEmpty() );

  // Testing us-ascii
  QEXPECT_FAIL("", "KCharsets alias us-ascii to iso-8859-1 without warning", Continue);
  QCOMPARE( Locale::toMimeCharset( "us-ascii" ), QString( "US-ASCII" ) );

  // Idempotency
  kcharsetNames = kcharsetVsMime.keys();
  foreach ( const QString &encName, kcharsetNames ) {
    const QString res = Locale::toMimeCharset( encName );
    QCOMPARE( res, Locale::toMimeCharset( res ) );
  }
}


QTEST_KDEMAIN( KNodeTest, NoGUI )

