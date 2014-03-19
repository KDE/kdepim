/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net,
    Author: Tobias Koenig <tokoe@kdab.com>

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

#ifndef MAILCOMMON_SNIPPETSMODEL_P_H
#define MAILCOMMON_SNIPPETSMODEL_P_H

#include <QAbstractItemModel>

namespace MailCommon {

class SnippetItem;

class SnippetsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Role {
        IsGroupRole = Qt::UserRole + 1, ///< Returns whether the index represents a group
        NameRole,                       ///< The name of a snippet or group
        TextRole,                       ///< The text of a snippet
        KeySequenceRole                 ///< The key sequence to activate a snippet
    };

    explicit SnippetsModel( QObject *parent = 0 );
    ~SnippetsModel();

    virtual bool setData( const QModelIndex &index, const QVariant &value,
                          int role = Qt::EditRole );

    virtual QVariant data( const QModelIndex &index, int role ) const;

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual QModelIndex index( int row, int column,
                               const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QStringList mimeTypes() const;

    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;

    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                               int row, int column, const QModelIndex &parent );

    virtual Qt::DropActions supportedDropActions () const;

protected:
    virtual bool insertRows( int row, int count,
                             const QModelIndex &parent = QModelIndex() );

    virtual bool removeRows( int row, int count,
                             const QModelIndex &parent = QModelIndex() );

signals:
    void dndDone();
private:
    SnippetItem *mRootItem;
};

}

#endif
