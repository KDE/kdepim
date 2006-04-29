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
	mLabel.clear();
}

Attachment::Attachment(const char *base64, const QString& mime)
  : d( new Attachment::Private )
{
  mMimeType = mime;
  mData = QString::fromUtf8(base64);
  mBinary = true;
	mShowInline = false;
	mLabel.clear();
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
    return QString();
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
    return mData.toUtf8().data();
  else
    return 0;
}

QByteArray &Attachment::decodedData() const
{
  if ( d->mDataCache.isNull() ) {
    d->mDataCache = QByteArray::fromBase64( mData.toUtf8() );
  }
  
  return d->mDataCache;
}

void Attachment::setDecodedData( const QByteArray &data )
{
  setData( data.toBase64() );
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
