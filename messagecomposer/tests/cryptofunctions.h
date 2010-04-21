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
#include <gpgme++/key.h>
#include <QByteArray>
#include <messageviewer/objecttreeemptysource.h>

namespace KMime {
  class Content;
}

// We can't use EmptySource, since that doesn't provide a HTML writer. Therefore, derive
// from EmptySource so we can provide our own HTML writer.
// This is only needed because ObjectTreeParser has a bug and doesn't decrypt inline PGP messages
// when there is no HTML writer, see FIXME comment in ObjectTreeParser::writeBodyString().
class TestObjectTreeSource : public MessageViewer::EmptySource
{
  public:
    TestObjectTreeSource( MessageViewer::HtmlWriter *writer,
                          MessageViewer::CSSHelper *cssHelper )
      : mWriter( writer ), mCSSHelper( cssHelper )
    {
    }

    virtual MessageViewer::HtmlWriter * htmlWriter() { return mWriter; }
    virtual MessageViewer::CSSHelper * cssHelper() { return mCSSHelper; }

  private:
    MessageViewer::HtmlWriter *mWriter;
    MessageViewer::CSSHelper *mCSSHelper;
};

namespace ComposerTestUtil
{
  /**
  * Returns list of keys used in various crypto routines
  */
  std::vector<GpgME::Key> getKeys( bool smime = false );

  /**
  * Verifies that the given MIME content is signed and that the text is equal.
  */
  bool verifySignature( KMime::Content* content, QByteArray signedContent, Kleo::CryptoMessageFormat f );

  /**
  * Verifies that the given MIME content is encrypted, and that the text is equal
  */
  bool verifyEncryption( KMime::Content* content, QByteArray encrContent, Kleo::CryptoMessageFormat f );

  /**
  * Verifies that the given MIME content is signed and then encrypted, and the original text is as specified
  */
  bool verifySignatureAndEncryption( KMime::Content* content, QByteArray origContent, Kleo::CryptoMessageFormat f, bool withAttachment = false );


}

#endif
