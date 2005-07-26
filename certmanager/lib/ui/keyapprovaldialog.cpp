/*  -*- c++ -*-
    keyapprovaldialog.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�lvdalens Datakonsult AB

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "keyapprovaldialog.h"

#include "keyrequester.h"

#include <cryptplugfactory.h>
#include <kleo/cryptobackend.h>

#include <klocale.h>
#include <kglobalsettings.h>
#include <kseparator.h>

#include <qstringlist.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qscrollview.h>
#include <qpushbutton.h>

#include <gpgmepp/key.h>

#include <assert.h>

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
  return QStringList() << i18n("<none>")
		       << i18n("Never Encrypt with This Key")
		       << i18n("Always Encrypt with This Key")
		       << i18n("Encrypt Whenever Encryption is Possible")
		       << i18n("Always Ask")
		       << i18n("Ask Whenever Encryption is Possible");
}


struct Kleo::KeyApprovalDialog::Private {
  Private() : selfRequester( 0 ), prefsChanged( false ) {}

  Kleo::KeyRequester * selfRequester;
  QStringList addresses;
  std::vector<Kleo::KeyRequester*> requesters;
  std::vector<QComboBox*> preferences;
  bool prefsChanged;
};

Kleo::KeyApprovalDialog::KeyApprovalDialog( const std::vector<Item> & recipients,
                                      const std::vector<GpgME::Key> & sender,
                                      QWidget * parent, const char * name,
                                      bool modal )
  : KDialogBase( parent, name, modal, i18n("Encryption Key Approval"), Ok|Cancel, Ok ),
    d( 0 )
{
  assert( !recipients.empty() );

  d = new Private();

  QFrame *page = makeMainWidget();
  QVBoxLayout * vlay = new QVBoxLayout( page, 0, spacingHint() );

  vlay->addWidget( new QLabel( i18n("The following keys will be used for encryption:"), page ) );

  QScrollView * sv = new QScrollView( page );
  sv->setResizePolicy( QScrollView::AutoOneFit );
  vlay->addWidget( sv );

  QWidget * view = new QWidget( sv->viewport() );

  QGridLayout * glay = new QGridLayout( view, 3, 2, marginHint(), spacingHint() );
  glay->setColStretch( 1, 1 );
  sv->addChild( view );

  int row = -1;

  if ( !sender.empty() ) {
    ++row;
    glay->addWidget( new QLabel( i18n("Your keys:"), view ), row, 0 );
    d->selfRequester = new EncryptionKeyRequester( true, EncryptionKeyRequester::AllProtocols, view );
    d->selfRequester->setKeys( sender );
    glay->addWidget( d->selfRequester, row, 1 );
    ++row;
    glay->addMultiCellWidget( new KSeparator( Horizontal, view ), row, row, 0, 1 );
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
    QComboBox * cb = new QComboBox( false, view );
    cb->insertStringList( prefs );
    glay->addWidget( cb, row, 1 );
    cb->setCurrentItem( pref2cb( it->pref ) );
    connect( cb, SIGNAL(activated(int)), SLOT(slotPrefsChanged()) );
    d->preferences.push_back( cb );
  }

  // calculate the optimal width for the dialog
  const int dialogWidth = marginHint()
                  + sv->frameWidth()
                  + view->sizeHint().width()
                  + sv->verticalScrollBar()->sizeHint().width()
                  + sv->frameWidth()
                  + marginHint()
                  + 2;
  // calculate the optimal height for the dialog
  const int dialogHeight = marginHint()
                   + fontMetrics().height()
                   + spacingHint()
                   + sv->frameWidth()
                   + view->sizeHint().height()
                   + sv->horizontalScrollBar()->sizeHint().height()
                   + sv->frameWidth()
                   + spacingHint()
                   + actionButton( KDialogBase::Cancel )->sizeHint().height()
                   + marginHint()
                   + 2;

  // don't make the dialog too large
  const QRect desk = KGlobalSettings::desktopGeometry( this );
  setInitialSize( QSize( kMin( dialogWidth, 3 * desk.width() / 4 ),
			 kMin( dialogHeight, 7 * desk.height() / 8 ) ) );
}

Kleo::KeyApprovalDialog::~KeyApprovalDialog() {
  delete d; d = 0;
}

std::vector<GpgME::Key> Kleo::KeyApprovalDialog::senderKeys() const {
  return d->selfRequester ? d->selfRequester->keys() : std::vector<GpgME::Key>() ;
}

std::vector<Kleo::KeyApprovalDialog::Item> Kleo::KeyApprovalDialog::items() const {
  assert( d->requesters.size() == d->addresses.size() );
  assert( d->requesters.size() == d->preferences.size() );

  std::vector<Item> result;
  result.reserve( d->requesters.size() );
  QStringList::const_iterator ait = d->addresses.begin();
  std::vector<KeyRequester*>::const_iterator rit = d->requesters.begin();
  std::vector<QComboBox*>::const_iterator cit = d->preferences.begin();
  while ( ait != d->addresses.end() )
    result.push_back( Item( *ait++, (*rit++)->keys(), cb2pref( (*cit++)->currentItem() ) ) );
  return result;
}

bool Kleo::KeyApprovalDialog::preferencesChanged() const {
  return d->prefsChanged;
}

void Kleo::KeyApprovalDialog::slotPrefsChanged() {
  d->prefsChanged = true;
}

#include "keyapprovaldialog.moc"
