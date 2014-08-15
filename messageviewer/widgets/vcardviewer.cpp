/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2013 Laurent Montel <montel@kde.org>

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
#include "settings/globalsettings.h"
#include "kaddressbookgrantlee/widget/grantleecontactviewer.h"

#include <kabc/vcardconverter.h>
using KABC::VCardConverter;
using KABC::Addressee;

#include <KLocalizedString>

#include <libkdepim/job/addcontactjob.h>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <KGuiItem>
#include <QVBoxLayout>

#ifndef KABC_ADDRESSEE_METATYPE_DEFINED
Q_DECLARE_METATYPE( KABC::Addressee )
#endif

using namespace MessageViewer;

VCardViewer::VCardViewer(QWidget *parent, const QByteArray& vCard)
    : QDialog( parent )
{
    setWindowTitle( i18n("vCard Viewer") );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    mUser2Button = new QPushButton;
    buttonBox->addButton(mUser2Button, QDialogButtonBox::ActionRole);
    mUser3Button = new QPushButton;
    buttonBox->addButton(mUser3Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    setModal( false );
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    
    KGuiItem::assign(user1Button, KGuiItem(i18n("&Import")));
    KGuiItem::assign(user1Button, KGuiItem(i18n("&Next Card")));
    KGuiItem::assign(user1Button, KGuiItem(i18n("&Previous Card")));

    mContactViewer = new KAddressBookGrantlee::GrantleeContactViewer( this );
    mContactViewer->setForceDisableQRCode(true);
    mainLayout->addWidget(mContactViewer);
    mainLayout->addWidget(buttonBox);

    VCardConverter vcc;
    mAddresseeList = vcc.parseVCards( vCard );
    if ( !mAddresseeList.empty() ) {
        itAddresseeList = mAddresseeList.constBegin();
        mContactViewer->setRawContact( *itAddresseeList );
        if ( mAddresseeList.size() <= 1 ) {
            mUser2Button->setVisible(false);
            mUser3Button->setVisible(false);
        }
        else
            mUser3Button->setEnabled(false);
        connect(user1Button, SIGNAL(clicked()), SLOT(slotUser1()) );
        connect(mUser2Button, SIGNAL(clicked()), SLOT(slotUser2()) );
        connect(mUser3Button, SIGNAL(clicked()), SLOT(slotUser3()) );
    } else {
        mContactViewer->setRawContact(KABC::Addressee());
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
    KConfigGroup group( MessageViewer::GlobalSettings::self()->config(), "VCardViewer" );
    const QSize size = group.readEntry( "Size", QSize(300, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void VCardViewer::writeConfig()
{
    KConfigGroup group( MessageViewer::GlobalSettings::self()->config(), "VCardViewer" );
    group.writeEntry( "Size", size() );
    group.sync();
}


void VCardViewer::slotUser1()
{
    const KABC::Addressee contact = *itAddresseeList;

    KPIM::AddContactJob *job = new KPIM::AddContactJob( contact, this, this );
    job->start();
}

void VCardViewer::slotUser2()
{
    // next vcard
    mContactViewer->setRawContact( *(++itAddresseeList) );
    if ( itAddresseeList == --(mAddresseeList.constEnd()) )
        mUser2Button->setEnabled(false);
    mUser3Button->setEnabled(true);
}

void VCardViewer::slotUser3()
{
    // previous vcard
    mContactViewer->setRawContact( *(--itAddresseeList) );
    if ( itAddresseeList == mAddresseeList.constBegin() )
        mUser3Button->setEnabled(false);
    mUser2Button->setEnabled(true);
}

