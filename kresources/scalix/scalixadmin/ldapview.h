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
#ifndef LDAPVIEW_H
#define LDAPVIEW_H

#include <QAbstractListModel>
#include <QListView>

namespace KLDAP {
class LdapObject;
}

namespace KPIM {
class LdapClient;
}

class LdapModel : public QAbstractListModel
{
  Q_OBJECT

  public:
    LdapModel( QObject *parent = 0 );
    ~LdapModel();

    virtual int rowCount( const QModelIndex &parent ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  public Q_SLOTS:
    void setQuery( const QString &query );

  private Q_SLOTS:
    void entryAdded( const KPIM::LdapClient&, const KLDAP::LdapObject& );
    void error( const QString& );

  private:
    KPIM::LdapClient *mClient;
    QList< QPair<QString, QString> > mMap;
};

class LdapView : public QListView
{
  Q_OBJECT

  public:
    LdapView( QWidget *parent = 0 );
    ~LdapView();

    QString selectedUser() const;

  public Q_SLOTS:
    void setQuery( const QString &query );

  private:
    LdapModel *mModel;
};

#endif
