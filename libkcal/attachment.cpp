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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

Attachment::Attachment(const QString& uri, const QString& mime)
{
  mMimeType = mime;
  mData = uri;
  mBinary = false;
	mShowInline = false;
	mLabel = QString::null;
}

Attachment::Attachment(const char *base64, const QString& mime)
{
  mMimeType = mime;
  mData = QString::fromUtf8(base64);
  mBinary = true;
	mShowInline = false;
	mLabel = QString::null;
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

void Attachment::setData(const char *base64)
{
  mData = QString::fromUtf8(base64);
  mBinary = true;
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

