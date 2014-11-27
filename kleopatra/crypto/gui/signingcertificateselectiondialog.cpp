/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signingcertificateselectiondialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include <config-kleopatra.h>

#include "signingcertificateselectiondialog.h"
#include "signingcertificateselectionwidget.h"

#include <KLocalizedString>

#include <cassert>
#include <KConfigGroup>

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;

SigningCertificateSelectionDialog::SigningCertificateSelectionDialog(QWidget *parent)
    : QDialog(parent),
      widget(new SigningCertificateSelectionWidget(this))
{
    setWindowTitle(i18n("Select Signing Certificates"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SigningCertificateSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SigningCertificateSelectionDialog::reject);
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);
}

SigningCertificateSelectionDialog::~SigningCertificateSelectionDialog() {}

void SigningCertificateSelectionDialog::setSelectedCertificates(const QMap<GpgME::Protocol, GpgME::Key> &certificates)
{
    widget->setSelectedCertificates(certificates);
}

QMap<GpgME::Protocol, GpgME::Key> SigningCertificateSelectionDialog::selectedCertificates() const
{
    return widget->selectedCertificates();
}

bool SigningCertificateSelectionDialog::rememberAsDefault() const
{
    return widget->rememberAsDefault();
}

void SigningCertificateSelectionDialog::setAllowedProtocols(const QVector<GpgME::Protocol> &allowedProtocols)
{
    widget->setAllowedProtocols(allowedProtocols);
}

