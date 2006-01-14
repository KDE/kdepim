/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>

#include "rfc2047test.h"
#include "rfc2047test.moc"

#include <kmime_util.h>

QTEST_KDEMAIN( RFC2047Test, NoGUI )

void RFC2047Test::testRFC2047decode()
{
  QByteArray encCharset;
  // empty
  QCOMPARE( KMime::decodeRFC2047String( QByteArray(), encCharset, "utf-8", false ), QString() );
  // identity
  QCOMPARE( KMime::decodeRFC2047String( "bla", encCharset, "utf-8", false ), QString( "bla" ) );
  // utf-8
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>", encCharset, "utf-8", false ),
            QString::fromUtf8( "Ingo Klöcker <kloecker@kde.org>" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>", encCharset, "iso8859-1", false ),
            QString::fromUtf8( "Ingo Klöcker <kloecker@kde.org>" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Ingo Klöcker" ) );
  QCOMPARE( encCharset, QByteArray( "UTF-8" ) );
  // whitespaces between two encoded words
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?=       =?utf-8?q?Ingo=20Kl=C3=B6cker?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Ingo KlöckerIngo Klöcker" ) );
  // iso-8859-x
  QCOMPARE( KMime::decodeRFC2047String( "=?ISO-8859-1?Q?Andr=E9s_Ot=F3n?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Andrés Otón" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-1" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?iso-8859-2?q?Rafa=B3_Rzepecki?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Rafał Rzepecki" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-2" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?iso-8859-9?Q?S=2E=C7a=F0lar?= Onur", encCharset, "utf-8", false ),
            QString::fromUtf8( "S.Çağlar Onur" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-9" ) );
  QCOMPARE( KMime::decodeRFC2047String( "Rafael =?iso-8859-15?q?Rodr=EDguez?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Rafael Rodríguez" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-15" ) );
  // wrong charset + charset overwrite
  QCOMPARE( KMime::decodeRFC2047String( "=?iso-8859-1?q?Ingo=20Kl=C3=B6cker?=", encCharset, "utf-8", true ),
            QString::fromUtf8( "Ingo Klöcker" ) );
}

void RFC2047Test::testRFC2047encode()
{
  // empty
  QCOMPARE( KMime::encodeRFC2047String( QString(), "utf-8" ), QByteArray() );
  // identity
  QCOMPARE( KMime::encodeRFC2047String( "bla", "utf-8" ), QByteArray( "bla" ) );
  // utf-8
  // expected value is probably wrong, libkmime will chose 'B' instead of 'Q' encoding
  QEXPECT_FAIL( "", "libkmime will chose 'B' instead of 'Q' encoding", Continue );
  QCOMPARE( KMime::encodeRFC2047String( QString::fromUtf8( "Ingo Klöcker <kloecker@kde.org>" ), "utf-8" ).constData(),
            "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>" );
}
