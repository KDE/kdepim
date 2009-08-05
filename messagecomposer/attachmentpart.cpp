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

#include "attachmentpart.h"

#include "util.h"

#include <KDebug>
#include <KUrl>

using namespace MessageComposer;

class MessageComposer::AttachmentPart::Private
{
  public:
    QString name;
    QString fileName;
    QString description;
    bool isInline;
    bool autoEncoding;
    KMime::Headers::contentEncoding encoding;
    QByteArray mimeType;
    bool compressed;
    bool toEncrypt;
    bool toSign;
    QByteArray data;
    qint64 size;
};

AttachmentPart::AttachmentPart( QObject *parent )
  : MessagePart( parent )
  , d( new Private )
{
  d->isInline = false;
  d->autoEncoding = true;
  d->encoding = KMime::Headers::CE7Bit;
  d->compressed = false;
  d->toEncrypt = false;
  d->toSign = false;
  d->size = -1;
}

AttachmentPart::~AttachmentPart()
{
  delete d;
}

QString AttachmentPart::name() const
{
  return d->name;
}

void AttachmentPart::setName( const QString &name )
{
  d->name = name;
}

QString AttachmentPart::fileName() const
{
  return d->fileName;
}

void AttachmentPart::setFileName( const QString &name )
{
  d->fileName = name;
}

QString AttachmentPart::description() const
{
  return d->description;
}

void AttachmentPart::setDescription( const QString &description )
{
  d->description = description;
}

bool AttachmentPart::isInline() const
{
  return d->isInline;
}

void AttachmentPart::setInline( bool inl )
{
  d->isInline = inl;
}

bool AttachmentPart::isAutoEncoding() const
{
  return d->autoEncoding;
}

void AttachmentPart::setAutoEncoding( bool enabled )
{
  d->autoEncoding = enabled;
  if( enabled ) {
    d->encoding = encodingsForData( d->data ).first();
  }
  d->size = sizeWithEncoding( d->data, d->encoding );
}

KMime::Headers::contentEncoding AttachmentPart::encoding() const
{
  return d->encoding;
}

void AttachmentPart::setEncoding( KMime::Headers::contentEncoding encoding )
{
  d->autoEncoding = false;
  d->encoding = encoding;
  d->size = sizeWithEncoding( d->data, d->encoding );
}

QByteArray AttachmentPart::mimeType() const
{
  return d->mimeType;
}

void AttachmentPart::setMimeType( const QByteArray &mimeType )
{
  d->mimeType = mimeType;
}

bool AttachmentPart::isCompressed() const
{
  return d->compressed;
}

void AttachmentPart::setCompressed( bool compressed )
{
  d->compressed = compressed;
}

bool AttachmentPart::isEncrypted() const
{
  return d->toEncrypt;
}

void AttachmentPart::setEncrypted( bool encrypted )
{
  d->toEncrypt = encrypted;
}

bool AttachmentPart::isSigned() const
{
  return d->toSign;
}

void AttachmentPart::setSigned( bool sign )
{
  d->toSign = sign;
}

QByteArray AttachmentPart::data() const
{
  return d->data;
}

void AttachmentPart::setData( const QByteArray &data )
{
  d->data = data;
  if( d->autoEncoding ) {
    d->encoding = encodingsForData( data ).first();
  };
  d->size = sizeWithEncoding( d->data, d->encoding );
}

qint64 AttachmentPart::size() const
{
  return d->size;
}

#include "attachmentpart.moc"
