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

#include <qdebug.h>
#include <kmime/kmime_content.h>
#include <kmime/kmime_util.h>

using namespace MessageCore;

uint MessageCore::qHash(const MessageCore::AttachmentPart::Ptr &ptr)
{
    return ::qHash(ptr.get());
}

// TODO move to kmime_util?
static qint64 sizeWithEncoding(const QByteArray &data,
                               KMime::Headers::contentEncoding encoding)  // local
{
    KMime::Content *content = new KMime::Content;
    content->setBody(data);
    content->contentTransferEncoding()->setEncoding(encoding);

    const int size = content->size();
    delete content;

    return size;
}

class MessageCore::AttachmentPart::Private
{
public:
    Private()
        : mEncoding( KMime::Headers::CE7Bit ),
          mSize( -1 ),
          mIsInline( false ),
          mAutoEncoding( true ),
          mCompressed( false ),
          mToEncrypt( false ),
          mToSign( false )
    {
    }

    QString mName;
    QString mFileName;
    QString mDescription;
    QByteArray mCharset;
    QByteArray mMimeType;
    QByteArray mData;
    KMime::Headers::contentEncoding mEncoding;
    qint64 mSize;
    bool mIsInline;
    bool mAutoEncoding;
    bool mCompressed;
    bool mToEncrypt;
    bool mToSign;
};

AttachmentPart::AttachmentPart()
    : d(new Private)
{
}

AttachmentPart::~AttachmentPart()
{
    delete d;
}

QString AttachmentPart::name() const
{
    return d->mName;
}

void AttachmentPart::setName(const QString &name)
{
    d->mName = name;
}

QString AttachmentPart::fileName() const
{
    return d->mFileName;
}

void AttachmentPart::setFileName(const QString &name)
{
    d->mFileName = name;
}

QString AttachmentPart::description() const
{
    return d->mDescription;
}

void AttachmentPart::setDescription(const QString &description)
{
    d->mDescription = description;
}

bool AttachmentPart::isInline() const
{
    return d->mIsInline;
}

void AttachmentPart::setInline(bool inl)
{
    d->mIsInline = inl;
}

bool AttachmentPart::isAutoEncoding() const
{
    return d->mAutoEncoding;
}

void AttachmentPart::setAutoEncoding(bool enabled)
{
    d->mAutoEncoding = enabled;

    if (enabled) {
        d->mEncoding = KMime::encodingsForData(d->mData).first();
    }

    d->mSize = sizeWithEncoding(d->mData, d->mEncoding);
}

KMime::Headers::contentEncoding AttachmentPart::encoding() const
{
    return d->mEncoding;
}

void AttachmentPart::setEncoding(KMime::Headers::contentEncoding encoding)
{
    d->mAutoEncoding = false;
    d->mEncoding = encoding;
    d->mSize = sizeWithEncoding(d->mData, d->mEncoding);
}

QByteArray AttachmentPart::charset() const
{
    return d->mCharset;
}

void AttachmentPart::setCharset(const QByteArray &charset)
{
    d->mCharset = charset;
}

QByteArray AttachmentPart::mimeType() const
{
    return d->mMimeType;
}

void AttachmentPart::setMimeType(const QByteArray &mimeType)
{
    d->mMimeType = mimeType;
}

bool AttachmentPart::isCompressed() const
{
    return d->mCompressed;
}

void AttachmentPart::setCompressed(bool compressed)
{
    d->mCompressed = compressed;
}

bool AttachmentPart::isEncrypted() const
{
    return d->mToEncrypt;
}

void AttachmentPart::setEncrypted(bool encrypted)
{
    d->mToEncrypt = encrypted;
}

bool AttachmentPart::isSigned() const
{
    return d->mToSign;
}

void AttachmentPart::setSigned(bool sign)
{
    d->mToSign = sign;
}

QByteArray AttachmentPart::data() const
{
    return d->mData;
}

void AttachmentPart::setData(const QByteArray &data)
{
    d->mData = data;

    if (d->mAutoEncoding) {
        QList<KMime::Headers::contentEncoding> possibleEncodings = KMime::encodingsForData(data);
        possibleEncodings.removeAll(KMime::Headers::CE8Bit);
        d->mEncoding = possibleEncodings.first();
    }

    d->mSize = sizeWithEncoding(d->mData, d->mEncoding);
}

qint64 AttachmentPart::size() const
{
    return d->mSize;
}

bool AttachmentPart::isMessageOrMessageCollection() const
{
    return (mimeType() == "message/rfc822") || (mimeType() == "multipart/digest");
}
