/*  -*- c++ -*-
    keyapprovaldialog.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Based on kpgpui.h
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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


#include "keyapprovaldialog.h"

#include "keyrequester.h"

#include "kleo/cryptobackend.h"
#include <qpushbutton.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kseparator.h>

#include <QStringList>
#include <QLabel>
#include <QComboBox>
#include <QScrollArea>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>

#include <gpgme++/key.h>

#include <assert.h>
#include <QApplication>
#include <QDesktopWidget>

static Kleo::EncryptionPreference cb2pref( int i ) {
  switch ( i ) {
  default:
  case 0: return Kleo::UnknownPreference;
  case 1: return Kleo::NeverEncrypt;
  case 2: return Kleo::AlwaysEncrypt;
  case 3: return Kleo::AlwaysEncryptIfPossible;
  case 4: return Kleo::AlwaysAskForEncryption;
  case 5: return Kleo::AskWheneverPossible;
  }
}

static int pref2cb( Kleo::EncryptionPreference p ) {
  switch ( p ) {
  default:                            return 0;
  case Kleo::NeverEncrypt:            return 1;
  case Kleo::AlwaysEncrypt:           return 2;
  case Kleo::AlwaysEncryptIfPossible: return 3;
  case Kleo::AlwaysAskForEncryption:  return 4;
  case Kleo::AskWheneverPossible:     return 5;
  }
}

static QStringList preferencesStrings() {
  return QStringList() << i18n("<placeholder>none</placeholder>")
                       << i18n("Never Encrypt with This Key")
                       << i18n("Always Encrypt with This Key")
                       << i18n("Encrypt Whenever Encryption is Possible")
                       << i18n("Always Ask")
                       << i18n("Ask Whenever Encryption is Possible");
}


class Kleo::KeyApprovalDialog::Private {
public:
  Private() : selfRequester( 0 ), prefsChanged( false ) {}

  Kleo::KeyRequester * selfRequester;
  QStringList addresses;
  std::vector<Kleo::KeyRequester*> requesters;
  std::vector<QComboBox*> preferences;
  bool prefsChanged;
};

Kleo::KeyApprovalDialog::KeyApprovalDialog( const std::vector<Item> & recipients,
                                            const std::vector<GpgME::Key> & sender,
                                            QWidget * parent )
  : KDialog( parent ),
    d( new Private() )
{
  setCaption( i18n("Encryption Key Approval") );
  setButtons(  Ok|Cancel );
  setDefaultButton(  Ok );
  assert( !recipients.empty() );


  QFrame *page = new QFrame( this );
  setMainWidget( page );
  QVBoxLayout * vlay = new QVBoxLayout( page );
  vlay->setMargin( 0 );
  vlay->setSpacing( spacingHint() );

  vlay->addWidget( new QLabel( i18n("The following keys will be used for encryption:"), page ) );

  QScrollArea * sv = new QScrollArea( page );
  //Laurent not sure
  sv->setWidgetResizable(true);
  //sv->setResizePolicy( Q3ScrollView::AutoOneFit );
  vlay->addWidget( sv );

  QWidget * view = new QWidget( sv->viewport() );

  QGridLayout * glay = new QGridLayout( view );
  glay->setMargin( marginHint() );
  glay->setSpacing( spacingHint() );
  glay->setColumnStretch( 1, 1 );
  sv->setWidget( view );

  int row = -1;

  if ( !sender.empty() ) {
    ++row;
    glay->addWidget( new QLabel( i18n("Your keys:"), view ), row, 0 );
    d->selfRequester = new EncryptionKeyRequester( true, EncryptionKeyRequester::AllProtocols, view );
    d->selfRequester->setKeys( sender );
    glay->addWidget( d->selfRequester, row, 1 );
    ++row;
    glay->addWidget( new KSeparator( Qt::Horizontal, view ), row, 0, 1, 2 );
  }

  const QStringList prefs = preferencesStrings();

  for ( std::vector<Item>::const_iterator it = recipients.begin() ; it != recipients.end() ; ++it ) {
    ++row;
    glay->addWidget( new QLabel( i18n("Recipient:"), view ), row, 0 );
    glay->addWidget( new QLabel( it->address, view ), row, 1 );
    d->addresses.push_back( it->address );

    ++row;
    glay->addWidget( new QLabel( i18n("Encryption keys:"), view ), row, 0 );
    KeyRequester * req = new EncryptionKeyRequester( true, EncryptionKeyRequester::AllProtocols, view );
    req->setKeys( it->keys );
    glay->addWidget( req, row, 1 );
    d->requesters.push_back( req );

    ++row;
    glay->addWidget( new QLabel( i18n("Encryption preference:"), view ), row, 0 );
    QComboBox * cb = new QComboBox( view );
    cb->setEditable( false );
    cb->addItems( prefs );
    glay->addWidget( cb, row, 1 );
    cb->setCurrentIndex( pref2cb( it->pref ) );
    connect( cb, SIGNAL(activated(int)), SLOT(slotPrefsChanged()) );
    d->preferences.push_back( cb );
  }

  QSize size = sizeHint();

  // don't make the dialog too large
  const QRect desk = QApplication::desktop()->screenGeometry( this );
  setInitialSize( QSize( qMin( size.width(), 3 * desk.width() / 4 ),
                         qMin( size.height(), 7 * desk.height() / 8 ) ) );
}

Kleo::KeyApprovalDialog::~KeyApprovalDialog() {
  delete d;
}

std::vector<GpgME::Key> Kleo::KeyApprovalDialog::senderKeys() const {
  return d->selfRequester ? d->selfRequester->keys() : std::vector<GpgME::Key>() ;
}

std::vector<Kleo::KeyApprovalDialog::Item> Kleo::KeyApprovalDialog::items() const {
  assert( d->requesters.size() == static_cast<unsigned int>( d->addresses.size() ) );
  assert( d->requesters.size() == d->preferences.size() );

  std::vector<Item> result;
  result.reserve( d->requesters.size() );
  QStringList::const_iterator ait = d->addresses.constBegin();
  std::vector<KeyRequester*>::iterator rit = d->requesters.begin();
  std::vector<QComboBox*>::iterator cit = d->preferences.begin();
  while ( ait != d->addresses.constEnd() )
    result.push_back( Item( *ait++, (*rit++)->keys(), cb2pref( (*cit++)->currentIndex() ) ) );
  return result;
}

bool Kleo::KeyApprovalDialog::preferencesChanged() const {
  return d->prefsChanged;
}

void Kleo::KeyApprovalDialog::slotPrefsChanged() {
  d->prefsChanged = true;
}

