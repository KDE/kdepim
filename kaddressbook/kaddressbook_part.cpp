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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kparts/genericfactory.h>

#include "actionmanager.h"
#include "kaddressbook.h"
#include "kaddressbookiface.h"
#include "kaddressbooktableview.h"
#include "viewmanager.h"

#include "kaddressbook_part.h"

typedef KParts::GenericFactory< KAddressbookPart > KAddressbookFactory;
K_EXPORT_COMPONENT_FACTORY( libkaddressbookpart, KAddressbookFactory );

KAddressbookPart::KAddressbookPart( QWidget *parentWidget, const char *widgetName,
                                    QObject *parent, const char *name,
                                    const QStringList & )
  : KParts::ReadOnlyPart( parent, name ), DCOPObject( "KAddressBookIface" )
{
  kdDebug(5720) << "KAddressbookPart()" << endl;
  kdDebug(5720) << "  InstanceName: " << kapp->instanceName() << endl;

  setInstance( KAddressbookFactory::instance() );

  kdDebug(5720) << "KAddressbookPart()..." << endl;
  kdDebug(5720) << "  InstanceName: " << kapp->instanceName() << endl;

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget, widgetName );
  canvas->setFocusPolicy( QWidget::ClickFocus );
  setWidget( canvas );

  mExtension = new KAddressbookBrowserExtension( this );

  QVBoxLayout *topLayout = new QVBoxLayout( canvas );

  KGlobal::iconLoader()->addAppDir( "kaddressbook" );

  mWidget = new KAddressBook( canvas );
  mWidget->readConfig();
  topLayout->addWidget( mWidget );
  mWidget->viewManager()->setActiveExtension( 0 );

  mWidget->show();

  mActionManager = new ActionManager( this, mWidget, true, this );

  setXMLFile( "kaddressbook_part.rc" );
}

KAddressbookPart::~KAddressbookPart()
{
  mWidget->save();
  closeURL();
}

KAboutData *KAddressbookPart::createAboutData()
{
  KAboutData *about = new KAboutData( "kaddressbook", I18N_NOOP( "KAddressBook" ),
                                      "3.1", I18N_NOOP( "The KDE Address Book" ),
                                      KAboutData::License_BSD,
                                      I18N_NOOP( "(c) 1997-2003, The KDE PIM Team" ) );
  about->addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );
  about->addAuthor( "Don Sanders", I18N_NOOP( "Original author" ) );
  about->addAuthor( "Cornelius Schumacher",
                    I18N_NOOP( "Co-maintainer, libkabc port, CSV import/export" ),
                    "schumacher@kde.org" );
  about->addAuthor( "Mike Pilone", I18N_NOOP( "GUI and framework redesign" ),
                    "mpilone@slac.com" );
  about->addAuthor( "Greg Stern", I18N_NOOP( "DCOP interface" ) );
  about->addAuthor( "Mark Westcott", I18N_NOOP( "Contact pinning" ) );
  about->addAuthor( "Mischel Boyer de la Giroday", I18N_NOOP( "LDAP Lookup" ),
                    "michel@klaralvdalens-datakonsult.se" );
  about->addAuthor( "Steffen Hansen", I18N_NOOP( "LDAP Lookup" ),
                    "hansen@kde.org" );

  return about;
}

void KAddressbookPart::addEmail( QString addr )
{
  mWidget->addEmail( addr );
}

ASYNC KAddressbookPart::showContactEditor( QString uid )
{
  mWidget->showContactEditor( uid );
}

void KAddressbookPart::newContact()
{
  mWidget->newContact();
}

QString KAddressbookPart::getNameByPhone( QString phone )
{
  return mWidget->getNameByPhone( phone );
}

void KAddressbookPart::save()
{
  mWidget->save();
}

void KAddressbookPart::exit()
{
  delete this;
}

void KAddressbookPart::updateEditMenu()
{
}

bool KAddressbookPart::openFile()
{
  kdDebug(5720) << "KAddressbookPart:openFile()" << endl;

  mWidget->show();
  return true;
}

void KAddressbookPart::guiActivateEvent( KParts::GUIActivateEvent *e )
{
  kdDebug(5720) << "KAddressbookPart::guiActivateEvent" << endl;
  KParts::ReadOnlyPart::guiActivateEvent( e );

  mActionManager->initActionViewList();
}

KAddressbookBrowserExtension::KAddressbookBrowserExtension( KAddressbookPart *parent )
  : KParts::BrowserExtension( parent, "KAddressbookBrowserExtension" )
{
}

KAddressbookBrowserExtension::~KAddressbookBrowserExtension()
{
}

using namespace KParts;

#include "kaddressbook_part.moc"
