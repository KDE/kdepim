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

#include <qttest_kde.h>

#include "rfc2047test.h"
#include "rfc2047test.moc"

#include <kmime_util.h>

QTTEST_KDEMAIN( RFC2047Test, NoGUI )

void RFC2047Test::testRFC2047decode()
{
  qDebug( "Testing RFC2047 decoding" );
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
}

void RFC2047Test::testRFC2047encode()
{
  qDebug( "Testing RFC2047 encoding" );
  // empty
  QCOMPARE( KMime::encodeRFC2047String( QString(), "utf-8" ), QByteArray() );
  // identity
  QCOMPARE( KMime::encodeRFC2047String( "bla", "utf-8" ), QByteArray( "bla" ) );
  // utf-8
  // expected value is probably wrong, libkmime will chose 'B' instead of 'Q' encoding
  QCOMPARE( KMime::encodeRFC2047String( QString::fromUtf8( "Ingo Klöcker <kloecker@kde.org>" ), "utf-8" ).constData(),
            "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>" );
}
