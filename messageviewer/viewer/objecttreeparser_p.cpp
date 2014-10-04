/*  -*- mode: C++; c-file-style: "gnu" -*-
    objecttreeparser_p.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Klarälvdalens Datakonsult AB
    Authors: Marc Mutz <marc@kdab.net>
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

#include "objecttreeparser_p.h"

#include <qdebug.h>
#include <kleo/decryptverifyjob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/verifyopaquejob.h>
#include <kleo/keylistjob.h>

#include <gpgme++/keylistresult.h>

#include <qtimer.h>
#include <qstringlist.h>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace MessageViewer;

CryptoBodyPartMemento::CryptoBodyPartMemento()
    : QObject(0),
      Interface::BodyPartMemento(),
      m_running(false)
{

}

CryptoBodyPartMemento::~CryptoBodyPartMemento() {}

void CryptoBodyPartMemento::setAuditLog(const Error &err, const QString &log)
{
    m_auditLogError = err;
    m_auditLog = log;
}

void CryptoBodyPartMemento::setRunning(bool running)
{
    m_running = running;
}

void CryptoBodyPartMemento::detach()
{
    disconnect(this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0);
}

DecryptVerifyBodyPartMemento::DecryptVerifyBodyPartMemento(DecryptVerifyJob *job, const QByteArray &cipherText)
    : CryptoBodyPartMemento(),
      m_cipherText(cipherText),
      m_job(job)
{
    assert(m_job);
}

DecryptVerifyBodyPartMemento::~DecryptVerifyBodyPartMemento()
{
    if (m_job) {
        m_job->slotCancel();
    }
}

bool DecryptVerifyBodyPartMemento::start()
{
    assert(m_job);
    if (const Error err = m_job->start(m_cipherText)) {
        m_dr = DecryptionResult(err);
        return false;
    }
    connect(m_job, SIGNAL(result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)),
            this, SLOT(slotResult(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)));
    setRunning(true);
    return true;
}

void DecryptVerifyBodyPartMemento::exec()
{
    assert(m_job);
    QByteArray plainText;
    setRunning(true);
    const std::pair<DecryptionResult, VerificationResult> p = m_job->exec(m_cipherText, plainText);
    saveResult(p.first, p.second, plainText);
    m_job->deleteLater(); // exec'ed jobs don't delete themselves
    m_job = 0;
}

void DecryptVerifyBodyPartMemento::saveResult(const DecryptionResult &dr,
        const VerificationResult &vr,
        const QByteArray &plainText)
{
    assert(m_job);
    setRunning(false);
    m_dr = dr;
    m_vr = vr;
    m_plainText = plainText;
    setAuditLog(m_job->auditLogError(), m_job->auditLogAsHtml());
}

void DecryptVerifyBodyPartMemento::slotResult(const DecryptionResult &dr,
        const VerificationResult &vr,
        const QByteArray &plainText)
{
    saveResult(dr, vr, plainText);
    m_job = 0;
    notify();
}

VerifyDetachedBodyPartMemento::VerifyDetachedBodyPartMemento(VerifyDetachedJob *job,
        KeyListJob *klj,
        const QByteArray &signature,
        const QByteArray &plainText)
    : CryptoBodyPartMemento(),
      m_signature(signature),
      m_plainText(plainText),
      m_job(job),
      m_keylistjob(klj)
{
    assert(m_job);
}

VerifyDetachedBodyPartMemento::~VerifyDetachedBodyPartMemento()
{
    if (m_job) {
        m_job->slotCancel();
    }
    if (m_keylistjob) {
        m_keylistjob->slotCancel();
    }
}

bool VerifyDetachedBodyPartMemento::start()
{
    assert(m_job);
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyDetachedBodyPartMemento started";
#endif
    connect(m_job, SIGNAL(result(GpgME::VerificationResult)),
            this, SLOT(slotResult(GpgME::VerificationResult)));
    if (const Error err = m_job->start(m_signature, m_plainText)) {
        m_vr = VerificationResult(err);
#ifdef DEBUG_SIGNATURE
        qDebug() << "tokoe: VerifyDetachedBodyPartMemento stopped with error";
#endif
        return false;
    }
    setRunning(true);
    return true;
}

void VerifyDetachedBodyPartMemento::exec()
{
    assert(m_job);
    setRunning(true);
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyDetachedBodyPartMemento execed";
#endif
    saveResult(m_job->exec(m_signature, m_plainText));
    m_job->deleteLater(); // exec'ed jobs don't delete themselves
    m_job = 0;
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyDetachedBodyPartMemento after execed";
#endif
    if (canStartKeyListJob()) {
        std::vector<GpgME::Key> keys;
        m_keylistjob->exec(keyListPattern(), /*secretOnly=*/false, keys);
        if (!keys.empty()) {
            m_key = keys.back();
        }
    }
    if (m_keylistjob) {
        m_keylistjob->deleteLater();    // exec'ed jobs don't delete themselves
    }
    m_keylistjob = 0;
    setRunning(false);
}

