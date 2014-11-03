/* -*- mode: c++; c-basic-offset:4 -*-
    gui/certificaterequester.h

    This file is part of KleopatraClient, the Kleopatra interface library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    KleopatraClient is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KleopatraClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "certificaterequester.h"

#include <libkleopatraclient/core/selectcertificatecommand.h>

#include <QPointer>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QHBoxLayout>

#include <memory>

using namespace KleopatraClientCopy;
using namespace KleopatraClientCopy::Gui;

class CertificateRequester::Private
{
    friend class ::KleopatraClientCopy::Gui::CertificateRequester;
    CertificateRequester *const q;
public:
    explicit Private(CertificateRequester *qq)
        : q(qq),
          selectedCertificates(),
          command(),
          multipleCertificatesAllowed(false),
          onlySigningCertificatesAllowed(false),
          onlyEncryptionCertificatesAllowed(false),
          onlyOpenPGPCertificatesAllowed(false),
          onlyX509CertificatesAllowed(false),
          onlySecretKeysAllowed(false),
          ui(q)
    {

    }

private:
    void updateLineEdit()
    {
        ui.lineEdit.setText(selectedCertificates.join(QLatin1String(" ")));
    }
    void createCommand()
    {
        std::auto_ptr<SelectCertificateCommand> cmd(new SelectCertificateCommand);

        cmd->setMultipleCertificatesAllowed(multipleCertificatesAllowed);
        cmd->setOnlySigningCertificatesAllowed(onlySigningCertificatesAllowed);
        cmd->setOnlyEncryptionCertificatesAllowed(onlyEncryptionCertificatesAllowed);
        cmd->setOnlyOpenPGPCertificatesAllowed(onlyOpenPGPCertificatesAllowed);
        cmd->setOnlyX509CertificatesAllowed(onlyX509CertificatesAllowed);
        cmd->setOnlySecretKeysAllowed(onlySecretKeysAllowed);

        cmd->setSelectedCertificates(selectedCertificates);

        if (const QWidget *const window = q->window()) {
            cmd->setParentWId(window->effectiveWinId());
        }

        connect(cmd.get(), SIGNAL(finished()), q, SLOT(slotCommandFinished()));

        command = cmd.release();
    }

    void slotButtonClicked();
    void slotCommandFinished();

private:
    QStringList selectedCertificates;

    QPointer<SelectCertificateCommand> command;

    bool multipleCertificatesAllowed : 1;
    bool onlySigningCertificatesAllowed : 1;
    bool onlyEncryptionCertificatesAllowed : 1;
    bool onlyOpenPGPCertificatesAllowed : 1;
    bool onlyX509CertificatesAllowed : 1;
    bool onlySecretKeysAllowed : 1;

    struct Ui {
        QLineEdit lineEdit;
        QPushButton button;
        QHBoxLayout hlay;

        explicit Ui(CertificateRequester *qq)
            : lineEdit(qq),
              button(tr("Change..."), qq),
              hlay(qq)
        {
            lineEdit.setObjectName(QLatin1String("lineEdit"));
            button.setObjectName(QLatin1String("button"));
            hlay.setObjectName(QLatin1String("hlay"));

            hlay.addWidget(&lineEdit, 1);
            hlay.addWidget(&button);

            lineEdit.setReadOnly(true);

            connect(&button, SIGNAL(clicked()),
                    qq, SLOT(slotButtonClicked()));
        }

    } ui;
};

CertificateRequester::CertificateRequester(QWidget *p, Qt::WindowFlags f)
    : QWidget(p, f), d(new Private(this))
{

}

CertificateRequester::~CertificateRequester()
{
    delete d; d = 0;
}

void CertificateRequester::setMultipleCertificatesAllowed(bool allow)
{
    if (allow == d->multipleCertificatesAllowed) {
        return;
    }
    d->multipleCertificatesAllowed = allow;
}

bool CertificateRequester::multipleCertificatesAllowed() const
{
    return d->multipleCertificatesAllowed;
}

void CertificateRequester::setOnlySigningCertificatesAllowed(bool allow)
{
    if (allow == d->onlySigningCertificatesAllowed) {
        return;
    }
    d->onlySigningCertificatesAllowed = allow;
}

bool CertificateRequester::onlySigningCertificatesAllowed() const
{
    return d->onlySigningCertificatesAllowed;
}

void CertificateRequester::setOnlyEncryptionCertificatesAllowed(bool allow)
{
    if (allow == d->onlyEncryptionCertificatesAllowed) {
        return;
    }
    d->onlyEncryptionCertificatesAllowed = allow;
}

bool CertificateRequester::onlyEncryptionCertificatesAllowed() const
{
    return d->onlyEncryptionCertificatesAllowed;
}

void CertificateRequester::setOnlyOpenPGPCertificatesAllowed(bool allow)
{
    if (allow == d->onlyOpenPGPCertificatesAllowed) {
        return;
    }
    d->onlyOpenPGPCertificatesAllowed = allow;
}

bool CertificateRequester::onlyOpenPGPCertificatesAllowed() const
{
    return d->onlyOpenPGPCertificatesAllowed;
}

void CertificateRequester::setOnlyX509CertificatesAllowed(bool allow)
{
    if (allow == d->onlyX509CertificatesAllowed) {
        return;
    }
    d->onlyX509CertificatesAllowed = allow;
}

bool CertificateRequester::onlyX509CertificatesAllowed() const
{
    return d->onlyX509CertificatesAllowed;
}

void CertificateRequester::setOnlySecretKeysAllowed(bool allow)
{
    if (allow == d->onlySecretKeysAllowed) {
        return;
    }
    d->onlySecretKeysAllowed = allow;
}

bool CertificateRequester::onlySecretKeysAllowed() const
{
    return d->onlySecretKeysAllowed;
}

void CertificateRequester::setSelectedCertificates(const QStringList &certs)
{
    if (certs == d->selectedCertificates) {
        return;
    }
    d->selectedCertificates = certs;
    d->updateLineEdit();
    /*emit*/ selectedCertificatesChanged(certs);
}

QStringList CertificateRequester::selectedCertificates() const
{
    return d->selectedCertificates;
}

void CertificateRequester::setSelectedCertificate(const QString &cert)
{
    setSelectedCertificates(QStringList(cert));
}

QString CertificateRequester::selectedCertificate() const
{
    return d->selectedCertificates.empty() ? QString() : d->selectedCertificates.front() ;
}

void CertificateRequester::Private::slotButtonClicked()
{
    if (command) {
        return;
    }
    createCommand();
    command->start();
    ui.button.setEnabled(false);
}

void CertificateRequester::Private::slotCommandFinished()
{
    if (command->wasCanceled())
        /* do nothing */;
    else if (command->error())
        QMessageBox::information(q,
                                 tr("Kleopatra Error"),
                                 tr("There was an error while connecting to Kleopatra: %1")
                                 .arg(command->errorString()));
    else {
        q->setSelectedCertificates(command->selectedCertificates());
    }
    ui.button.setEnabled(true);
    delete command;
}

#include "moc_certificaterequester.cpp"
