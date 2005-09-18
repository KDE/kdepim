/*
    This file is part of libkcal.

    Copyright (c) 2002 Michael Brade <brade@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "attachment.h"

#include <libkmime/kmime_codecs.h>
//Added by qt3to4:
#include <QByteArray>

using namespace KCal;

class Attachment::Private
{
public:
  mutable QByteArray mDataCache;
  mutable uint mSize;
};

Attachment::Attachment( const Attachment &attachment)
  : d( new Attachment::Private )
{
  mMimeType = attachment.mMimeType;
  mData = attachment.mData;
  mBinary = attachment.mBinary;
	mShowInline = attachment.mShowInline;
	mLabel = attachment.mLabel;
	mLocal = attachment.mLocal;
}

Attachment::Attachment(const QString& uri, const QString& mime)
  : d( new Attachment::Private )
{
  mMimeType = mime;
  mData = uri;
  mBinary = false;
	mShowInline = false;
	mLocal = false;
	mLabel = QString::null;
}

Attachment::Attachment(const char *base64, const QString& mime)
  : d( new Attachment::Private )
{
  mMimeType = mime;
  mData = QString::fromUtf8(base64);
  mBinary = true;
	mShowInline = false;
	mLabel = QString::null;
}

Attachment::~Attachment()
{
  delete d;
}

bool Attachment::isUri() const
{
  return !mBinary;
}

QString Attachment::uri() const
{
  if (!mBinary)
    return mData;
  else
    return QString::null;
}

void Attachment::setUri(const QString& uri)
{
  mData = uri;
  mBinary = false;
}

bool Attachment::isBinary() const
{
  return mBinary;
}

char *Attachment::data() const
{
  if (mBinary)
    return mData.utf8().data();
  else
    return 0;
}

QByteArray &Attachment::decodedData() const
{
  if ( d->mDataCache.isNull() ) {
    QByteArray in;
    const QByteArray data = mData.utf8();
    in.setRawData( data.data(), data.size() );
    KMime::Codec * codec = KMime::Codec::codecForName( "base64" );
    d->mDataCache = codec->decode( in );
    in.resetRawData( data.data(), data.size() );
  }
  
  return d->mDataCache;
}

void Attachment::setDecodedData( const QByteArray &data )
{
  KMime::Codec *codec = KMime::Codec::codecForName( "base64" );
  setData( codec->encode( data ).data() );
  d->mDataCache = data;
}

void Attachment::setData(const char *base64)
{
  mData = QString::fromUtf8(base64);
  mBinary = true;
  d->mDataCache = QByteArray();
  d->mSize = 0;
}

uint Attachment::size() const
{
  if ( isUri() )
    return 0;
  if ( !d->mSize )
    d->mSize = decodedData().size();
  
  return d->mSize;
}

QString Attachment::mimeType() const
{
  return mMimeType;
}

void Attachment::setMimeType(const QString& mime)
{
  mMimeType = mime;
}

bool Attachment::showInline() const
{
  return mShowInline;
}

void Attachment::setShowInline( bool showinline )
{
  mShowInline = showinline;
}

QString Attachment::label() const
{
  return mLabel;
}

void Attachment::setLabel( const QString& label )
{
  mLabel = label;
}

bool Attachment::isLocal() const
{
  return mLocal;
}

void Attachment::setLocal( bool local )
{
  mLocal = local;
}
