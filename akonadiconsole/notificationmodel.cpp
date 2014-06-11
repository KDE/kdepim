/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "notificationmodel.h"
#include "notificationmanagerinterface.h"

#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusConnection>

#include <KDebug>
#include <KLocale>


#include <AkonadiCore/ServerManager>

#include <akonadi/private/imapparser_p.h>
#include <boost/concept_check.hpp>
#include <akonadi/private/notificationmessagev3_p.h>

using namespace Akonadi;

class NotificationModel::Item {
  public:
    Item(int type_): type(type_) {}
    virtual ~Item() {}

    int type;
};

class NotificationModel::NotificationBlock: public NotificationModel::Item {
  public:
    NotificationBlock( const Akonadi::NotificationMessageV3::List &msgs );
    ~NotificationBlock();

    QList<NotificationNode*> nodes;
    QDateTime timestamp;
};

class NotificationModel::NotificationNode: public NotificationModel::Item {
  public:
    NotificationNode( const NotificationMessageV3 &msg_, NotificationBlock *parent_ );
    ~NotificationNode();

    QByteArray sessionId;
    NotificationMessageV2::Type type;
    NotificationMessageV2::Operation operation;
    QByteArray resource;
    QByteArray destResource;
    NotificationMessageV2::Id parentCollection;
    NotificationMessageV2::Id destCollection;
    QSet<QByteArray> parts;
    QSet<QByteArray> addedFlags;
    QSet<QByteArray> removedFlags;
    QList<NotificationEntity*> entities;
    NotificationBlock *parent;
};

class NotificationModel::NotificationEntity: public NotificationModel::Item {
  public:
    NotificationEntity( const NotificationMessageV2::Entity &entity, NotificationNode *parent_ );

    NotificationMessageV2::Id id;
    QString remoteId;
    QString remoteRevision;
    QString mimeType;
    NotificationNode *parent;
};


NotificationModel::NotificationBlock::NotificationBlock( const NotificationMessageV3::List &msgs ) :
  Item( 0 )
{
  timestamp = QDateTime::currentDateTime();
  Q_FOREACH ( const NotificationMessageV3 &msg, msgs ) {
    nodes << new NotificationNode( msg, this );
  }
}

NotificationModel::NotificationBlock::~NotificationBlock()
{
  qDeleteAll( nodes );
}


NotificationModel::NotificationNode::NotificationNode( const NotificationMessageV3& msg, NotificationModel::NotificationBlock* parent_ ):
  Item( 1 ),
  sessionId( msg.sessionId() ),
  type( msg.type() ),
  operation( msg.operation() ),
  resource( msg.resource() ),
  destResource( msg.destinationResource() ),
  parentCollection( msg.parentCollection() ),
  destCollection( msg.parentDestCollection() ),
  parts( msg.itemParts() ),
  addedFlags( msg.addedFlags() ),
  removedFlags( msg.removedFlags() ),
  parent( parent_ )
{
  Q_FOREACH ( const NotificationMessageV2::Entity &entity, msg.entities()) {
    entities << new NotificationEntity( entity, this );
  }
}

NotificationModel::NotificationNode::~NotificationNode()
{
  qDeleteAll( entities );
}


NotificationModel::NotificationEntity::NotificationEntity( const NotificationMessageV2::Entity &entity, NotificationModel::NotificationNode* parent_ ):
  Item( 2 ),
  id( entity.id ),
  remoteId( entity.remoteId ),
  remoteRevision( entity.remoteRevision ),
  mimeType( entity.mimeType ),
  parent( parent_ )
{
}

NotificationModel::NotificationModel( QObject* parent ) :
  QAbstractItemModel( parent ),
  m_source( 0 )
{
  NotificationMessageV2::registerDBusTypes();

  QString service = QLatin1String( "org.freedesktop.Akonadi" );
  if ( Akonadi::ServerManager::hasInstanceIdentifier() ) {
    service += "." + Akonadi::ServerManager::instanceIdentifier();
  }
  m_manager = new org::freedesktop::Akonadi::NotificationManager( service,
                                                          QLatin1String( "/notifications" ),
                                                          QDBusConnection::sessionBus(), this );
  if ( !m_manager ) {
    kWarning( 5250 ) << "Unable to connect to notification manager";
    return;
  }
}

