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

#include "attachmentfrommimecontentjobtest.h"
#include "qtest_messagecore.h"

#include <boost/shared_ptr.hpp>

#include <QDebug>
#include <qtest.h>

#include <kmime/kmime_content.h>
#include <kmime/kmime_headers.h>
using namespace KMime;

#include <messagecore/attachment/attachmentfrommimecontentjob.h>
using namespace MessageCore;

QTEST_MAIN( AttachmentFromMimeContentJobTest )

void AttachmentFromMimeContentJobTest::testAttachment()
{
  const QByteArray mimeType( "x-some/x-type" );
  const QString name = QString::fromLatin1( "name abcd" );
  const QString description = QString::fromLatin1( "description" );
  const QByteArray charset( "utf-8" );
  const QString fileName = QString::fromLatin1( "filename abcd" );
  const Headers::contentEncoding encoding = Headers::CEquPr;
  const Headers::contentDisposition disposition = Headers::CDinline;
  const QByteArray data( "ocean soul" );

  Content *content = new Content;
  content->contentType()->setMimeType( mimeType );
  content->contentType()->setName( name, charset );
  content->contentType()->setCharset( charset );
  content->contentTransferEncoding()->setEncoding( encoding );
  content->contentDisposition()->setDisposition( disposition );
  content->contentDisposition()->setFilename( fileName );
  content->contentDescription()->fromUnicodeString( description, charset );
  content->setBody( data );
  content->assemble();
  //qDebug() << "Encoded content:" << content->encodedContent();
  //qDebug() << "Decoded content:" << content->decodedContent();

  AttachmentFromMimeContentJob *job = new AttachmentFromMimeContentJob( content, this );
  QVERIFY( job->uiDelegate() == 0 ); // No GUI thankyouverymuch.
  VERIFYEXEC( job );
  delete content;
  content = 0;
  AttachmentPart::Ptr part = job->attachmentPart();
  delete job;
  job = 0;

  QCOMPARE( part->mimeType(), mimeType );
  QCOMPARE( part->name(), name );
  QCOMPARE( part->description(), description );
  //QCOMPARE( part->charset(), charset ); // TODO will probably need charsets in AttachmentPart :(
  QCOMPARE( part->fileName(), fileName );
  QVERIFY( part->encoding() == encoding );
  QVERIFY( part->isInline() );
  QCOMPARE( part->data(), data );
}

