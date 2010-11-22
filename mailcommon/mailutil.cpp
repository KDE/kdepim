/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/


#include "mailutil.h"
#include "imapsettings.h"
#include "mailkernel.h"
#include "calendarinterface.h"

#include "messagecore/stringutil.h"
#include "messagecomposer/messagehelper.h"

#include <kmime/kmime_message.h>
#include <kpimutils/email.h>
#include <kimap/loginjob.h>
#include <mailtransport/transport.h>
#include <Akonadi/AgentManager>
#include <Akonadi/EntityTreeModel>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/kmime/messageparts.h>

#include <incidenceeditor-ng/incidencedialogfactory.h>

#include <KStandardDirs>
#include <kascii.h>
#include <KCharsets>
#include <KJob>
#include <kio/jobuidelegate.h>
#include <KLocale>

#include <stdlib.h>

#include "foldercollection.h"
#include <KDBusServiceStarter>
#include <messagecore/messagehelpers.h>
#include <KTemporaryFile>


OrgKdeAkonadiImapSettingsInterface *MailCommon::Util::createImapSettingsInterface( const QString &ident )
{
  return new OrgKdeAkonadiImapSettingsInterface("org.freedesktop.Akonadi.Resource." + ident, "/Settings", QDBusConnection::sessionBus() );
}


bool MailCommon::Util::isVirtualCollection(const Akonadi::Collection & collection)
{
  return ( collection.resource() == QLatin1String( "akonadi_nepomuktag_resource" ) || collection.resource() == QLatin1String( "akonadi_search_resource" ) );

}

QString MailCommon::Util::fullCollectionPath( const Akonadi::Collection& collection )
{
  QString fullPath;
  QModelIndex idx = Akonadi::EntityTreeModel::modelIndexForCollection( KernelIf->collectionModel(), collection );
  if ( !idx.isValid() )
    return fullPath;
  fullPath = idx.data().toString();
  idx = idx.parent();
  while ( idx != QModelIndex() ) {
    fullPath = idx.data().toString() + '/' + fullPath;
    idx = idx.parent();
  }
  return fullPath;
}

void MailCommon::Util::showJobErrorMessage( KJob *job )
{
  if ( job->error() ) {
    if ( static_cast<KIO::Job*>( job )->ui() )
      static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
    else
      kDebug()<<" job->errorString() :"<<job->errorString();
  }
}

Akonadi::AgentInstance::List MailCommon::Util::agentInstances()
{
  Akonadi::AgentInstance::List relevantInstances;
  foreach ( const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances() ) {
    if ( instance.type().mimeTypes().contains( KMime::Message::mimeType() ) &&
         instance.type().capabilities().contains( "Resource" ) &&
         !instance.type().capabilities().contains( "Virtual" ) ) {
      relevantInstances << instance;
    }
  }
  return relevantInstances;
}

void ensureKorganizerRunning( bool switchTo )
{
  QString error;
  QString dbusService;
  int result = KDBusServiceStarter::self()->findServiceFor( "DBUS/Organizer", QString(),
                                                            &error, &dbusService );
  if ( result == 0 ) {
    // OK, so korganizer (or kontact) is running. Now ensure the object we want is loaded.
    QDBusInterface iface( "org.kde.korganizer", "/MainApplication",
                          "org.kde.KUniqueApplication" );
    if ( iface.isValid() ) {
      if ( switchTo ) {
        iface.call( "newInstance" ); // activate korganizer window
      }
      QDBusInterface pimIface( "org.kde.korganizer", "/korganizer_PimApplication",
                                "org.kde.KUniqueApplication" );
      QDBusReply<bool> r = pimIface.call( "load" );
      if ( !r.isValid() || !r.value() ) {
        kWarning() << "Loading korganizer failed: " << pimIface.lastError().message();
      }
    } else {
      kWarning() << "Couldn't obtain korganizer D-Bus interface" << iface.lastError().message();
    }

    // We don't do anything with it, we just need it to be running so that it handles
    // the incoming directory.
  } else {
    kWarning() << "Couldn't start DBUS/Organizer:" << dbusService << error;
  }
}

static bool createIncidenceFromMail( KCalCore::IncidenceBase::IncidenceType type, const Akonadi::Item &mailItem )
{
  Akonadi::Item item( mailItem );

  // We need the full payload to attach the mail to the incidence
  if ( !item.loadedPayloadParts().contains( Akonadi::MessagePart::Body ) ) {
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item );
    job->fetchScope().fetchFullPayload();
    if ( job->exec() ) {
      if ( job->items().count() == 1 )
        item = job->items().first();
    }
  }

  KMime::Message::Ptr msg = MessageCore::Util::message( item );

  if ( !msg )
    return false;

  KTemporaryFile tf;
  tf.setAutoRemove( true );

  if ( !tf.open() ) {
    kWarning() << "CreateIncidenceFromMail: Unable to open temp file.";
    return false;
  }

  const QString incidenceDescription = i18n( "From: %1\nTo: %2\nSubject: %3", msg->from()->asUnicodeString(),
                                             msg->to()->asUnicodeString(), msg->subject()->asUnicodeString() );

  tf.write( msg->encodedContent() );
  tf.flush();

  QStringList attachmentUris;
  attachmentUris << tf.fileName();

  QStringList attachmentMimeTypes;
  attachmentMimeTypes << QLatin1String( "message/rfc822" );

