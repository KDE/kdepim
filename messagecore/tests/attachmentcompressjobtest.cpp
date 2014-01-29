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

#include "attachmentcompressjobtest.h"
#include "qtest_messagecore.h"

#include <boost/shared_ptr.hpp>

#include <KDebug>
#include <KZip>
#include <qtest_kde.h>

#include <messagecore/attachment/attachmentcompressjob.h>
using namespace MessageCore;

QTEST_KDEMAIN( AttachmentCompressJobTest, NoGUI )

void AttachmentCompressJobTest::testCompress()
{
  // Some data.
  QByteArray data;
  for( int i = 0; i < 100; ++i ) {
    data += "This is some highly compressible text...\n";
  }
  const QString name = QString::fromLatin1( "name" );
  const QString fileName = QString::fromLatin1( "name.txt" );
  const QString description = QString::fromLatin1( "description" );

  // Create the original part.
  AttachmentPart::Ptr origPart = AttachmentPart::Ptr( new AttachmentPart );
  origPart->setName( name );
  origPart->setFileName( fileName );
  origPart->setDescription( description );
  origPart->setMimeType( "text/plain" );
  origPart->setEncoding( KMime::Headers::CE7Bit );
  QVERIFY( !origPart->isAutoEncoding() );
  origPart->setData( data );
  QVERIFY( !origPart->isCompressed() );

  // Compress the part and verify it.
  AttachmentCompressJob *cjob = new AttachmentCompressJob( origPart, this );
  VERIFYEXEC( cjob );
  QCOMPARE( cjob->originalPart(), origPart );
  AttachmentPart::Ptr zipPart = cjob->compressedPart();
  //kDebug() << data;
  //kDebug() << zipPart->data();
  QVERIFY( zipPart->isAutoEncoding() );
  QVERIFY( zipPart->isCompressed() );
  QCOMPARE( zipPart->name(), QString( name + QString::fromLatin1( ".zip" ) ) );
  QCOMPARE( zipPart->fileName(), QString( fileName + QString::fromLatin1( ".zip" ) ) );
  QCOMPARE( zipPart->description(), description );
  QCOMPARE( zipPart->mimeType(), QByteArray( "application/zip" ) );

  // Uncompress the data and verify it.
  // (Stuff below is stolen from KMail code.)
  QByteArray zipData = zipPart->data();
  QBuffer buffer( &zipData );
  KZip zip( &buffer );
  QVERIFY( zip.open( QIODevice::ReadOnly ) );
  const KArchiveDirectory *dir = zip.directory();
  QCOMPARE( dir->entries().count(), 1 );
  const KZipFileEntry *entry = (KZipFileEntry*)dir->entry( dir->entries()[0] );
  QCOMPARE( entry->data(), data );
  QCOMPARE( entry->name(), name );
  zip.close();
}

void AttachmentCompressJobTest::testCompressedSizeLarger()
{
  // Some data.
  QByteArray data( "This is short enough that compressing it is not efficient." );
  const QString name = QString::fromLatin1( "name.txt" );
  const QString description = QString::fromLatin1( "description" );

  // Create the original part.
  AttachmentPart::Ptr origPart = AttachmentPart::Ptr( new AttachmentPart );
  origPart->setName( name );
  origPart->setDescription( description );
  origPart->setMimeType( "text/plain" );
  origPart->setEncoding( KMime::Headers::CE7Bit );
  QVERIFY( !origPart->isAutoEncoding() );
  origPart->setData( data );
  QVERIFY( !origPart->isCompressed() );

  // Compress the part and verify that it is aware of its folly.
  AttachmentCompressJob *cjob = new AttachmentCompressJob( origPart, this );
  VERIFYEXEC( cjob );
  QVERIFY( cjob->isCompressedPartLarger() );
}


#include "moc_attachmentcompressjobtest.cpp"
