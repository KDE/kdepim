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
#include "globalsettings.h"
#include "partmetadata.h"
#include "interfaces/bodypart.h"
#include "util.h"

#include <messagecore/nodehelper.h>
#include "messagecore/globalsettings.h"

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kascii.h>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kde_file.h>
#include <kpimutils/kfileio.h>

#include <QDir>
#include <QTextCodec>



namespace MessageViewer {

QStringList replySubjPrefixes(QStringList() << "Re\\s*:" << "Re\\[\\d+\\]:" << "Re\\d+:");
QStringList forwardSubjPrefixes( QStringList() << "Fwd:" << "FW:");

NodeHelper::NodeHelper()
{
  //TODO(Andras) add methods to modify these prefixes

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
}

void NodeHelper::setNodeProcessed(KMime::Content* node, bool recurse )
{
  if ( !node )
    return;
  mProcessedNodes.append( node );
  kDebug() << "Node processed: " << node->index().toString() << node->contentType()->as7BitString() << " decodedContent" << node->decodedContent();
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

  //avoid double addition of extra nodes, eg. encrypted attachments
  for ( QMap<KMime::Message::Ptr, QList<KMime::Content*> >::iterator it = mExtraContents.begin(); it != mExtraContents.end(); ++it) {
    if ( node == dynamic_cast<KMime::Content*>( it.key().get() ) ) {
      qDeleteAll( it.value() );
      kDebug() << "mExtraContents deleted for" << it.key().get() ;
      mExtraContents.remove( it.key() );
    }
  }

  kDebug() << "Node UNprocessed: " << node;
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
  mUnencryptedMessages.clear();
  mOverrideCodecs.clear();
  for ( QMap<KMime::Content*, QMap< QByteArray, Interface::BodyPartMemento*> >::iterator
        it = mBodyPartMementoMap.begin(), end = mBodyPartMementoMap.end();
        it != end; ++it ) {
    clearBodyPartMemento( it.value() );
  }
  mBodyPartMementoMap.clear();

  for ( QMap<KMime::Message::Ptr, QList<KMime::Content*> >::iterator it = mExtraContents.begin(); it != mExtraContents.end(); ++it) {
    qDeleteAll( it.value() );
    kDebug() << "mExtraContents deleted for" << it.key().get() ;
  }
  mExtraContents.clear();
  mDisplayEmbeddedNodes.clear();
}


void NodeHelper::clearBodyPartMemento(QMap<QByteArray, Interface::BodyPartMemento*> bodyPartMementoMap)
{
  for ( QMap<QByteArray, Interface::BodyPartMemento*>::iterator
        it = bodyPartMementoMap.begin(), end = bodyPartMementoMap.end();
        it != end; ++it ) {
    Interface::BodyPartMemento *memento = it.value();
    memento->detach();
    delete memento;
  }
  bodyPartMementoMap.clear();
}


void NodeHelper::setEncryptionState( KMime::Content* node, const KMMsgEncryptionState state )
{
  mEncryptionState[node] = state;
}

KMMsgEncryptionState NodeHelper::encryptionState( KMime::Content *node ) const
{
  if ( mEncryptionState.contains( node ) )
    return mEncryptionState[node];

  return KMMsgNotEncrypted;
}

void NodeHelper::setSignatureState( KMime::Content* node, const KMMsgSignatureState state )
{
  mSignatureState[node] = state;
}

KMMsgSignatureState NodeHelper::signatureState( KMime::Content *node ) const
{
  if ( mSignatureState.contains( node ) )
    return mSignatureState[node];

  return KMMsgNotSigned;
}

PartMetaData NodeHelper::partMetaData(KMime::Content* node)
{
  return mPartMetaDatas.value( node );
}

void NodeHelper::setPartMetaData(KMime::Content* node, const PartMetaData& metaData)
{
  mPartMetaDatas.insert( node, metaData );
}

QString NodeHelper::writeNodeToTempFile(KMime::Content* node)
{
  // If the message part is already written to a file, no point in doing it again.
  // This function is called twice actually, once from the rendering of the attachment
  // in the body and once for the header.
  KUrl existingFileName = tempFileUrlFromNode( node );
  if ( !existingFileName.isEmpty() ) {
    return existingFileName.toLocalFile();
  }

  QString fileName = NodeHelper::fileName( node );

  QString fname = createTempDir( node->index().toString() );
  if ( fname.isEmpty() )
    return QString();

  // strip off a leading path
  int slashPos = fileName.lastIndexOf( '/' );
  if( -1 != slashPos )
    fileName = fileName.mid( slashPos + 1 );
  if( fileName.isEmpty() )
    fileName = "unnamed";
  fname += '/' + fileName;

  kDebug() << "Create temp file: " << fname;

  QByteArray data = node->decodedContent();
  if ( node->contentType()->isText() && data.size() > 0 ) {
    // convert CRLF to LF before writing text attachments to disk
    data = KMime::CRLFtoLF( data );
  }
  if( !KPIMUtils::kByteArrayToFile( data, fname, false, false, false ) )
    return QString();

  mTempFiles.append( fname );
  // make file read-only so that nobody gets the impression that he might
  // edit attached files (cf. bug #52813)
  ::chmod( QFile::encodeName( fname ), S_IRUSR );

  return fname;
}



KUrl NodeHelper::tempFileUrlFromNode( const KMime::Content *node )
{
  if (!node)
    return KUrl();

  QString index = node->index().toString();

  foreach ( const QString &path, mTempFiles ) {
    int right = path.lastIndexOf( '/' );
    int left = path.lastIndexOf( ".index.", right );
    if ( left != -1 )
        left += 7;

    QString storedIndex = path.mid( left, right - left );
    if ( left != -1 && storedIndex == index )
      return KUrl( path );
  }
  return KUrl();
}


QString NodeHelper::createTempDir( const QString &param )
{
  KTemporaryFile *tempFile = new KTemporaryFile();
  tempFile->setSuffix( ".index." + param );
  tempFile->open();
  QString fname = tempFile->fileName();
  delete tempFile;

  if ( ::access( QFile::encodeName( fname ), W_OK ) != 0 ) {
    // Not there or not writable
    if( KDE_mkdir( QFile::encodeName( fname ), 0 ) != 0 ||
        ::chmod( QFile::encodeName( fname ), S_IRWXU ) != 0 ) {
      return QString(); //failed create
    }
  }

  Q_ASSERT( !fname.isNull() );

  mTempDirs.append( fname );
  return fname;
}


void NodeHelper::removeTempFiles()
{
  for (QStringList::Iterator it = mTempFiles.begin(); it != mTempFiles.end();
    ++it)
  {
    QFile::remove(*it);
  }
  mTempFiles.clear();
  for (QStringList::Iterator it = mTempDirs.begin(); it != mTempDirs.end();
    it++)
  {
    QDir(*it).rmdir(*it);
  }
  mTempDirs.clear();
}

void NodeHelper::addTempFile( const QString& file )
{
  mTempFiles.append( file );
}

bool NodeHelper::isToltecMessage( KMime::Content* node )
{
  if ( !node->contentType( false ) )
    return false;

  if ( node->contentType()->mediaType().toLower() != "multipart" ||
       node->contentType()->subType().toLower() != "mixed" )
    return false;

  if ( node->contents().size() != 3 )
    return false;

  const KMime::Headers::Base *libraryHeader = node->headerByType( "X-Library" );
  if ( !libraryHeader )
    return false;

  if ( QString::fromLatin1( libraryHeader->as7BitString( false ) ).toLower() !=
       QLatin1String( "toltec" ) )
    return false;

  const KMime::Headers::Base *kolabTypeHeader = node->headerByType( "X-Kolab-Type" );
  if ( !kolabTypeHeader )
    return false;

  if ( !QString::fromLatin1( kolabTypeHeader->as7BitString( false ) ).toLower().startsWith(
         QLatin1String( "application/x-vnd.kolab" ) ) )
    return false;

  return true;
}

QByteArray NodeHelper::charset( KMime::Content *node )
{
  if ( node->contentType( false ) )
    return node->contentType( false )->charset();
  else
    return node->defaultCharset();
}

KMMsgEncryptionState NodeHelper::overallEncryptionState( KMime::Content *node ) const
{
    KMMsgEncryptionState myState = KMMsgEncryptionStateUnknown;
    if ( !node )
      return myState;

    if( encryptionState( node ) == KMMsgNotEncrypted ) {
        // NOTE: children are tested ONLY when parent is not encrypted
        KMime::Content *child = MessageCore::NodeHelper::firstChild( node );
        if ( child )
            myState = overallEncryptionState( child );
        else
            myState = KMMsgNotEncrypted;
    }
    else { // part is partially or fully encrypted
        myState = encryptionState( node );
    }
    // siblings are tested always
    KMime::Content * next = MessageCore::NodeHelper::nextSibling( node );
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
        KMime::Content* child = MessageCore::NodeHelper::firstChild( node );
        if( child )
            myState = overallSignatureState( child );
        else
            myState = KMMsgNotSigned;
    }
    else { // part is partially or fully signed
        myState = signatureState( node );
    }
    // siblings are tested always
    KMime::Content *next = MessageCore::NodeHelper::nextSibling( node );
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
  return Util::fileNameForMimetype( mimeType, size, node->contentDisposition()->filename(),
                                    node->contentType()->name() );
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

QString NodeHelper::cleanSubject( KMime::Message::Ptr message )
{
  return cleanSubject( message, replySubjPrefixes + forwardSubjPrefixes,
                       true, QString() ).trimmed();
}

QString NodeHelper::cleanSubject( KMime::Message::Ptr message, const QStringList & prefixRegExps,
                                  bool replace,
                                  const QString & newPrefix )
{
  return NodeHelper::replacePrefixes( message->subject()->asUnicodeString(), prefixRegExps, replace,
                                      newPrefix );
}

void NodeHelper::attachUnencryptedMessage( KMime::Message::Ptr message,
                                           KMime::Message::Ptr unencrypted )
{
  if ( !message )
    return;

  mUnencryptedMessages[message] = unencrypted;
}



void NodeHelper::setOverrideCodec( KMime::Content* node, const QTextCodec* codec )
{
  if ( !node )
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
  if ( !c ) {
    // Ok, no override and nothing in the message, let's use the fallback
    // the user configured
    c = codecForName( MessageCore::GlobalSettings::self()->fallbackCharacterEncoding().toLatin1() );
  }
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

QByteArray NodeHelper::path(const KMime::Content* node)
{
  if ( !node->parent() ) {
    return QByteArray( ":" );
  }
  const KMime::Content *p = node->parent();

  // count number of siblings with the same type as us:
  int nth = 0;
  for ( KMime::Content *c = MessageCore::NodeHelper::firstChild(p); c != node; c = MessageCore::NodeHelper::nextSibling(c) ) {
    if ( c->contentType()->mediaType() == const_cast<KMime::Content*>(node)->contentType()->mediaType() && c->contentType()->subType() == const_cast<KMime::Content*>(node)->contentType()->subType() ) {
      ++nth;
    }
  }
  QString subpath;
  return NodeHelper::path(p) + subpath.sprintf( ":%X/%X[%X]", const_cast<KMime::Content*>(node)->contentType()->mediaType().constData(), const_cast<KMime::Content*>(node)->contentType()->subType().constData(), nth ).toLocal8Bit();
}

QString NodeHelper::fileName( const KMime::Content *node )
{
  QString name = const_cast<KMime::Content*>( node )->contentDisposition()->filename();
  if ( name.isEmpty() )
    name = const_cast<KMime::Content*>( node )->contentType()->name();

  name = name.trimmed();
  return name;
}

//FIXME(Andras) review it (by Marc?) to see if I got it right. This is supposed to be the partNode::internalBodyPartMemento replacement
Interface::BodyPartMemento *NodeHelper::bodyPartMemento( KMime::Content *node,
                                                         const QByteArray &which ) const
{
  if ( !mBodyPartMementoMap.contains( node ) )
    return 0;
  const QMap<QByteArray,Interface::BodyPartMemento*>::const_iterator it =
  mBodyPartMementoMap[node].find( which.toLower() );
  return it != mBodyPartMementoMap[node].end() ? it.value() : 0 ;
}

 //FIXME(Andras) review it (by Marc?) to see if I got it right. This is supposed to be the partNode::internalSetBodyPartMemento replacement
void NodeHelper::setBodyPartMemento( KMime::Content* node, const QByteArray &which,
                                     Interface::BodyPartMemento *memento )
{
  const QMap<QByteArray,Interface::BodyPartMemento*>::iterator it =
    mBodyPartMementoMap[node].lowerBound( which.toLower() );

  if ( it != mBodyPartMementoMap[node].end() && it.key() == which.toLower() ) {
    delete it.value();
    if ( memento ) {
      it.value() = memento;
    } else {
      mBodyPartMementoMap[node].erase( it );
    }
  } else {
    mBodyPartMementoMap[node].insert( which.toLower(), memento );
  }
}

bool NodeHelper::isNodeDisplayedEmbedded( KMime::Content* node ) const
{
  kDebug() << "IS NODE: " << mDisplayEmbeddedNodes.contains( node );
  return mDisplayEmbeddedNodes.contains( node );
}

void NodeHelper::setNodeDisplayedEmbedded( KMime::Content* node, bool displayedEmbedded )
{
  kDebug() << "SET NODE: " << node << displayedEmbedded;
  if ( displayedEmbedded )
    mDisplayEmbeddedNodes.insert( node );
  else
    mDisplayEmbeddedNodes.remove( node );
}

QString NodeHelper::asHREF( const KMime::Content* node, const QString &place )
{
  if ( !node )
    return QString();
  else {
    QString indexStr = node->index().toString();
    //if the node is an extra node, prepent the index of the extra node to the url
    for ( QMap<KMime::Message::Ptr, QList<KMime::Content*> >::iterator it = mExtraContents.begin(); it != mExtraContents.end(); ++it) {
      QList<KMime::Content*> extraNodes = it.value();
      for ( uint i = 0; i < extraNodes.size(); ++i )  {
        if ( node->topLevel() == extraNodes[i] ) {
          indexStr.prepend( QString("%1.").arg(i) );
          it = mExtraContents.end();
          --it;
          break;
        }
      }
    }
    return QString( "attachment:%1?place=%2" ).arg( indexStr ).arg( place );
  }
}

QString NodeHelper::fixEncoding( const QString &encoding )
{
  QString returnEncoding = encoding;
  // According to http://www.iana.org/assignments/character-sets, uppercase is
  // preferred in MIME headers
  if ( returnEncoding.toUpper().contains( "ISO " ) ) {
    returnEncoding = returnEncoding.toUpper();
    returnEncoding.replace( "ISO ", "ISO-" );
  }
  return returnEncoding;
}


//-----------------------------------------------------------------------------
QString NodeHelper::encodingForName( const QString &descriptiveName )
{
  QString encoding = KGlobal::charsets()->encodingForName( descriptiveName );
  return NodeHelper::fixEncoding( encoding );
}

QStringList NodeHelper::supportedEncodings(bool usAscii)
{
  QStringList encodingNames = KGlobal::charsets()->availableEncodingNames();
  QStringList encodings;
  QMap<QString,bool> mimeNames;
  for (QStringList::Iterator it = encodingNames.begin();
    it != encodingNames.end(); ++it)
  {
    QTextCodec *codec = KGlobal::charsets()->codecForName(*it);
    QString mimeName = (codec) ? QString(codec->name()).toLower() : (*it);
    if (!mimeNames.contains(mimeName) )
    {
      encodings.append( KGlobal::charsets()->descriptionForEncoding(*it) );
      mimeNames.insert( mimeName, true );
    }
  }
  encodings.sort();
  if (usAscii)
    encodings.prepend(KGlobal::charsets()->descriptionForEncoding("us-ascii") );
  return encodings;
}


QByteArray NodeHelper::autoDetectCharset(const QByteArray &_encoding, const QStringList &encodingList, const QString &text)
{
    QStringList charsets = encodingList;
    if (!_encoding.isEmpty())
    {
       QString currentCharset = QString::fromLatin1(_encoding);
       charsets.removeAll(currentCharset);
       charsets.prepend(currentCharset);
    }

    QStringList::ConstIterator it = charsets.constBegin();
    for (; it != charsets.constEnd(); ++it)
    {
       QByteArray encoding = (*it).toLatin1();
       if (encoding == "locale")
       {
         encoding = QTextCodec::codecForName( KGlobal::locale()->encoding() )->name();
         kAsciiToLower(encoding.data());
       }
       if (text.isEmpty())
         return encoding;
       if (encoding == "us-ascii") {
         bool ok;
         (void) toUsAscii(text, &ok);
         if (ok)
            return encoding;
       }
       else
       {
         const QTextCodec *codec = codecForName(encoding);
         if (!codec) {
           kDebug() << "Auto-Charset: Something is wrong and I can not get a codec:" << encoding;
         } else {
           if (codec->canEncode(text))
              return encoding;
         }
       }
    }
    return 0;
}

QByteArray NodeHelper::toUsAscii(const QString& _str, bool *ok)
{
  bool all_ok =true;
  QString result = _str;
  int len = result.length();
  for (int i = 0; i < len; i++)
    if (result.at(i).unicode() >= 128) {
      result[i] = '?';
      all_ok = false;
    }
  if (ok)
    *ok = all_ok;
  return result.toLatin1();
}

QString NodeHelper::fromAsString( KMime::Content* node )
{
  KMime::Message* topLevel = dynamic_cast<KMime::Message*>( node->topLevel() );
  if ( topLevel )
    return topLevel->from()->asUnicodeString();
  return QString();
}

void NodeHelper::attachExtraContent( KMime::Message::Ptr node, KMime::Content* content )
{
   kDebug() << "mExtraContents added for" << node.get() << " extra content: " << content;
  mExtraContents[node].append( content );
}

void NodeHelper::removeExtraContent(KMime::Message::Ptr node )
{
  if ( mExtraContents.contains( node ) ) {
    qDeleteAll( mExtraContents[node] );
    mExtraContents.remove( node );
  }    
}

QList< KMime::Content* > NodeHelper::extraContents( KMime::Message::Ptr node )
{
 if ( mExtraContents.contains( node ) ) {
    return mExtraContents[node];
 } else
   return QList< KMime::Content* >();
}


}
