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

#include <ldapclient.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "settings.h"

#include "ldapview.h"

LdapModel::LdapModel( QObject *parent )
  : QAbstractListModel( parent ),
    mClient( new KPIM::LdapClient( 0, this ) )
{
  KLDAP::LdapServer server;

  server.setHost( Settings::self()->ldapHost() );
  server.setPort( Settings::self()->ldapPort().toInt() );
  server.setBaseDn( KLDAP::LdapDN( Settings::self()->ldapBase() ) );
  server.setBindDn( Settings::self()->ldapBindDn() );
  server.setPassword( Settings::self()->ldapPassword() );

  mClient->setServer( server );

  QStringList attrs;
  attrs << "surname" << "mail";
  mClient->setAttrs( attrs );

  connect( mClient, SIGNAL( result( const KPIM::LdapClient&, const KLDAP::LdapObject& ) ),
           this, SLOT( entryAdded( const KPIM::LdapClient&, const KLDAP::LdapObject& ) ) );
  connect( mClient, SIGNAL( error( const QString& ) ),
           this, SLOT( error( const QString& ) ) );
}

LdapModel::~LdapModel()
{
  mClient->cancelQuery();
  delete mClient;
}

int LdapModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return mMap.count();
  else
    return 0;
}

QVariant LdapModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= mMap.count() )
    return QVariant();

  if ( role == Qt::DisplayRole )
    return mMap.at( index.row() ).first;
  else if ( role == Qt::UserRole )
    return mMap.at( index.row() ).second;

  return QVariant();
}

QVariant LdapModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( section != 0 || orientation == Qt::Vertical || role != Qt::DisplayRole )
    return QVariant();

  return i18n( "User" );
}

void LdapModel::setQuery( const QString &query )
{
  mMap.clear();
  reset();
  mClient->startQuery( query );
}

void LdapModel::entryAdded( const KPIM::LdapClient&, const KLDAP::LdapObject &obj )
{
  const QString text = QString( "%1 (%2)" ).arg( QString::fromUtf8( obj.value( "surname" ) ) )
                                           .arg( QString::fromUtf8( obj.value( "mail" ) ) );

  beginInsertRows( QModelIndex(), mMap.count(), mMap.count() + 1 );
  mMap.append( QPair<QString, QString>( text, QString::fromUtf8( obj.value( "mail" ) ) ) );
  endInsertRows();
}

void LdapModel::error( const QString &msg )
{
  KMessageBox::error( qobject_cast<QWidget*>( QObject::parent() ), msg );
}


LdapView::LdapView( QWidget *parent )
  : QListView( parent )
{
  mModel = new LdapModel( this );

  setModel( mModel );
}

LdapView::~LdapView()
{
}

QString LdapView::selectedUser() const
{
  const QModelIndex index = selectionModel()->currentIndex();
  if ( !index.isValid() )
    return QString();

  return index.data( Qt::UserRole ).toString();
}

void LdapView::setQuery( const QString &query )
{
  mModel->setQuery( query );
}

#include "ldapview.moc"
