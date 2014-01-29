/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "stringutiltest.h"

#include "../utils/stringutil.h"

#include "qtest_kde.h"

using namespace MessageCore;

QTEST_KDEMAIN( StringUtilTest, GUI )
#define lineLength 40

void StringUtilTest::test_SmartQuote()
{
  QFETCH( QString, originalString );
  QFETCH( QString, quotedString );

  QEXPECT_FAIL( "hard linebreak", "Currently no way to differentiate between hard and soft line breaks", Continue );
  const QString result = StringUtil::smartQuote( originalString, lineLength );
  const QStringList resultList = result.split(QLatin1Char('\n'));
  const QStringList expectedList = quotedString.split(QLatin1Char('\n'));
  qDebug() << "result  :" << resultList;
  qDebug() << "expected:" << expectedList;
  QCOMPARE( resultList, expectedList );
  //QCOMPARE( result, quotedString );
}

void StringUtilTest::test_SmartQuote_data()
{
  QTest::addColumn<QString>( "originalString" );
  QTest::addColumn<QString>( "quotedString" );

  QTest::newRow( "1" ) << "Some short text"
                       << "Some short text";

  //                                                               40
  //                                                               ↓
  QTest::newRow( "2" ) << "Some much longer text that exceeds our limit by far."
                       << "Some much longer text that exceeds our\nlimit by far.";

  QTest::newRow( "3" ) << " Space at start."
                       << " Space at start.";

  //                                                               40
  //                                                               ↓
  QTest::newRow( "4" ) << " Space at start, but also two lines in this long sentennce."
                       << " Space at start, but also two lines in\nthis long sentennce.";

  QTest::newRow( "5" ) << " Space at start and end. "
                       << " Space at start and end.";

  QTest::newRow( "6" ) << "Space at end of pre-broken line. \n"
                          "Yet another line of text."
                       << "Space at end of pre-broken line.\n"
                          "Yet another line of text.";

  //                                                               40
  //                                                               ↓
  QTest::newRow( "7" ) << "Long long line, followed by another line starting with a space.\n"
                          " Starts with a space."
                       << "Long long line, followed by another line\n"
                          "starting with a space. Starts with a\n"
                          "space.";

  //                                                               40
  //                                                               ↓
  QTest::newRow( "8" ) << "Two lines that don't need to be\nchanged in any way."
                       << "Two lines that don't need to be\nchanged in any way.";

  //                                                               40
  //                                                               ↓
  QTest::newRow( "9" ) << "Many lines.\n"
                          "Only one needs to be broken.\n"
                          "This is the very much too long line that needs to be broken.\n"
                          "This line is ok again."
                       << "Many lines.\n"
                          "Only one needs to be broken.\n"
                          "This is the very much too long line that\n"
                          "needs to be broken. This line is ok\n"
                          "again.";

  //                                                                40
  //                                                                ↓
  QTest::newRow( "10" ) << "> >Very long quoted line, that is very very long"
                        << "> >Very long quoted line, that is very\n"
                           "> >very long";

  //                                                                40
  //                                                                ↓
  QTest::newRow( "11" ) << "> > Very long quoted line, that is very very long"
                        << "> > Very long quoted line, that is very\n"
                           "> > very long";

  //                                                                40
  //                                                                ↓
  QTest::newRow( "12" ) << "> > Very long quoted line, that is very very long. \n"
                           "> > Another line here."
                        << "> > Very long quoted line, that is very\n"
                           "> > very long. Another line here.";

  //                                                                40
  //                                                                ↓
  QTest::newRow( "13" ) << "> > Very long quoted line, that is very very long. \n"
                           "Unquoted line, for a change.\n"
                           "> > Another line here."
                        << "> > Very long quoted line, that is very\n"
                           "> > very long.\n"
                           "Unquoted line, for a change.\n"
                           "\n"
                           "> > Another line here.";

  //                                                                40
  //                                                                ↓
  QTest::newRow( "14" ) << "> Quote level 1 with long long long long text, that is long.\n"
                           "> Quote level 1 still.\n"
                           "> > Quote level 2 now, also with a long long long long text.\n"
                           "> > Quote level 2 still.\n"
                           "No quotes."
                        << "> Quote level 1 with long long long long\n"
                           "> text, that is long. Quote level 1\n"
                           "> still.\n"
                           "> \n"
                           "> > Quote level 2 now, also with a long\n"
                           "> > long long long text. Quote level 2\n"
                           "> > still.\n"
                           "\n"
                           "No quotes.";

  //                                                                40
  //                                                                ↓
  QTest::newRow( "15" ) << "Some much longer text that exceeds our limit by far.\n"
                           "\n"
                           "Line after an empty one."
                        << "Some much longer text that exceeds our\n"
                           "limit by far.\n"
                           "\n"
                           "Line after an empty one.";

  // Make sure the "You wrote:" line is not broken, that would look strange
  //                                                                40
  //                                                                ↓
  QTest::newRow( "16" ) << "Yesterday, Hans Martin Ulrich Franz August wrote:\n"
                           "> Bla Bla Bla Bla..."
                        << "Yesterday, Hans Martin Ulrich Franz August wrote:\n"
                           "> Bla Bla Bla Bla...";

  //                                                                40
  //                                                                ↓
  QTest::newRow( "17" ) << "Yesterday, Hans Martin Ulrich Franz August wrote:\n"
                           "\n"
                           "> Bla Bla Bla Bla..."
                        << "Yesterday, Hans Martin Ulrich Franz August wrote:\n"
                           "> Bla Bla Bla Bla...";

  // This test shows a fundamental flaw when breaking lines: The table header gets broken,
  // which is ok. However, the following line is appended to the table header, which leads
  // the data columns and the table header to by not aligned anymore.
  // The problem is that the linebreak after the table header is supposed to be a hard line break,
  // but there is no way to know that, so we treat it as a soft line break instead.
  //
  // We can't treat every linebreak as hard linebreaks, as quoting natural text would look strange then.
  // The problem here is that the sender word-wraps the email with hard linebreaks.
  // This is only possible to fix correctly by supporting "flowed" text, as per RFC 2646
  //
  // (this is marked as QEXPECT_FAIL above)
  //
  // The solution would be to let the caller remove soft linebreaks manually (as only the caller
  // can now), and let smartQuote() treat all linebreaks as hard linebreaks, which would fix this.
  //                                                                           40
  //                                                                            ↓
  QTest::newRow( "hard linebreak" ) << "==== Date ======== Amount ======= Type ======\n"
                                       "   12.12.09          5            Car        \n"
                                    << "==== Date ======== Amount ======= Type\n"
                                       "======\n"
                                       "   12.12.09          5            Car        \n";
}

