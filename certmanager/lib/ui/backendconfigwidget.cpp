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
  QListViewItem * top = 0;
  for ( unsigned int i = 0 ; const CryptoBackend * b = d->backendFactory->backend( i ) ; ++i ) {
    const CryptoBackend::Protocol * openpgp = b->openpgp();
    const CryptoBackend::Protocol * smime = b->smime();

    top = new QListViewItem( d->listView, top, b->displayName() );
    QCheckListItem * last = 0;
    if ( openpgp ) {
      last = new QCheckListItem( top, last,
				 i18n("OpenPGP (%1)").arg( openpgp->displayName() ),
				 QCheckListItem::CheckBox );
      last->setOn( openpgp == d->backendFactory->openpgp() );
    } else if ( b->supportsOpenPGP() ) {
      last = new QCheckListItem( top, last, i18n("OpenPGP (failed)"),
				 QCheckListItem::CheckBox );
      last->setOn( false );
      last->setEnabled( false );
    }
    if ( smime ) {
      last = new QCheckListItem( top, last,
				 i18n("S/MIME (%1)").arg( smime->displayName() ),
				 QCheckListItem::CheckBox );
      last->setOn( smime == d->backendFactory->smime() );
    } else if ( b->supportsSMIME() ) {
      last = new QCheckListItem( top, last, i18n("S/MIME (failed)"),
				 QCheckListItem::CheckBox );
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

void Kleo::BackendConfigWidget::slotSelectionChanged( QListViewItem * item ) {
  d->configureButton->setEnabled( item );
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
  qDebug( "Sorry, not implemented: Kleo::BackendConfigWidget::slotConfigureButtonClicked()" );
}

void Kleo::BackendConfigWidget::save() const {
  qDebug( "Sorry, not implemented: Kleo::BackendConfigWidget::save()" );
}

void Kleo::BackendConfigWidget::virtual_hook( int, void* ) {}

#include "backendconfigwidget.moc"
