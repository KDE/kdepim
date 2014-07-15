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

#include "receivermodel.h"


ReceiverModel::ReceiverModel(QObject *parent) : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;

    roles[Name] = "name";
    roles[Email] = "email";
    roles[Type] = "type";

    setRoleNames (roles);

}


int ReceiverModel::rowCount( const QModelIndex &parent ) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mRecipients.count();
}


QVariant ReceiverModel::data( const QModelIndex &index, int role ) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= mRecipients.size() || index.row() < 0) {
        return QVariant();
    }

    int row = index.row();

    switch(role) {

        case Name:
            return mRecipients[row]->name();
        case Email:
            return mRecipients[row]->email();
        case Type:
            return mRecipients[row]->type();
    }

    return QVariant();
}


bool ReceiverModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if (index.isValid() && role == Qt::EditRole && !(index.row() >= mRecipients.size() || index.row() < 0)) {
        int row = index.row();

        switch(role) {

        case Name:
            mRecipients[row]->setName(value.toString());
            break;
        case Email:
            mRecipients[row]->setEmail(value.toString());
            break;
        case Type:
            mRecipients[row]->setType(MessageComposer::Recipient::idToType (value.toInt()));
            break;
        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}


Qt::ItemFlags ReceiverModel::flags( const QModelIndex &index ) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}


bool ReceiverModel::addRecipient( const MessageComposer::Recipient::Ptr &recipient )
{
    if ( mRecipients.contains( recipient ) ) {
        return false;
    }

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    mRecipients.append( recipient );
    endInsertRows();
    return true;

}


bool ReceiverModel::removeRecipient( const MessageComposer::Recipient::Ptr &recipient )
{
   int index = mRecipients.indexOf( recipient );

    if ( index < 0 ) {
        return false;
    }

    beginRemoveRows( QModelIndex(), rowCount(), rowCount() );
    mRecipients.removeAt( index );
    endRemoveRows();
    return true;

}

QStringList ReceiverModel::recipientStringList( MessageComposer::Recipient::Type type ) const
{
    QStringList selectedRecipients;
    foreach ( const MessageComposer::Recipient::Ptr &r, mRecipients ) {
        if ( r->type() == type ) {
            selectedRecipients << r->email();
        }
    }

    return selectedRecipients;
}

QString ReceiverModel::recipientString( MessageComposer::Recipient::Type type ) const
{
    return recipientStringList( type ).join( QLatin1String(", ") );
}

void ReceiverModel::setRecipientString( const QList< KMime::Types::Mailbox >& mailboxes, MessageComposer::Recipient::Type type )
{
    foreach ( const KMime::Types::Mailbox &mailbox, mailboxes ) {

        addRecipient( mailbox.prettyAddress( KMime::Types::Mailbox::QuoteWhenNecessary ), type );

    }

}

void ReceiverModel::addRecipient( const QString &email , MessageComposer::Recipient::Type type )
{
    MessageComposer::Recipient::Ptr rec (new MessageComposer::Recipient);
    rec->setEmail ( email );
    rec->setType ( MessageComposer::Recipient::idToType(type) );
    addRecipient ( rec );
}
