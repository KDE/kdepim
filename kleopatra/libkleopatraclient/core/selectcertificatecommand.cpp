/* -*- mode: c++; c-basic-offset:4 -*-
    core/selectcertificatecommand.cpp

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

#include "selectcertificatecommand.h"

using namespace KleopatraClientCopy;

SelectCertificateCommand::SelectCertificateCommand(QObject *p)
    : Command(p)
{
    setCommand("SELECT_CERTIFICATE");
}

SelectCertificateCommand::~SelectCertificateCommand() {}

void SelectCertificateCommand::setMultipleCertificatesAllowed(bool allow)
{
    if (allow) {
        setOption("multi", true);
    } else {
        unsetOption("multi");
    }
}

bool SelectCertificateCommand::multipleCertificatesAllowed() const
{
    return isOptionSet("multi");
}

void SelectCertificateCommand::setOnlySigningCertificatesAllowed(bool allow)
{
    if (allow) {
        setOption("sign-only", true);
    } else {
        unsetOption("sign-only");
    }
}

bool SelectCertificateCommand::onlySigningCertificatesAllowed() const
{
    return isOptionSet("sign-only");
}

void SelectCertificateCommand::setOnlyEncryptionCertificatesAllowed(bool allow)
{
    if (allow) {
        setOption("encrypt-only", true);
    } else {
        unsetOption("encrypt-only");
    }
}

bool SelectCertificateCommand::onlyEncryptionCertificatesAllowed() const
{
    return isOptionSet("encrypt-only");
}

void SelectCertificateCommand::setOnlyOpenPGPCertificatesAllowed(bool allow)
{
    if (allow) {
        setOption("openpgp-only", true);
    } else {
        unsetOption("openpgp-only");
    }
}

bool SelectCertificateCommand::onlyOpenPGPCertificatesAllowed() const
{
    return isOptionSet("openpgp-only");
}

void SelectCertificateCommand::setOnlyX509CertificatesAllowed(bool allow)
{
    if (allow) {
        setOption("x509-only", true);
    } else {
        unsetOption("x509-only");
    }
}

bool SelectCertificateCommand::onlyX509CertificatesAllowed() const
{
    return isOptionSet("x509-only");
}

void SelectCertificateCommand::setOnlySecretKeysAllowed(bool allow)
{
    if (allow) {
        setOption("secret-only", true);
    } else {
        unsetOption("secret-only");
    }
}

bool SelectCertificateCommand::onlySecretKeysAllowed() const
{
    return isOptionSet("secret-only");
}

void SelectCertificateCommand::setSelectedCertificates(const QStringList &certs)
{
    QByteArray data;
    Q_FOREACH (const QString &s, certs)
        if (s.isEmpty()) {
            qWarning("SelectCertificateCommand::setSelectedCertificates: empty certificate!");
        } else {
            data += s.toUtf8() += '\n';
        }
    setInquireData("SELECTED_CERTIFICATES", data);
}

QStringList SelectCertificateCommand::selectedCertificates() const
{
    const QByteArray data = receivedData();
    return QString::fromLatin1(data.data(), data.size()).split(QLatin1Char('\n'), QString::SkipEmptyParts);
}

void SelectCertificateCommand::setSelectedCertificate(const QString &cert)
{
    setSelectedCertificates(QStringList(cert));
}

QString SelectCertificateCommand::selectedCertificate() const
{
    const QStringList sl = selectedCertificates();
    return sl.empty() ? QString() : sl.front() ;
}

