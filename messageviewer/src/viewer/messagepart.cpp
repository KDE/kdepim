/*
   Copyright (c) 2015 Sandro Knauß <sknauss@kde.org>

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

#include "messagepart.h"
#include "messageviewer_debug.h"
#include "objecttreeparser.h"

#include <MessageCore/StringUtil>

#include <libkleo/importjob.h>

#include <interfaces/htmlwriter.h>
#include <htmlwriter/queuehtmlwriter.h>
#include <kmime/kmime_content.h>
#include <gpgme++/key.h>
#include <gpgme.h>

#include <QTextCodec>
#include <QApplication>

#include <KLocalizedString>

using namespace MessageViewer;

//--------CryptoBlock-------------------
CryptoBlock::CryptoBlock(ObjectTreeParser *otp,
                         PartMetaData *block,
                         const Kleo::CryptoBackend::Protocol *cryptoProto,
                         const QString &fromAddress,
                         KMime::Content *node)
    : HTMLBlock()
    , mOtp(otp)
    , mMetaData(block)
    , mCryptoProto(cryptoProto)
    , mFromAddress(fromAddress)
    , mNode(node)
{
    internalEnter();
}

CryptoBlock::~CryptoBlock()
{
    internalExit();
}

void CryptoBlock::internalEnter()
{
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    if (writer && !entered) {
        entered = true;
        writer->queue(mOtp->writeSigstatHeader(*mMetaData, mCryptoProto, mFromAddress, mNode));
    }
}

void CryptoBlock::internalExit()
{
    if (!entered) {
        return;
    }
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    writer->queue(mOtp->writeSigstatFooter(*mMetaData));
    entered = false;
}

AttachmentMarkBlock::AttachmentMarkBlock(MessageViewer::HtmlWriter *writer, KMime::Content *node)
    : mNode(node)
    , mWriter(writer)
{
    internalEnter();
}

AttachmentMarkBlock::~AttachmentMarkBlock()
{
    internalExit();
}

void AttachmentMarkBlock::internalEnter()
{
    if (mWriter && !entered) {
        const QString index = mNode->index().toString();
        mWriter->queue(QStringLiteral("<a name=\"att%1\"></a>").arg(index));
        mWriter->queue(QStringLiteral("<div id=\"attachmentDiv%1\">\n").arg(index));
        entered = true;
    }
}

void AttachmentMarkBlock::internalExit()
{
    if (!entered) {
        return;
    }

    mWriter->queue(QStringLiteral("</div>"));
    entered = false;
}

TextBlock::TextBlock(MessageViewer::HtmlWriter *writer, MessageViewer::NodeHelper *nodeHelper, KMime::Content *node, bool link)
    : mWriter(writer)
    , mNodeHelper(nodeHelper)
    , mNode(node)
    , mLink(link)
{
    internalEnter();
}

TextBlock::~TextBlock()
{
    internalExit();
}

void TextBlock::internalEnter()
{
    if (!mWriter || entered) {
        return;
    }
    entered = true;

    const QString label = MessageCore::StringUtil::quoteHtmlChars(NodeHelper::fileName(mNode), true);

    const QString comment =
        MessageCore::StringUtil::quoteHtmlChars(mNode->contentDescription()->asUnicodeString(), true);

    const QString dir = QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");

    mWriter->queue(QLatin1String("<table cellspacing=\"1\" class=\"textAtm\">"
                                 "<tr class=\"textAtmH\"><td dir=\"") + dir + QLatin1String("\">"));
    if (!mLink)
        mWriter->queue(QLatin1String("<a href=\"") + mNodeHelper->asHREF(mNode, QStringLiteral("body")) + QLatin1String("\">")
                       + label + QLatin1String("</a>"));
    else {
        mWriter->queue(label);
    }
    if (!comment.isEmpty()) {
        mWriter->queue(QLatin1String("<br/>") + comment);
    }
    mWriter->queue(QLatin1String("</td></tr><tr class=\"textAtmB\"><td>"));
}

void TextBlock::internalExit()
{
    if (!entered) {
        return;
    }

    entered = false;

    mWriter->queue(QStringLiteral("</td></tr></table>"));
}

//------MessagePart-----------------------
MessagePart::MessagePart(ObjectTreeParser *otp,
                         const QString &text)
    : mText(text)
    , mOtp(otp)
    , mSubOtp(Q_NULLPTR)
{

}

MessagePart::~MessagePart()
{
    if (mSubOtp) {
        delete mSubOtp->htmlWriter();
        delete mSubOtp;
        mSubOtp = Q_NULLPTR;
    }
}

PartMetaData *MessagePart::partMetaData()
{
    return &mMetaData;
}

QString MessagePart::text() const
{
    return mText;
}

void MessagePart::setText(const QString &text)
{
    mText = text;
}

void MessagePart::html(bool decorate)
{
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    if (!writer) {
        return;
    }

    const CryptoBlock block(mOtp, &mMetaData, Q_NULLPTR, QString(), Q_NULLPTR);
    writer->queue(mOtp->quotedHTML(text(), decorate));
}

void MessagePart::parseInternal(KMime::Content *node, bool onlyOneMimePart)
{
    mSubOtp = new ObjectTreeParser(mOtp, onlyOneMimePart);
    mSubOtp->setAllowAsync(mOtp->allowAsync());
    if (mOtp->htmlWriter()) {
        mSubOtp->mHtmlWriter = new QueueHtmlWriter(mOtp->htmlWriter());
    }
    mSubOtp->parseObjectTreeInternal(node);
}

void MessagePart::renderInternalHtml() const
{
    if (mSubOtp) {
        static_cast<QueueHtmlWriter *>(mSubOtp->htmlWriter())->replay();
    }
}

void MessagePart::copyContentFrom() const
{
    if (mSubOtp) {
        mOtp->copyContentFrom(mSubOtp);
    }
}

QString MessagePart::renderInternalText() const
{
    if (!mSubOtp) {
        return QString();
    }
    return mSubOtp->plainTextContent();
}

//-----TextMessageBlock----------------------

TextMessagePart::TextMessagePart(ObjectTreeParser *otp, KMime::Content *node, bool drawFrame, bool showLink)
    : MessagePart(otp, QString())
    , mNode(node)
    , mDrawFrame(drawFrame)
    , mShowLink(showLink)
{
    if (!mNode) {
        qCWarning(MESSAGEVIEWER_LOG) << "not a valid node";
        return;
    }

    mBlocks = mOtp->writeBodyStr2(mNode->decodedContent(), mOtp->codecFor(mNode), NodeHelper::fromAsString(mNode), mSignatureState, mEncryptionState);
}

TextMessagePart::~TextMessagePart()
{

}

void TextMessagePart::html(bool decorate)
{
    HTMLBlock::Ptr block;
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    if (mDrawFrame) {
        block = HTMLBlock::Ptr(new TextBlock(writer, mOtp->nodeHelper(), mNode, mShowLink));
    }

    foreach (const MessagePart::Ptr &mp, mBlocks) {
        mp->html(decorate);
    }
}

QString TextMessagePart::text() const
{
    QString text;
    foreach (const MessagePart::Ptr &mp, mBlocks) {
        text += mp->text();
    }
    return text;
}

KMMsgEncryptionState TextMessagePart::encryptionState() const
{
    return mEncryptionState;
}

KMMsgSignatureState TextMessagePart::signatureState() const
{
    return mSignatureState;
}

//-----MimeMessageBlock----------------------

MimeMessagePart::MimeMessagePart(ObjectTreeParser *otp, KMime::Content *node, bool onlyOneMimePart)
    : MessagePart(otp, QString())
    , mNode(node)
    , mOnlyOneMimePart(onlyOneMimePart)
{
    if (!mNode) {
        qCWarning(MESSAGEVIEWER_LOG) << "not a valid node";
        return;
    }

    parseInternal(mNode, mOnlyOneMimePart);
}

MimeMessagePart::~MimeMessagePart()
{

}

void MimeMessagePart::html(bool decorate)
{
    copyContentFrom();
    renderInternalHtml();
}

QString MimeMessagePart::text() const
{
    return renderInternalText();
}

//-----CryptMessageBlock---------------------

CryptoMessagePart::CryptoMessagePart(ObjectTreeParser *otp,
                                     const QString &text,
                                     const Kleo::CryptoBackend::Protocol *cryptoProto,
                                     const QString &fromAddress,
                                     KMime::Content *node)
    : MessagePart(otp, text)
    , mCryptoProto(cryptoProto)
    , mFromAddress(fromAddress)
    , mNode(node)
    , mDecryptMessage(false)
{
    mMetaData.technicalProblem = (mCryptoProto == 0);
    mMetaData.isSigned = false;
    mMetaData.isGoodSignature = false;
    mMetaData.isEncrypted = false;
    mMetaData.isDecryptable = false;
    mMetaData.keyTrust = GpgME::Signature::Unknown;
    mMetaData.status = i18n("Wrong Crypto Plug-In.");
    mMetaData.status_code = GPGME_SIG_STAT_NONE;
}

CryptoMessagePart::~CryptoMessagePart()
{

}

void CryptoMessagePart::startDecryption(const QByteArray &text, const QTextCodec *aCodec)
{
    mDecryptMessage = true;

    KMime::Content *content = new KMime::Content;
    content->setBody(text);
    content->parse();

    startDecryption(content);

    if (!mMetaData.inProgress && mMetaData.isDecryptable) {
        setText(aCodec->toUnicode(mDecryptedData));
    }
}

void CryptoMessagePart::startDecryption(KMime::Content *data)
{
    if (!mNode && !data) {
        return;
    }

    if (!data) {
        data = mNode;
    }

    mDecryptMessage = true;

    bool signatureFound;
    bool actuallyEncrypted = true;
    bool decryptionStarted;

    bool bOkDecrypt = mOtp->okDecryptMIME(*data,
                                          mDecryptedData,
                                          signatureFound,
                                          mSignatures,
                                          true,
                                          mPassphraseError,
                                          actuallyEncrypted,
                                          decryptionStarted,
                                          mMetaData);
    if (decryptionStarted) {
        mMetaData.inProgress = true;
        return;
    }
    mMetaData.isDecryptable = bOkDecrypt;
    mMetaData.isEncrypted = actuallyEncrypted;
    mMetaData.isSigned = signatureFound;

    if (!mMetaData.isDecryptable) {
        setText(QString::fromUtf8(mDecryptedData.constData()));
    }

    if (mMetaData.isSigned) {
        mOtp->sigStatusToMetaData(mSignatures, mCryptoProto, mMetaData, GpgME::Key());
        mVerifiedText = mDecryptedData;
    }

    if (mMetaData.isEncrypted && !mDecryptMessage) {
        mMetaData.isDecryptable = true;
    }

    if (mNode) {
        mOtp->mNodeHelper->setPartMetaData(mNode, mMetaData);

        if (mDecryptMessage) {
            auto tempNode = new KMime::Content();
            tempNode->setContent(KMime::CRLFtoLF(mDecryptedData.constData()));
            tempNode->parse();

            if (!tempNode->head().isEmpty()) {
                tempNode->contentDescription()->from7BitString("encrypted data");
            }
            mOtp->mNodeHelper->attachExtraContent(mNode, tempNode);

            parseInternal(tempNode, false);
        }
    }
}

void CryptoMessagePart::startVerification(const QByteArray &text, const QTextCodec *aCodec)
{
    startVerificationDetached(text, 0, QByteArray());

    if (!mNode && mMetaData.isSigned) {
        setText(aCodec->toUnicode(mVerifiedText));
    }
}

void CryptoMessagePart::startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray &signature)
{
    mMetaData.isEncrypted = false;
    mMetaData.isDecryptable = false;

    mOtp->okVerify(text, mCryptoProto, mMetaData, mVerifiedText, mSignatures, signature, mNode);

    if (mMetaData.isSigned) {
        mOtp->sigStatusToMetaData(mSignatures, mCryptoProto, mMetaData, GpgME::Key());
    } else {
        mMetaData.creationTime = QDateTime();
    }

    if (mNode) {
        if (textNode && !signature.isEmpty()) {
            mVerifiedText = text;
        } else if (!mVerifiedText.isEmpty()) {
            textNode = new KMime::Content();
            textNode->setContent(KMime::CRLFtoLF(mVerifiedText.constData()));
            textNode->parse();

            if (!textNode->head().isEmpty()) {
                textNode->contentDescription()->from7BitString("opaque signed data");
            }
            mOtp->mNodeHelper->attachExtraContent(mNode, textNode);
        }

        if (!mVerifiedText.isEmpty() && textNode) {
            parseInternal(textNode, false);
        }
    }

}

void CryptoMessagePart::writeDeferredDecryptionBlock() const
{
    Q_ASSERT(!mMetaData.isEncrypted);
    Q_ASSERT(mDecryptMessage);

    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    if (!writer) {
        return;
    }

    const QString iconName = QLatin1String("file:///") + KIconLoader::global()->iconPath(QStringLiteral("document-decrypt"),
                             KIconLoader::Small);
    writer->queue(QLatin1String("<div style=\"font-size:large; text-align:center;"
                                "padding-top:20pt;\">")
                  + i18n("This message is encrypted.")
                  + QLatin1String("</div>"
                                  "<div style=\"text-align:center; padding-bottom:20pt;\">"
                                  "<a href=\"kmail:decryptMessage\">"
                                  "<img src=\"") + iconName + QLatin1String("\"/>")
                  + i18n("Decrypt Message")
                  + QLatin1String("</a></div>"));
}

void CryptoMessagePart::html(bool decorate)
{

    bool hideErrors = false;
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    //TODO: still the following part should not be here
    copyContentFrom();

    if (!writer) {
        return;
    }

    if (mMetaData.isEncrypted && !mDecryptMessage) {
        const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
        writeDeferredDecryptionBlock();
    } else if (mMetaData.inProgress) {
        const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
        // In progress has no special body
    } else if (mMetaData.isEncrypted && !mMetaData.isDecryptable) {
        const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
        writer->queue(text());           //Do not quote ErrorText
    } else {
        if (mMetaData.isSigned && mVerifiedText.isEmpty() && !hideErrors) {
            const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
            writer->queue(QStringLiteral("<hr><b><h2>"));
            writer->queue(i18n("The crypto engine returned no cleartext data."));
            writer->queue(QStringLiteral("</h2></b>"));
            writer->queue(QStringLiteral("<br/>&nbsp;<br/>"));
            writer->queue(i18n("Status: "));
            if (!mMetaData.status.isEmpty()) {
                writer->queue(QStringLiteral("<i>"));
                writer->queue(mMetaData.status);
                writer->queue(QStringLiteral("</i>"));
            } else {
                writer->queue(i18nc("Status of message unknown.", "(unknown)"));
            }
        } else if (mNode) {
            const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
            renderInternalHtml();
        } else {
            MessagePart::html(decorate);
        }
    }
}

EncapsulatedRfc822MessagePart::EncapsulatedRfc822MessagePart(ObjectTreeParser *otp, KMime::Content *node, const KMime::Message::Ptr &message)
    : MessagePart(otp, QString())
    , mMessage(message)
    , mNode(node)
{
    mMetaData.isEncrypted = false;
    mMetaData.isSigned = false;
    mMetaData.isEncapsulatedRfc822Message = true;

    mOtp->nodeHelper()->setNodeDisplayedEmbedded(mNode, true);
    mOtp->nodeHelper()->setPartMetaData(mNode, mMetaData);

    if (!mMessage) {
        qCWarning(MESSAGEVIEWER_LOG) << "Node is of type message/rfc822 but doesn't have a message!";
        return;
    }

    // The link to "Encapsulated message" is clickable, therefore the temp file needs to exists,
    // since the user can click the link and expect to have normal attachment operations there.
    mOtp->nodeHelper()->writeNodeToTempFile(message.data());

    parseInternal(message.data(), false);
}

EncapsulatedRfc822MessagePart::~EncapsulatedRfc822MessagePart()
{

}

void EncapsulatedRfc822MessagePart::html(bool decorate)
{
    Q_UNUSED(decorate)
    if (!mSubOtp) {
        return;
    }

    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    if (!writer) {
        return;
    }

    const CryptoBlock block(mOtp, &mMetaData, Q_NULLPTR, mMessage->from()->asUnicodeString(), mMessage.data());
    writer->queue(mOtp->mSource->createMessageHeader(mMessage.data()));
    renderInternalHtml();

    mOtp->nodeHelper()->setPartMetaData(mNode, mMetaData);
}

QString EncapsulatedRfc822MessagePart::text() const
{
    return renderInternalText();
}

