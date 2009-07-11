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
#include "nodehelper.h"

#include <kmime/kmime_content.h>

KMail::NodeHelper * KMail::NodeHelper::mSelf = 0;

namespace KMail {

NodeHelper * KMail::NodeHelper::instance()
{
  if ( !mSelf )
    mSelf = new NodeHelper();
  return mSelf;
}

NodeHelper::NodeHelper()
{
  mSelf = this;
}

NodeHelper::~NodeHelper()
{
  mSelf = 0;
}

void NodeHelper::setNodeProcessed(KMime::Content* node, bool recurse )
{
  mProcessedNodes.append( node );
  if ( recurse ) {
    KMime::Content::List contents = node->contents();
    Q_FOREACH( KMime::Content *c, contents )
    {
      setNodeProcessed( c, true );
    }
  }
}

void NodeHelper::setNodeUnprocessed(KMime::Content* node, bool recurse )
{
  mProcessedNodes.removeAll( node );
  if ( recurse ) {
    KMime::Content::List contents = node->contents();
    Q_FOREACH( KMime::Content *c, contents )
    {
      setNodeUnprocessed( c, true );
    }
  }
}

bool NodeHelper::nodeProcessed( KMime::Content* node ) const
{
  return mProcessedNodes.contains( node );
}

void NodeHelper::clear()
{
  mProcessedNodes.clear();
  mEncryptionState.clear();
  mSignatureState.clear();
}

void NodeHelper::setEncryptionState( KMime::Content* node, const KMMsgEncryptionState state )
{
  mEncryptionState[node] = state;
}

KMMsgEncryptionState NodeHelper::encryptionState( KMime::Content *node ) const
{
  if ( mEncryptionState.contains( node ) )
    return mEncryptionState[node];

  return KMMsgEncryptionStateUnknown;
}

void NodeHelper::setSignatureState( KMime::Content* node, const KMMsgSignatureState state )
{
  mSignatureState[node] = state;
}

KMMsgSignatureState NodeHelper::signatureState( KMime::Content *node ) const
{
  if ( mSignatureState.contains( node ) )
    return mSignatureState[node];

  return KMMsgSignatureStateUnknown;
}

KMime::Content *NodeHelper::firstChild( KMime::Content* node ) const
{
  if ( !node )
    return 0;

  KMime::Content *child = 0;
  if ( !node->contents().isEmpty() )
    child = node->contents().at(0);

  return child;
}

KMime::Content *NodeHelper::nextSibling( KMime::Content* node ) const
{
  if ( !node )
    return 0;

  KMime::Content *next = 0;
  KMime::Content *parent = node->parent();
  if ( parent ) {
    KMime::Content::List contents = parent->contents();
    int index = contents.indexOf( node ) + 1;
    if ( index < contents.size() ) //next on the same level
      next =  contents.at( index );
  }

  return next;
}

KMMsgEncryptionState NodeHelper::overallEncryptionState( KMime::Content *node ) const
{
    KMMsgEncryptionState myState = KMMsgEncryptionStateUnknown;
    if ( !node )
      return myState;

    if( encryptionState( node ) == KMMsgNotEncrypted ) {
        // NOTE: children are tested ONLY when parent is not encrypted
        KMime::Content *child = firstChild( node );
        if ( child )
            myState = overallEncryptionState( child );
        else
            myState = KMMsgNotEncrypted;
    }
    else { // part is partially or fully encrypted
        myState = encryptionState( node );
    }
    // siblings are tested always
    KMime::Content * next = nextSibling( node );
    if( next ) {
        KMMsgEncryptionState otherState = overallEncryptionState( next );
        switch( otherState ) {
        case KMMsgEncryptionStateUnknown:
            break;
        case KMMsgNotEncrypted:
            if( myState == KMMsgFullyEncrypted )
                myState = KMMsgPartiallyEncrypted;
            else if( myState != KMMsgPartiallyEncrypted )
                myState = KMMsgNotEncrypted;
            break;
        case KMMsgPartiallyEncrypted:
            myState = KMMsgPartiallyEncrypted;
            break;
        case KMMsgFullyEncrypted:
            if( myState != KMMsgFullyEncrypted )
                myState = KMMsgPartiallyEncrypted;
            break;
        case KMMsgEncryptionProblematic:
            break;
        }
    }

//kDebug() <<"\n\n  KMMsgEncryptionState:" << myState;

    return myState;
}


KMMsgSignatureState NodeHelper::overallSignatureState( KMime::Content* node ) const
{
    KMMsgSignatureState myState = KMMsgSignatureStateUnknown;
    if ( !node )
      return myState;

    if( signatureState( node ) == KMMsgNotSigned ) {
        // children are tested ONLY when parent is not signed
        KMime::Content* child = firstChild( node );
        if( child )
            myState = overallSignatureState( child );
        else
            myState = KMMsgNotSigned;
    }
    else { // part is partially or fully signed
        myState = signatureState( node );
    }
    // siblings are tested always
    KMime::Content *next = nextSibling( node );
    if( next ) {
        KMMsgSignatureState otherState = overallSignatureState( next );
        switch( otherState ) {
        case KMMsgSignatureStateUnknown:
            break;
        case KMMsgNotSigned:
            if( myState == KMMsgFullySigned )
                myState = KMMsgPartiallySigned;
            else if( myState != KMMsgPartiallySigned )
                myState = KMMsgNotSigned;
            break;
        case KMMsgPartiallySigned:
            myState = KMMsgPartiallySigned;
            break;
        case KMMsgFullySigned:
            if( myState != KMMsgFullySigned )
                myState = KMMsgPartiallySigned;
            break;
        case KMMsgSignatureProblematic:
            break;
        }
    }

//kDebug() <<"\n\n  KMMsgSignatureState:" << myState;

    return myState;
}

}
