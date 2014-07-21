/*  -*- mode: C++; c-file-style: "gnu" -*-
    objecttreeparser.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>
    Copyright (C) 2002-2003, 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Andras Mantia <andras@kdab.net>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef _MESSAGEVIEWER_OBJECTTREEPARSER_H_
#define _MESSAGEVIEWER_OBJECTTREEPARSER_H_

#include "messageviewer_export.h"

#include "viewer/nodehelper.h"
#include "objecttreesourceif.h"

#include <libkleo/kleo/cryptobackend.h>
#include <gpgme++/verificationresult.h>

#include <QList>
#include "objecttreeemptysource.h"

class QString;

namespace KMime {
class Content;
}

namespace GpgME {
class Error;
class ImportResult;
}


namespace MessageViewer {

class PartMetaData;
class ViewerPrivate;
class HtmlWriter;
class CSSHelper;
class AttachmentStrategy;
class NodeHelper;

class ProcessResult {
public:
    explicit ProcessResult( NodeHelper *nodeHelper, KMMsgSignatureState  inlineSignatureState  = KMMsgNotSigned,
                            KMMsgEncryptionState inlineEncryptionState = KMMsgNotEncrypted,
                            bool neverDisplayInline = false,
                            bool isImage = false )
        : mInlineSignatureState( inlineSignatureState ),
          mInlineEncryptionState( inlineEncryptionState ),
          mNeverDisplayInline( neverDisplayInline ),
          mIsImage( isImage ),
          mNodeHelper( nodeHelper ) {}

    KMMsgSignatureState inlineSignatureState() const {
        return mInlineSignatureState;
    }
    void setInlineSignatureState( KMMsgSignatureState state ) {
        mInlineSignatureState = state;
    }

    KMMsgEncryptionState inlineEncryptionState() const {
        return mInlineEncryptionState;
    }
    void setInlineEncryptionState( KMMsgEncryptionState state ) {
        mInlineEncryptionState = state;
    }

    bool neverDisplayInline() const { return mNeverDisplayInline; }
    void setNeverDisplayInline( bool display ) {
        mNeverDisplayInline = display;
    }

    bool isImage() const { return mIsImage; }
    void setIsImage( bool image ) {
        mIsImage = image;
    }

    void adjustCryptoStatesOfNode( KMime::Content * node ) const;

private:
    KMMsgSignatureState mInlineSignatureState;
    KMMsgEncryptionState mInlineEncryptionState;
    bool mNeverDisplayInline : 1;
    bool mIsImage : 1;
    NodeHelper* mNodeHelper;
};

/**
\brief Parses messages and generates HTML display code out of them

\par Introduction

First, have a look at the documentation in Mainpage.dox and at the documentation of ViewerPrivate
to understand the broader picture.

Just a note on the terminology: 'Node' refers to a MIME part here, which in KMime is a
KMime::Content.

\par Basics

The ObjectTreeParser basically has two modes: Generating the HTML code for the Viewer, or only
extracting the plainTextContent() for situations where only the message text is needed, for example
when inline forwarding a message. The mode depends on the ObjectTreeSourceIf passed to the
constructor: If ObjectTreeSourceIf::htmlWriter() is not 0, then the HTML code generation mode is
used.

Basically, all the ObjectTreeParser does is going through the tree of MIME parts and operating on
those nodes. Operating here means creating the HTML code for the node or extracting the textual
content from it. This process is started with parseObjectTree(), where we loop over the subnodes
of the current root node. For each of those subnodes, we try to find a BodyPartFormatter that can
handle the type of the node. This can either be an internal function, such as
processMultiPartAlternativeSubtype() or processTextHtmlSubtype(), or it can be an external plugin.
More on external plugins later. When no matching formatter is found, defaultHandling() is called
for that node.

\par Multipart Nodes

Those nodes that are of type multipart have subnodes. If one of those children needs to be
processed normally, the processMultipartXXX() functions call stdChildHandling() for the node that
should be handled normally. stdChildHandling() creates its own ObjectTreeParser, which is a clone
of the current ObjectTreeParser, and processes the node. stdChildHandling() is not called for all
children of the multipart node, for example processMultiPartAlternativeSubtype() only calls it on
one of the children, as the other one doesn't need to be displayed. Similary,
processMultiPartSignedSubtype() doesn't call stdChildHandling() for the signature node, only for the
signed node.

\par Processed and Unprocessed Nodes

When a BodyPartFormatter has finished processing a node, it is processed. Nodes are set to being
not processed at the beginning of parseObjectTree(). The processed state of a node is saved in a
list in NodeHelper, see NodeHelper::setNodeProcessed(), NodeHelper::nodeProcessed() and the other
related helper functions.

It is the responsibility of the BodyPartFormatter to correctly call setNodeProcessed() and the
related functions. This is important so that processing the same node twice can be prevented. The
check that prevents duplicate processing is in parseObjectTree().

An example where duplicate processing would happen if we didn't check for it is in stdChildHandling(),
which is for example called from processMultiPartAlternativeSubtype(). Let's say the setting is to
prefer HTML over plain text. In this case, processMultiPartAlternativeSubtype() would call
stdChildHandling() on the HTML node, which would create a new ObjectTreeParser and call
parseObjectTree() on it. parseObjectTree() processes the node and all its siblings, and one of the
siblings is the plain text node, which shouldn't be processed! Therefore
processMultiPartAlternativeSubtype() sets the plain text node as been processed already.

\par Plain Text Output

Various nodes have plain text that should be displayed. This plain text is usually processed though
writeBodyString() first. That method checks if the provided text is an inline PGP text and decrypts
it if necessary. It also pushes the text through quotedHTML(), which does a number of things like
coloring quoted lines or detecting links and creating real link tags for them.

\par Modifying the Message

The ObjectTreeParser does not only parse its message, in some circumstances it also modifies it
before displaying. This is for example the case when displaying a decrypted message: The original
message only contains a binary blob of crypto data, and processMultiPartEncryptedSubtype() decrypts
that blob. After decryption, the current node is replaced with the decrypted node, which happens
in insertAndParseNewChildNode().

\par Crypto Operations

For signature and decryption handling, there are functions which help with generating the HTML code
for the signature header and footer. These are writeDeferredDecryptionBlock(), writeSigstatFooter()
and writeSigstatHeader(). As the name writeDeferredDecryptionBlock() suggests, a setting can cause
the message to not be decrypted unless the user clicks a link. Whether the message should be
decrypted or not can be controlled by ObjectTreeSourceIf::decryptMessage(). When the user clicks the
decryption link, the URLHandler for 'kmail:' URLs sets that variable to true and triggers an update
of the Viewer, which will cause parseObjectTree() to be called again.

\par Async Crypto Operations

The above case describes decryption the message in place. However, decryption and also verifying of
the signature can take a long time, so synchronous decryption and verifing would cause the Viewer to
block. Therefore it is possible to run these operations in async mode, see allowAsync().
In the first run of the async mode, all the ObjectTreeParser does is starting the decrypt or the
verify job, and informing the user that the operation is in progress with
writeDecryptionInProgressBlock() or with writeSigstatHeader(). Then, it creates and associates a
BodyPartMemento with the current node, for example a VerifyDetachedBodyPartMemento. Each node can
have multiple mementos associated with it, which are differeniated by name.

NodeHelper::setBodyPartMemento() and NodeHelper::bodyPartMemento() provide means to store and
retrieve these mementos. A memento is basically a thin wrapper around the crypto job, it stores the
job pointer, the job input data and the job result. Mementos can be used for any async situation,
not just for crypto jobs, but I'll describe crypto jobs here.

So in the first run of decrypting or verifying a message, the BodyPartFormatter only starts the
crypto job, creates the BodyPartMemento and writes the HTML code that tells the user that the
operation is in progress. parseObjectTree() thus finishes without waiting for anything, and the
message is displayed.

At some point, the crypto jobs then finish, which will cause slotResult() of the BodyPartMemento
to be called. slotResult() then saves the result to some member variable and calls
BodyPartMemento::notify(), which in the end will trigger an update of the Viewer. That update
will, in ViewerPrivate::parseMsg(), create a new ObjectTreeParser and call parseObjectTree() on it.
This is where the second run begins.

The functions that deal with decrypting of verifying, like processMultiPartSignedSubtype() or
processMultiPartEncryptedSubtype() will look if they find a BodyPartMemento that is associated with
the current node. Now it finds that memento, since it was created in the first run. It checks if the
memento's job has finished, and if so, the result can be written out (either the decrypted data or
the verified signature).

When dealing with encrypted nodes, new nodes are created with the decrypted data. It is important to
note that the original MIME tree is never modified, and remains the same as the original one. The method
createAndParseTempNode is called with the newly decrypted data, and it generates a new temporary node to
store the decrypted data. When these nodes are created, it is important to keep track of them as otherwise
some mementos that are added to the newly created temporary nodes will be constantly regenerated. As the
regeneration triggers a viewer update when complete, it results in an infinite refresh loop. The function
NodeHelper::linkAsPermanentDecrypted will create a link between the newly created node and the original parent.
Conversely, the function NodeHelper::attachExtraContent will create a link in the other direction, from the parent
node to the newly created temporary node.

When generating some mementos for nodes that may be temporary nodes (for example, contact photo mementos), the
function NodeHelper::setBodyPartMementoForPermanentParent is used. This will save the given body part memento for
the closest found permanent parent node, rather than the transient node itself. Then when checking for the existence
of a certain memento in a node, NodeHelper::findPermanentParentBodyPartMemento will check to see if any parent of the
given temporary node is a permanent (encrypted) node that has been used to generate the asked-for node.

To conclude: For async operations, parseObjectTree() is called twice: The first call starts the
crypto operation and creates the BodyPartMemento, the second calls sees that the BodyPartMemento is
there and can use its result for writing out the HTML.

\par PartMetaData and ProcessResult

For crypto operations, the class PartMetaData is used a lot, mainly to pass around info about the
crypto state of a node. A PartMetaData can also be associated with a node by using
NodeHelper::setPartMetaData(). The only user of that however is MessageAnalyzer::processPart() of
the Nepomuk E-Mail Feeder, which also uses the ObjectTreeParser to analyze the message.

You'll notice that a ProcessResult is passed to each formatter. The formatter is supposed to modify
the ProcessResult to tell the callers something about the state of the nodes that were processed.
One example for its use is to tell the caller about the crypto state of the node.

\par BodyPartFormatter Plugins

As mentioned way earlier, BodyPartFormatter can either be plugins or be internal. bodypartformatter.cpp
contains some trickery so that the processXXX() methods of the ObjectTreeParser are called from
a BodyPartFormatter associated with them, see the CREATE_BODY_PART_FORMATTER macro.

The BodyPartFormatter code is work in progress, it was supposed to be refactored, but that has not
yet happened at the time of writing. Therefore the code can seem a bit chaotic.

External plugins are loaded with loadPlugins() in bodypartformatterfactory.cpp. External plugins
can only use the classes in the interfaces/ directory, they include BodyPart, BodyPartMemento,
BodyPartFormatterPlugin, BodyPartFormatter, BodyPartURLHandler, HtmlWriter and URLHandler. Therefore
external plugins have powerful capabilities, which are needed for example in the iCal formatter or
in the vCard formatter.

\par Special HTML tags

As also mentioned in the documentation of ViewerPrivate, the ObjectTreeParser writes out special
links that are only understood by the viewer, for example 'kmail:' URLs or 'attachment:' URLs.
Also, some special HTML tags are created, which the Viewer later uses for post-processing. For
example a div with the id 'attachmentInjectionPoint', or a div with the id 'attachmentDiv', which
is used to mark an attachment in the body with a yellow border when the user clicks the attachment
in the header. Finally, parseObjectTree() creates an anchor with the id 'att%1', which is used in
the Viewer to scroll to the attachment.
*/
class MESSAGEVIEWER_EXPORT ObjectTreeParser {
    class CryptoProtocolSaver;
    /**
   * @internal
   * Copies the context of @p other, but not it's rawDecryptedBody, plainTextContent or htmlContent.
   */
    ObjectTreeParser( const ObjectTreeParser & other );

public:
    explicit ObjectTreeParser( ObjectTreeSourceIf * source,
                               NodeHelper *nodeHelper = 0,
                               const Kleo::CryptoBackend::Protocol * protocol=0,
                               bool showOneMimePart=false, bool keepEncryptions=false,
                               bool includeSignatures=true,
                               const AttachmentStrategy * attachmentStrategy=0 );

