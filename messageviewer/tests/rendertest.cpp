/*
  Copyright (c) 2010 Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "htmlwriter/filehtmlwriter.h"
#include "viewer/objecttreeparser.h"
#include "csshelper.h"
#include "messagecore/tests/util.h"

#include <KMime/Message>
#include <qtest_kde.h>
#include <QDir>
#include <QObject>

// This is used to override the default message output handler. In unit tests, the special message
// output handler can write messages to stdout delayed, i.e. after the actual kDebug() call. This
// interfers with KPGP, since KPGP reads output from stdout, which needs to be kept clean.
void nullMessageOutput( QtMsgType type, const char *msg )
{
  Q_UNUSED( type );
  Q_UNUSED( msg );
}

using namespace MessageViewer;

class RenderTest : public QObject
{
  Q_OBJECT

  private slots:
    void initTestCase()
    {
      MessageCore::Test::setupEnv();
    }

    void testRender_data()
    {
      QTest::addColumn<QString>( "mailFileName" );
      QTest::addColumn<QString>( "referenceFileName" );
      QTest::addColumn<QString>( "outFileName" );

      QDir dir( MAIL_DATA_DIR );
      foreach ( const QString &file, dir.entryList( QStringList("*.mbox"), QDir::Files | QDir::Readable | QDir::NoSymLinks  ) ) {
        if ( !QFile::exists(dir.path() + '/' + file + ".html") )
          continue;
        QTest::newRow( file.toLatin1() ) << QString(dir.path() + '/' +  file) << QString(dir.path() + '/' + file + ".html") << QString(file + ".out");
      }
    }

    void testRender()
    {
      QFETCH( QString, mailFileName );
      QFETCH( QString, referenceFileName );
      QFETCH( QString, outFileName );

      const QString htmlFileName = outFileName + ".html";

      // load input mail
      QFile mailFile( mailFileName );
      QVERIFY( mailFile.open( QIODevice::ReadOnly ) );
      const QByteArray mailData = KMime::CRLFtoLF( mailFile.readAll() );
      QVERIFY( !mailData.isEmpty() );
      KMime::Message::Ptr msg( new KMime::Message );
      msg->setContent( mailData );
      msg->parse();

      // render the mail
      FileHtmlWriter fileWriter( outFileName );
      QImage paintDevice;
      CSSHelper cssHelper( &paintDevice );
      NodeHelper nodeHelper;
      MessageCore::Test::TestObjectTreeSource testSource( &fileWriter, &cssHelper );
      testSource.setAllowDecryption( true );
      ObjectTreeParser otp( &testSource, &nodeHelper );

      fileWriter.begin( QString() );
      fileWriter.queue( cssHelper.htmlHead( false ) );

      qInstallMsgHandler( nullMessageOutput );
      otp.parseObjectTree( msg.get() );
      qInstallMsgHandler( 0 );

      fileWriter.queue("</body></html>");
      fileWriter.flush();
      fileWriter.end();

      QVERIFY( QFile::exists( outFileName ) );

      // validate xml and pretty-print for comparisson
      // TODO add proper cmake check for xmllint and diff
      QStringList args = QStringList()
        << "--format"
        << "--output"
        << htmlFileName
        << outFileName;
      QCOMPARE( QProcess::execute( "xmllint", args ),  0 );

      // get rid of system dependent or random paths
      {
        QFile f( htmlFileName );
        QVERIFY( f.open( QIODevice::ReadOnly ) );
        QString content = QString::fromUtf8( f.readAll() );
        f.close();
        content.replace( QRegExp( "\"file:[^\"]*[/(?:%2F)]([^\"/(?:%2F)]*)\"" ), "\"file:\\1\"" );
        QVERIFY( f.open( QIODevice::WriteOnly | QIODevice::Truncate ) );
        f.write( content.toUtf8() );
        f.close();
      }

      // compare to reference file
      args = QStringList()
        << "-u"
        << referenceFileName
        << htmlFileName;
      QProcess proc;
      proc.setProcessChannelMode( QProcess::ForwardedChannels );
      proc.start( "diff", args );
      QVERIFY( proc.waitForFinished() );

      QEXPECT_FAIL( "forward-openpgp-signed-encrypted.mbox", "Signature verification is currently broken in the testsetup", Continue );
      QEXPECT_FAIL( "openpgp-signed-encrypted.mbox", "Signature verification is currently broken in the testsetup", Continue );
      QEXPECT_FAIL( "signed-forward-openpgp-signed-encrypted.mbox", "Signature verification is currently broken in the testsetup", Continue );
      QEXPECT_FAIL( "smime-signed-encrypted.mbox", "Signature verification is currently broken in the testsetup", Continue );
      QEXPECT_FAIL( "smime-encrypted.mbox", "Signature verification is currently broken in the testsetup", Continue );
      QEXPECT_FAIL( "smime-encrypted-octet-stream.mbox", "Signature verification is currently broken in the testsetup", Continue );
      QEXPECT_FAIL( "openpgp-signed-mailinglist.mbox", "Signature verification is currently broken in the testsetup", Continue );
      QEXPECT_FAIL( "openpgp-encrypted.mbox", "Signature verification is currently broken in the testsetup", Continue );

      QCOMPARE( proc.exitCode(), 0 );
    }
};

QTEST_KDEMAIN( RenderTest, GUI )

#include "rendertest.moc"
