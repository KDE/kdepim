/*  -*- c++ -*-
    backendconfigwidget.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2002,2004,2005 Klar�vdalens Datakonsult AB
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

#include "backendconfigwidget.h"
#include "cryptoconfigdialog.h"

#include "kleo/cryptobackendfactory.h"
#include "ui/keylistview.h" // for lvi_cast<>

#include <k3listview.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <QPushButton>
#include <QLayout>
#include <q3header.h>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDBusMessage>
#include <QDBusConnection>
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

class Kleo::BackendListView : public K3ListView
{
public:
  BackendListView( BackendConfigWidget* parent )
    : K3ListView( parent ) {}

  /// return backend for currently selected (/current) item. Used by Configure button.
  const Kleo::CryptoBackend* currentBackend() const;

  /// return which protocol implementation was chosen (checked) for each type (used when saving)
  const Kleo::CryptoBackend* chosenBackend( const char * protocol );

  /// deselect all except one for a given protocol type (radiobutton-like exclusivity)
  void deselectAll( const char * protocol, Q3CheckListItem* except );

  void emitChanged() { static_cast<BackendConfigWidget *>( parentWidget() )->emitChanged( true ); }
};

// Toplevel listviewitem for a given backend (e.g. "GpgME", "Kgpg/gpg v2")
class Kleo::BackendListViewItem : public Q3ListViewItem
{
public:
  BackendListViewItem( K3ListView* lv, Q3ListViewItem *prev, const CryptoBackend *cryptoBackend )
    : Q3ListViewItem( lv, prev, cryptoBackend->displayName() ), mCryptoBackend( cryptoBackend )
    {}

  const CryptoBackend *cryptoBackend() const { return mCryptoBackend; }
  enum { RTTI = 0x2EAE3BE0, RTTI_MASK = 0xFFFFFFFF };
  int rtti() const { return RTTI; }

private:
  const CryptoBackend *mCryptoBackend;
};


// Checklist item under a BackendListViewItem
// (e.g. "GpgME supports protocol OpenPGP")
class Kleo::ProtocolCheckListItem : public Q3CheckListItem
{
public:
  ProtocolCheckListItem( BackendListViewItem* blvi,
                         Q3ListViewItem* prev, const char * protocolName,
                         const CryptoBackend::Protocol* protocol ) // can be 0
    : Q3CheckListItem( blvi, prev, itemText( protocolName, protocol ),
                      Q3CheckListItem::CheckBox ),
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
    Q3CheckListItem::stateChange( b );
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
    return i18nc( "Items in Kleo::BackendConfigWidget listview (1: protocol; 2: implementation name)",
                  "%1 (%2)", protoName, impName );
  }

  const CryptoBackend::Protocol* mProtocol; // can be 0
  const char * mProtocolName;
};

const Kleo::CryptoBackend* Kleo::BackendListView::currentBackend() const {
  const Q3ListViewItem* curItem = currentItem();
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
  for ( Q3ListViewItemIterator it( this /*, QListViewItemIterator::Checked doesn't work*/ ) ;
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

void Kleo::BackendListView::deselectAll( const char * protocolName, Q3CheckListItem* except )
{
  for ( Q3ListViewItemIterator it( this /*, QListViewItemIterator::Checked doesn't work*/ ) ;
        it.current() ; ++it ) {
    if ( it.current() == except ) continue;
    if ( ProtocolCheckListItem * p = lvi_cast<ProtocolCheckListItem>( it.current() ) )
      if ( p->isOn() && qstricmp( p->protocolName(), protocolName ) == 0 )
        p->setOn( false );
  }
}

////

Kleo::BackendConfigWidget::BackendConfigWidget( CryptoBackendFactory * factory, QWidget * parent, const char * name, Qt::WFlags f )
  : QWidget( parent, f ), d( 0 )
{
  setObjectName( name) ;
  assert( factory );
  d = new Private();
  d->backendFactory = factory;

  QHBoxLayout * hlay =
    new QHBoxLayout( this );
  hlay->setMargin( 0 );
  hlay->setSpacing( KDialog::spacingHint() );

  d->listView = new BackendListView( this );
  d->listView->setObjectName( "d->listView" );
  d->listView->addColumn( i18n("Available Backends") );
  d->listView->setAllColumnsShowFocus( true );
  d->listView->setSorting( -1 );
  d->listView->header()->setClickEnabled( false );
  d->listView->setFullWidth( true );

  hlay->addWidget( d->listView, 1 );

  connect( d->listView, SIGNAL(selectionChanged(Q3ListViewItem*)),
	   SLOT(slotSelectionChanged(Q3ListViewItem*)) );

  QVBoxLayout * vlay = new QVBoxLayout();
  hlay->addLayout(vlay);

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
  for ( unsigned int i = 0 ;; ++i ) {
    const CryptoBackend * b = d->backendFactory->backend( i );
    if ( !b )
      break;

    top = new Kleo::BackendListViewItem( d->listView, top, b );
    ProtocolCheckListItem *last = 0;
    for ( int i = 0 ;; ++i ) {
      const char * name = b->enumerateProtocols( i );
      if ( !name )
        break;

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

void Kleo::BackendConfigWidget::slotSelectionChanged( Q3ListViewItem * ) {
  const CryptoBackend* backend = d->listView->currentBackend();
  if ( backend && !backend->config() )
    kDebug(5150) <<"Backend w/o config object!";
  d->configureButton->setEnabled( backend && backend->config() );
}


void Kleo::BackendConfigWidget::slotRescanButtonClicked() {
  QStringList reasons;
  d->backendFactory->scanForBackends( &reasons );
  if ( !reasons.empty() )
    KMessageBox::informationList( this,
				  i18n("The following problems where encountered during scanning:"),
				  reasons, i18nc("@title:window Results of the scanning", "Scan Results") );
  load();
  emit changed( true );
}

void Kleo::BackendConfigWidget::slotConfigureButtonClicked() {
  const CryptoBackend* backend = d->listView->currentBackend();
  if ( backend && backend->config() ) {
    Kleo::CryptoConfigDialog dlg( backend->config(), this );
    int result = dlg.exec();
    if ( result == QDialog::Accepted ) {
      // Tell other users of gpgconf (e.g. the s/mime page) that the gpgconf data might have changed
      QDBusMessage message =
          QDBusMessage::createSignal("/", "org.kde.kleo.CryptoConfig", "changed");
      QDBusConnection::sessionBus().send(message);
      
      // and schedule a rescan, in case the updates make a backend valid
      QTimer::singleShot( 0, this, SLOT(slotRescanButtonClicked()) );
    }
  }
  else // shouldn't happen, button is disabled
    kWarning(5150) <<"Can't configure backend, no config object available";
}

void Kleo::BackendConfigWidget::save() const {
  for ( int i = 0 ;; ++i ) {
    const char * name = d->backendFactory->enumerateProtocols( i );
    if ( !name )
      break;
    d->backendFactory->setProtocolBackend( name,  d->listView->chosenBackend( name ) );
  }
}

void Kleo::BackendConfigWidget::virtual_hook( int, void* ) {}

#include "backendconfigwidget.moc"
