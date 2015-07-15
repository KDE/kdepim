/* -*- mode: c++; c-basic-offset:4 -*-
    decryptverifytask.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef __KLEOPATRA_CRYPTO_DECRYPTVERIFYTASK_H__
#define __KLEOPATRA_CRYPTO_DECRYPTVERIFYTASK_H__

#include "task.h"

#include <utils/types.h>

#include <gpgme++/verificationresult.h>

#include <boost/shared_ptr.hpp>

namespace KMime
{
namespace Types
{
class Mailbox;
}
}
namespace GpgME
{
class DecryptionResult;
class VerificationResult;
class Key;
class Signature;
}

namespace Kleo
{
class Input;
class Output;
class AuditLog;
}

namespace Kleo
{
namespace Crypto
{

class DecryptVerifyResult;

class AbstractDecryptVerifyTask : public Task
{
    Q_OBJECT
public:
    explicit AbstractDecryptVerifyTask(QObject *parent = Q_NULLPTR);
    virtual ~AbstractDecryptVerifyTask();
    virtual void autodetectProtocolFromInput() = 0;

    KMime::Types::Mailbox informativeSender() const;
    void setInformativeSender(const KMime::Types::Mailbox &senders);

Q_SIGNALS:
    void decryptVerifyResult(const boost::shared_ptr<const Kleo::Crypto::DecryptVerifyResult> &);

protected:
    boost::shared_ptr<DecryptVerifyResult> fromDecryptResult(const GpgME::DecryptionResult &dr, const QByteArray &plaintext, const AuditLog &auditLog);
    boost::shared_ptr<DecryptVerifyResult> fromDecryptResult(const GpgME::Error &err, const QString &details, const AuditLog &auditLog);
    boost::shared_ptr<DecryptVerifyResult> fromDecryptVerifyResult(const GpgME::DecryptionResult &dr, const GpgME::VerificationResult &vr, const QByteArray &plaintext, const AuditLog &auditLog);
    boost::shared_ptr<DecryptVerifyResult> fromDecryptVerifyResult(const GpgME::Error &err, const QString &what, const AuditLog &auditLog);
    boost::shared_ptr<DecryptVerifyResult> fromVerifyOpaqueResult(const GpgME::VerificationResult &vr, const QByteArray &plaintext, const AuditLog &auditLog);
    boost::shared_ptr<DecryptVerifyResult> fromVerifyOpaqueResult(const GpgME::Error &err, const QString &details, const AuditLog &auditLog);
    boost::shared_ptr<DecryptVerifyResult> fromVerifyDetachedResult(const GpgME::VerificationResult &vr, const AuditLog &auditLog);
    boost::shared_ptr<DecryptVerifyResult> fromVerifyDetachedResult(const GpgME::Error &err, const QString &details, const AuditLog &auditLog);

    virtual QString inputLabel() const = 0;
    virtual QString outputLabel() const = 0;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

class DecryptTask : public AbstractDecryptVerifyTask
{
    Q_OBJECT
public:
    explicit DecryptTask(QObject *parent = Q_NULLPTR);
    ~DecryptTask();

    void setInput(const boost::shared_ptr<Input> &input);
    void setOutput(const boost::shared_ptr<Output> &output);

    void setProtocol(GpgME::Protocol prot);
    void autodetectProtocolFromInput() Q_DECL_OVERRIDE;

    QString label() const Q_DECL_OVERRIDE;

    GpgME::Protocol protocol() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void cancel() Q_DECL_OVERRIDE;

private:
    void doStart() Q_DECL_OVERRIDE;
    QString inputLabel() const Q_DECL_OVERRIDE;
    QString outputLabel() const Q_DECL_OVERRIDE;
    unsigned long long inputSize() const Q_DECL_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotResult(GpgME::DecryptionResult, QByteArray))
};

class VerifyDetachedTask : public AbstractDecryptVerifyTask
{
    Q_OBJECT
public:
    explicit VerifyDetachedTask(QObject *parent = Q_NULLPTR);
    ~VerifyDetachedTask();

    void setInput(const boost::shared_ptr<Input> &input);
    void setSignedData(const boost::shared_ptr<Input> &signedData);

    void setProtocol(GpgME::Protocol prot);
    void autodetectProtocolFromInput() Q_DECL_OVERRIDE;

    QString label() const Q_DECL_OVERRIDE;

    GpgME::Protocol protocol() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void cancel() Q_DECL_OVERRIDE;

private:
    void doStart() Q_DECL_OVERRIDE;
    QString inputLabel() const Q_DECL_OVERRIDE;
    QString outputLabel() const Q_DECL_OVERRIDE;
    unsigned long long inputSize() const Q_DECL_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotResult(GpgME::VerificationResult))
};

class VerifyOpaqueTask : public AbstractDecryptVerifyTask
{
    Q_OBJECT
public:
    explicit VerifyOpaqueTask(QObject *parent = Q_NULLPTR);
    ~VerifyOpaqueTask();

    void setInput(const boost::shared_ptr<Input> &input);
    void setOutput(const boost::shared_ptr<Output> &output);

    void setProtocol(GpgME::Protocol prot);
    void autodetectProtocolFromInput()Q_DECL_OVERRIDE;

    QString label() const Q_DECL_OVERRIDE;
    GpgME::Protocol protocol() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void cancel() Q_DECL_OVERRIDE;

private:
    void doStart() Q_DECL_OVERRIDE;
    QString inputLabel() const Q_DECL_OVERRIDE;
    QString outputLabel() const Q_DECL_OVERRIDE;
    unsigned long long inputSize() const Q_DECL_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotResult(GpgME::VerificationResult, QByteArray))
};

class DecryptVerifyTask : public AbstractDecryptVerifyTask
{
    Q_OBJECT
public:
    explicit DecryptVerifyTask(QObject *parent = Q_NULLPTR);
    ~DecryptVerifyTask();

    void setInput(const boost::shared_ptr<Input> &input);
    void setSignedData(const boost::shared_ptr<Input> &signedData);
    void setOutput(const boost::shared_ptr<Output> &output);

    void setProtocol(GpgME::Protocol prot);
    void autodetectProtocolFromInput() Q_DECL_OVERRIDE;

    QString label() const Q_DECL_OVERRIDE;

    GpgME::Protocol protocol() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void cancel() Q_DECL_OVERRIDE;

private:
    void doStart() Q_DECL_OVERRIDE;
    QString inputLabel() const Q_DECL_OVERRIDE;
    QString outputLabel() const Q_DECL_OVERRIDE;
    unsigned long long inputSize() const Q_DECL_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotResult(GpgME::DecryptionResult, GpgME::VerificationResult, QByteArray))
};

class DecryptVerifyResult : public Task::Result
{
    friend class ::Kleo::Crypto::AbstractDecryptVerifyTask;
public:
    class SenderInfo;

    QString overview() const Q_DECL_OVERRIDE;
    QString details() const Q_DECL_OVERRIDE;
    bool hasError() const;
    int errorCode() const Q_DECL_OVERRIDE;
    QString errorString() const Q_DECL_OVERRIDE;
    VisualCode code() const Q_DECL_OVERRIDE;
    AuditLog auditLog() const Q_DECL_OVERRIDE;

    GpgME::VerificationResult verificationResult() const;

    static const GpgME::Key &keyForSignature(const GpgME::Signature &sig, const std::vector<GpgME::Key> &keys);

private:
    static QString keyToString(const GpgME::Key &key);

private:
    DecryptVerifyResult();
    DecryptVerifyResult(const DecryptVerifyResult &);
    DecryptVerifyResult &operator=(const DecryptVerifyResult &other);

    DecryptVerifyResult(DecryptVerifyOperation op,
                        const GpgME::VerificationResult &vr,
                        const GpgME::DecryptionResult &dr,
                        const QByteArray &stuff,
                        int errCode,
                        const QString &errString,
                        const QString &inputLabel,
                        const QString &outputLabel,
                        const AuditLog &auditLog,
                        const KMime::Types::Mailbox &informativeSender);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};
}
}

#endif //__KLEOPATRA_CRYPTO_DECRYPTVERIFYTASK_H__
