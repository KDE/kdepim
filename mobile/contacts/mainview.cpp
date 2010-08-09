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

  KAction *action = new KAction( i18n( "New Contact" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( newContact() ) );
  actionCollection()->addAction( "kab_mobile_new_contact", action );

  action = new KAction( i18n( "Edit Contact" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( editContact() ) );
  actionCollection()->addAction( "kab_mobile_edit_contact", action );

  action = new KAction( i18n( "New Contact Group" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( newContactGroup() ) );
  actionCollection()->addAction( "kab_mobile_new_contactgroup", action );

  action = new KAction( i18n( "Edit Contact Group" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( editContactGroup() ) );
  actionCollection()->addAction( "kab_mobile_edit_contactgroup", action );

  action = new KAction( i18n( "New Address Book" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( launchAccountWizard() ) );
  actionCollection()->addAction( "kab_mobile_new_addressbook", action );
}

void MainView::newContact()
{
  ContactEditorView *editor = new ContactEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->show();
}

void MainView::editContact()
{
  if ( !itemSelectionModel() )
    return;

  const QModelIndexList indexes = itemSelectionModel()->selectedIndexes();
  if ( indexes.isEmpty() )
    return;

  const QModelIndex index = indexes.first();
  if ( !index.isValid() )
    return;

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  editContact( item );
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

void MainView::editContactGroup()
{
  if ( !itemSelectionModel() )
    return;

  const QModelIndexList indexes = itemSelectionModel()->selectedIndexes();
  if ( indexes.isEmpty() )
    return;

  const QModelIndex index = indexes.first();
  if ( !index.isValid() )
    return;

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  editContactGroup( item );
}

void MainView::editContactGroup( const Akonadi::Item &item )
{
  ContactGroupEditorView *editor = new ContactGroupEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->loadContactGroup( item );
  editor->show();
}

#include "mainview.moc"
