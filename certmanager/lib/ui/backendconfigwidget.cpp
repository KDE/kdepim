/*  -*- c++ -*-
    backendconfigwidget.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2002,2004,2005 Klarälvdalens Datakonsult AB
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

#include "kleo/cryptobackendfactory.h"
#include "ui/keylistview.h" // for lvi_cast<>

#include <klistview.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <dcopclient.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qheader.h>
#include <qtimer.h>

#include <assert.h>

namespace Kleo {
  class BackendListView;
}

class Kleo::BackendConfigWidget::Private {
public:
  Kleo::BackendListView * listView;
  QPushButton * configureButton;
  QPushButton * rescanButton;
  Kleo::CryptoBackendFactory * backendFactory;
};

namespace Kleo {
  class BackendListViewItem;
  class ProtocolCheckListItem;
}

class Kleo::BackendListView : public KListView
{
public:
  BackendListView( BackendConfigWidget* parent, const char* name = 0 )
    : KListView( parent, name ) {}

  /// return backend for currently selected (/current) item. Used by Configure button.
  const Kleo::CryptoBackend* currentBackend() const;

  /// return which protocol implementation was chosen (checked) for each type (used when saving)
  const Kleo::CryptoBackend* chosenBackend( const char * protocol );

  /// deselect all except one for a given protocol type (radiobutton-like exclusivity)
  void deselectAll( const char * protocol, QCheckListItem* except );

  void emitChanged() { static_cast<BackendConfigWidget *>( parentWidget() )->emitChanged( true ); }
};

// Toplevel listviewitem for a given backend (e.g. "GpgME", "Kgpg/gpg v2")
class Kleo::BackendListViewItem : public QListViewItem
{
public:
  BackendListViewItem( KListView* lv, QListViewItem *prev, const CryptoBackend *cryptoBackend )
    : QListViewItem( lv, prev, cryptoBackend->displayName() ), mCryptoBackend( cryptoBackend )
    {}

  const CryptoBackend *cryptoBackend() const { return mCryptoBackend; }
  enum { RTTI = 0x2EAE3BE0, RTTI_MASK = 0xFFFFFFFF };
  int rtti() const { return RTTI; }

private:
  const CryptoBackend *mCryptoBackend;
};


// Checklist item under a BackendListViewItem
// (e.g. "GpgME supports protocol OpenPGP")
class Kleo::ProtocolCheckListItem : public QCheckListItem
{
public:
  ProtocolCheckListItem( BackendListViewItem* blvi,
                         QListViewItem* prev, const char * protocolName,
                         const CryptoBackend::Protocol* protocol ) // can be 0
    : QCheckListItem( blvi, prev, itemText( protocolName, protocol ),
                      QCheckListItem::CheckBox ),
      mProtocol( protocol ), mProtocolName( protocolName )
    {}

  enum { RTTI = 0x2EAE3BE1, RTTI_MASK = 0xFFFFFFFF };
  virtual int rtti() const { return RTTI; }

  // can be 0
  const CryptoBackend::Protocol* protocol() const { return mProtocol; }
  const char * protocolName() const { return mProtocolName; }

protected:
  virtual void stateChange( bool b ) {
    BackendListView* lv = static_cast<BackendListView *>( listView() );
    // "radio-button-like" behavior for the protocol checkboxes
    if ( b )
      lv->deselectAll( mProtocolName, this );
    lv->emitChanged();
    QCheckListItem::stateChange( b );
  }

private:
  // Helper for the constructor.
  static QString itemText( const char * protocolName, const CryptoBackend::Protocol* protocol ) {
    // First one is the generic name (find a nice one for OpenPGP, SMIME)
    const QString protoName = qstricmp( protocolName, "openpgp" ) != 0
                              ? qstricmp( protocolName, "smime" ) != 0
                              ? QString::fromLatin1( protocolName )
                              : i18n( "S/MIME" )
                              : i18n( "OpenPGP" );
    // second one is implementation name (gpg, gpgsm...)
    const QString impName = protocol ? protocol->displayName() : i18n( "failed" );
    return i18n( "Items in Kleo::BackendConfigWidget listview (1: protocol; 2: implementation name)",
                 "%1 (%2)" ).arg( protoName, impName );
  }

  const CryptoBackend::Protocol* mProtocol; // can be 0
  const char * mProtocolName;
};

const Kleo::CryptoBackend* Kleo::BackendListView::currentBackend() const {
  const QListViewItem* curItem = currentItem();
  if ( !curItem ) // can't happen
    return 0;
  if ( lvi_cast<Kleo::ProtocolCheckListItem>( curItem ) )
    curItem = curItem->parent();
  if ( const Kleo::BackendListViewItem * blvi = lvi_cast<Kleo::BackendListViewItem>( curItem ) )
    return blvi->cryptoBackend();
  return 0;
}

// can't be const method due to QListViewItemIterator (why?)
const Kleo::CryptoBackend* Kleo::BackendListView::chosenBackend( const char * protocolName )
{
  for ( QListViewItemIterator it( this /*, QListViewItemIterator::Checked doesn't work*/ ) ;
        it.current() ; ++it )
    if ( ProtocolCheckListItem * p = lvi_cast<ProtocolCheckListItem>( it.current() ) )
      if ( p->isOn() && qstricmp( p->protocolName(), protocolName ) == 0 ) {
        // OK that's the one. Now go up to the parent backend
        // (need to do that in the listview since Protocol doesn't know it)
        if ( const BackendListViewItem * parItem = lvi_cast<BackendListViewItem>( it.current()->parent() ) )
          return parItem->cryptoBackend();
      }
  return 0;
}

