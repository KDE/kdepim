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
#include "contactlistproxy.h"

#include <QtDeclarative/QDeclarativeEngine>

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <Akonadi/ItemFetchScope>

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "kaddressbook-mobile", new ContactListProxy, parent )
{
  addMimeType( KABC::Addressee::mimeType() );
  addMimeType( KABC::ContactGroup::mimeType() );
  itemFetchScope().fetchFullPayload();
}

void MainView::newContact()
{
  ContactEditorView *editor = new ContactEditorView;
  editor->show();
}

void MainView::editContact( const Akonadi::Item &item )
{
  ContactEditorView *editor = new ContactEditorView;
  editor->loadContact( item );
  editor->show();
}

#include "mainview.moc"
