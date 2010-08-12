/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
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
#include "composerview.h"
#include "messagelistproxy.h"
#include "mailactionmanager.h"
#include "global.h"

#include <KDE/KDebug>
#include <KActionCollection>
#include <KAction>
#include <KCmdLineArgs>
#include <kselectionproxymodel.h>
#include <klocalizedstring.h>
#include <kstandarddirs.h>

#include <KMime/Message>
#include <akonadi/kmime/messageparts.h>
#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>
#include <akonadi/kmime/messagestatus.h>
#include "messagecore/messagehelpers.h"
#include <akonadi/mail/standardmailactionmanager.h>

#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/KMime/SpecialMailCollectionsRequestJob>

#include <QTimer>
#include <QDir>

MainView::MainView(QWidget* parent) :
  KDeclarativeMainView( QLatin1String( "kmail-mobile" ), new MessageListProxy, parent )
{
}

void MainView::delayedInit()
{
  kDebug();
  KDeclarativeMainView::delayedInit();
  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");

  QTime t;
  if ( debugTiming ) {
    t.start();
    kWarning() << "Start MainView ctor" << &t << " - " << QDateTime::currentDateTime();
  }

  addMimeType( KMime::Message::mimeType() );
  itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope );
  setWindowTitle( i18n( "KMail Mobile" ) );

  MailActionManager *mailActionManager = new MailActionManager(actionCollection(), this);
  mailActionManager->setItemSelectionModel(itemSelectionModel());

  connect(actionCollection()->action("mark_message_important"), SIGNAL(triggered(bool)), SLOT(markImportant(bool)));
  connect(actionCollection()->action("mark_message_action_item"), SIGNAL(triggered(bool)), SLOT(markMailTask(bool)));
  connect(actionCollection()->action("write_new_email"), SIGNAL(triggered(bool)), SLOT(startComposer()));

  connect(itemSelectionModel()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged()));

  // lazy load of the default single folders
  QTimer::singleShot(3000, this, SLOT(initDefaultFolders()));

  // Is there messages to recover? Do it if needed.
  recoverAutoSavedMessages();

  if ( debugTiming ) {
    kWarning() << "Finished MainView ctor: " << t.elapsed() << " - "<< &t;
  }
}

void MainView::recoverAutoSavedMessages()
{
  kDebug() << "Any message to recover?";
  QDir autoSaveDir( KStandardDirs::locateLocal( "data", QLatin1String( "kmail2/") ) + QLatin1String( "autosave" ) );
  //### move directory creation to here

  const QFileInfoList savedMessages = autoSaveDir.entryInfoList( QDir::Files );

  if ( savedMessages.empty() ) {
    kDebug() << "No messages to recover";
    return;
  }

  foreach ( const QFileInfo savedMessage, savedMessages ) {
    QFile file( savedMessage.absoluteFilePath() );

    if ( file.open( QIODevice::ReadOnly ) ) {
      const KMime::Message::Ptr messagePtr ( new KMime::Message() );
      messagePtr->setContent( file.readAll() );
      messagePtr->parse();

      // load the autosaved message in a new composer
      ComposerView *composer = new ComposerView;
      composer->setMessage( messagePtr );
      composer->setAutoSaveFileName( savedMessage.fileName() );
      composer->show();

      file.close();
    } else {
      //### TODO: error message
      kDebug() << "error!!";
    }
  }
}

void MainView::startComposer()
{
    ComposerView *composer = new ComposerView;
    composer->show();
}

void MainView::restoreDraft( quint64 id )
{
    Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
    fetch->fetchScope().fetchFullPayload();
    fetch->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    connect( fetch, SIGNAL(result(KJob*)), SLOT(composeFetchResult(KJob*)) );
}

void MainView::composeFetchResult( KJob *job )
{
  Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() ) {
    // ###: emit ERROR
    return;
  }

  const Akonadi::Item item = fetch->items().first();
  if (!item.isValid() && !item.parentCollection().isValid() ) {
    // ###: emit ERROR
    return;
  }

  KMime::Message::Ptr msg = MessageCore::Util::message( item );
  if ( !msg ) {
    // ###: emit ERROR
    return;
  }

  // delete from the drafts folder
  // ###: do we need an option for this?)
  Akonadi::ItemDeleteJob *djob = new Akonadi::ItemDeleteJob( item );
  connect( djob, SIGNAL( result( KJob* ) ), this, SLOT( deleteItemResult( KJob* ) ) );

  // create the composer and fill it with the retrieved message
  ComposerView *composer = new ComposerView;
  composer->setMessage( msg );
  composer->show();
}

void MainView::reply( quint64 id )
{
  reply( id, MessageComposer::ReplySmart );
}

void MainView::replyToAll(quint64 id)
{
  reply( id, MessageComposer::ReplyAll );
}

void MainView::reply(quint64 id, MessageComposer::ReplyStrategy replyStrategy)
{
  Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
  fetch->fetchScope().fetchFullPayload();
  fetch->setProperty( "replyStrategy", QVariant::fromValue( replyStrategy ) );
  connect( fetch, SIGNAL(result(KJob*)), SLOT(replyFetchResult(KJob*)) );
}

