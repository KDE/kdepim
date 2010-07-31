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

using namespace KCal;

Attachment::Attachment( const Attachment &attachment)
{
  mMimeType = attachment.mMimeType;
  mData = attachment.mData;
  mBinary = attachment.mBinary;
	mShowInline = attachment.mShowInline;
	mLabel = attachment.mLabel;
}

Attachment::Attachment(const TQString& uri, const TQString& mime)
{
  mMimeType = mime;
  mData = uri;
  mBinary = false;
	mShowInline = false;
	mLabel = TQString::null;
}

Attachment::Attachment(const char *base64, const TQString& mime)
{
  mMimeType = mime;
  mData = TQString::fromUtf8(base64);
  mBinary = true;
	mShowInline = false;
	mLabel = TQString::null;
}

bool Attachment::isUri() const
{
  return !mBinary;
}

TQString Attachment::uri() const
{
  if (!mBinary)
    return mData;
  else
    return TQString::null;
}

void Attachment::setUri(const TQString& uri)
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
    // this method actually return a const char*, but that can't be done because of the uneededly non-const libical API
    return const_cast<char*>( mData.latin1() ); //mData.utf8().data();
  else
    return 0;
}

void Attachment::setData(const char *base64)
{
  mData = TQString::fromUtf8(base64);
  mBinary = true;
}

TQString Attachment::mimeType() const
{
  return mMimeType;
}

void Attachment::setMimeType(const TQString& mime)
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

TQString Attachment::label() const
{
  return mLabel;
}

void Attachment::setLabel( const TQString& label )
{
  mLabel = label;
}

