/*  -*- c++ -*-
    backendconfigwidget.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2002,2004 Klarälvdalens Datakonsult AB
    Copyright (c) 2002,2003 Marc Mutz <mutz@kde.org>

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include "backendconfigwidget.h"
#include "cryptoconfigdialog.h"

#include "cryptplugfactory.h"
#include "cryptplugwrapperlist.h"
#include "cryptplugwrapper.h"

#include <klistview.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qheader.h>

#include <assert.h>

struct Kleo::BackendConfigWidget::Private {
  KListView * listView;
  QPushButton * configureButton;
  QPushButton * rescanButton;
  Kleo::CryptPlugFactory * backendFactory;
};

namespace Kleo {
  class BackendListViewItem;
  class ProtocolCheckListItem;
}

// Toplevel listviewitem for a given backend (e.g. "GpgME", "Kgpg/gpg v2")
class Kleo::BackendListViewItem : public QListViewItem
{
public:
  BackendListViewItem( KListView* lv, QListViewItem *prev, const CryptoBackend *cryptoBackend )
    : QListViewItem( lv, prev, cryptoBackend->displayName() ), mCryptoBackend( cryptoBackend )
    {}

  const CryptoBackend *cryptoBackend() const { return mCryptoBackend; }
  static const int RTTI = 20001;
  virtual int rtti() const { return RTTI; }
private:
  const CryptoBackend *mCryptoBackend;
};


// Checklist item under a BackendListViewItem
// (e.g. "GpgME supports protocol OpenPGP")
class Kleo::ProtocolCheckListItem : public QCheckListItem
{
public:
  enum ProtocolType { OpenPGP, SMIME };
  ProtocolCheckListItem( BackendListViewItem* blvi,
                         QListViewItem* prev,
                         ProtocolType protocolType,
                         const CryptoBackend::Protocol* protocol ) // can be 0
    : QCheckListItem( blvi, prev, itemText( protocolType, protocol ),
                      QCheckListItem::CheckBox ),
      mProtocol( protocol )
    {}

  static const int RTTI = 20002;
  virtual int rtti() const { return RTTI; }

private:
  // Helper for the constructor.
  static QString itemText( ProtocolType protocolType, const CryptoBackend::Protocol* protocol ) {
    // First one is the generic name (OpenPGP, SMIME)
    QString protoTypeName = protocolType == OpenPGP ? i18n( "OpenPGP" ) : i18n( "S/MIME" );
    // second one is implementation name (gpg, gpgsm...)
    QString impName = protocol ? protocol->displayName() : i18n( "failed" );
    return QString( "%1 (%2)" ).arg( protoTypeName ).arg( impName );
  }

  const CryptoBackend::Protocol* mProtocol; // can be 0
};

static const Kleo::CryptoBackend* currentBackend( KListView* listView ) {
  QListViewItem* curItem = listView->currentItem();
  if ( !curItem ) // can't happen
    return 0;
  if ( curItem->rtti() == Kleo::ProtocolCheckListItem::RTTI )
    curItem = curItem->parent();
  if ( curItem && curItem->rtti() == Kleo::BackendListViewItem::RTTI )
    return static_cast<Kleo::BackendListViewItem *>( curItem )->cryptoBackend();
  return 0;
}

Kleo::BackendConfigWidget::BackendConfigWidget( CryptPlugFactory * factory, QWidget * parent, const char * name, WFlags f )
  : QWidget( parent, name, f ), d( 0 )
{
  assert( factory );
  d = new Private();
  d->backendFactory = factory;

  QHBoxLayout * hlay =
    new QHBoxLayout( this, 0, KDialog::spacingHint() );

  d->listView = new KListView( this, "d->listView" );
  d->listView->addColumn( i18n("Available Backends") );
  d->listView->setAllColumnsShowFocus( true );
  d->listView->setSorting( -1 );
  d->listView->header()->setClickEnabled( false );
  d->listView->setFullWidth( true );

  hlay->addWidget( d->listView, 1 );

  connect( d->listView, SIGNAL(selectionChanged(QListViewItem*)),
	   SLOT(slotSelectionChanged(QListViewItem*)) );

  QVBoxLayout * vlay = new QVBoxLayout( hlay ); // inherits spacing

  d->configureButton = new QPushButton( i18n("Confi&gure..."), this );
  d->configureButton->setAutoDefault( false );
  vlay->addWidget( d->configureButton );

  connect( d->configureButton, SIGNAL(clicked()),
	   SLOT(slotConfigureButtonClicked()) );

  d->rescanButton = new QPushButton( i18n("Rescan"), this );
  d->rescanButton->setAutoDefault( false );
  vlay->addWidget( d->rescanButton );

  connect( d->rescanButton, SIGNAL(clicked()),
	   SLOT(slotRescanButtonClicked()) );

  vlay->addStretch( 1 );
}

Kleo::BackendConfigWidget::~BackendConfigWidget() {
  delete d; d = 0;
}

void Kleo::BackendConfigWidget::load() {
  d->listView->clear();

  unsigned int backendCount = 0;

  // populate the plugin list:
  BackendListViewItem * top = 0;
  for ( unsigned int i = 0 ; const CryptoBackend * b = d->backendFactory->backend( i ) ; ++i ) {
    const CryptoBackend::Protocol * openpgp = b->openpgp();
    const CryptoBackend::Protocol * smime = b->smime();

    top = new Kleo::BackendListViewItem( d->listView, top, b );
    ProtocolCheckListItem * last = 0;
    if ( openpgp ) {
      last = new ProtocolCheckListItem( top, last, ProtocolCheckListItem::OpenPGP, openpgp );
      last->setOn( openpgp == d->backendFactory->openpgp() );
    } else if ( b->supportsOpenPGP() ) {
      last = new ProtocolCheckListItem( top, last, ProtocolCheckListItem::OpenPGP, 0 );
      last->setOn( false );
      last->setEnabled( false );
    }
    if ( smime ) {
      last = new ProtocolCheckListItem( top, last, ProtocolCheckListItem::SMIME, smime );
      last->setOn( smime == d->backendFactory->smime() );
    } else if ( b->supportsSMIME() ) {
      last = new ProtocolCheckListItem( top, last, ProtocolCheckListItem::SMIME, 0 );
      last->setOn( false );
      last->setEnabled( false );
    }
    top->setOpen( true );

    ++backendCount;
  }

  if ( backendCount ) {
    d->listView->setCurrentItem( d->listView->firstChild() );
    d->listView->setSelected( d->listView->firstChild(), true );
  }

  slotSelectionChanged( d->listView->firstChild() );
}

void Kleo::BackendConfigWidget::slotSelectionChanged( QListViewItem * ) {
  const CryptoBackend* backend = currentBackend( d->listView );
  d->configureButton->setEnabled( backend && backend->config() );
}


void Kleo::BackendConfigWidget::slotRescanButtonClicked() {
  QStringList reasons;
  d->backendFactory->scanForBackends( &reasons );
  if ( !reasons.empty() )
    KMessageBox::informationList( this,
				  i18n("The following problems where encountered during scanning:"),
				  reasons, i18n("Scan Results") );
  load();
}

void Kleo::BackendConfigWidget::slotConfigureButtonClicked() {
  const CryptoBackend* backend = currentBackend( d->listView );
  if ( backend && backend->config() ) {
    Kleo::CryptoConfigDialog dlg( backend->config() );
    dlg.exec();
  }
  else // shouldn't happen, button is disabled
    kdWarning(5150) << "Can't configure backend, no config object available" << endl;
}

void Kleo::BackendConfigWidget::save() const {
  qDebug( "Sorry, not implemented: Kleo::BackendConfigWidget::save()" );
}

void Kleo::BackendConfigWidget::virtual_hook( int, void* ) {}

#include "backendconfigwidget.moc"