    explicit ObjectTreeParser( const ObjectTreeParser *topLevelParser,
                               bool showOneMimePart=false, bool keepEncryptions=false,
                               bool includeSignatures=true,
                               const AttachmentStrategy * attachmentStrategy=0 );
    virtual ~ObjectTreeParser();

    void setAllowAsync( bool allow ) { assert( !mHasPendingAsyncJobs ); mAllowAsync = allow; }
    bool allowAsync() const { return mAllowAsync; }

    bool hasPendingAsyncJobs() const { return mHasPendingAsyncJobs; }

    /**
   * The origin and purpose of this function is unknown, the ancient wisdom about it got lost during
   * the centuries.
   *
   * Historicans believe that the intent of the function is to return the raw body of the mail,
   * i.e. no charset decoding has been done yet. Sometimes CTE decoding has been done, sometimes
   * not. For encrypted parts, this returns the content of the decrypted part. For a mail with
   * multiple MIME parts, the results are conecated together. Not all parts are included in this.
   *
   * Although conecating multiple undecoded body parts with potentially different CTEs together might
   * not seem to make any sense in these modern times, it is assumed that initially this function
   * performed quite well, but the ancient scrolls got damaged with the ravages of time
   * and were re-written multiple times.
   *
   * Do not use. Use plainTextContent() and htmlContent() instead.
   */
    MESSAGEVIEWER_DEPRECATED_EXPORT QByteArray rawDecryptedBody() const { return mRawDecryptedBody; }

