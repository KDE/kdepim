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

#include <interfaces/htmlwriter.h>
#include <htmlwriter/queuehtmlwriter.h>
#include <kmime/kmime_content.h>
#include <gpgme++/key.h>
#include <gpgme.h>
#include <KLocalizedString>

#include <QTextCodec>

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

AttachmentMarkBlock::AttachmentMarkBlock(MessageViewer::HtmlWriter* writer, KMime::Content* node)
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
        mWriter->queue(QStringLiteral("<div id=\"attachmentDiv%1\">\n").arg(mNode->index().toString()));
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

//------MessagePart-----------------------
MessagePart::MessagePart(ObjectTreeParser *otp,
                         PartMetaData *block,
                         const QString &text)
    : mText(text)
    , mOtp(otp)
    , mMetaData(block)
{

}

PartMetaData *MessagePart::partMetaData() const
{
    return mMetaData;
}

QString MessagePart::text() const
{
    return mText;
}

void MessagePart::setText(const QString &text)
{
    mText = text;
}

void MessagePart::html(bool decorate) const
{
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    if (!writer) {
        return;
    }

    const CryptoBlock block(mOtp, mMetaData, Q_NULLPTR, QString(), Q_NULLPTR);
    writer->queue(mOtp->quotedHTML(text(), decorate));
}

//-----CryptMessageBlock---------------------

CryptoMessagePart::CryptoMessagePart(ObjectTreeParser *otp,
                                     PartMetaData *block,
                                     const QString &text,
                                     const Kleo::CryptoBackend::Protocol *cryptoProto,
                                     const QString &fromAddress,
                                     KMime::Content *node)
    : MessagePart(otp, block, text)
    , mSubOtp(0)
    , mCryptoProto(cryptoProto)
    , mFromAddress(fromAddress)
    , mNode(node)
    , mDecryptMessage(false)
{
    mMetaData->technicalProblem = (mCryptoProto == 0);
    mMetaData->isSigned = false;
    mMetaData->isGoodSignature = false;
    mMetaData->isEncrypted = false;
    mMetaData->isDecryptable = false;
    mMetaData->keyTrust = GpgME::Signature::Unknown;
    mMetaData->status = i18n("Wrong Crypto Plug-In.");
    mMetaData->status_code = GPGME_SIG_STAT_NONE;
}

CryptoMessagePart::~CryptoMessagePart()
{
    if (mSubOtp) {
        delete mSubOtp->mHtmlWriter;
        delete mSubOtp;
        mSubOtp = 0;
    }
}

void CryptoMessagePart::startDecryption(const QByteArray &text, const QTextCodec *aCodec)
{
    mDecryptMessage = true;

    KMime::Content *content = new KMime::Content;
    content->setBody(text);
    content->parse();

    startDecryption(content);

    if (!mMetaData->inProgress && mMetaData->isDecryptable) {
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
                                          *mMetaData);
    if (decryptionStarted) {
        mMetaData->inProgress = true;
        return;
    }
    mMetaData->isDecryptable = bOkDecrypt;
    mMetaData->isEncrypted = actuallyEncrypted;
    mMetaData->isSigned = signatureFound;

    if (!mMetaData->isDecryptable) {
        setText(QString::fromUtf8(mDecryptedData.constData()));
    }

    if (mMetaData->isSigned) {
        mOtp->sigStatusToMetaData(mSignatures, mCryptoProto, *mMetaData, GpgME::Key());
        mVerifiedText = mDecryptedData;
    }

    if (mNode) {
        mOtp->mNodeHelper->setPartMetaData(mNode, *mMetaData);

        if (mDecryptMessage) {
            auto tempNode = new KMime::Content();
            tempNode->setContent(KMime::CRLFtoLF(mDecryptedData.constData()));
            tempNode->parse();

            if (!tempNode->head().isEmpty()) {
                tempNode->contentDescription()->from7BitString("encrypted data");
            }
            mOtp->mNodeHelper->attachExtraContent(mNode, tempNode);

            mSubOtp = new ObjectTreeParser(mOtp, true);
            mSubOtp->setAllowAsync(mOtp->allowAsync());
            if (mOtp->htmlWriter()) {
                mSubOtp->mHtmlWriter = new QueueHtmlWriter(mOtp->htmlWriter());
            }
            mSubOtp->parseObjectTreeInternal(tempNode);
        }
    }
}

void CryptoMessagePart::startVerification(const QByteArray &text, const QTextCodec *aCodec)
{
    startVerificationDetached(text, 0, QByteArray());

    if (!mNode && mMetaData->isSigned) {
        setText(aCodec->toUnicode(mVerifiedText));
    }
}