void Kleo::BackendListView::deselectAll( const char * protocolName, QCheckListItem* except )
{
  for ( QListViewItemIterator it( this /*, QListViewItemIterator::Checked doesn't work*/ ) ;
        it.current() ; ++it ) {
    if ( it.current() == except ) continue;
    if ( ProtocolCheckListItem * p = lvi_cast<ProtocolCheckListItem>( it.current() ) )
      if ( p->isOn() && qstricmp( p->protocolName(), protocolName ) == 0 )
        p->setOn( false );
  }
}

////

Kleo::BackendConfigWidget::BackendConfigWidget( CryptoBackendFactory * factory, QWidget * parent, const char * name, WFlags f )
  : QWidget( parent, name, f ), d( 0 )
{
  assert( factory );
  d = new Private();
  d->backendFactory = factory;

  QHBoxLayout * hlay =
    new QHBoxLayout( this, 0, KDialog::spacingHint() );

  d->listView = new BackendListView( this, "d->listView" );
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

    top = new Kleo::BackendListViewItem( d->listView, top, b );

    ProtocolCheckListItem * last = 0;
    for ( int i = 0 ; const char * name = b->enumerateProtocols( i ) ; ++i ) {
      const CryptoBackend::Protocol * protocol = b->protocol( name );

      if ( protocol ) {
        last = new ProtocolCheckListItem( top, last, name, protocol );
        last->setOn( protocol == d->backendFactory->protocol( name ) );
      } else if ( b->supportsProtocol( name ) ) {
        last = new ProtocolCheckListItem( top, last, name, 0 );
        last->setOn( false );
        last->setEnabled( false );
      }
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
  const CryptoBackend* backend = d->listView->currentBackend();
  if ( backend && !backend->config() )
    kdDebug(5150) << "Backend w/o config object!" << endl;
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
  emit changed( true );
}

void Kleo::BackendConfigWidget::slotConfigureButtonClicked() {
  const CryptoBackend* backend = d->listView->currentBackend();
  if ( backend && backend->config() ) {
    Kleo::CryptoConfigDialog dlg( backend->config() );
    int result = dlg.exec();
    if ( result == QDialog::Accepted ) {
      // Tell other users of gpgconf (e.g. the s/mime page) that the gpgconf data might have changed
      kapp->dcopClient()->emitDCOPSignal( "KPIM::CryptoConfig", "changed()", QByteArray() );
      // and schedule a rescan, in case the updates make a backend valid
      QTimer::singleShot( 0, this, SLOT(slotRescanButtonClicked()) );
    }
  }
  else // shouldn't happen, button is disabled
    kdWarning(5150) << "Can't configure backend, no config object available" << endl;
}

void Kleo::BackendConfigWidget::save() const {
  for ( int i = 0 ; const char * name = d->backendFactory->enumerateProtocols( i ) ; ++i )
    d->backendFactory->setProtocolBackend( name,  d->listView->chosenBackend( name ) );
}

void Kleo::BackendConfigWidget::virtual_hook( int, void* ) {}

#include "backendconfigwidget.moc"