    /**
   * The text of the message, ie. what would appear in the
   * composer's text editor if this was edited or replied to.
   * This is usually the content of the first text/plain MIME part.
   */
    QString plainTextContent() const { return mPlainTextContent; }

    /**
   * Similar to plainTextContent(), but returns the HTML source of the first text/html MIME part.
   *
   * Not to be consfused with the HTML code that the message viewer widget displays, that HTML
   * is written out by htmlWriter() and a totally different pair of shoes.
   */
    QString htmlContent() const { return mHtmlContent; }

    /**
   * Returns a plain text version of the content, which is either plainTextContent() if that exists,
   * or htmlContent() converted to plain text otherwise.
   */
    QString convertedTextContent() const;

    /** Returns a HTML version of the plain text mail. If the HTML content is already available, it
   * returns the HTML content as it is.
   */
    QString convertedHtmlContent() const;

    /**
   * The original charset of MIME part the plain text was extracted from.
   *
   * If there were more than one text/plain MIME parts in the mail, the this is the charset
   * of the last MIME part processed.
   */
    QByteArray plainTextContentCharset() const { return mPlainTextContentCharset; }
    QByteArray htmlContentCharset() const { return mHtmlContentCharset; }

    void setCryptoProtocol( const Kleo::CryptoBackend::Protocol * protocol ) {
        mCryptoProtocol = protocol;
    }
    const Kleo::CryptoBackend::Protocol* cryptoProtocol() const {
        return mCryptoProtocol;
    }

