/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "attachmentfromurljobtest.h"
#include "qtest_messagecomposer.h"

#include <qtest_kde.h>

#include <messagecomposer/attachmentfromurljob.h>
#include <messagecomposer/composer.h>
#include <messagecomposer/globalpart.h>
using namespace MessageComposer;

QTEST_KDEMAIN( AttachmentFromUrlJobTest, NoGUI )

void AttachmentFromUrlJobTest::testAttachments_data()
{
  QTest::addColumn<KUrl>( "url" );
  QTest::addColumn<QString>( "filename" );
  QTest::addColumn<QByteArray>( "mimetype" );

  // PATH_ATTACHMENTS is defined by CMake.
  QTest::newRow( "png image" ) << KUrl::fromPath( PATH_ATTACHMENTS + QString::fromLatin1( "image.png" ) )
                               << QString::fromLatin1( "image.png" )
                               << QByteArray( "image/png" );
  QTest::newRow( "pdf doc" ) << KUrl::fromPath( PATH_ATTACHMENTS + QString::fromLatin1( "doc.pdf" ) )
                             << QString::fromLatin1( "doc.pdf" )
                             << QByteArray( "application/pdf" );
  QTest::newRow( "text file" ) << KUrl::fromPath( PATH_ATTACHMENTS + QString::fromLatin1( "file.txt" ) )
                               << QString::fromLatin1( "file.txt" )
                               << QByteArray( "text/plain" );
}

void AttachmentFromUrlJobTest::testAttachments()
{
  QFETCH( KUrl, url );
  QFETCH( QString, filename );
  QFETCH( QByteArray, mimetype );

  QFile file( url.path() );
  QVERIFY( file.open(QIODevice::ReadOnly) );
  QByteArray data = file.readAll();
  file.close();

  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  AttachmentFromUrlJob *ljob = new AttachmentFromUrlJob( url, composer );
  VERIFYEXEC( ljob );
  AttachmentPart *part = ljob->attachmentPart();
  delete ljob;
  ljob = 0;

  QCOMPARE( part->name(), filename );
  QCOMPARE( part->filename(), filename );
  QVERIFY( !part->isInline() );
  QCOMPARE( part->mimeType(), mimetype );
  QCOMPARE( part->data(), data );
}

void AttachmentFromUrlJobTest::testAttachmentTooBig()
{
  const KUrl url = KUrl::fromPath( PATH_ATTACHMENTS + QString::fromLatin1( "doc.pdf" ) );
  const QString name = QString::fromLatin1( "doc.pdf" );
  const QByteArray mimetype( "application/pdf" );

  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  AttachmentFromUrlJob *ljob = new AttachmentFromUrlJob( url, composer );
  ljob->setMaximumAllowedSize( 1024 ); // 1KiB, whereas the file is >9KiB.
  QVERIFY( !ljob->exec() );
  QCOMPARE( ljob->error(), int( JobBase::UserError ) );
}

#include "attachmentfromurljobtest.moc"
