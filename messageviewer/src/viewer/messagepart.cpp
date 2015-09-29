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
#include "objecttreeparser.h"

#include <interfaces/htmlwriter.h>
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
    MessageViewer::HtmlWriter* writer = mOtp->htmlWriter();
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
    MessageViewer::HtmlWriter* writer = mOtp->htmlWriter();
    writer->queue(mOtp->writeSigstatFooter(*mMetaData));
    entered = false;
}


//------MessagePart-----------------------
MessagePart::MessagePart(ObjectTreeParser *otp,
            PartMetaData *block,
            const QString &text)
: mOtp(otp)
, mText(text)
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

void MessagePart::setText(const QString &text) {
    mText = text;
}

void MessagePart::html(bool decorate) const
{
    MessageViewer::HtmlWriter* writer = mOtp->htmlWriter();

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
    , mCryptoProto(cryptoProto)
    , mDecryptMessage(false)
    , mFromAddress(fromAddress)
    , mNode(node)
    , mTextNode(Q_NULLPTR)
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
    }
}

void CryptoMessagePart::startVerification(const QByteArray &text, const QTextCodec *aCodec)
{
    startVerificationDetached(text, 0, QByteArray());

    if (!mNode && mMetaData->isSigned) {
        setText(aCodec->toUnicode(mVerifiedText));
    }
}

void CryptoMessagePart::startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray& signature)
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
            mTextNode = textNode;
            mVerifiedText = text;
        }
    }

}

void CryptoMessagePart::html(bool decorate) const
{

    bool hideErrors = false;
    MessageViewer::HtmlWriter* writer = mOtp->htmlWriter();
    if (!writer) {
        if (mNode && (mDecryptMessage || !mVerifiedText.isEmpty())) {
            //TODO: Bad hack, we need the TempNodeParsing anycase
            // but till we not make sure that the nodeparsing also creates html directly we need to have this hack.
            if (!mVerifiedText.isEmpty() && mTextNode) {
                auto otp = new ObjectTreeParser(mOtp, true);
                otp->setAllowAsync(mOtp->allowAsync());
                otp->parseObjectTreeInternal(mTextNode);
                mOtp->copyContentFrom(otp);
            } else if (!mVerifiedText.isEmpty()) {
                mOtp->createAndParseTempNode(mNode, mVerifiedText.constData(), "opaque signed data");
            } else {
                mOtp->createAndParseTempNode(mNode, mDecryptedData.constData(), "encrypted node");
            }
        }
        return;
    }

    if (mMetaData->isEncrypted && !mDecryptMessage) {
        mOtp->writeDeferredDecryptionBlock();
    } else if (mMetaData->inProgress) {
        mOtp->writeDecryptionInProgressBlock();
    } else if (mMetaData->isEncrypted && !mMetaData->isDecryptable) {
        const CryptoBlock block(mOtp, mMetaData, mCryptoProto, mFromAddress, mNode);
        writer->queue(text());           //Do not quote ErrorText
    } else {
        if (mMetaData->isSigned && mVerifiedText.isEmpty() && !hideErrors) {
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
            if (!mVerifiedText.isEmpty() && mTextNode) {
                auto otp = new ObjectTreeParser(mOtp, true);
                otp->setAllowAsync(mOtp->allowAsync());
                otp->parseObjectTreeInternal(mTextNode);
                mOtp->copyContentFrom(otp);
            } else if (!mVerifiedText.isEmpty()) {
                mOtp->createAndParseTempNode(mNode, mVerifiedText.constData(), "opaque signed data");
            } else {
                mOtp->createAndParseTempNode(mNode, mDecryptedData.constData(), "encrypted node");
            }
        } else {
            MessagePart::html(decorate);
        }
    }
}