NotificationModel::~NotificationModel()
{
  if ( m_source ) {
    m_source->unsubscribe();
  }
}

int NotificationModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 10;
}

int NotificationModel::rowCount( const QModelIndex& parent ) const
{
  if ( !parent.isValid() )
    return m_data.size();

  Item *parentItem = static_cast<Item*>( parent.internalPointer() );
  if ( parentItem ) {
    if ( parentItem->type == 0 ) {
      return static_cast<NotificationBlock*>( parentItem )->nodes.count();
    } else if ( parentItem->type == 1 ) {
      return static_cast<NotificationNode*>( parentItem )->entities.count();
    }
  }

  return 0;
}

QModelIndex NotificationModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !parent.isValid() ) {
    if ( row >= m_data.count() ) {
      return QModelIndex();
    }
    return createIndex( row, column, m_data.at( row ) );
  }

  Item *parentItem = static_cast<Item*>( parent.internalPointer() );
  if ( parentItem  ) {
    if ( parentItem ->type == 0 ) {
      NotificationBlock *parentBlock = static_cast<NotificationBlock*>( parentItem  );
      return createIndex( row, column, parentBlock->nodes.at( row ) );
    } else if ( parentItem->type == 1 ) {
      NotificationNode *parentNode = static_cast<NotificationNode*>( parentItem );
      return createIndex( row, column, parentNode->entities.at( row ) );
    }
  }

  return QModelIndex();
}

QModelIndex NotificationModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() ) {
    return QModelIndex();
  }

  Item *childItem = static_cast<Item*>( child.internalPointer() );
  if ( childItem ) {
    if ( childItem->type == 0 ) {
      return QModelIndex();
    } else if ( childItem->type == 1 ) {
      NotificationNode *childNode = static_cast<NotificationNode*>( childItem );
      return createIndex( m_data.indexOf( childNode->parent ), 0, childNode->parent );
    } else if ( childItem->type == 2 ) {
      NotificationEntity *childEntity = static_cast<NotificationEntity*>( childItem );
      NotificationNode *parentNode = childEntity->parent;
      return createIndex( parentNode->parent->nodes.indexOf( parentNode ), 0, childEntity->parent );
    }
  }

  return QModelIndex();
}

