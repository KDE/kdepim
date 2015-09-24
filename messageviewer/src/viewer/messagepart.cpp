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

    const CryptoBlock block(mOtp, mMetaData, 0, QString(), 0);
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
{

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
    }

    if (mNode) {
        mOtp->mNodeHelper->setPartMetaData(mNode, *mMetaData);
    }
}

void CryptoMessagePart::startVerification(const QByteArray &text, const QTextCodec *aCodec)
{
    mMetaData->isEncrypted = false;
    mMetaData->isDecryptable = false;

    QByteArray verifiedText;
    if (mOtp->okVerify(text, mCryptoProto, *mMetaData, verifiedText, mSignatures, QByteArray(), mNode)) {
        setText(aCodec->toUnicode(verifiedText));
    }
    mOtp->sigStatusToMetaData(mSignatures, mCryptoProto, *mMetaData, GpgME::Key());
}

void CryptoMessagePart::html(bool decorate) const
{
    MessageViewer::HtmlWriter* writer = mOtp->htmlWriter();
    if (!writer) {
        if (mNode && mDecryptMessage) {
            //TODO: Bad hack, we need the TempNodeParsing anycase
            // but till we not make sure that the nodeparsing also creates html directly we need to have this hack.
            mOtp->createAndParseTempNode(mNode, mDecryptedData.constData(), "encrypted node");
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
        if (mNode) {
            const CryptoBlock block(mOtp, mMetaData, mCryptoProto, mFromAddress, mNode);
            mOtp->createAndParseTempNode(mNode, mDecryptedData.constData(), "encrypted node");
        } else {
            MessagePart::html(decorate);
        }
    }
}
