/*
    This file is part of KAddressBook.

    Copyright (c) 2009 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kaddressbookpart.h"
#include "aboutdata.h"
#include "mainwidget.h"

#include <QtGui/QVBoxLayout>

#include <kcomponentdata.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/genericfactory.h>
#include <kparts/statusbarextension.h>

K_PLUGIN_FACTORY( KAddressBookFactory, registerPlugin<KAddressBookPart>(); )
K_EXPORT_PLUGIN( KAddressBookFactory( AboutData() ) )

KAddressBookPart::KAddressBookPart( QWidget *parentWidget, QObject *parent,
                                    const QVariantList & )
  : KParts::ReadOnlyPart( parent )
{
  setComponentData( KAddressBookFactory::componentData() );

  KIconLoader::global()->addAppDir( "kaddressbook" );
  setXMLFile( "kaddressbookui.rc" );
  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget );
  canvas->setFocusPolicy( Qt::ClickFocus );
  setWidget( canvas );
  QVBoxLayout *topLayout = new QVBoxLayout( canvas );

  mMainWidget = new MainWidget( this, canvas );

  topLayout->addWidget( mMainWidget );
  topLayout->setMargin(0);
}

void KAddressBookPart::newContact()
{
  mMainWidget->newContact();
}

void KAddressBookPart::newGroup()
{
  mMainWidget->newGroup();
}

KAddressBookPart::~KAddressBookPart()
{
}

bool KAddressBookPart::openFile()
{
  return false;
}

void KAddressBookPart::guiActivateEvent(KParts::GUIActivateEvent *e)
{
   kDebug();
   KParts::ReadOnlyPart::guiActivateEvent(e);
}
