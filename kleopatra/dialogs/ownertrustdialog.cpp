/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/ownertrustdialog.cpp

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

#include <config-kleopatra.h>

#include "ownertrustdialog.h"
#include "ui_ownertrustdialog.h"

#include <utils/formatting.h>

#include <KLocalizedString>

#include <QPushButton>

#include <cassert>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace GpgME;

class OwnerTrustDialog::Private
{
    friend class ::Kleo::Dialogs::OwnerTrustDialog;
    OwnerTrustDialog *const q;
public:
    explicit Private(OwnerTrustDialog *qq)
        : q(qq),
          formattedCertificateName(i18n("(unknown certificate)")),
          originalTrust(Key::Undefined),
          hasSecret(false),
          advancedMode(false),
          ui(qq)
    {

    }

private:
    void slotTrustLevelChanged()
    {
        enableDisableWidgets();
    }

    void enableDisableWidgets();

private:
    QString formattedCertificateName;
    Key::OwnerTrust originalTrust;
    bool hasSecret : 1;
    bool advancedMode : 1;

    struct UI : public Ui::OwnerTrustDialog {
        explicit UI(Dialogs::OwnerTrustDialog *qq)
            : Ui::OwnerTrustDialog(), q(qq)
        {
            QWidget *mainWidget = new QWidget(q);

            setupUi(mainWidget);
            QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            QVBoxLayout *mainLayout = new QVBoxLayout;
            q->setLayout(mainLayout);
            mainLayout->addWidget(mainWidget);
            okButton = buttonBox->button(QDialogButtonBox::Ok);
            okButton->setDefault(true);
            okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
            q->connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
            q->connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));
            mainLayout->addWidget(buttonBox);
        }

        QPushButton *okPB() const
        {
            return okButton;
        }
        QPushButton *okButton;
        Dialogs::OwnerTrustDialog *q;
    } ui;
};

OwnerTrustDialog::OwnerTrustDialog(QWidget *p)
    : QDialog(p), d(new Private(this))
{
    connect(d->ui.unknownRB, SIGNAL(toggled(bool)), this, SLOT(slotTrustLevelChanged()));
    connect(d->ui.neverRB, SIGNAL(toggled(bool)), this, SLOT(slotTrustLevelChanged()));
    connect(d->ui.marginalRB, SIGNAL(toggled(bool)), this, SLOT(slotTrustLevelChanged()));
    connect(d->ui.fullRB, SIGNAL(toggled(bool)), this, SLOT(slotTrustLevelChanged()));
    connect(d->ui.ultimateRB, SIGNAL(toggled(bool)), this, SLOT(slotTrustLevelChanged()));
}

OwnerTrustDialog::~OwnerTrustDialog() {}

void OwnerTrustDialog::setFormattedCertificateName(const QString &formatted)
{
    if (formatted.isEmpty()) {
        return;
    }
    d->formattedCertificateName = formatted;
    setWindowTitle(i18nc("@title", "Change Trust Level of %1", formatted));
    d->ui.label->setText(i18nc("@info", "How much do you trust certifications made by <b>%1</b> to correctly verify authenticity of certificates?", formatted));
}

QString OwnerTrustDialog::formattedCertificateName() const
{
    return d->formattedCertificateName;
}

void OwnerTrustDialog::setHasSecretKey(bool secret)
{
    d->hasSecret = secret;
    d->enableDisableWidgets();
    setOwnerTrust(ownerTrust());
}

bool OwnerTrustDialog::hasSecretKey() const
{
    return d->hasSecret;
}

void OwnerTrustDialog::setAdvancedMode(bool advanced)
{
    d->advancedMode = advanced;
    d->enableDisableWidgets();
    setOwnerTrust(ownerTrust());
}

bool OwnerTrustDialog::isAdvancedMode() const
{
    return d->advancedMode;
}

void OwnerTrustDialog::Private::enableDisableWidgets()
{
    ui.unknownRB ->setEnabled(!hasSecret || advancedMode);
    ui.neverRB   ->setEnabled(!hasSecret || advancedMode);
    ui.marginalRB->setEnabled(!hasSecret || advancedMode);
    ui.fullRB    ->setEnabled(!hasSecret || advancedMode);
    ui.ultimateRB->setEnabled(hasSecret || advancedMode);
    ui.okPB()->setEnabled(q->ownerTrust() != Key::Undefined && q->ownerTrust() != originalTrust);
}

static void force_set_checked(QAbstractButton *b, bool on)
{
    // work around Qt bug (tested: 4.1.4, 4.2.3, 4.3.4)
    const bool autoExclusive = b->autoExclusive();
    b->setAutoExclusive(false);
    b->setChecked(b->isEnabled() && on);
    b->setAutoExclusive(autoExclusive);
}

void OwnerTrustDialog::setOwnerTrust(Key::OwnerTrust trust)
{
    d->originalTrust = trust;
    force_set_checked(d->ui.unknownRB, trust == Key::Unknown);
    force_set_checked(d->ui.neverRB,   trust == Key::Never);
    force_set_checked(d->ui.marginalRB, trust == Key::Marginal);
    force_set_checked(d->ui.fullRB,    trust == Key::Full);
    force_set_checked(d->ui.ultimateRB, trust == Key::Ultimate);
    d->enableDisableWidgets();
}

Key::OwnerTrust OwnerTrustDialog::ownerTrust() const
{
    if (d->ui.unknownRB->isChecked()) {
        return Key::Unknown;
    }
    if (d->ui.neverRB->isChecked()) {
        return Key::Never;
    }
    if (d->ui.marginalRB->isChecked()) {
        return Key::Marginal;
    }
    if (d->ui.fullRB->isChecked()) {
        return Key::Full;
    }
    if (d->ui.ultimateRB->isChecked()) {
        return Key::Ultimate;
    }
    return Key::Undefined;
}

#include "moc_ownertrustdialog.cpp"