bool VerifyDetachedBodyPartMemento::canStartKeyListJob() const
{
    if (!m_keylistjob) {
        return false;
    }
    const char *const fpr = m_vr.signature(0).fingerprint();
    return fpr && *fpr;
}

QStringList VerifyDetachedBodyPartMemento::keyListPattern() const
{
    assert(canStartKeyListJob());
    return QStringList(QString::fromLatin1(m_vr.signature(0).fingerprint()));
}

void VerifyDetachedBodyPartMemento::saveResult(const VerificationResult &vr)
{
    assert(m_job);
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyDetachedBodyPartMemento::saveResult called";
#endif
    m_vr = vr;
    setAuditLog(m_job->auditLogError(), m_job->auditLogAsHtml());
}

void VerifyDetachedBodyPartMemento::slotResult(const VerificationResult &vr)
{
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyDetachedBodyPartMemento::slotResult called";
#endif
    saveResult(vr);
    m_job = 0;
    if (canStartKeyListJob() && startKeyListJob()) {
#ifdef DEBUG_SIGNATURE
        qDebug() << "tokoe: VerifyDetachedBodyPartMemento: canStartKeyListJob && startKeyListJob";
#endif
        return;
    }
    if (m_keylistjob) {
        m_keylistjob->deleteLater();
    }
    m_keylistjob = 0;
    setRunning(false);
    notify();
}

bool VerifyDetachedBodyPartMemento::startKeyListJob()
{
    assert(canStartKeyListJob());
    if (const GpgME::Error err = m_keylistjob->start(keyListPattern())) {
        return false;
    }
    connect(m_keylistjob, SIGNAL(done()), this, SLOT(slotKeyListJobDone()));
    connect(m_keylistjob, SIGNAL(nextKey(GpgME::Key)),
            this, SLOT(slotNextKey(GpgME::Key)));
    return true;
}

void VerifyDetachedBodyPartMemento::slotNextKey(const GpgME::Key &key)
{
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyDetachedBodyPartMemento::slotNextKey called";
#endif
    m_key = key;
}

void VerifyDetachedBodyPartMemento::slotKeyListJobDone()
{
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyDetachedBodyPartMemento::slotKeyListJobDone called";
#endif
    m_keylistjob = 0;
    setRunning(false);
    notify();
}

VerifyOpaqueBodyPartMemento::VerifyOpaqueBodyPartMemento(VerifyOpaqueJob *job,
        KeyListJob   *klj,
        const QByteArray &signature)
    : CryptoBodyPartMemento(),
      m_signature(signature),
      m_job(job),
      m_keylistjob(klj)
{
    assert(m_job);
}

VerifyOpaqueBodyPartMemento::~VerifyOpaqueBodyPartMemento()
{
    if (m_job) {
        m_job->slotCancel();
    }
    if (m_keylistjob) {
        m_keylistjob->slotCancel();
    }
}

