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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QLayout>
//Added by qt3to4:
#include <QFrame>
#include <QGridLayout>

#include <kapplication.h>
#include <kdialog.h>
#include <klibloader.h>
#include <kservicetypetrader.h>

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
  kWarning( !kapp, 7520 ) <<"No QApplication object available!";

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
  kDebug(5720) <<"ContactEditorWidgetManager::reload()";
  const KService::List plugins = KServiceTypeTrader::self()->query( "KAddressBook/ContactEditorWidget",
    QString( "[X-KDE-KAddressBook-CEWPluginVersion] == %1" ).arg( KAB_CEW_PLUGIN_VERSION ) );

  KService::List::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    KLibFactory *factory = KLibLoader::self()->factory( (*it)->library().toLatin1() );
    if ( !factory ) {
      kDebug(5720) <<"ContactEditorWidgetManager::reload(): Factory creation failed";
      continue;
    }

    KAB::ContactEditorWidgetFactory *pageFactory =
                          static_cast<KAB::ContactEditorWidgetFactory*>( factory );

    if ( !pageFactory ) {
      kDebug(5720) <<"ContactEditorWidgetManager::reload(): Cast failed";
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

ContactEditorTabPage::ContactEditorTabPage( QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QGridLayout( this );
  mLayout->setSpacing( KDialog::spacingHint() );
  mLayout->setMargin( KDialog::marginHint() );
}

void ContactEditorTabPage::addWidget( KAB::ContactEditorWidget *widget )
{
  if(!widget)
	  return;

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
  KAB::ContactEditorWidget::List::ConstIterator it, last;
  int row = 0;
  if( mWidgets.isEmpty() )
    return;
  last = mWidgets.end();
  --last;
  for ( it = mWidgets.begin(); it != mWidgets.end() ; ++it ) {
    if ( (*it)->logicalWidth() == 2 ) {
      mLayout->addWidget( *it, row, 0, (*it)->logicalHeight(), 2 );
      row += (*it)->logicalHeight();
      if ( it != mWidgets.end() ) {
        QFrame *frame = new QFrame( this );
        frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        mLayout->addWidget( frame, row, 0, 1, 2 );
        row++;
      }
      continue;
    }

    // fill left side
    int leftHeight = (*it)->logicalHeight();

    if ( it == last ) { // last widget gets full width
      mLayout->addWidget( *it, row, 0, leftHeight, 2 );
      return;
    } else {
      mLayout->addWidget( *it, row, 0, leftHeight, 1);
      QFrame *frame = new QFrame( this );
      frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
      mLayout->addWidget( frame, row + leftHeight, 0, 1, 2 );
    }

    // fill right side
    for ( int i = 0; i < leftHeight; ++i ) {
      ++it;
      if ( it == last )
        break;

      int rightHeight = (*it)->logicalHeight();
      if ( rightHeight + i <= leftHeight )
        mLayout->addWidget( *it, row + i, 1, rightHeight, 1);
      else {
        --it;
        break;
      }
    }

    row += 2;
  }
}

#include "contacteditorwidgetmanager.moc"