    bool showOnlyOneMimePart() const { return mShowOnlyOneMimePart; }
    void setShowOnlyOneMimePart( bool show ) {
        mShowOnlyOneMimePart = show;
    }

    bool keepEncryptions() const { return mKeepEncryptions; }
    void setKeepEncryptions( bool keep ) {
        mKeepEncryptions = keep;
    }

    bool includeSignatures() const { return mIncludeSignatures; }
    void setIncludeSignatures( bool include ) {
        mIncludeSignatures = include;
    }

    // Controls whether Toltec invitations are displayed in their raw form or as a replacement text,
    // which is used in processToltecMail().
    void setShowRawToltecMail( bool showRawToltecMail ) { mShowRawToltecMail = showRawToltecMail; }
    bool showRawToltecMail() const { return mShowRawToltecMail; }

    /// Default text for processToltecMail(), which is used in messageviewer.kcfg, therefore it
    /// needs to be static here.
    static QString defaultToltecReplacementText();

    const AttachmentStrategy * attachmentStrategy() const {
        return mAttachmentStrategy;
    }

    HtmlWriter * htmlWriter() const { return mSource->htmlWriter(); }

    CSSHelper * cssHelper() const { return mSource->cssHelper(); }

    NodeHelper * nodeHelper() const { return mNodeHelper; }

    /** Parse beginning at a given node and recursively parsing
      the children of that node and it's next sibling. */
    void parseObjectTree( KMime::Content * node );

private:
    void extractNodeInfos( KMime::Content *curNode, bool isFirstTextPart );

