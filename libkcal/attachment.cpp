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
#include <kmdcodec.h>

using namespace KCal;

Attachment::Attachment( const Attachment &attachment )
{
  mSize = attachment.mSize;
  mMimeType = attachment.mMimeType;
  mUri = attachment.mUri;
  mData = qstrdup( attachment.mData );
  mLabel = attachment.mLabel;
  mBinary = attachment.mBinary;
  mLocal = attachment.mLocal;
  mShowInline = attachment.mShowInline;
}

Attachment::Attachment( const QString &uri, const QString &mime )
{
  mSize = 0;
  mMimeType = mime;
  mUri = uri;
  mData = 0;
  mBinary = false;
  mLocal = false;
  mShowInline = false;
}

Attachment::Attachment( const char *base64, const QString &mime )
{
  mSize = 0;
  mMimeType = mime;
  mData = qstrdup( base64 );
  mBinary = true;
  mLocal = false;
  mShowInline = false;
}

Attachment::~Attachment()
{
  delete[] mData;
}

bool Attachment::isUri() const
{
  return !mBinary;
}

QString Attachment::uri() const
{
  if ( !mBinary ) {
    return mUri;
  } else {
    return QString::null;
  }
}

void Attachment::setUri( const QString &uri )
{
  mUri = uri;
  mBinary = false;
}

bool Attachment::isBinary() const
{
  return mBinary;
}

char *Attachment::data() const
{
  if ( mBinary ) {
    return mData;
  } else {
    return 0;
  }
}

QByteArray &Attachment::decodedData()
{
  if ( mDataCache.isNull() ) {
    mDataCache = KCodecs::base64Decode( QCString( mData ) );
  }

  return mDataCache;
}

void Attachment::setDecodedData( const QByteArray &data )
{
  setData( KCodecs::base64Encode( QCString( data.data() ) ) );
  mDataCache = data;
  mSize = mDataCache.size();
}

void Attachment::setData( const char *base64 )
{
  delete[] mData;
  mData = qstrdup( base64 );
  mBinary = true;
  mDataCache = QByteArray();
  mSize = 0;
}

uint Attachment::size()
{
  if ( isUri() ) {
    return 0;
  }
  if ( !mSize ) {
    mSize = decodedData().size();
  }

  return mSize;
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
