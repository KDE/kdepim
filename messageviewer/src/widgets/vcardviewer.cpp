/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "vcardviewer.h"
#include "settings/messageviewersettings.h"
#include "KaddressbookGrantlee/GrantleeContactViewer"

#include <kcontacts/vcardconverter.h>
using KContacts::VCardConverter;
using KContacts::Addressee;

#include <KLocalizedString>

#include <Libkdepim/AddContactJob>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <KGuiItem>
#include <QVBoxLayout>

using namespace MessageViewer;

VCardViewer::VCardViewer(QWidget *parent, const QByteArray &vCard)
    : QDialog(parent),
      mAddresseeListIndex(0)
{
    setWindowTitle(i18n("vCard Viewer"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    mUser2Button = new QPushButton;
    mUser3Button = new QPushButton;
    buttonBox->addButton(mUser3Button, QDialogButtonBox::ActionRole);
    buttonBox->addButton(mUser2Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &VCardViewer::reject);
    setModal(false);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    KGuiItem::assign(user1Button, KGuiItem(i18n("&Import")));
    KGuiItem::assign(mUser2Button, KGuiItem(i18n("&Next Card")));
    KGuiItem::assign(mUser3Button, KGuiItem(i18n("&Previous Card")));

    mContactViewer = new KAddressBookGrantlee::GrantleeContactViewer(this);
    mContactViewer->setForceDisableQRCode(true);
    mainLayout->addWidget(mContactViewer);
    mainLayout->addWidget(buttonBox);

    VCardConverter vcc;
    mAddresseeList = vcc.parseVCards(vCard);
    if (!mAddresseeList.empty()) {
        mContactViewer->setRawContact(mAddresseeList.at(0));
        if (mAddresseeList.size() <= 1) {
            mUser2Button->setVisible(false);
            mUser3Button->setVisible(false);
        } else {
            mUser3Button->setEnabled(false);
        }
        connect(user1Button, &QPushButton::clicked, this, &VCardViewer::slotUser1);
        connect(mUser2Button, &QPushButton::clicked, this, &VCardViewer::slotUser2);
        connect(mUser3Button, &QPushButton::clicked, this, &VCardViewer::slotUser3);
    } else {
        mContactViewer->setRawContact(KContacts::Addressee());
        user1Button->setEnabled(false);
        mUser2Button->setVisible(false);
        mUser3Button->setVisible(false);
    }
    readConfig();
}

VCardViewer::~VCardViewer()
{
    writeConfig();
}

void VCardViewer::readConfig()
{
    KConfigGroup group(MessageViewer::MessageViewerSettings::self()->config(), "VCardViewer");
    const QSize size = group.readEntry("Size", QSize(300, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void VCardViewer::writeConfig()
{
    KConfigGroup group(MessageViewer::MessageViewerSettings::self()->config(), "VCardViewer");
    group.writeEntry("Size", size());
    group.sync();
}

void VCardViewer::slotUser1()
{
    const KContacts::Addressee contact = mAddresseeList.at(mAddresseeListIndex);

    KPIM::AddContactJob *job = new KPIM::AddContactJob(contact, this, this);
    job->start();
}

void VCardViewer::slotUser2()
{
    // next vcard
    mContactViewer->setRawContact(mAddresseeList.at(++mAddresseeListIndex));
    if ((mAddresseeListIndex + 1) == (mAddresseeList.count())) {
        mUser2Button->setEnabled(false);
    }
    mUser3Button->setEnabled(true);
}

void VCardViewer::slotUser3()
{
    // previous vcard
    mContactViewer->setRawContact(mAddresseeList.at(--mAddresseeListIndex));
    if (mAddresseeListIndex == 0) {
        mUser3Button->setEnabled(false);
    }
    mUser2Button->setEnabled(true);
}