bool VerifyOpaqueBodyPartMemento::start()
{
    assert(m_job);
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyOpaqueBodyPartMemento started";
#endif
    if (const Error err = m_job->start(m_signature)) {
        m_vr = VerificationResult(err);
#ifdef DEBUG_SIGNATURE
        qDebug() << "tokoe: VerifyOpaqueBodyPartMemento stopped with error";
#endif
        return false;
    }
    connect(m_job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
            this, SLOT(slotResult(GpgME::VerificationResult,QByteArray)));
    setRunning(true);
    return true;
}

void VerifyOpaqueBodyPartMemento::exec()
{
    assert(m_job);
    setRunning(true);
    QByteArray plainText;
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyOpaqueBodyPartMemento execed";
#endif
    saveResult(m_job->exec(m_signature, plainText), plainText);
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyOpaqueBodyPartMemento after execed";
#endif
    m_job->deleteLater(); // exec'ed jobs don't delete themselves
    m_job = 0;
    if (canStartKeyListJob()) {
        std::vector<GpgME::Key> keys;
        m_keylistjob->exec(keyListPattern(), /*secretOnly=*/false, keys);
        if (!keys.empty()) {
            m_key = keys.back();
        }
    }
    if (m_keylistjob) {
        m_keylistjob->deleteLater();    // exec'ed jobs don't delete themselves
    }
    m_keylistjob = 0;
    setRunning(false);
}

bool VerifyOpaqueBodyPartMemento::canStartKeyListJob() const
{
    if (!m_keylistjob) {
        return false;
    }
    const char *const fpr = m_vr.signature(0).fingerprint();
    return fpr && *fpr;
}

QStringList VerifyOpaqueBodyPartMemento::keyListPattern() const
{
    assert(canStartKeyListJob());
    return QStringList(QString::fromLatin1(m_vr.signature(0).fingerprint()));
}

void VerifyOpaqueBodyPartMemento::saveResult(const VerificationResult &vr,
        const QByteArray &plainText)
{
    assert(m_job);
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyOpaqueBodyPartMemento::saveResult called";
#endif
    m_vr = vr;
    m_plainText = plainText;
    setAuditLog(m_job->auditLogError(), m_job->auditLogAsHtml());
}

void VerifyOpaqueBodyPartMemento::slotResult(const VerificationResult &vr,
        const QByteArray &plainText)
{
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyOpaqueBodyPartMemento::slotResult called";
#endif
    saveResult(vr, plainText);
    m_job = 0;
    if (canStartKeyListJob() && startKeyListJob()) {
#ifdef DEBUG_SIGNATURE
        qDebug() << "tokoe: VerifyOpaqueBodyPartMemento: canStartKeyListJob && startKeyListJob";
#endif
        return;
    }
    if (m_keylistjob) {
        m_keylistjob->deleteLater();
    }
    m_keylistjob = 0;
    setRunning(false);
    notify();
}

bool VerifyOpaqueBodyPartMemento::startKeyListJob()
{
    assert(canStartKeyListJob());
    if (const GpgME::Error err = m_keylistjob->start(keyListPattern())) {
        return false;
    }
    connect(m_keylistjob, SIGNAL(done()), this, SLOT(slotKeyListJobDone()));
    connect(m_keylistjob, SIGNAL(nextKey(GpgME::Key)),
            this, SLOT(slotNextKey(GpgME::Key)));
    return true;
}

void VerifyOpaqueBodyPartMemento::slotNextKey(const GpgME::Key &key)
{
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyOpaqueBodyPartMemento::slotNextKey called";
#endif
    m_key = key;
}

void VerifyOpaqueBodyPartMemento::slotKeyListJobDone()
{
#ifdef DEBUG_SIGNATURE
    qDebug() << "tokoe: VerifyOpaqueBodyPartMemento::slotKeyListJobDone called";
#endif
    m_keylistjob = 0;
    setRunning(false);
    notify();
}