QVariant NotificationModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ) {
    return QVariant();
  }

  Item *item = static_cast<Item*>( index.internalPointer() );
  if ( !item ) {
    return QVariant();
  }

  if ( item->type == 0 ) {
    NotificationBlock *block = static_cast<NotificationBlock*>( item );
    if ( role == Qt::DisplayRole ) {
      switch ( index.column() ) {
        case 0:
          return QString( KLocale::global()->formatTime( block->timestamp.time(), true ) +
                QString::fromLatin1( ".%1" ).arg( block->timestamp.time().msec(), 3, 10, QLatin1Char('0') ) );
        case 1:
          return block->nodes.count();
      }
    }
  } else if ( item->type == 1 ) {
    NotificationNode *node = static_cast<NotificationNode*>( item );
    if ( role == Qt::DisplayRole ) {
      switch ( index.column() ) {
        case 0:
        {
          switch ( node->operation ) {
            case NotificationMessageV2::Add: return QLatin1String( "Add" );
            case NotificationMessageV2::Modify: return QLatin1String( "Modify" );
            case NotificationMessageV2::ModifyFlags: return QLatin1String( "ModifyFlags" );
            case NotificationMessageV2::ModifyTags: return QLatin1String( "ModifyTags" );
            case NotificationMessageV2::Move: return QLatin1String( "Move" );
            case NotificationMessageV2::Remove: return QLatin1String( "Delete" );
            case NotificationMessageV2::Link: return QLatin1String( "Link" );
            case NotificationMessageV2::Unlink: return QLatin1String( "Unlink" );
            case NotificationMessageV2::Subscribe: return QLatin1String( "Subscribe" );
            case NotificationMessageV2::Unsubscribe: return QLatin1String( "Unsubscribe" );
            default: return QString( "Invalid" );
          }
        }
        case 1:
        {
          switch ( node->type ) {
            case NotificationMessageV2::Collections: return QLatin1String( "Collections" );
            case NotificationMessageV2::Items: return QLatin1String( "Items" );
            case NotificationMessageV2::Tags: return QLatin1String( "Tags" );
            default: return QString( "Invalid" );
          }
        }
        case 2:
          return QString::fromUtf8( node->sessionId );
        case 3:
          return QString::fromUtf8( node->resource );
        case 4:
          return QString::fromUtf8( node->destResource );
        case 5:
          return QString::number( node->parentCollection );
        case 6:
          return QString::number( node->destCollection );
        case 7:
          return QString::fromUtf8( Akonadi::ImapParser::join( node->parts, ", " ) );
        case 8:
          return QString::fromUtf8( Akonadi::ImapParser::join( node->addedFlags, ", " ) );
        case 9:
          return QString::fromUtf8( Akonadi::ImapParser::join( node->removedFlags, ", " ) );
      }
    }
  } else if ( item->type == 2 ) {
    NotificationEntity *entity = static_cast<NotificationEntity*>( item );
    if ( role == Qt::DisplayRole ) {
      switch ( index.column() ) {
        case 0:
          return entity->id;
        case 1:
          return entity->remoteId;
        case 2:
          return entity->remoteRevision;
        case 3:
          return entity->mimeType;
      }
    }
  }

  return QVariant();
}

QVariant NotificationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
    switch ( section ) {
      case 0: return QString( "Operation / ID" );
      case 1: return QString( "Type / RID" );
      case 2: return QString( "Session / REV" );
      case 3: return QString( "Resource / MimeType" );
      case 4: return QString( "Destination Resource" );
      case 5: return QString( "Parent" );
      case 6: return QString( "Destination" );
      case 7: return QString( "Parts" );
      case 8: return QString( "Added Flags" );
      case 9: return QString( "Removed Flags" );
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}


void NotificationModel::slotNotify(const Akonadi::NotificationMessageV3::List& msgs)
{
  beginInsertRows( QModelIndex(), m_data.size(), m_data.size() );
  m_data.append( new NotificationBlock( msgs ) );
  endInsertRows();
}

void NotificationModel::clear()
{
  qDeleteAll( m_data );
  m_data.clear();
  reset();
}

void NotificationModel::setEnabled( bool enable )
{
  if ( enable && !m_source ) {
    const QString identifier = QString::fromLatin1("akonadiconsole_%1_notificationmodel").arg( QString::number( QCoreApplication::applicationPid() ) );
    m_manager->subscribe( identifier );

    QString service = QLatin1String( "org.freedesktop.Akonadi" );
    if ( Akonadi::ServerManager::hasInstanceIdentifier() ) {
      service += "." + Akonadi::ServerManager::instanceIdentifier();
    }
    m_source = new org::freedesktop::Akonadi::NotificationSource( service,
                                                      QLatin1String( "/subscriber/" ) + identifier,
                                                      QDBusConnection::sessionBus(), this );
    if ( !m_source ) {
      kWarning( 5250 ) << "Unable to connect to notification source";
      return;
    }

    connect( m_source, SIGNAL(notifyV3(Akonadi::NotificationMessageV3::List)),
             this, SLOT(slotNotify(Akonadi::NotificationMessageV3::List)) );

  } else if ( !enable && m_source ) {

    disconnect( m_source, SIGNAL(notifyV3(Akonadi::NotificationMessageV3::List)) );
    m_source->unsubscribe();
    m_source->deleteLater();
    m_source = 0;
  }
}