    /**
   * Does the actual work for parseObjectTree. Unlike parseObjectTree(), this does not change the
   * top-level content.
   */
    void parseObjectTreeInternal( KMime::Content * node );

    /** Standard children handling a.k.a. multipart/mixed (w/o
      kroupware hacks) */
    void stdChildHandling( KMime::Content * child );

    void defaultHandling( KMime::Content * node, ProcessResult & result );

    /** 1. Create a new partNode using 'content' data and Content-Description
          found in 'cntDesc'.
      2. Parse the 'node' to display the content.
   */
    void createAndParseTempNode( KMime::Content* parentNode, const char * content, const char * cntDesc );

    /** if data is 0:
      Feeds the HTML widget with the contents of the opaque signed
          data found in partNode 'sign'.
      if data is set:
          Feeds the HTML widget with the contents of the given
          multipart/signed object.
      Signature is tested.  May contain body parts.

      Returns whether a signature was found or not: use this to
      find out if opaque data is signed or not. */
    bool writeOpaqueOrMultipartSignedData( KMime::Content * data,
                                           KMime::Content & sign,
                                           const QString & fromAddress,
                                           bool doCheck=true,
                                           QByteArray * cleartextData=0,
                                           const std::vector<GpgME::Signature> & paramSignatures = std::vector<GpgME::Signature>(),
                                           bool hideErrors=false );

    /** Writes out the block that we use when the node is encrypted,
      but we're deferring decryption for later. */
    void writeDeferredDecryptionBlock();

    /** Writes out the block that we use when the node is encrypted,
      but we've just kicked off async decryption. */
    void writeDecryptionInProgressBlock();

    /** Writes out the information contained in a GpgME::ImportResult */
    void writeCertificateImportResult( const GpgME::ImportResult & res );


    /** Returns the contents of the given multipart/encrypted
      object. Data is decypted.  May contain body parts. */
    bool okDecryptMIME( KMime::Content& data,
                        QByteArray& decryptedData,
                        bool& signatureFound,
                        std::vector<GpgME::Signature> &signatures,
                        bool showWarning,
                        bool& passphraseError,
                        bool& actuallyEncrypted,
                        bool& decryptionStarted,
                        PartMetaData &partMetaData );

    /**
   * This is called for all multipart/mixed nodes. It checks if that belongs to a Toltec mail,
   * by checking various criteria.
   * If it is a toltec mail, a special text, instead of the confusing toltec text, will be
   * displayed.
   *
   * @return true if the mail was indeed a toltec mail, in which case the node should not be
   *              processed further
   */
    bool processToltecMail( KMime::Content *node );

    bool processMailmanMessage( KMime::Content* node );

    /** Checks whether @p str contains external references. To be precise,
      we only check whether @p str contains 'xxx="http[s]:' where xxx is
      not href. Obfuscated external references are ignored on purpose.
  */
    static bool containsExternalReferences(const QString & str , const QString &extraHead);

public:// (during refactoring)

    bool processTextHtmlSubtype( KMime::Content * node, ProcessResult & result );
    bool processTextPlainSubtype( KMime::Content *node, ProcessResult & result );

    bool processMultiPartMixedSubtype( KMime::Content * node, ProcessResult & result );
    bool processMultiPartAlternativeSubtype( KMime::Content * node, ProcessResult & result );
    bool processMultiPartDigestSubtype( KMime::Content * node, ProcessResult & result );
    bool processMultiPartParallelSubtype( KMime::Content * node, ProcessResult & result );
    bool processMultiPartSignedSubtype( KMime::Content * node, ProcessResult & result );
    bool processMultiPartEncryptedSubtype( KMime::Content * node, ProcessResult & result );

    bool processMessageRfc822Subtype( KMime::Content * node, ProcessResult & result );

    bool processApplicationOctetStreamSubtype( KMime::Content * node, ProcessResult & result );
    bool processApplicationPkcs7MimeSubtype( KMime::Content * node, ProcessResult & result );
    bool processApplicationChiasmusTextSubtype( KMime::Content * node, ProcessResult & result );

