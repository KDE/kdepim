/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "vcardviewerdialog.h"
#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QLabel>
#include <QVBoxLayout>
#include <widget/grantleecontactviewer.h>
#include <QPushButton>
#include <QDialogButtonBox>

VCardViewerDialog::VCardViewerDialog(const KContacts::Addressee::List &list, QWidget *parent)
    : QDialog(parent),
      mContacts(list)
{
    setWindowTitle(i18nc("@title:window", "Import vCard"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    QPushButton *user2Button = new QPushButton;
    buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &VCardViewerDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &VCardViewerDialog::reject);
    KGuiItem::assign(user1Button, KStandardGuiItem::no());
    KGuiItem::assign(user2Button, KStandardGuiItem::yes());
    mApplyButton = buttonBox->button(QDialogButtonBox::Apply);
    user1Button->setDefault(true);
    setModal(true);

    QFrame *page = new QFrame(this);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *label =
        new QLabel(
        i18nc("@info", "Do you want to import this contact into your address book?"), page);
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    layout->addWidget(label);

    mView = new KAddressBookGrantlee::GrantleeContactViewer(page);

    layout->addWidget(mView);

    buttonBox->button(QDialogButtonBox::Apply)->setText(i18nc("@action:button", "Import All..."));

    mIt = mContacts.begin();

    connect(user2Button, &QPushButton::clicked, this, &VCardViewerDialog::slotYes);
    connect(user1Button, &QPushButton::clicked, this, &VCardViewerDialog::slotNo);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &VCardViewerDialog::slotApply);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &VCardViewerDialog::slotCancel);

    updateView();
    readConfig();
}

VCardViewerDialog::~VCardViewerDialog()
{
    writeConfig();
}

void VCardViewerDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "VCardViewerDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void VCardViewerDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "VCardViewerDialog");
    group.writeEntry("Size", size());
    group.sync();
}

KContacts::Addressee::List VCardViewerDialog::contacts() const
{
    return mContacts;
}

void VCardViewerDialog::updateView()
{
    mView->setRawContact(*mIt);

    KContacts::Addressee::List::Iterator it = mIt;
    mApplyButton->setEnabled(++it != mContacts.end());
}

void VCardViewerDialog::slotYes()
{
    mIt++;

    if (mIt == mContacts.end()) {
        slotApply();
        return;
    }

    updateView();
}

void VCardViewerDialog::slotNo()
{
    if (mIt == mContacts.end()) {
        accept();
        return;
    }
    // remove the current contact from the result set
    mIt = mContacts.erase(mIt);
    if (mIt == mContacts.end()) {
        return;
    }

    updateView();
}

void VCardViewerDialog::slotApply()
{
    QDialog::accept();
}

void VCardViewerDialog::slotCancel()
{
    mContacts.clear();
    reject();
}
