/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kvbox.h>

#include "mainwidget.h"

#include "part.h"

typedef KParts::GenericFactory< KitchenSyncPart > KitchenSyncFactory;
K_EXPORT_COMPONENT_FACTORY( libkitchensyncpart, KitchenSyncFactory )

KitchenSyncPart::KitchenSyncPart( QWidget *parentWidget, QObject *parent,
                                  const QStringList &list )
  : KParts::ReadOnlyPart( parent )
{
  Q_UNUSED( list );
  setComponentData( KitchenSyncFactory::componentData() );

  KVBox *canvas = new KVBox( parentWidget );
  setWidget( canvas );

  new MainWidget( this, canvas );

  KIconLoader::global()->addAppDir( "kitchensync" );

  setXMLFile( "kitchensync_part.rc" );
}

KitchenSyncPart::~KitchenSyncPart()
{
  closeUrl();
}

KAboutData *KitchenSyncPart::createAboutData()
{
  return MainWidget::aboutData();
}

void KitchenSyncPart::exit()
{
  delete this;
}

bool KitchenSyncPart::openUrl( const KUrl &url )
{
  emit setWindowCaption( url.prettyUrl() );

  return true;
}

bool KitchenSyncPart::openFile()
{
  return true;
}

void KitchenSyncPart::guiActivateEvent( KParts::GUIActivateEvent *e )
{
  KParts::ReadOnlyPart::guiActivateEvent( e );
}

#include "part.moc"

