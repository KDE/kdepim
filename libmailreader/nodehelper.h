/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KMAILNODEHELPER_H
#define KMAILNODEHELPER_H

#include <QList>
#include <QMap>

namespace KMime {
  class Content;
}

namespace KMail {
/**
  @author Andras Mantia <andras@kdab.net>
*/

/** Flags for the encryption state. */
typedef enum
{
    KMMsgEncryptionStateUnknown=' ',
    KMMsgNotEncrypted='N',
    KMMsgPartiallyEncrypted='P',
    KMMsgFullyEncrypted='F',
    KMMsgEncryptionProblematic='X'
} KMMsgEncryptionState;

/** Flags for the signature state. */
typedef enum
{
    KMMsgSignatureStateUnknown=' ',
    KMMsgNotSigned='N',
    KMMsgPartiallySigned='P',
    KMMsgFullySigned='F',
    KMMsgSignatureProblematic='X'
} KMMsgSignatureState;

class NodeHelper{
public:
    static NodeHelper * instance();

    ~NodeHelper();

    void setNodeProcessed( KMime::Content* node, bool recurse );
    bool nodeProcessed( KMime::Content* node ) const;
    void clear();

    void setEncryptionState( KMime::Content* node, const KMMsgEncryptionState state );
    KMMsgEncryptionState encryptionState( KMime::Content *node ) const;

    void setSignatureState( KMime::Content* node, const KMMsgSignatureState state );
    KMMsgSignatureState signatureState( KMime::Content *node ) const;

    KMMsgSignatureState overallSignatureState( KMime::Content* node ) const;
    KMMsgEncryptionState overallEncryptionState( KMime::Content *node ) const;

    KMime::Content *nextSibling( KMime::Content* node ) const;
    KMime::Content *firstChild( KMime::Content* node ) const;

private:
    NodeHelper();
    static NodeHelper * mSelf;

    QList<KMime::Content*> mProcessedNodes;
    QMap<KMime::Content *, KMMsgEncryptionState> mEncryptionState;
    QMap<KMime::Content *, KMMsgSignatureState> mSignatureState;
};

}

#endif
