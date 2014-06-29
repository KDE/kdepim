/*
Copyright 2014 Abhijeet Nikam connect08nikam@gmail.com

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

#ifndef RECEIVERMODEL_H
#define RECEIVERMODEL_H

#include <QAbstractListModel>
#include <QStringList>

#include "recipient/recipient.h"

class ReceiverModel : public QAbstractListModel
{

    Q_OBJECT

public:
    explicit ReceiverModel(QObject *parent = 0);

    enum Roles {
        Name = Qt::UserRole + 1,
        Email,
        Type
    };

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    bool addRecipient( const MessageComposer::Recipient::Ptr &recipient );
    bool removeRecipient( const MessageComposer::Recipient::Ptr &recipient );
    QStringList recipientStringList( MessageComposer::Recipient::Type ) const;
    QString recipientString( MessageComposer::Recipient::Type type ) const;

private:

    MessageComposer::Recipient::List mRecipients;

};



#endif