void MainView::replyFetchResult(KJob* job)
{
  Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Akonadi::Item item = fetch->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( Global::identityManager() );
  factory.setReplyStrategy( fetch->property( "replyStrategy" ).value<MessageComposer::ReplyStrategy>() );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createReply().msg );
  composer->show();
}

void MainView::forwardInline(quint64 id)
{
  Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
  fetch->fetchScope().fetchFullPayload();
  connect( fetch, SIGNAL(result(KJob*)), SLOT(forwardInlineFetchResult(KJob*)) );
}

void MainView::forwardInlineFetchResult( KJob* job )
{
  Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Akonadi::Item item = fetch->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( Global::identityManager() );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createForward() );
  composer->show();
}

void MainView::markImportant(bool checked)
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if (checked && status.isImportant())
    return;
  if (checked)
      status.setImportant();
  else
      status.setImportant(false);
  item.setFlags(status.statusFlags());

  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
  connect(job, SIGNAL(result(KJob *)), SLOT(modifyDone(KJob *)));
}

void MainView::markMailTask(bool checked)
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if (checked && status.isToAct())
    return;
  if (checked)
      status.setToAct();
  else
      status.setToAct(false);
  item.setFlags(status.statusFlags());

  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
  connect(job, SIGNAL(result(KJob *)), SLOT(modifyDone(KJob *)));
}

void MainView::modifyDone(KJob *job)
{
  if (job->error())
  {
    kWarning() << "Modify error: " << job->errorString();
    return;
  }
}

void MainView::dataChanged()
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags(item.flags());

  actionCollection()->action("mark_message_important")->setChecked(status.isImportant());
  actionCollection()->action("mark_message_action_item")->setChecked(status.isToAct());
}

// FIXME: remove and put mark-as-read logic into messageviewer (shared with kmail)
void MainView::setListSelectedRow(int row)
{
  static const int column = 0;
  const QModelIndex idx = itemSelectionModel()->model()->index( row, column );
  itemSelectionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
  Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

  Akonadi::MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if ( status.isUnread() )
  {
    status.setRead();
    item.setFlags(status.statusFlags());
    Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(item);
  }
}

bool MainView::isDraft( int row )
{
  static const int column = 0;
  const QModelIndex idx = itemSelectionModel()->model()->index( row, column );
  itemSelectionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
  Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

  Akonadi::Collection &collection = item.parentCollection();
  return folderIsDrafts(collection);
}

// #############################################################
// ### Share the code between these marks with KMail Desktop?

void MainView::initDefaultFolders()
{
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Inbox );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Outbox );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::SentMail );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Drafts );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Trash );
  findCreateDefaultCollection( Akonadi::SpecialMailCollections::Templates );
}

void MainView::findCreateDefaultCollection( Akonadi::SpecialMailCollections::Type type )
{
  if( Akonadi::SpecialMailCollections::self()->hasDefaultCollection( type ) ) {
    const Akonadi::Collection col = Akonadi::SpecialMailCollections::self()->defaultCollection( type );
    if ( !( col.rights() & Akonadi::Collection::AllRights ) )
      kDebug() << "You do not have read/write permission to your inbox folder";
  } else {
    Akonadi::SpecialMailCollectionsRequestJob *job =
        new Akonadi::SpecialMailCollectionsRequestJob( this );

    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( createDefaultCollectionDone( KJob* ) ) );
    job->requestDefaultCollection( type );
  }
}

void MainView::createDefaultCollectionDone( KJob *job)
{
  if ( job->error() ) {
    kDebug() << "Error creating default collection: " << job->errorText();
    return;
  }

  Akonadi::SpecialMailCollectionsRequestJob *requestJob =
      qobject_cast<Akonadi::SpecialMailCollectionsRequestJob*>( job );

  const Akonadi::Collection col = requestJob->collection();
  if ( !( col.rights() & Akonadi::Collection::AllRights ) )
    kDebug() << "You do not have read/write permission to your inbox folder.";

  connect( Akonadi::SpecialMailCollections::self(), SIGNAL( defaultCollectionsChanged() ),
           this, SLOT( initDefaultFolders() ) );
}

bool MainView::folderIsDrafts(const Akonadi::Collection &col)
{
  Akonadi::Collection defaultDraft = Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Drafts );

  // check if this is the default draft folder
  if ( col == defaultDraft )
    return true;

  // check for invalid collection
  const QString idString = QString::number( col.id() );
  if ( idString.isEmpty() )
    return false;

  // search the identities if the folder matches the drafts-folder
  const KPIMIdentities::IdentityManager *im = Global::identityManager();
  for( KPIMIdentities::IdentityManager::ConstIterator it = im->begin(); it != im->end(); ++it ) {
    if ( (*it).drafts() == idString )
      return true;
  }

  return false;
}

void MainView::deleteItemResult( KJob *job )
{
    // ###: Need proper UI for this.
  if ( job->error() ) {
      kDebug() << "Error trying to delete item";
  }
}

void MainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel )
{
  Akonadi::StandardMailActionManager *manager = new Akonadi::StandardMailActionManager( actionCollection(), this );
  manager->setCollectionSelectionModel( collectionSelectionModel );
  manager->setItemSelectionModel( itemSelectionModel );

  manager->createAllActions();
}

// #############################################################

#include "mainview.moc"
