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
#include "iconnamecache.h"

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kascii.h>
#include <klocale.h>
#include <kcharsets.h>

#include <QTextCodec>

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
  //TODO(Andras) add methods to modify these prefixes
  mReplySubjPrefixes << "Re\\s*:" << "Re\\[\\d+\\]:" << "Re\\d+:";
  mForwardSubjPrefixes << "Fwd:" << "FW:";

  mLocalCodec = QTextCodec::codecForName( KGlobal::locale()->encoding() );

  // In the case of Japan. Japanese locale name is "eucjp" but
  // The Japanese mail systems normally used "iso-2022-jp" of locale name.
  // We want to change locale name from eucjp to iso-2022-jp at KMail only.

  // (Introduction to i18n, 6.6 Limit of Locale technology):
  // EUC-JP is the de-facto standard for UNIX systems, ISO 2022-JP
  // is the standard for Internet, and Shift-JIS is the encoding
  // for Windows and Macintosh.
  if ( mLocalCodec->name().toLower() == "eucjp"
#if defined Q_WS_WIN || defined Q_WS_MACX
    || mLocalCodec->name().toLower() == "shift-jis" // OK?
#endif
  )
  {
    mLocalCodec = QTextCodec::codecForName("jis7");
    // QTextCodec *cdc = QTextCodec::codecForName("jis7");
    // QTextCodec::setCodecForLocale(cdc);
    // KGlobal::locale()->setEncoding(cdc->mibEnum());
  }

}

NodeHelper::~NodeHelper()
{
  mSelf = 0;
}

void NodeHelper::setNodeProcessed(KMime::Content* node, bool recurse )
{
  if ( !node )
    return;
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
  if ( !node )
    return;
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
  if ( !node )
    return true;
  return mProcessedNodes.contains( node );
}

void NodeHelper::clear()
{
  mProcessedNodes.clear();
  mEncryptionState.clear();
  mSignatureState.clear();
  qDeleteAll(mUnencryptedMessages);
  mUnencryptedMessages.clear();
  mOverrideCodecs.clear();
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

KMime::Content *NodeHelper::firstChild( KMime::Content* node )
{
  if ( !node )
    return 0;

  KMime::Content *child = 0;
  if ( !node->contents().isEmpty() )
    child = node->contents().at(0);

  return child;
}

KMime::Content *NodeHelper::nextSibling( KMime::Content* node )
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

QString NodeHelper::iconName( KMime::Content *node, int size ) const
{
  if ( !node )
    return QString();

  QByteArray mimeType = node->contentType()->mimeType();
  kAsciiToLower( mimeType.data() );

  QString fileName;
  KMimeType::Ptr mime = KMimeType::mimeType( mimeType, KMimeType::ResolveAliases );
  if (mime) {
    fileName = mime->iconName();
  } else {
    kWarning() <<"unknown mimetype" << mimeType;
  }

  if ( fileName.isEmpty() )
  {
    fileName = node->contentDisposition()->filename();
    if ( fileName.isEmpty() ) fileName = node->contentType()->name();
    if ( !fileName.isEmpty() )
    {
      fileName = KMimeType::findByPath( "/tmp/"+fileName, 0, true )->iconName();
    }
  }

  return IconNameCache::instance()->iconPath( fileName, size );
}

void NodeHelper::magicSetType( KMime::Content* node, bool aAutoDecode )
{
  const QByteArray body = ( aAutoDecode ) ? node->decodedContent() : node->body() ;
  KMimeType::Ptr mime = KMimeType::findByContent( body );

  QString mimetype = mime->name();
  node->contentType()->setMimeType( mimetype.toLatin1() );
}

// static
QString NodeHelper::replacePrefixes( const QString& str,
                                    const QStringList& prefixRegExps,
                                    bool replace,
                                    const QString& newPrefix )
{
  bool recognized = false;
  // construct a big regexp that
  // 1. is anchored to the beginning of str (sans whitespace)
  // 2. matches at least one of the part regexps in prefixRegExps
  QString bigRegExp = QString::fromLatin1("^(?:\\s+|(?:%1))+\\s*")
                      .arg( prefixRegExps.join(")|(?:") );
  QRegExp rx( bigRegExp, Qt::CaseInsensitive );
  if ( !rx.isValid() ) {
    kWarning() << "bigRegExp = \""
                   << bigRegExp << "\"\n"
                   << "prefix regexp is invalid!";
    // try good ole Re/Fwd:
    recognized = str.startsWith( newPrefix );
  } else { // valid rx
    QString tmp = str;
    if ( rx.indexIn( tmp ) == 0 ) {
      recognized = true;
      if ( replace )
        return tmp.replace( 0, rx.matchedLength(), newPrefix + ' ' );
    }
  }
  if ( !recognized )
    return newPrefix + ' ' + str;
  else
    return str;
}

QString NodeHelper::cleanSubject( KMime::Message* message ) const
{
  return cleanSubject( message, mReplySubjPrefixes + mForwardSubjPrefixes,
           true, QString() ).trimmed();
}

QString NodeHelper::cleanSubject( KMime::Message* message, const QStringList & prefixRegExps,
                                 bool replace,
                                 const QString & newPrefix ) const
{
  return NodeHelper::replacePrefixes( message->subject()->asUnicodeString(), prefixRegExps, replace,
                                     newPrefix );
}

void NodeHelper::attachUnencryptedMessage( KMime::Message* message, KMime::Message* unencrypted)
{
 if ( !message )
  return;

 if ( mUnencryptedMessages.contains( message ) )
    delete mUnencryptedMessages[message];
  mUnencryptedMessages[message] = unencrypted;
}

void NodeHelper::setOverrideCodec( KMime::Content* node, const QTextCodec* codec )
{
  if (! node )
    return;

  mOverrideCodecs[node] = codec;
}

const QTextCodec * NodeHelper::codec( KMime::Content* node )
{
  if (! node )
    return mLocalCodec;

  const QTextCodec *c = mOverrideCodecs[node];
  if ( !c ) {
    // no override-codec set for this message, try the CT charset parameter:
    c = codecForName( node->contentType()->charset() );
  }
  /*FIXME(Andras) read from configuration or provide a method to se the fallback codec
  if ( !c ) {
    // Ok, no override and nothing in the message, let's use the fallback
    // the user configured
    c = KMMsgBase::codecForName( GlobalSettings::self()->fallbackCharacterEncoding().toLatin1() );
  }
  */
  if ( !c ) {
    // no charset means us-ascii (RFC 2045), so using local encoding should
    // be okay
    c = mLocalCodec;
  }
  return c;
}

const QTextCodec* NodeHelper::codecForName(const QByteArray& _str)
{
  if (_str.isEmpty())
    return 0;
  QByteArray codec = _str;
  kAsciiToLower(codec.data());
  return KGlobal::charsets()->codecForName(codec);
}


}
