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

#include <klocale.h>

#include <libkdepim/job/addcontactjob.h>

#ifndef KABC_ADDRESSEE_METATYPE_DEFINED
Q_DECLARE_METATYPE( KABC::Addressee )
#endif

using namespace MessageViewer;

VCardViewer::VCardViewer(QWidget *parent, const QByteArray& vCard)
    : KDialog( parent )
{
    setCaption( i18n("vCard Viewer") );
    setButtons( User1|User2|User3|Close );
    setModal( false );
    setDefaultButton( Close );
    setButtonGuiItem( User1, KGuiItem(i18n("&Import")) );
    setButtonGuiItem( User2, KGuiItem(i18n("&Next Card")) );
    setButtonGuiItem( User3, KGuiItem(i18n("&Previous Card")) );

    mContactViewer = new KAddressBookGrantlee::GrantleeContactViewer( this );
    setMainWidget(mContactViewer);

    VCardConverter vcc;
    mAddresseeList = vcc.parseVCards( vCard );
    if ( !mAddresseeList.empty() ) {
        itAddresseeList = mAddresseeList.constBegin();
        mContactViewer->setRawContact( *itAddresseeList );
        if ( mAddresseeList.size() <= 1 ) {
            showButton(User2, false);
            showButton(User3, false);
        }
        else
            enableButton(User3, false);
        connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
        connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );
        connect( this, SIGNAL(user3Clicked()), SLOT(slotUser3()) );
    } else {
        mContactViewer->setRawContact(KABC::Addressee());
        enableButton(User1, false);
        showButton(User2, false);
        showButton(User3, false);
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
        enableButton(User2, false);
    enableButton(User3, true);
}

void VCardViewer::slotUser3()
{
    // previous vcard
    mContactViewer->setRawContact( *(--itAddresseeList) );
    if ( itAddresseeList == mAddresseeList.constBegin() )
        enableButton(User3, false);
    enableButton(User2, true);
}

