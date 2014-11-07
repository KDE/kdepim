/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    This file was part of KMail.
    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "recipientspicker.h"
#include "settings/messagecomposersettings.h"

#include <Akonadi/Contact/EmailAddressSelectionWidget>
#include <kcontacts/contactgroup.h>
#include <libkdepim/ldap/ldapsearchdialog.h>
#include <KPIMUtils/kpimutils/email.h>

#include <kconfiggroup.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>

#include <QKeyEvent>
#include <QTreeView>
#include <QVBoxLayout>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>

using namespace MessageComposer;

RecipientsPicker::RecipientsPicker(QWidget *parent)
    : QDialog(parent),
      mLdapSearchDialog(0)
{
    setObjectName(QLatin1String("RecipientsPicker"));
    setWindowTitle(i18n("Select Recipient"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mView = new Akonadi::EmailAddressSelectionWidget(this);
    mainLayout->addWidget(mView);
    mView->view()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mView->view()->setAlternatingRowColors(true);
    mView->view()->setSortingEnabled(true);
    mView->view()->sortByColumn(0, Qt::AscendingOrder);
    mainLayout->setStretchFactor(mView, 1);

    connect(mView->view()->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(slotSelectionChanged()));
    connect(mView->view(), SIGNAL(doubleClicked(QModelIndex)),
            SLOT(slotPicked()));

    QPushButton *searchLDAPButton = new QPushButton(i18n("Search &Directory Service"), this);
    connect(searchLDAPButton, &QPushButton::clicked, this, &RecipientsPicker::slotSearchLDAP);
    mainLayout->addWidget(searchLDAPButton);

    KConfig config(QLatin1String("kabldaprc"));
    KConfigGroup group = config.group("LDAP");
    int numHosts = group.readEntry("NumSelectedHosts", 0);
    if (!numHosts) {
        searchLDAPButton->setVisible(false);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    mUser2Button = new QPushButton;
    buttonBox->addButton(mUser2Button, QDialogButtonBox::ActionRole);
    mUser3Button = new QPushButton;
    buttonBox->addButton(mUser3Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RecipientsPicker::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RecipientsPicker::reject);
    mainLayout->addWidget(buttonBox);
    mUser3Button->setText(i18n("Add as &To"));
    mUser2Button->setText(i18n("Add as CC"));
    mUser1Button->setText(i18n("Add as &BCC"));
    connect(mUser1Button, &QPushButton::clicked, this, &RecipientsPicker::slotBccClicked);
    connect(mUser2Button, &QPushButton::clicked, this, &RecipientsPicker::slotCcClicked);
    connect(mUser3Button, &QPushButton::clicked, this, &RecipientsPicker::slotToClicked);

    mView->searchLineEdit()->setFocus();

    readConfig();

    slotSelectionChanged();
}

RecipientsPicker::~RecipientsPicker()
{
    writeConfig();
}

void RecipientsPicker::slotSelectionChanged()
{
    const bool hasSelection = !mView->selectedAddresses().isEmpty();
    mUser1Button->setEnabled(hasSelection);
    mUser2Button->setEnabled(hasSelection);
    mUser3Button->setEnabled(hasSelection);
}

void RecipientsPicker::setRecipients(const Recipient::List &)
{
    mView->view()->selectionModel()->clear();
}

void RecipientsPicker::setDefaultType(Recipient::Type type)
{
    mDefaultType = type;
    mUser1Button->setDefault(type == Recipient::To);
    mUser2Button->setDefault(type == Recipient::Cc);
    mUser3Button->setDefault(type == Recipient::Bcc);
}

void RecipientsPicker::slotToClicked()
{
    pick(Recipient::To);
}

void RecipientsPicker::slotCcClicked()
{
    pick(Recipient::Cc);
}

void RecipientsPicker::slotBccClicked()
{
    pick(Recipient::Bcc);
}

void RecipientsPicker::slotPicked()
{
    pick(mDefaultType);
}

void RecipientsPicker::pick(Recipient::Type type)
{
    qDebug() << int(type);

    const Akonadi::EmailAddressSelection::List selections = mView->selectedAddresses();

    const int count = selections.count();
    if (count == 0) {
        return;
    }

    if (count > MessageComposerSettings::self()->maximumRecipients()) {
        KMessageBox::sorry(this,
                           i18np("You selected 1 recipient. The maximum supported number of "
                                 "recipients is %2. Please adapt the selection.",
                                 "You selected %1 recipients. The maximum supported number of "
                                 "recipients is %2. Please adapt the selection.", count,
                                 MessageComposerSettings::self()->maximumRecipients()));
        return;
    }

    bool tooManyAddress = false;
    foreach (const Akonadi::EmailAddressSelection &selection, selections) {
        Recipient recipient;
        recipient.setType(type);
        recipient.setEmail(selection.quotedEmail());

        emit pickedRecipient(recipient, tooManyAddress);
        if (tooManyAddress) {
            break;
        }
    }
}

void RecipientsPicker::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }

    QDialog::keyPressEvent(event);
}

void RecipientsPicker::readConfig()
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    KConfigGroup group(cfg, "RecipientsPicker");
    QSize size = group.readEntry("Size", QSize());
    if (!size.isEmpty()) {
        resize(size);
    }
}

void RecipientsPicker::writeConfig()
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    KConfigGroup group(cfg, "RecipientsPicker");
    group.writeEntry("Size", size());
}

void RecipientsPicker::slotSearchLDAP()
{
    if (!mLdapSearchDialog) {
        mLdapSearchDialog = new KLDAP::LdapSearchDialog(this);
        connect(mLdapSearchDialog, &KLDAP::LdapSearchDialog::contactsAdded, this, &RecipientsPicker::ldapSearchResult);
    }

    mLdapSearchDialog->setSearchText(mView->searchLineEdit()->text());
    mLdapSearchDialog->show();
}

void RecipientsPicker::ldapSearchResult()
{
    const KContacts::Addressee::List contacts = mLdapSearchDialog->selectedContacts();
    foreach (const KContacts::Addressee &contact, contacts) {
        bool tooManyAddress = false;
        emit pickedRecipient(Recipient(contact.fullEmail(), Recipient::Undefined), tooManyAddress);
        if (tooManyAddress) {
            break;
        }
    }
}

