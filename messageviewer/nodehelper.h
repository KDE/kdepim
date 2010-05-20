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

#ifndef _MESSAGEVIEWER_NODEHELPER_H
#define _MESSAGEVIEWER_NODEHELPER_H
#include "messageviewer_export.h"

#include "partmetadata.h"

#include <KMime/Message>

#include <KIconLoader>

#include <QList>
#include <QMap>
#include <QSet>

class KUrl;
class QTextCodec;

namespace MessageViewer {
  namespace Interface {
    class BodyPartMemento;
  }
}

namespace MessageViewer {

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


/**
 * @author Andras Mantia <andras@kdab.net>
 */
class MESSAGEVIEWER_EXPORT NodeHelper {
public:
    NodeHelper();

    ~NodeHelper();

    void setNodeProcessed( KMime::Content* node, bool recurse );
    void setNodeUnprocessed( KMime::Content* node, bool recurse );
    bool nodeProcessed( KMime::Content* node ) const;
    void clear();

    void setEncryptionState( KMime::Content* node, const KMMsgEncryptionState state );
    KMMsgEncryptionState encryptionState( KMime::Content *node ) const;

    void setSignatureState( KMime::Content* node, const KMMsgSignatureState state );
    KMMsgSignatureState signatureState( KMime::Content *node ) const;

    KMMsgSignatureState overallSignatureState( KMime::Content* node ) const;
    KMMsgEncryptionState overallEncryptionState( KMime::Content *node ) const;

    void setPartMetaData( KMime::Content* node, const PartMetaData &metaData );
    PartMetaData partMetaData( KMime::Content* node );

    QString iconName( KMime::Content *node, int size = KIconLoader::Desktop ) const;

    /**
     *  Set the 'Content-Type' by mime-magic from the contents of the body.
     *  If autoDecode is true the decoded body will be used for mime type
     *  determination (this does not change the body itself).
     */
    void magicSetType( KMime::Content *node, bool autoDecode=true );


    /**
     *  Return this mails subject, with all "forward" and "reply"
     *  prefixes removed
     */
    static QString cleanSubject( KMime::Message *message );

    /** Attach an unencrypted message to an encrypted one */
    void attachUnencryptedMessage( KMime::Message::Ptr message, KMime::Message::Ptr unencrypted );

    /** Attach an extra node to an existing node */
    void attachExtraContent( KMime::Content *topLevelNode, KMime::Content* content );
    void removeAllExtraContent( KMime::Content *topLevelNode );

    /** Get the extra nodes attached to the @param topLevelNode and all sub-nodes of @param topLevelNode */
    QList<KMime::Content*> extraContents( KMime::Content *topLevelNode );

    /** Return a modified message (node tree) starting from @param topLevelNode that has the original nodes and the extra nodes.
        The caller has the responsibility to delete the new message.
     */
    KMime::Message *messageWithExtraContent( KMime::Content* topLevelNode );

     /** Get a QTextCodec suitable for this message part */
    const QTextCodec * codec( KMime::Content* node );

    /** Set the charset the user selected for the message to display */
    void setOverrideCodec( KMime::Content* node, const QTextCodec* codec );

    const QTextCodec * localCodec() const { return mLocalCodec;}

    Interface::BodyPartMemento *bodyPartMemento( KMime::Content* node, const QByteArray &which ) const;

    void setBodyPartMemento( KMime::Content* node, const QByteArray &which,
                             Interface::BodyPartMemento *memento );

    // A flag to remember if the node was embedded. This is useful for attachment nodes, the reader
    // needs to know if they were displayed inline or not.
    bool isNodeDisplayedEmbedded( KMime::Content* node ) const;
    void setNodeDisplayedEmbedded( KMime::Content* node, bool displayedEmbedded );

    /**
     * Writes the given message part to a temporary file and returns the
     * name of this file or QString() if writing failed.
     */
    QString writeNodeToTempFile( KMime::Content* node );

    /**
     * Returns the temporary file path and name where this node was saved, or an empty url
     * if it wasn't saved yet with writeNodeToTempFile()
     */
    KUrl tempFileUrlFromNode( const KMime::Content *node );

    /**
     * Creates a temporary dir for saving attachments, etc.
     * Will be automatically deleted when another message is viewed.
     * @param param Optional part of the directory name.
     */
    QString createTempDir( const QString &param = QString() );

