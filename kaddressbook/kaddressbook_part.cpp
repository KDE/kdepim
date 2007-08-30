/*
    This file is part of KAddressbook.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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

#include <qlayout.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kparts/genericfactory.h>
#include <kparts/statusbarextension.h>
#include <kstatusbar.h>

#include "kabcore.h"
#include "kabprefs.h"
#include "kaddressbookiface.h"

#include "kaddressbook_part.h"

typedef KParts::GenericFactory< KAddressbookPart > KAddressbookFactory;
K_EXPORT_COMPONENT_FACTORY( libkaddressbookpart, KAddressbookFactory )

KAddressbookPart::KAddressbookPart( QWidget *parentWidget, const char *widgetName,
                                    QObject *parent, const char *name,
                                    const QStringList & )
  : DCOPObject( "KAddressBookIface" ), KParts::ReadOnlyPart( parent, name )
{
  setInstance( KAddressbookFactory::instance() );

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget, widgetName );
  canvas->setFocusPolicy( QWidget::ClickFocus );
  setWidget( canvas );

  QVBoxLayout *topLayout = new QVBoxLayout( canvas );

  KGlobal::iconLoader()->addAppDir( "kaddressbook" );

  mCore = new KABCore( this, true, canvas );
  mCore->restoreSettings();
  topLayout->addWidget( mCore->widget() );

  KParts::StatusBarExtension *statusBar = new KParts::StatusBarExtension( this );
  mCore->setStatusBar( statusBar->statusBar() );

  setXMLFile( "kaddressbook_part.rc" );
}

KAddressbookPart::~KAddressbookPart()
{
  mCore->save();
  mCore->saveSettings();

  KABPrefs::instance()->writeConfig();
  closeURL();
}

KAboutData *KAddressbookPart::createAboutData()
{
  return KABCore::createAboutData();
}

void KAddressbookPart::addEmail( QString addr )
{
  mCore->addEmail( addr );
}

void KAddressbookPart::importVCard( const QString& vCardURL )
{
  mCore->importVCard( vCardURL );
}

ASYNC KAddressbookPart::showContactEditor( QString uid )
{
  mCore->editContact( uid );
}

void KAddressbookPart::newContact()
{
  mCore->newContact();
}


void KAddressbookPart::newDistributionList()
{
  mCore->newDistributionList();
}

QString KAddressbookPart::getNameByPhone( QString phone )
{
  return mCore->getNameByPhone( phone );
}

void KAddressbookPart::save()
{
  mCore->save();
}

void KAddressbookPart::exit()
{
  mCore->queryClose();

  delete this;
}

bool KAddressbookPart::openURL( const KURL &url )
{
  kdDebug(5720) << "KAddressbookPart:openFile()" << endl;

  mCore->widget()->show();

  if ( !url.isEmpty() )
    mCore->importVCard( url );

  emit setWindowCaption( url.prettyURL() );

  return true;
}

bool KAddressbookPart::openFile()
{
  return false;
}

bool KAddressbookPart::handleCommandLine()
{
  return mCore->handleCommandLine( this );
}

void KAddressbookPart::guiActivateEvent( KParts::GUIActivateEvent *e )
{
  kdDebug(5720) << "KAddressbookPart::guiActivateEvent" << endl;
  KParts::ReadOnlyPart::guiActivateEvent( e );

  if ( e->activated() )
    mCore->reinitXMLGUI();

  if ( !e->activated() ) {
    mCore->statusBar()->removeItem( 1 );
    mCore->statusBar()->removeItem( 2 );
  }
}

void KAddressbookPart::loadProfile( const QString& )
{
}

void KAddressbookPart::saveToProfile( const QString& ) const
{
}

#include "kaddressbook_part.moc"
