/*
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

#ifndef CRYPTO_FUNCTIONS_H
#define CRYPTO_FUNCTIONS_H

#include <kleo/enum.h>
#include <KMime/kmime_headers.h>
#include <QByteArray>

namespace KMime {
  class Content;
}

namespace ComposerTestUtil
{

  /**
   * gate function to run verifySignature, verifyEncryption or verifySignatureAndEncryption.
   */

  void verify( bool sign, bool encrypt, KMime::Content* content, QByteArray origContent, Kleo::CryptoMessageFormat f, KMime::Headers::contentEncoding encoding );

  /**
   * Verifies that the given MIME content is signed and that the text is equal
   */
  void verifySignature( KMime::Content* content, QByteArray signedContent, Kleo::CryptoMessageFormat f, KMime::Headers::contentEncoding encoding );

  /**
   * Verifies that the given MIME content is encrypted, and that the text is equal
   */
  void verifyEncryption( KMime::Content* content, QByteArray encrContent, Kleo::CryptoMessageFormat f, bool withAttachment = false );

  /**
   * Verifies that the given MIME content is signed and then encrypted, and the original text is as specified
   */
  void verifySignatureAndEncryption( KMime::Content* content, QByteArray origContent, Kleo::CryptoMessageFormat f, bool withAttachment = false );


}

#endif
