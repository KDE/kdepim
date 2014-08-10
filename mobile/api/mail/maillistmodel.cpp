/*
 Copyright 2014  Michael Bohlender michael.bohlender@kdemail.net

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "maillistmodel.h"

#include <Akonadi/KMime/MessageStatus>

#include <KMime/Message>

#include <QUrl>

MailListModel::MailListModel( QObject *parent ) : QAbstractListModel( parent ), m_msgs()
{
    QHash<int, QByteArray> roles;

    roles[Subject] = "subject";
    roles[SenderList] = "senderList";
    roles[Date] = "date";
    roles[IsUnread] = "isUnread";
    roles[IsImportant] = "isImportant";
    roles[StatusIcon] = "statusIcon";
    roles[Url] = "url";

    setRoleNames( roles );
}

MailListModel::~MailListModel()
{

}

QVariant MailListModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() ) {
      return QVariant();
    }

    if  ( index.row() >= m_msgs.count() || index.row() < 0 ) {
      return QVariant();
    }

    Akonadi::Item item = m_msgs.at( index.row() );

    if ( item.isValid() && item.hasPayload<KMime::Message::Ptr>() ) {
        const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

        Akonadi::MessageStatus messageStatus;
        messageStatus.setStatusFromFlags( item.flags() );

        switch (role) {
        case Subject:
            return msg->subject()->asUnicodeString().trimmed();;
        case SenderList: {
            QStringList l;
            foreach ( const KMime::Types::Mailbox &mbox, msg->from()->mailboxes() ) {
              if ( mbox.hasName() ) {
                l.append( mbox.name() );
              } else {
                l.append( mbox.addrSpec().asPrettyString() );
              }
            }
            return l;
        }
        case IsImportant:
            return messageStatus.isImportant();
        case IsUnread:
            return !messageStatus.isRead();
        case StatusIcon: {
            if ( messageStatus.isReplied() ) {
                if ( messageStatus.isForwarded() ) {
                  return QLatin1String ( "mail-forwarded-replied" );
                } else {
                  return QLatin1String ( "mail-replied" );
                }
            }

            if ( messageStatus.isForwarded() ) {
              return QLatin1String ( "mail-forwarded" );
            }

            if ( messageStatus.isRead() ) {
              return QLatin1String ( "mail-read" );
            }

            return QLatin1String ( "mail-unread" );
        }
        case Date:
          return msg->date()->dateTime().dateTime();
        case Url:
            return item.url();
        }
    }
    return QVariant();
}

int MailListModel::rowCount( const QModelIndex& ) const
{
    return m_msgs.size();
}

bool MailListModel::addMails( const Akonadi::Item::List &items )
{
    beginInsertRows( QModelIndex(), rowCount(), rowCount() + items.size() -1 );

    foreach ( const Akonadi::Item &item, items ) {
        m_msgs.append( item );
    }

    endInsertRows();

    return true;
}

void MailListModel::clearMails()
{
    if ( !m_msgs.isEmpty() ) {
        beginResetModel();
        m_msgs.clear();
        endResetModel();
    }
}