void StringUtilTest::test_signatureStripping()
{
  //QStringList tests;
  const QString test1 = QLatin1String(
      "text1\n"
      "-- \n"
      "Signature Block1\n"
      "Signature Block1\n\n"
      "> text2\n"
      "> -- \n"
      "> Signature Block 2\n"
      "> Signature Block 2\n"
      ">> text3 -- not a signature block\n"
      ">> text3\n"
      ">>> text4\n"
      "> -- \n"
      "> Signature Block 4\n"
      "> Signature Block 4\n"
      ">>-------------\n"
      ">>-- text5 --\n"
      ">>-------------------\n"
      ">>-- \n"
      ">>\n"
      ">> Signature Block 5\n"
      "text6\n"
      "-- \n"
      "Signature Block 6\n");

  const QString test1Result = QLatin1String(
      "text1\n"
      "> text2\n"
      ">> text3 -- not a signature block\n"
      ">> text3\n"
      ">>> text4\n"
      ">>-------------\n"
      ">>-- text5 --\n"
      ">>-------------------\n"
      "text6\n");

  QCOMPARE( StringUtil::stripSignature( test1 ), test1Result );


  const QString test2 = QLatin1String(
      "text1\n"
      "> text2\n"
      ">> text3 -- not a signature block\n"
      ">> text3\n"
      ">>> text4\n"
      ">>-------------\n"
      ">>-- text5 --\n"
      ">>-------------------\n"
      "text6\n");

  // No actual signature - should stay the same
  QCOMPARE( StringUtil::stripSignature( test2 ), test2 );

  const QString test3 = QLatin1String(
      "text1\n"
      "-- \n"
      "Signature Block1\n"
      "Signature Block1\n\n"
      ">text2\n"
      ">-- \n"
      ">Signature Block 2\n"
      ">Signature Block 2\n"
      "> >text3\n"
      "> >text3\n"
      "> >-- \n"
      ">>Not Signature Block 3\n"
      "> > Not Signature Block 3\n"
      ">text4\n"
      ">-- \n"
      ">Signature Block 4\n"
      ">Signature Block 4\n"
      "text5\n"
      "-- \n"
      "Signature Block 5");

  const QString test3Result = QLatin1String(
      "text1\n"
      ">text2\n"
      "> >text3\n"
      "> >text3\n"
      ">>Not Signature Block 3\n"
      "> > Not Signature Block 3\n"
      ">text4\n"
      "text5\n");

  QCOMPARE( StringUtil::stripSignature( test3 ), test3Result );

  const QString test4 = QLatin1String(
      "Text 1\n"
      "-- \n"
      "First sign\n\n\n"
      "> From: bla\n"
      "> Texto 2\n\n"
      "> Aqui algo de texto.\n\n"
      ">> --\n"
      ">> Not Signature Block 2\n\n"
      "> Adios\n\n"
      ">> Texto 3\n\n"
      ">> --\n"
      ">> Not Signature block 3\n");

  const QString test4Result = QLatin1String(
      "Text 1\n"
      "> From: bla\n"
      "> Texto 2\n\n"
      "> Aqui algo de texto.\n\n"
      ">> --\n"
      ">> Not Signature Block 2\n\n"
      "> Adios\n\n"
      ">> Texto 3\n\n"
      ">> --\n"
      ">> Not Signature block 3\n");

  QCOMPARE( StringUtil::stripSignature( test4 ), test4Result );

  const QString test5 = QLatin1String(
      "-- \n"
      "-- ACME, Inc\n"
      "-- Joe User\n"
      "-- PHB\n"
      "-- Tel.: 555 1234\n"
      "--");

  QCOMPARE( StringUtil::stripSignature( test5 ), QString() );

  const QString test6 = QLatin1String(
      "Text 1\n\n\n\n"
      "> From: bla\n"
      "> Texto 2\n\n"
      "> Aqui algo de texto.\n\n"
      ">> Not Signature Block 2\n\n"
      "> Adios\n\n"
      ">> Texto 3\n\n"
      ">> --\n"
      ">> Not Signature block 3\n");

  // Again, no actual signature in here
  QCOMPARE( StringUtil::stripSignature( test6 ), test6 );
}

void StringUtilTest::test_isCryptoPart()
{
  QVERIFY( StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("pgp-encrypted"), QString() ) );
  QVERIFY( StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("pgp-signature"), QString() ) );
  QVERIFY( StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("pkcs7-mime"), QString() ) );
  QVERIFY( StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("pkcs7-signature"), QString() ) );
  QVERIFY( StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("x-pkcs7-signature"), QString() ) );
  QVERIFY( StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("octet-stream"), QLatin1String("msg.asc") ) );
  QVERIFY( !StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("octet-stream"), QLatin1String("bla.foo") ) );
  QVERIFY( !StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("foo"), QString() ) );
  QVERIFY( !StringUtil::isCryptoPart( QLatin1String("application"), QLatin1String("foo"), QLatin1String("msg.asc") ) );
}

void StringUtilTest::test_stripOffMessagePrefix()
{
  const QString subject = QLatin1String( "Fwd: Hello World Subject" );
  QBENCHMARK {
    StringUtil::stripOffPrefixes( subject );
  }
}


#include "moc_stringutiltest.cpp"
