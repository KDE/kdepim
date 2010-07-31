/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kabc/ldapclient.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "settings.h"

#include "ldapview.h"

class LdapItem : public QListViewItem
{
  public:
    LdapItem( TQListView *parent, const TQString &text, const TQString &email )
      : TQListViewItem( parent )
    {
      setText( 0, text );
      setText( 1, email );
    }
};


LdapView::LdapView( TQWidget *parent )
  : KListView( parent )
{
  addColumn( i18n( "User" ) );
  setFullWidth( true );

  mClient = new KABC::LdapClient;

  mClient->setHost( Settings::self()->ldapHost() );
  mClient->setPort( Settings::self()->ldapPort() );
  mClient->setBase( Settings::self()->ldapBase() );
  mClient->setBindDN( Settings::self()->ldapBindDn() );
  mClient->setPwdBindDN( Settings::self()->ldapPassword() );

  TQStringList attrs;
  attrs << "surname" << "mail";
  mClient->setAttrs( attrs );

  connect( mClient, TQT_SIGNAL( result( const KABC::LdapObject& ) ),
           this, TQT_SLOT( entryAdded( const KABC::LdapObject& ) ) );
  connect( mClient, TQT_SIGNAL( error( const TQString& ) ),
           this, TQT_SLOT( error( const TQString& ) ) );
}

LdapView::~LdapView()
{
  mClient->cancelQuery();
  delete mClient;
}

TQString LdapView::selectedUser() const
{
  TQListViewItem *item = selectedItem();
  if ( !item )
    return TQString();
  else
    return item->text( 1 );
}

void LdapView::setQuery( const TQString &query )
{
  clear();
  mClient->startQuery( query );
}

void LdapView::entryAdded( const KABC::LdapObject &obj )
{
  const TQString text = TQString( "%1 (%2)" ).arg( obj.attrs[ "surname" ].first() )
                                           .arg( obj.attrs[ "mail" ].first() );

  new LdapItem( this, text, obj.attrs[ "mail" ].first() );
}

void LdapView::error( const TQString &msg )
{
  KMessageBox::error( this, msg );
}

#include "ldapview.moc"