    /**
     * Cleanup the attachment temp files
     */
    void removeTempFiles();

    /**
     * Add a file to the list of managed temporary files
     */
    void addTempFile( const QString& file );

    // Get a href in the form attachment:<nodeId>?place=<place>, used by ObjectTreeParser and
    // UrlHandlerManager.
    QString asHREF( const KMime::Content* node, const QString &place );

    static bool isToltecMessage( KMime::Content* node );

    /**
     * Returns the charset for the given node. If no charset is specified
     * for the node, the defaultCharset() is returned.
     */
    static QByteArray charset( KMime::Content *node );

    /**
     * Check for prefixes @p prefixRegExps in @p str. If none
     * is found, @p newPrefix + ' ' is prepended to @p str and the
     * resulting string is returned. If @p replace is true, any
     * sequence of whitespace-delimited prefixes at the beginning of
     * @p str is replaced by @p newPrefix.
     */
    static QString replacePrefixes( const QString& str,
                                  const QStringList& prefixRegExps,
                                  bool replace,
                                  const QString& newPrefix );

    /**
     * Return a QTextCodec for the specified charset.
     * This function is a bit more tolerant, than QTextCodec::codecForName
     */
    static const QTextCodec* codecForName(const QByteArray& _str);

    static QByteArray path(const KMime::Content* node);

    /**
     * Returns a usable filename for a node, that can be the filename from the
     * content disposition header, or if that one is empty, the name from the
     * content type header.
     */
    static QString fileName( const KMime::Content *node );

    /**
     * Fixes an encoding received by a KDE function and returns the proper,
     * MIME-compilant encoding name instead.
     * @see encodingForName
     */
    static QString fixEncoding( const QString &encoding ); //TODO(Andras) move to a utility class?

    /**
     * Drop-in replacement for KCharsets::encodingForName(). The problem with
     * the KCharsets function is that it returns "human-readable" encoding names
     * like "ISO 8859-15" instead of valid encoding names like "ISO-8859-15".
     * This function fixes this by replacing whitespace with a hyphen.
     */
    static QString encodingForName( const QString &descriptiveName ); //TODO(Andras) move to a utility class?

    /**
     * Return a list of the supported encodings
     * @param usAscii if true, US-Ascii encoding will be prepended to the list.
     */
    static QStringList supportedEncodings( bool usAscii ); //TODO(Andras) move to a utility class?

    static QByteArray autoDetectCharset(const QByteArray &_encoding, const QStringList &encodingList, const QString &text);
    static QByteArray toUsAscii(const QString& _str, bool *ok);

    static QString fromAsString( KMime::Content* node );
private:

    /** Check for prefixes @p prefixRegExps in #subject(). If none
        is found, @p newPrefix + ' ' is prepended to the subject and the
        resulting string is returned. If @p replace is true, any
        sequence of whitespace-delimited prefixes at the beginning of
        #subject() is replaced by @p newPrefix
    **/
    static QString cleanSubject( KMime::Message *message, const QStringList& prefixRegExps,
                                 bool replace, const QString& newPrefix );

    void clearBodyPartMemento( QMap<QByteArray, Interface::BodyPartMemento*> bodyPartMementoMap );

    void mergeExtraNodes( KMime::Content *node );
    void cleanFromExtraNodes( KMime::Content *node );

    QList<KMime::Content*> mProcessedNodes;
    QList<KMime::Content*> mNodesUnderProcess;
    QMap<KMime::Content *, KMMsgEncryptionState> mEncryptionState;
    QMap<KMime::Content *, KMMsgSignatureState> mSignatureState;
    QMap<KMime::Message::Ptr, KMime::Message::Ptr > mUnencryptedMessages;
    QSet<KMime::Content *> mDisplayEmbeddedNodes;
    QTextCodec *mLocalCodec;
    QMap<KMime::Content*, const QTextCodec*> mOverrideCodecs;
    QMap<KMime::Content*, QMap<QByteArray, Interface::BodyPartMemento*> > mBodyPartMementoMap;
    QStringList mTempFiles;
    QStringList mTempDirs;
    QMap<KMime::Content*, PartMetaData> mPartMetaDatas;
    QMap<KMime::Message::Content*, QList<KMime::Content*> > mExtraContents;
};

}

#endif
