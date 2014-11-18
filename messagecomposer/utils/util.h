/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

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

#ifndef MESSAGECOMPOSER_UTIL_H
#define MESSAGECOMPOSER_UTIL_H

#include "messagecomposer_export.h"
#include "kleo/enum.h"
#include <KMime/Message>

#include <QTextEdit>
class QUrl;
namespace KMime
{
class Content;
}

namespace MailTransport
{
class MessageQueueJob;
}

namespace MessageComposer
{

namespace Util
{

/**
      * Sets the proper structural information such as content-type, cte, and charset on the encoded body content. Depending on the crypto options,
      *  original content may be needed to determine some of the values
      */
KMime::Content *composeHeadersAndBody(KMime::Content *orig, QByteArray encodedBody,  Kleo::CryptoMessageFormat format, bool sign, QByteArray hashAlgo = "pgp-sha1");

/**
      * Sets the content-type for the top level of the mime content, based on the crypto format and if a signature is used.
      */
void makeToplevelContentType(KMime::Content *content, Kleo::CryptoMessageFormat format, bool sign, QByteArray hashAlgo = "pgp-sha1");

/**
      * Sets the nested content type of the content, for crypto operations.
      */
void setNestedContentType(KMime::Content *content, Kleo::CryptoMessageFormat format, bool sign);

/**
      * Sets the nested content dispositions for the crypto operations.
      */
void setNestedContentDisposition(KMime::Content *content, Kleo::CryptoMessageFormat format, bool sign);

/**
      * Helper that returns whether or not the current combination of crypto format and signing choice means that the
      * resulting message will be a mime message or not.
      */
bool makeMultiMime(Kleo::CryptoMessageFormat f, bool sign);

/**
      * Whether or not to make the message signed and multi-part
      */
bool makeMultiPartSigned(Kleo::CryptoMessageFormat f);

MESSAGECOMPOSER_EXPORT QByteArray selectCharset(const QList<QByteArray> &charsets,
        const QString &text);

MESSAGECOMPOSER_EXPORT QStringList AttachmentKeywords();
MESSAGECOMPOSER_EXPORT QString cleanedUpHeaderString(const QString &s);

void addSendReplyForwardAction(const KMime::Message::Ptr &message, MailTransport::MessageQueueJob *qjob);
MESSAGECOMPOSER_EXPORT bool sendMailDispatcherIsOnline(QWidget *parent = 0);
MESSAGECOMPOSER_EXPORT QString rot13(const QString &s);
MESSAGECOMPOSER_EXPORT void addTextBox(QTextEdit *edit);
}

}

#endif