    bool decryptChiasmus( const QByteArray& data, QByteArray& bodyDecoded, QString& errorText );
    void writeBodyString( const QByteArray & bodyString,
                          const QString & fromAddress,
                          const QTextCodec * codec,
                          ProcessResult & result, bool decorate );

    void writePartIcon( KMime::Content * msgPart, bool inlineImage = false );

    QString sigStatusToString( const Kleo::CryptoBackend::Protocol * cryptProto,
                               int status_code,
                               GpgME::Signature::Summary summary,
                               int & frameColor,
                               bool & showKeyInfos );
    QString writeSigstatHeader( PartMetaData & part,
                                const Kleo::CryptoBackend::Protocol * cryptProto,
                                const QString & fromAddress,
                                KMime::Content *node = 0);
    QString writeSigstatFooter( PartMetaData & part );

    // The attachment mark is a div that is placed around the attchment. It is used for drawing
    // a yellow border around the attachment when scrolling to it. When scrolling to it, the border
    // color of the div is changed, see KMReaderWin::scrollToAttachment().
    void writeAttachmentMarkHeader( KMime::Content *node );
    void writeAttachmentMarkFooter();

    void writeBodyStr( const QByteArray & bodyString,
                       const QTextCodec * aCodec,
                       const QString & fromAddress,
                       KMMsgSignatureState &  inlineSignatureState,
                       KMMsgEncryptionState & inlineEncryptionState,
                       bool decorate );

    bool isMailmanMessage( KMime::Content * curNode );

public: // KMReaderWin still needs this...
    void writeBodyStr( const QByteArray & bodyString,
                       const QTextCodec * aCodec,
                       const QString & fromAddress );
    static KMime::Content* findType( KMime::Content* content, const QByteArray& mimeType, bool deep, bool wide );

    static KMime::Content* findType( KMime::Content* content, const QByteArray& mediaType, const QByteArray& subType, bool deep, bool wide );

    static KMime::Content* findTypeNot( KMime::Content* content, const QByteArray& mediaType, const QByteArray& subType, bool deep=true, bool wide=true );


private:

    /** ctor helper */
    void init();

    /** Change the string to `quoted' html (meaning, that the quoted
      part of the message get italized */
    QString quotedHTML(const QString& pos, bool decorate);

    const QTextCodec * codecFor( KMime::Content * node ) const;
    /** Check if the newline at position @p newLinePos in string @p s
      seems to separate two paragraphs (important for correct BiDi
      behavior, but is heuristic because paragraphs are not
      well-defined) */
    bool looksLikeParaBreak(const QString& s, unsigned int newLinePos) const;

#ifdef MARCS_DEBUG
    void dumpToFile( const char * filename, const char * dataStart, size_t dataLen );
#else
    void dumpToFile( const char *, const char *, size_t ) {}
#endif

    void copyContentFrom( const ObjectTreeParser *other );

private:
    ObjectTreeSourceIf* mSource;
    NodeHelper* mNodeHelper;
    QByteArray mRawDecryptedBody;
    QByteArray mPlainTextContentCharset;
    QByteArray mHtmlContentCharset;
    QString mPlainTextContent;
    QString mHtmlContent;
    KMime::Content *mTopLevelContent;
    const Kleo::CryptoBackend::Protocol * mCryptoProtocol;

    /// Show only one mime part means that the user has selected some node in the message structure
    /// viewer that is not the root, which means the user wants to only see the selected node and its
    /// children. If that is the case, this variable is set to true.
    /// The code needs to behave differently if this is set. For example, it should not process the
    /// siblings. Also, consider inline images: Normally, those nodes are completely hidden, as the
    /// HTML node embedds them. However, when showing only the node of the image, one has to show them,
    /// as their is no HTML node in which they are displayed. There are many more cases where this
    /// variable needs to be obeyed.
    /// This variable is set to false again when processing the children in stdChildHandling(), as
    /// the children can be completely displayed again.
    bool mShowOnlyOneMimePart;

    bool mKeepEncryptions;
    bool mIncludeSignatures;
    bool mHasPendingAsyncJobs;
    bool mAllowAsync;
    bool mShowRawToltecMail;
    const AttachmentStrategy * mAttachmentStrategy;
    // DataUrl Icons cache
    QString mCollapseIcon;
    QString mExpandIcon;
    bool mDeleteNodeHelper;

};

}

#endif // _KMAIL_OBJECTTREEPARSER_H_

