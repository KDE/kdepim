/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


// System includes
#include <stdlib.h>             // setenv()

// KDE includes
// This makes us rely on kdeui !
//#include <klineeditdlg.h>
#include <klocale.h>

// Local includes
#include "EmpathSecurityProcess.h"
#include "EmpathDefines.h"

EmpathSecurityProcess::EmpathSecurityProcess()
{
    empathDebug("ctor");

    QObject::connect(&p, SIGNAL(receivedStdout(KProcess *, char *, int)),
            this, SLOT(s_pgpSentOutput(KProcess *, char *, int)));

    QObject::connect(&p, SIGNAL(receivedStderr(KProcess *, char *, int)),
            this, SLOT(s_pgpSentError(KProcess *, char *, int)));

    QObject::connect(&p, SIGNAL(processExited(KProcess *)),
        this, SLOT(s_pgpFinished(KProcess *)));
}

EmpathSecurityProcess::~EmpathSecurityProcess()
{
    empathDebug("dtor");
    p.kill();
}

    void
EmpathSecurityProcess::encrypt(
    const QCString & s, const QCString & r, QObject * parent)
{
    EmpathSecurityProcess * p = new EmpathSecurityProcess;
    CHECK_PTR(p);

    p->_encrypt(s, r, parent);
}

    void
EmpathSecurityProcess::encryptAndSign(
    const QCString & s, const QCString & r, QObject * parent)
{
    EmpathSecurityProcess * p = new EmpathSecurityProcess;
    CHECK_PTR(p);

    p->_encryptAndSign(s, r, parent);
}

    void
EmpathSecurityProcess::decrypt(const QCString & s, QObject * parent)
{
    EmpathSecurityProcess * p = new EmpathSecurityProcess;
    CHECK_PTR(p);
    
    p->_decrypt(s, parent);
}

    void
EmpathSecurityProcess::s_pgpFinished(KProcess * p)
{
    emit(done(p->normalExit(), outputStr_));
    delete this;
}

    void
EmpathSecurityProcess::s_pgpSentOutput(KProcess *, char * s, int)
{
    outputStr_ += s;
}

    void
EmpathSecurityProcess::s_pgpSentError(KProcess *, char * s, int)
{
    errorStr_ = s;
}

    void
EmpathSecurityProcess::_encrypt(
    const QCString &, const QCString & recipient, QObject * parent)
{
    p    << "pgpe -atf +batchmode=1 -r " << recipient;

    QObject::connect(
        this,    SIGNAL(done(bool, QCString)),
        parent,    SLOT(s_encryptDone(bool, QCString)));
    
    if (!p.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start pgp process");
        emit(done(false, ""));
        delete this;
    }
}
    
    void
EmpathSecurityProcess::_encryptAndSign(
    const QCString &, const QCString & recipient, QObject * parent)
{
    p    << "pgpe -atf +batchmode=1 -s -r " << recipient;

    QObject::connect(
    this,    SIGNAL(done(bool, QCString)),
    parent,    SLOT(s_encryptAndSignDone(bool, QCString)));

    bool ok(false);

    // This makes us rely on kdeui !
    // Need to use EmpathUI to get this.
    QString passphrase; // =
#if 0
        KLineEditDlg::getText(
            i18n("PGP passphrase"),
            QString::null,
            &ok,
            static_cast<QWidget *>(0L)
        );
#endif
    
    if (!ok || passphrase.isEmpty()) {
        emit(done(false, ""));
        delete this;
    }
    
    if (!p.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start pgp process");
        emit(done(false, ""));
        delete this;
    }

    setenv("PGPPASSFD", "0", 1);
}

    void
EmpathSecurityProcess::_decrypt(
    const QCString &, QObject * parent)
{
    p    << "pgpv -f -z";
    
    QObject::connect(
        this,    SIGNAL(done(bool, QCString)),
        parent,    SLOT(s_decryptDone(bool, QCString)));
    
    if (!p.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start pgp process");
        emit(done(false, ""));
        delete this;
    }
}

// vim:ts=4:sw=4:tw=78