#ifndef _WIN32_WCE
  switch ( type ) {
    case KCalCore::IncidenceBase::TypeEvent:
      IncidenceEditorNG::IncidenceDialogFactory::createEventEditor( i18n("Mail: %1", msg->subject()->asUnicodeString() ),
                                                                    incidenceDescription,
                                                                    attachmentUris,
                                                                    QStringList() /* attendees */,
                                                                    attachmentMimeTypes,
                                                                    true /* inline */,
                                                                    Akonadi::Collection() );
      break;
    case KCalCore::IncidenceBase::TypeTodo:
      IncidenceEditorNG::IncidenceDialogFactory::createTodoEditor( i18n("Mail: %1", msg->subject()->asUnicodeString() ),
                                                                   incidenceDescription,
                                                                   attachmentUris,
                                                                   QStringList() /* attendees */,
                                                                   attachmentMimeTypes,
                                                                   true /* inline */,
                                                                   Akonadi::Collection() );
      break;
    default:
      Q_ASSERT( false );
      break;
  }
#endif

  tf.close();
  return true;
}

bool MailCommon::Util::createTodoFromMail( const Akonadi::Item &mailItem )
{
  return createIncidenceFromMail( KCalCore::IncidenceBase::TypeTodo, mailItem );
}

bool MailCommon::Util::createEventFromMail( const Akonadi::Item &mailItem )
{
  return createIncidenceFromMail( KCalCore::IncidenceBase::TypeEvent, mailItem );
}

uint MailCommon::Util::folderIdentity(const Akonadi::Item& item)
{
  uint id = 0;
  if( item.isValid() && item.parentCollection().isValid() ) {
        QSharedPointer<FolderCollection> fd = FolderCollection::forCollection( item.parentCollection() );
        id = fd->identity();
  }
  return id;
}

static QModelIndex indexBelow( QAbstractItemModel *model, const QModelIndex &current )
{
  // if we have children, return first child
  if ( model->rowCount( current ) > 0 )
    return model->index( 0, 0, current );

  // if we have siblings, return next sibling
  const QModelIndex parent = model->parent( current );
  const QModelIndex sibling = model->index( current.row() + 1, 0, parent );

  if ( sibling.isValid() ) // found valid sibling
    return sibling;

  if ( !parent.isValid() ) // our parent is the tree root and we have no siblings
    return QModelIndex(); // we reached the bottom of the tree

  // iterate over our parents back to root until we find a parent with a valid sibling
  QModelIndex currentParent = parent;
  QModelIndex grandParent = model->parent( currentParent );
  while ( currentParent.isValid() ) {
    // check if the parent has children except from us
    if ( model->rowCount( grandParent ) > currentParent.row() + 1 ) {
      const QModelIndex index = indexBelow( model, model->index( currentParent.row() + 1, 0, grandParent ) );
      if ( index.isValid() )
        return index;
    }

    currentParent = grandParent;
    grandParent = model->parent( currentParent );
  }

  return QModelIndex(); // nothing found -> end of tree
}

static QModelIndex lastChildOf( QAbstractItemModel *model, const QModelIndex &current )
{
  if ( model->rowCount( current ) == 0 )
    return current;

  return lastChildOf( model, model->index( model->rowCount( current ) - 1, 0, current ) );
}

static QModelIndex indexAbove( QAbstractItemModel *model, const QModelIndex &current )
{
  const QModelIndex parent = model->parent( current );

  if ( current.row() == 0 ) // we have no previous siblings -> our parent is the next item above us
    return parent;

  // find previous sibling
  const QModelIndex previousSibling = model->index( current.row() - 1, 0, parent );

  // the item above us is the last child (or grandchild, or grandgrandchild... etc) of our previous sibling
  return lastChildOf( model, previousSibling );
}

QModelIndex MailCommon::Util::nextUnreadCollection( QAbstractItemModel *model, const QModelIndex &current, SearchDirection direction )
{
  QModelIndex index = current;
  while ( true ) {
    if ( direction == MailCommon::Util::ForwardSearch ) {
      index = indexBelow( model, index );
    } else if ( direction == MailCommon::Util::BackwardSearch ) {
      index = indexAbove( model, index );
    }

    if ( !index.isValid() ) // reach end or top of the model
      return QModelIndex();

    // check if the index is a collection
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() ) {

      // check if it is unread
      if ( collection.statistics().unreadCount() > 0 )
        return index; // we found the next unread collection
    }
  }

  return QModelIndex(); // no unread collection found
}
