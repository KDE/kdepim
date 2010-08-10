/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mainview.h"
#include "contacteditorview.h"
#include "contactgroupeditorview.h"
#include "contactlistproxy.h"
#include "standardcontactactionmanager.h"

#include <QtDeclarative/QDeclarativeEngine>

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <Akonadi/ItemFetchScope>

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "kaddressbook-mobile", new ContactListProxy, parent )
{
}

void MainView::delayedInit()
{
  KDeclarativeMainView::delayedInit();

  addMimeType( KABC::Addressee::mimeType() );
  addMimeType( KABC::ContactGroup::mimeType() );
  itemFetchScope().fetchFullPayload();
}

void MainView::newContact()
{
  ContactEditorView *editor = new ContactEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->show();
}

void MainView::editItem( const Akonadi::Item &item )
{
  if ( !item.isValid() )
    return;

  if ( item.hasPayload<KABC::Addressee>() )
    editContact( item );
  else if ( item.hasPayload<KABC::ContactGroup>() )
    editContactGroup( item );
}

void MainView::editContact( const Akonadi::Item &item )
{
  ContactEditorView *editor = new ContactEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->loadContact( item );
  editor->show();
}

void MainView::newContactGroup()
{
  ContactGroupEditorView *editor = new ContactGroupEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->show();
}

void MainView::editContactGroup( const Akonadi::Item &item )
{
  ContactGroupEditorView *editor = new ContactGroupEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->loadContactGroup( item );
  editor->show();
}

void MainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel )
{
  Akonadi::StandardContactActionManager *manager = new Akonadi::StandardContactActionManager( actionCollection(), this );
  manager->setCollectionSelectionModel( collectionSelectionModel );
  manager->setItemSelectionModel( itemSelectionModel );

  manager->createAllActions();

  connect( manager->action( Akonadi::StandardContactActionManager::CreateContact ), SIGNAL( triggered( bool ) ),
           this, SLOT( newContact() ) );
  connect( manager->action( Akonadi::StandardContactActionManager::CreateContactGroup ), SIGNAL( triggered( bool ) ),
           this, SLOT( newContactGroup() ) );
  connect( manager, SIGNAL( editItem( const Akonadi::Item& ) ),
           this, SLOT( editItem( const Akonadi::Item& ) ) );
}

#include "mainview.moc"
