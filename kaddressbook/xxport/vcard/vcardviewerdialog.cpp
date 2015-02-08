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
#include <KGlobal>
#include <QLabel>
#include <QVBoxLayout>
#include <widget/grantleecontactviewer.h>

VCardViewerDialog::VCardViewerDialog( const KABC::Addressee::List &list, QWidget *parent )
    : KDialog( parent ),
      mContacts( list )
{
    setCaption( i18nc( "@title:window", "Import vCard" ) );
    setButtons( User1 | User2 | Apply | Cancel );
    setButtonGuiItem(User1, KStandardGuiItem::no());
    setButtonGuiItem(User2, KStandardGuiItem::yes());
    setDefaultButton( User1 );
    setModal( true );
    showButtonSeparator( true );

    QFrame *page = new QFrame( this );
    setMainWidget( page );

    QVBoxLayout *layout = new QVBoxLayout( page );
    layout->setSpacing( spacingHint() );
    layout->setMargin( marginHint() );

    QLabel *label =
            new QLabel(
                i18nc( "@info", "Do you want to import this contact into your address book?" ), page );
    QFont font = label->font();
    font.setBold( true );
    label->setFont( font );
    layout->addWidget( label );

    mView = new KAddressBookGrantlee::GrantleeContactViewer( page );

    layout->addWidget( mView );

    setButtonText( Apply, i18nc( "@action:button", "Import All..." ) );

    mIt = mContacts.begin();

    connect( this, SIGNAL(user2Clicked()), this, SLOT(slotYes()) );
    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotNo()) );
    connect( this, SIGNAL(applyClicked()), this, SLOT(slotApply()) );
    connect( this, SIGNAL(cancelClicked()), this, SLOT(slotCancel()) );

    updateView();
    readConfig();
}

VCardViewerDialog::~VCardViewerDialog()
{
    writeConfig();
}

void VCardViewerDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "VCardViewerDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void VCardViewerDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "VCardViewerDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}


KABC::Addressee::List VCardViewerDialog::contacts() const
{
    return mContacts;
}

void VCardViewerDialog::updateView()
{
    mView->setRawContact( *mIt );

    KABC::Addressee::List::Iterator it = mIt;
    enableButton( Apply, ( ++it ) != mContacts.end() );
}

void VCardViewerDialog::slotYes()
{
    mIt++;

    if ( mIt == mContacts.end() ) {
        slotApply();
        return;
    }

    updateView();
}

void VCardViewerDialog::slotNo()
{
    if ( mIt == mContacts.end() ) {
        accept();
        return;
    }
    // remove the current contact from the result set
    mIt = mContacts.erase( mIt );
    if ( mIt == mContacts.end() ) {
        return;
    }

    updateView();
}

void VCardViewerDialog::slotApply()
{
    KDialog::accept();
}

void VCardViewerDialog::slotCancel()
{
    mContacts.clear();
    reject();
}
