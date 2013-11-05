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

#include <akonadi/contact/emailaddressselectionwidget.h>
#include <kabc/contactgroup.h>
#include <libkdepim/ldap/ldapsearchdialog.h>
#include <kpimutils/email.h>

#include <kconfiggroup.h>
#include <khbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <KLineEdit>
#include <KPushButton>

#include <QKeyEvent>
#include <QTreeView>
#include <QLayout>
#include <QVBoxLayout>

using namespace MessageComposer;

RecipientsPicker::RecipientsPicker( QWidget *parent )
    : KDialog( parent ),
      mLdapSearchDialog( 0 )
{
    setObjectName( QLatin1String("RecipientsPicker") );
    setWindowTitle( i18n( "Select Recipient" ) );
    setButtons( None );

    QVBoxLayout *topLayout = new QVBoxLayout( mainWidget() );
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( 0 );

    mView = new Akonadi::EmailAddressSelectionWidget( mainWidget() );
    mView->view()->setSelectionMode( QAbstractItemView::ExtendedSelection );
    mView->view()->setAlternatingRowColors( true );
    mView->view()->setSortingEnabled( true );
    mView->view()->sortByColumn( 0, Qt::AscendingOrder );
    topLayout->addWidget( mView );
    topLayout->setStretchFactor( mView, 1 );

    connect( mView->view()->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             SLOT(slotSelectionChanged()) );
    connect( mView->view(), SIGNAL(doubleClicked(QModelIndex)),
             SLOT(slotPicked()) );

    QPushButton *searchLDAPButton = new QPushButton( i18n("Search &Directory Service"), mainWidget() );
    connect( searchLDAPButton, SIGNAL(clicked()), SLOT(slotSearchLDAP()) );
    topLayout->addWidget( searchLDAPButton );

    KConfig config( QLatin1String("kabldaprc") );
    KConfigGroup group = config.group( "LDAP" );
    int numHosts = group.readEntry( "NumSelectedHosts", 0 );
    if ( !numHosts )
        searchLDAPButton->setVisible( false );

    setButtons( Close|User1|User2|User3 );
    setButtonText( User3, i18n("Add as &To") );
    setButtonText( User2, i18n("Add as CC") );
    setButtonText( User1, i18n("Add as &BCC") );
    connect(this,SIGNAL(user1Clicked()),this,SLOT(slotBccClicked()));
    connect(this,SIGNAL(user2Clicked()),this,SLOT(slotCcClicked()));
    connect(this,SIGNAL(user3Clicked()),this,SLOT(slotToClicked()));

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
    enableButton(User1, hasSelection );
    enableButton(User2, hasSelection );
    enableButton(User3, hasSelection );
}

void RecipientsPicker::setRecipients( const Recipient::List& )
{
    mView->view()->selectionModel()->clear();
}

void RecipientsPicker::setDefaultType( Recipient::Type type )
{
    mDefaultType = type;
    button(User3)->setDefault( type == Recipient::To );
    button(User2)->setDefault( type == Recipient::Cc );
    button(User1)->setDefault( type == Recipient::Bcc );
}

void RecipientsPicker::slotToClicked()
{
    pick( Recipient::To );
}

void RecipientsPicker::slotCcClicked()
{
    pick( Recipient::Cc );
}

void RecipientsPicker::slotBccClicked()
{
    pick( Recipient::Bcc );
}

void RecipientsPicker::slotPicked()
{
    pick( mDefaultType );
}

void RecipientsPicker::pick( Recipient::Type type )
{
    kDebug() << int( type );

    const Akonadi::EmailAddressSelection::List selections = mView->selectedAddresses();

    const int count = selections.count();
    if ( count == 0 )
        return;

    if ( count > MessageComposerSettings::self()->maximumRecipients() ) {
        KMessageBox::sorry( this,
                            i18np( "You selected 1 recipient. The maximum supported number of "
                                   "recipients is %2. Please adapt the selection.",
                                   "You selected %1 recipients. The maximum supported number of "
                                   "recipients is %2. Please adapt the selection.", count,
                                   MessageComposerSettings::self()->maximumRecipients() ) );
        return;
    }

    foreach ( const Akonadi::EmailAddressSelection &selection, selections ) {
        Recipient recipient;
        recipient.setType( type );
        recipient.setEmail( selection.quotedEmail() );

        emit pickedRecipient( recipient );
    }
}

void RecipientsPicker::keyPressEvent( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_Escape )
        close();

    KDialog::keyPressEvent( event );
}

void RecipientsPicker::readConfig()
{
    KSharedConfig::Ptr cfg = KGlobal::config();
    KConfigGroup group( cfg, "RecipientsPicker" );
    QSize size = group.readEntry( "Size", QSize() );
    if ( !size.isEmpty() ) {
        resize( size );
    }
}

void RecipientsPicker::writeConfig()
{
    KSharedConfig::Ptr cfg = KGlobal::config();
    KConfigGroup group( cfg, "RecipientsPicker" );
    group.writeEntry( "Size", size() );
}

void RecipientsPicker::slotSearchLDAP()
{
    if ( !mLdapSearchDialog ) {
        mLdapSearchDialog = new KLDAP::LdapSearchDialog( this );
        connect( mLdapSearchDialog, SIGNAL(contactsAdded()),
                 SLOT(ldapSearchResult()) );
    }

    mLdapSearchDialog->setSearchText( mView->searchLineEdit()->text() );
    mLdapSearchDialog->show();
}

void RecipientsPicker::ldapSearchResult()
{
    const KABC::Addressee::List contacts = mLdapSearchDialog->selectedContacts();
    foreach ( const KABC::Addressee &contact, contacts ) {
        emit pickedRecipient( Recipient( contact.fullEmail(), Recipient::Undefined ) );
    }
}

