/*
  kmfawidgets.cpp - KMFilterAction parameter widgets
  Copyright (c) 2001 Marc Mutz <mutz@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kmfawidgets.h"

#include <akonadi/contact/emailaddressselectiondialog.h> // for the button in KMFilterActionWithAddress
#include <kabc/addressee.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QTreeView>

//=============================================================================
//
// class KMFilterActionWithAddressWidget
//
//=============================================================================

KMFilterActionWithAddressWidget::KMFilterActionWithAddressWidget( QWidget* parent, const char* name )
  : QWidget( parent )
{
  setObjectName( name );
  QHBoxLayout *hbl = new QHBoxLayout(this);
  hbl->setSpacing(4);
  hbl->setMargin( 0 );
  mLineEdit = new KLineEdit(this);
  mLineEdit->setObjectName( "addressEdit" );
  hbl->addWidget( mLineEdit, 1 /*stretch*/ );
  mBtn = new QPushButton( QString(),this );
  mBtn->setIcon( KIcon( "help-contents" ) );
  mBtn->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtn->setFixedHeight( mLineEdit->sizeHint().height() );
  mBtn->setToolTip( i18n( "Open Address Book" ) );
  hbl->addWidget( mBtn );

  connect( mBtn, SIGNAL(clicked()), this, SLOT(slotAddrBook()) );
  connect( mLineEdit, SIGNAL( textChanged(const QString &) ),
           this, SIGNAL( textChanged() ) );
}

void KMFilterActionWithAddressWidget::slotAddrBook()
{
  Akonadi::EmailAddressSelectionDialog dlg( this );
  dlg.view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );
  if ( !dlg.exec() )
    return;

  QStringList addrList;
  foreach ( const Akonadi::EmailAddressSelectionView::Selection &selection, dlg.selectedAddresses() ) {
    KABC::Addressee contact;
    contact.setNameFromString( selection.name() );
    contact.insertEmail( selection.email() );

    addrList << contact.fullEmail();
  }

  QString txt = mLineEdit->text().trimmed();

  if ( !txt.isEmpty() ) {
    if ( !txt.endsWith( ',' ) )
      txt += ", ";
    else
      txt += ' ';
  }

  mLineEdit->setText( txt + addrList.join(",") );
}
//--------------------------------------------
#include "kmfawidgets.moc"
