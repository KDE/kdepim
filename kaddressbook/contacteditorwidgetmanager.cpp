/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>

#include <kapplication.h>
#include <kdialog.h>
#include <klibloader.h>
#include <ktrader.h>

// include non-plugin contact editor widgets
#include "customfieldswidget.h"
#include "freebusywidget.h"
#include "geowidget.h"
#include "imagewidget.h"
#include "soundwidget.h"

#include "contacteditorwidget.h"
#include "contacteditorwidgetmanager.h"

ContactEditorWidgetManager *ContactEditorWidgetManager::mSelf = 0;

ContactEditorWidgetManager::ContactEditorWidgetManager()
  : QObject( qApp )
{
  reload();
}

ContactEditorWidgetManager::~ContactEditorWidgetManager()
{
  mFactories.clear();
}

ContactEditorWidgetManager *ContactEditorWidgetManager::self()
{
  kdWarning( !kapp, 7520 ) << "No QApplication object available!" << endl;

  if ( !mSelf )
    mSelf = new ContactEditorWidgetManager();

  return mSelf;
}

int ContactEditorWidgetManager::count() const
{
  return mFactories.count();
}

KAB::ContactEditorWidgetFactory *ContactEditorWidgetManager::factory( int pos ) const
{
  return mFactories[ pos ];
}

void ContactEditorWidgetManager::reload()
{
  mFactories.clear();
  kdDebug(5720) << "ContactEditorWidgetManager::reload()" << endl;
  const KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/ContactEditorWidget", 
    QString( "[X-KDE-KAddressBook-CEWPluginVersion] == %1" ).arg( KAB_CEW_PLUGIN_VERSION ) );

  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    KLibFactory *factory = KLibLoader::self()->factory( (*it)->library().latin1() );
    if ( !factory ) {
      kdDebug(5720) << "ContactEditorWidgetManager::reload(): Factory creation failed" << endl;
      continue;
    }

    KAB::ContactEditorWidgetFactory *pageFactory =
                          static_cast<KAB::ContactEditorWidgetFactory*>( factory );

    if ( !pageFactory ) {
      kdDebug(5720) << "ContactEditorWidgetManager::reload(): Cast failed" << endl;
      continue;
    }

    mFactories.append( pageFactory );
  }

  // add all non-plugin contact editor factories
  mFactories.append( new FreeBusyWidgetFactory );
  mFactories.append( new ImageWidgetFactory );
  mFactories.append( new SoundWidgetFactory );
  mFactories.append( new GeoWidgetFactory );
  mFactories.append( new CustomFieldsWidgetFactory );
}

///////////////////////////////////////////////////////////////////////////////

ContactEditorTabPage::ContactEditorTabPage( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  mLayout = new QGridLayout( this, 0, 2, KDialog::marginHint(), 
                             KDialog::spacingHint() );
}

void ContactEditorTabPage::addWidget( KAB::ContactEditorWidget *widget )
{
  if ( widget->logicalWidth() == 2 ) {
    mWidgets.prepend( widget );
    connect( widget, SIGNAL( changed() ), SIGNAL( changed() ) );
    return;
  }

  // insert it in descending order
  KAB::ContactEditorWidget::List::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it ) {
    if ( widget->logicalHeight() > (*it)->logicalHeight() &&
         (*it)->logicalWidth() == 1 ) {
      --it;
      break;
    }
  }
  mWidgets.insert( ++it, widget );

  connect( widget, SIGNAL( changed() ), SIGNAL( changed() ) );
}

void ContactEditorTabPage::loadContact( KABC::Addressee *addr )
{
  KAB::ContactEditorWidget::List::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it ) {
    (*it)->setModified( false );
    (*it)->loadContact( addr );
  }
}

void ContactEditorTabPage::storeContact( KABC::Addressee *addr )
{
  KAB::ContactEditorWidget::List::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it ) {
    if ( (*it)->modified() ) {
      (*it)->storeContact( addr );
      (*it)->setModified( false );
    }
  }
}

void ContactEditorTabPage::setReadOnly( bool readOnly )
{
  KAB::ContactEditorWidget::List::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it )
    (*it)->setReadOnly( readOnly );
}

void ContactEditorTabPage::updateLayout()
{
  KAB::ContactEditorWidget::List::ConstIterator it;

  int row = 0;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it ) {
    if ( (*it)->logicalWidth() == 2 ) {
      mLayout->addMultiCellWidget( *it, row, row + (*it)->logicalHeight() - 1, 0, 1 );
      row += (*it)->logicalHeight();

      if ( it != mWidgets.fromLast() ) {
        QFrame *frame = new QFrame( this );
        frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        mLayout->addMultiCellWidget( frame, row, row, 0, 1 );
        row++;
      }
      continue;
    }

    // fill left side
    int leftHeight = (*it)->logicalHeight();

    if ( it == mWidgets.fromLast() ) { // last widget gets full width
      mLayout->addMultiCellWidget( *it, row, row + leftHeight - 1, 0, 1 );
      return;
    } else {
      mLayout->addMultiCellWidget( *it, row, row + leftHeight - 1, 0, 0 );
      QFrame *frame = new QFrame( this );
      frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
      mLayout->addMultiCellWidget( frame, row + leftHeight, row + leftHeight, 0, 1 );
    }

    // fill right side
    for ( int i = 0; i < leftHeight; ++i ) {
      ++it;
      if ( it == mWidgets.end() )
        break;

      int rightHeight = (*it)->logicalHeight();
      if ( rightHeight + i <= leftHeight )
        mLayout->addMultiCellWidget( *it, row + i, row + i + rightHeight - 1, 1, 1 );
      else {
        --i;
        break;
      }
    }

    row += 2;
  }
}

#include "contacteditorwidgetmanager.moc"