void CryptoMessagePart::startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray &signature)
{
    mMetaData->isEncrypted = false;
    mMetaData->isDecryptable = false;

    mOtp->okVerify(text, mCryptoProto, *mMetaData, mVerifiedText, mSignatures, signature, mNode);

    if (mMetaData->isSigned) {
        mOtp->sigStatusToMetaData(mSignatures, mCryptoProto, *mMetaData, GpgME::Key());
    } else {
        mMetaData->creationTime = QDateTime();
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
            mSubOtp = new ObjectTreeParser(mOtp, true);
            mSubOtp->setAllowAsync(mOtp->allowAsync());
            if (mOtp->htmlWriter()) {
                mSubOtp->mHtmlWriter = new QueueHtmlWriter(mOtp->htmlWriter());
            }
            mSubOtp->parseObjectTreeInternal(textNode);
        }
    }

}

void CryptoMessagePart::writeDeferredDecryptionBlock() const
{
    Q_ASSERT(!mMetaData->isEncrypted);
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

void CryptoMessagePart::html(bool decorate) const
{

    bool hideErrors = false;
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    //TODO: still the following part should not be here
    if (mSubOtp) {
        mOtp->copyContentFrom(mSubOtp);
    }

    if (mMetaData->isEncrypted && !mDecryptMessage) {
        mMetaData->isDecryptable = true;
    }

    if (!writer) {
        return;
    }

    if (mMetaData->isEncrypted && !mDecryptMessage) {
        const CryptoBlock block(mOtp, mMetaData, mCryptoProto, mFromAddress, mNode);
        writeDeferredDecryptionBlock();
    } else if (mMetaData->inProgress) {
        const CryptoBlock block(mOtp, mMetaData, mCryptoProto, mFromAddress, mNode);
        // In progress has no special body
    } else if (mMetaData->isEncrypted && !mMetaData->isDecryptable) {
        const CryptoBlock block(mOtp, mMetaData, mCryptoProto, mFromAddress, mNode);
        writer->queue(text());           //Do not quote ErrorText
    } else {
        if (mMetaData->isSigned && mVerifiedText.isEmpty() && !hideErrors) {
            const CryptoBlock block(mOtp, mMetaData, mCryptoProto, mFromAddress, mNode);
            writer->queue(QStringLiteral("<hr><b><h2>"));
            writer->queue(i18n("The crypto engine returned no cleartext data."));
            writer->queue(QStringLiteral("</h2></b>"));
            writer->queue(QStringLiteral("<br/>&nbsp;<br/>"));
            writer->queue(i18n("Status: "));
            if (!mMetaData->status.isEmpty()) {
                writer->queue(QStringLiteral("<i>"));
                writer->queue(mMetaData->status);
                writer->queue(QStringLiteral("</i>"));
            } else {
                writer->queue(i18nc("Status of message unknown.", "(unknown)"));
            }
        } else if (mNode) {
            const CryptoBlock block(mOtp, mMetaData, mCryptoProto, mFromAddress, mNode);
            if (mSubOtp) {
                static_cast<QueueHtmlWriter *>(mSubOtp->htmlWriter())->replay();
            }
        } else {
            MessagePart::html(decorate);
        }
    }
}

EncapsulatedRfc822MessagePart::EncapsulatedRfc822MessagePart(ObjectTreeParser* otp, PartMetaData* block, KMime::Content* node, const KMime::Message::Ptr& message)
: MessagePart(otp, block, QString())
, mMessage(message)
, mNode(node)
, mSubOtp(0)
{
    mMetaData->isEncrypted = false;
    mMetaData->isSigned = false;
    mMetaData->isEncapsulatedRfc822Message = true;

    mOtp->nodeHelper()->setNodeDisplayedEmbedded(mNode, true);
    mOtp->nodeHelper()->setPartMetaData(mNode, *mMetaData);

    if (!mMessage) {
        qCWarning(MESSAGEVIEWER_LOG) << "Node is of type message/rfc822 but doesn't have a message!";
        return;
    }

    // The link to "Encapsulated message" is clickable, therefore the temp file needs to exists,
    // since the user can click the link and expect to have normal attachment operations there.
    mOtp->nodeHelper()->writeNodeToTempFile(message.data());

    mSubOtp = new ObjectTreeParser(mOtp, true);
    mSubOtp->setAllowAsync(mOtp->allowAsync());
    if (mOtp->htmlWriter()) {
        mSubOtp->mHtmlWriter = new QueueHtmlWriter(mOtp->htmlWriter());
    }
    mSubOtp->parseObjectTreeInternal(message.data());
}

EncapsulatedRfc822MessagePart::~EncapsulatedRfc822MessagePart()
{

}

void EncapsulatedRfc822MessagePart::html(bool decorate) const
{
    if (!mSubOtp) {
        return;
    }

    MessageViewer::HtmlWriter* writer = mOtp->htmlWriter();

    if (!writer) {
        return;
    }

    const CryptoBlock block(mOtp, mMetaData, 0, mMessage->from()->asUnicodeString(), mMessage.data());
    writer->queue(mOtp->mSource->createMessageHeader(mMessage.data()));
    static_cast<QueueHtmlWriter*>(mSubOtp->htmlWriter())->replay();

    mOtp->nodeHelper()->setPartMetaData(mNode, *mMetaData);
}


QString EncapsulatedRfc822MessagePart::text() const
{
    if (!mSubOtp) {
        return QString();
    }
    return mSubOtp->plainTextContent();
}
