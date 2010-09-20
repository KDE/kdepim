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
#include "messageviewitem.h"
#include "messageviewer/viewer.h"
#include <akonadi/collection.h>
#include <akonadi/collectionmodel.h>

#include "savemailcommand_p.h"

#include <KDE/KDebug>
#include <KActionCollection>
#include <KAction>
#include <KCmdLineArgs>
#include <KCMultiDialog>
#include <KMessageBox>
#include <kselectionproxymodel.h>
#include <klocalizedstring.h>
#include <kstandarddirs.h>

#include <KMime/Message>
#include <akonadi/agentactionmanager.h>
#include <akonadi/kmime/messageparts.h>
#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>
#include <akonadi/kmime/messagestatus.h>
#include "messagecore/messagehelpers.h"
#include "messageviewer/nodehelper.h"
#include <akonadi/kmime/standardmailactionmanager.h>
#ifdef KDEQMLPLUGIN_STATIC
#include "runtime/qml/kde/kdeintegration.h"
#include <QDeclarativeContext>
#endif

#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/KMime/SpecialMailCollectionsRequestJob>

#include <QTimer>
#include <QDir>
#include <messagecomposer/akonadisender.h>
#include <mailtransport/transportmanager.h>
#include <QSignalMapper>
#include <QLabel>


Q_DECLARE_METATYPE(KMime::Content*)
QML_DECLARE_TYPE(MessageViewer::MessageViewItem)

using namespace Akonadi;

static bool workOffline()
{
  KConfig config( QLatin1String( "akonadikderc" ) );
  const KConfigGroup group( &config, QLatin1String( "Actions" ) );

  return group.readEntry( "WorkOffline", false );
}

MainView::MainView(QWidget* parent) :
  KDeclarativeMainView( QLatin1String( "kmail-mobile" ), new MessageListProxy, parent )
{
  qRegisterMetaType<KMime::Content*>();
  mAskingToGoOnline = false;
  mTransportDialog = 0;
}

MainView::~MainView()
{
  delete mMessageSender;
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

  qmlRegisterType<MessageViewer::MessageViewItem>( "org.kde.messageviewer", 4, 5, "MessageView" );
#ifdef KDEQMLPLUGIN_STATIC
  rootContext()->setContextProperty( QLatin1String("KDE"), new KDEIntegration( this ) );
#endif

  addMimeType( KMime::Message::mimeType() );
  itemFetchScope().fetchPayloadPart( MessagePart::Envelope );
  setWindowTitle( i18n( "KMail Mobile" ) );
  mMessageSender = new AkonadiSender;

  MailActionManager *mMailActionManager = new MailActionManager(actionCollection(), this);
  mMailActionManager->setItemSelectionModel(itemSelectionModel());

  connect(actionCollection()->action("mark_message_important"), SIGNAL(triggered(bool)), SLOT(markImportant(bool)));
  connect(actionCollection()->action("mark_message_action_item"), SIGNAL(triggered(bool)), SLOT(markMailTask(bool)));
  connect(actionCollection()->action("write_new_email"), SIGNAL(triggered(bool)), SLOT(startComposer()));
  connect(actionCollection()->action("send_queued_emails"), SIGNAL(triggered(bool)), SLOT(sendQueued()));
  connect(actionCollection()->action("send_queued_emails_via"), SIGNAL(triggered(bool)), SLOT(sendQueuedVia()));
  connect(actionCollection()->action("message_reply"), SIGNAL(triggered(bool)), SLOT(replyToMessage()));
  connect(actionCollection()->action("message_reply_to_all"), SIGNAL(triggered(bool)), SLOT(replyToAll()));
  connect(actionCollection()->action("message_reply_to_author"), SIGNAL(triggered(bool)), SLOT(replyToAuthor()));
  connect(actionCollection()->action("message_reply_to_list"), SIGNAL(triggered(bool)), SLOT(replyToMailingList()));
  connect(actionCollection()->action("message_forward"), SIGNAL(triggered(bool)), SLOT(forwardMessage()));
  connect(actionCollection()->action("message_forward_as_attachment"), SIGNAL(triggered(bool)), SLOT(forwardAsAttachment()));
  connect(actionCollection()->action("message_redirect"), SIGNAL(triggered(bool)), SLOT(redirect()));
  connect(actionCollection()->action("message_send_again"), SIGNAL(triggered(bool)), SLOT(sendAgain()));
  connect(actionCollection()->action("message_edit"), SIGNAL(triggered(bool)), SLOT(sendAgain())); //do the same under a different name
  connect(actionCollection()->action("message_find_in"), SIGNAL(triggered(bool)), SLOT(findInMessage()));
  connect(actionCollection()->action("message_save_as"), SIGNAL(triggered(bool)), SLOT(saveMessage()));
  connect(actionCollection()->action("save_favorite"), SIGNAL(triggered(bool)), SLOT(saveFavorite()));
  connect(actionCollection()->action("prefer_html_to_plain"), SIGNAL(triggered(bool)), SLOT(preferHTML(bool)));
  connect(actionCollection()->action("load_external_ref"), SIGNAL(triggered(bool)), SLOT(loadExternalReferences(bool)));

  connect(itemSelectionModel()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged()));

  KAction *action = new KAction( i18n( "Identities" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( configureIdentity() ) );
  actionCollection()->addAction( "kmail_mobile_identities", action );
  action = new KAction( i18n( "New Email" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT(startComposer()) );
  actionCollection()->addAction( "add_new_mail", action );

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

  foreach ( const QFileInfo& savedMessage, savedMessages ) {
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
      kDebug() << "error!!";
      //###: review error string
      KMessageBox::sorry( this,
                          i18n("Could not recover a saved message."),
                          i18n("Recover Message Error"));
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
    ItemFetchJob *fetch = new ItemFetchJob( Item( id ), this );
    fetch->fetchScope().fetchFullPayload();
    fetch->fetchScope().setAncestorRetrieval( ItemFetchScope::Parent );
    connect( fetch, SIGNAL(result(KJob*)), SLOT(composeFetchResult(KJob*)) );
}

void MainView::composeFetchResult( KJob *job )
{
  ItemFetchJob *fetch = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() ) {
    kDebug() << "error!!";
    //###: review error string
    KMessageBox::sorry( this,
                        i18n("Could not restore a draft."),
                        i18n("Restore Draft Error"));
    return;
  }

  const Item item = fetch->items().first();
  if (!item.isValid() && !item.parentCollection().isValid() ) {
    //###: review error string
    KMessageBox::sorry( this,
                        i18n("Invalid draft message."),
                        i18n("Restore Draft Error"));
    return;
  }

  KMime::Message::Ptr msg = MessageCore::Util::message( item );
  if ( !msg ) {
    //###: review error string
    KMessageBox::sorry( this,
                        i18n("Message content error"),
                        i18n("Restore Draft Error"));
    return;
  }

  // delete from the drafts folder
  // ###: do we need an option for this?)
  ItemDeleteJob *djob = new ItemDeleteJob( item );
  connect( djob, SIGNAL( result( KJob* ) ), this, SLOT( deleteItemResult( KJob* ) ) );

  // create the composer and fill it with the retrieved message
  ComposerView *composer = new ComposerView;
  composer->setMessage( msg );
  composer->show();
}


void MainView::sendAgain()
{
    Item item = currentItem();
    if ( !item.isValid() )
      return;

    ItemFetchJob *fetch = new ItemFetchJob( Item( item.id() ), this );
    fetch->fetchScope().fetchFullPayload();
    connect( fetch, SIGNAL(result(KJob*)), SLOT(sendAgainFetchResult(KJob*)) );
}

void MainView::sendAgainFetchResult(KJob* job)
{
  ItemFetchJob *fetch = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Item item = fetch->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  KMime::Message::Ptr msg = MessageCore::Util::message( item );
  MessageComposer::MessageFactory factory( msg, item.id() );
  factory.setIdentityManager( Global::identityManager() );
  KMime::Message::Ptr newMsg = factory.createResend();
  newMsg->contentType()->setCharset( MessageViewer::NodeHelper::charset( msg.get() ) );

  ComposerView *composer = new ComposerView;
  composer->setMessage( newMsg );
  composer->show();
}

bool MainView::askToGoOnline()
{
  // already asking means we are offline and need to wait anyhow
  if ( mAskingToGoOnline ) {
    return false;
  }

  if ( workOffline() ) {
    mAskingToGoOnline = true;
    int rc =
    KMessageBox::questionYesNo( this,
                                i18n("KMail is currently in offline mode. "
                                     "How do you want to proceed?"),
                                i18n("Online/Offline"),
                                KGuiItem(i18n("Work Online")),
                                KGuiItem(i18n("Work Offline")));

    mAskingToGoOnline = false;
    if( rc == KMessageBox::No ) {
      return false;
    } else {
      ///emulate turning off offline mode
      QAction *workOffLineAction = mMailActionManager->action( StandardActionManager::ToggleWorkOffline );
      workOffLineAction->setChecked(true);
      workOffLineAction->trigger();
    }
  }
  return true;
}

void MainView::sendQueued()
{
  if ( !askToGoOnline() )
    return;

  mMessageSender->sendQueued();
}

void MainView::sendQueuedVia()
{
  if ( !askToGoOnline() )
    return;

  const QStringList availTransports= MailTransport::TransportManager::self()->transportNames();

  delete mTransportDialog;
  mTransportDialog = new QWidget( this, Qt::Dialog ); //not a real dialog though, should be done in QML
  mTransportDialog->setWindowTitle( i18n( "Send Queued Email Via" ) );
  QPalette pal = mTransportDialog->palette();
  pal.setColor( QPalette::Window, Qt::darkGray ); //make sure the label is readable...
  mTransportDialog->setPalette( pal );
  QVBoxLayout *layout = new QVBoxLayout( mTransportDialog );
  QLabel *label = new QLabel( i18n( "Send Queued Email Via" ) );
  layout->addWidget( label );
  QSignalMapper *mapper = new QSignalMapper( mTransportDialog );
  Q_FOREACH( QString transport, availTransports )
  {
    QPushButton *button = new QPushButton( transport );
    layout->addWidget( button );
    mapper->setMapping( button, transport);
    connect( button, SIGNAL( clicked() ), mapper, SLOT( map() ) );
 }
  connect( mapper, SIGNAL( mapped( QString ) ), this, SLOT( sendQueuedVia( QString ) ));
  QPushButton *button = new QPushButton( i18n("Discard") );
  layout->addWidget( button );
  connect( button, SIGNAL( clicked( bool ) ), mTransportDialog, SLOT( close() ) );
  mTransportDialog->show();
}

void MainView::sendQueuedVia(const QString& transport)
{
  mMessageSender->sendQueued( transport );
  delete mTransportDialog;
  mTransportDialog = 0;
}


void MainView::replyToAuthor()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyAuthor );
}

void MainView::replyToMailingList()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyList );
}

void MainView::reply(quint64 id, MessageComposer::ReplyStrategy replyStrategy)
{
  ItemFetchJob *fetch = new ItemFetchJob( Item( id ), this );
  fetch->fetchScope().fetchFullPayload();
  fetch->setProperty( "replyStrategy", QVariant::fromValue( replyStrategy ) );
  connect( fetch, SIGNAL(result(KJob*)), SLOT(replyFetchResult(KJob*)) );
}

void MainView::replyFetchResult(KJob* job)
{
  ItemFetchJob *fetch = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Item item = fetch->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( Global::identityManager() );
  factory.setReplyStrategy( fetch->property( "replyStrategy" ).value<MessageComposer::ReplyStrategy>() );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createReply().msg );
  composer->show();
}

void MainView::forward(quint64 id, ForwardMode mode)
{
  ItemFetchJob *fetch = new ItemFetchJob( Item( id ), this );
  fetch->fetchScope().fetchFullPayload();
  fetch->setProperty( "forwardMode", QVariant::fromValue( mode ) );
  connect( fetch, SIGNAL(result(KJob*)), SLOT(forwardFetchResult(KJob*)) );
}

void MainView::forwardFetchResult( KJob* job )
{
  ItemFetchJob *fetch = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Item item = fetch->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( Global::identityManager() );

  ComposerView *composer = new ComposerView;
  ForwardMode mode = fetch->property( "forwardMode" ).value<ForwardMode>();
  switch (mode) {
    case InLine:
      composer->setMessage( factory.createForward() );
      break;
    case AsAttachment: {
      QPair< KMime::Message::Ptr, QList< KMime::Content* > > fwdMsg = factory.createAttachedForward( QList< KMime::Message::Ptr >() << item.payload<KMime::Message::Ptr>());
      //the invokeMethods are there to be sure setMessage and addAttachment is called after composer->delayedInit
      QMetaObject::invokeMethod( composer, "setMessage", Qt::QueuedConnection, Q_ARG(KMime::Message::Ptr,  fwdMsg.first) );
      foreach( KMime::Content* attach, fwdMsg.second )
        QMetaObject::invokeMethod( composer, "addAttachment", Qt::QueuedConnection, Q_ARG(KMime::Content*,  attach ) );
      break;
    }
    case Redirect:
      composer->setMessage( factory.createRedirect("") );
      break;
  }
  composer->show();
}

void MainView::markImportant(bool checked)
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if (checked && status.isImportant())
    return;
  if (checked)
      status.setImportant();
  else
      status.setImportant(false);
  item.setFlags(status.statusFlags());

  ItemModifyJob *job = new ItemModifyJob(item);
  connect(job, SIGNAL(result(KJob *)), SLOT(modifyDone(KJob *)));
}

void MainView::markMailTask(bool checked)
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if (checked && status.isToAct())
    return;
  if (checked)
      status.setToAct();
  else
      status.setToAct(false);
  item.setFlags(status.statusFlags());

  ItemModifyJob *job = new ItemModifyJob(item);
  connect(job, SIGNAL(result(KJob *)), SLOT(modifyDone(KJob *)));
}

void MainView::replyToMessage()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplySmart );
}

void MainView::replyToAll()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyAll );
}

void MainView::forwardMessage()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), InLine );
}

void MainView::forwardAsAttachment()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), AsAttachment );
}


void MainView::redirect()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), Redirect );
}


Item MainView::currentItem()
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if (list.size() != 1)
    return Item();
  const QModelIndex idx = list.first();
  Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return Item();

  return item;
}


void MainView::modifyDone(KJob *job)
{
  if (job->error())
  {
    kWarning() << "Modify error: " << job->errorString();
    //###: review error string
    //## Use a notification instead?
    KMessageBox::sorry( this,
                        i18n("Error trying to set item status"),
                        i18n("Messages status error"));
    return;
  }
}

void MainView::dataChanged()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  MessageStatus status;
  status.setStatusFromFlags(item.flags());

  actionCollection()->action("mark_message_important")->setChecked(status.isImportant());
  actionCollection()->action("mark_message_action_item")->setChecked(status.isToAct());
}

// FIXME: remove and put mark-as-read logic into messageviewer (shared with kmail)
void MainView::setListSelectedRow(int row)
{
  static const int column = 0;
  const QModelIndex idx = itemSelectionModel()->model()->index( row, column );
  Q_ASSERT(idx.isValid());
  itemSelectionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
  itemActionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
  // FIXME this should all be in messageviewer and happen after mail download, see also similar code in KMCommands
  Item fullItem = idx.data(EntityTreeModel::ItemRole).value<Item>();
  MessageStatus status;
  status.setStatusFromFlags(fullItem.flags());
  if ( status.isUnread() )
  {
    Item sparseItem( fullItem.id() );
    sparseItem.setRevision( fullItem.revision() );

    status.setRead();
    sparseItem.setFlags(status.statusFlags());
    ItemModifyJob *modifyJob = new ItemModifyJob(sparseItem, this);
    modifyJob->disableRevisionCheck();
    modifyJob->ignorePayload();
  }
}

void MainView::configureIdentity()
{
  KCMultiDialog dlg;
  dlg.addModule( "kcm_kpimidentities" );
  dlg.currentPage()->setHeader( QLatin1String( "" ) ); // hide header to save space
  dlg.setButtons( KDialog::Ok | KDialog::Cancel );
  dlg.exec();
}

bool MainView::isDraft( int row )
{
  static const int column = 0;
  const QModelIndex idx = itemSelectionModel()->model()->index( row, column );
  kDebug() << "itemSelectionModel " << itemSelectionModel() << " model" << itemSelectionModel()->model() << " idx->model()" << idx.model();
  itemSelectionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
  Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();

  Collection &collection = item.parentCollection();
  return folderIsDrafts(collection);
}

// #############################################################
// ### Share the code between these marks with KMail Desktop?

void MainView::initDefaultFolders()
{
  findCreateDefaultCollection( SpecialMailCollections::Inbox );
  findCreateDefaultCollection( SpecialMailCollections::Outbox );
  findCreateDefaultCollection( SpecialMailCollections::SentMail );
  findCreateDefaultCollection( SpecialMailCollections::Drafts );
  findCreateDefaultCollection( SpecialMailCollections::Trash );
  //findCreateDefaultCollection( SpecialMailCollections::Templates );
}

void MainView::findCreateDefaultCollection( SpecialMailCollections::Type type )
{
  if( SpecialMailCollections::self()->hasDefaultCollection( type ) ) {
    const Collection col = SpecialMailCollections::self()->defaultCollection( type );
    if ( !( col.rights() & Collection::AllRights ) )
      kDebug() << "You do not have read/write permission to your inbox folder";
  } else {
    SpecialMailCollectionsRequestJob *job =
        new SpecialMailCollectionsRequestJob( this );

    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( createDefaultCollectionDone( KJob* ) ) );
    job->requestDefaultCollection( type );
  }
}

void MainView::createDefaultCollectionDone( KJob *job)
{
  if ( job->error() ) {
    kDebug() << "Error creating default collection: " << job->errorText();
    //###: review error string
    // diabled for now, triggers too often without good reason on the n900 (too short timeouts probably)
/*    KMessageBox::sorry( this,
                        i18n("Error creating default collection."),
                        i18n("Internal Error"));*/
    return;
  }

  SpecialMailCollectionsRequestJob *requestJob =
      qobject_cast<SpecialMailCollectionsRequestJob*>( job );

  const Collection col = requestJob->collection();
  if ( !( col.rights() & Collection::AllRights ) )
    kDebug() << "You do not have read/write permission to your inbox folder.";

  connect( SpecialMailCollections::self(), SIGNAL( defaultCollectionsChanged() ),
           this, SLOT( initDefaultFolders() ) );
}

bool MainView::folderIsDrafts(const Collection &col)
{
  Collection defaultDraft = SpecialMailCollections::self()->defaultCollection( SpecialMailCollections::Drafts );

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
  if ( job->error() ) {
      kDebug() << "Error trying to delete item";
      //###: review error string
      KMessageBox::sorry( this,
                          i18n("Can not delete draft."),
                          i18n("Delete Draft Error"));
  }
}

void MainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel )
{
  mMailActionManager = new StandardMailActionManager( actionCollection(), this );
  mMailActionManager->setCollectionSelectionModel( collectionSelectionModel );
  mMailActionManager->setItemSelectionModel( itemSelectionModel );

  //Don't use mMailActionManager->createAllActions() to save memory by not
  //creating actions that doesn't make sense in mobile.
  QList<StandardActionManager::Type> standardActions;
  standardActions << StandardActionManager::CreateCollection << StandardActionManager::CopyCollections
   << StandardActionManager:: DeleteCollections << StandardActionManager::SynchronizeCollections
   << StandardActionManager::CollectionProperties << StandardActionManager::CopyItems
   << StandardActionManager::Paste << StandardActionManager::DeleteItems
   << StandardActionManager::ManageLocalSubscriptions << StandardActionManager::AddToFavoriteCollections
   << StandardActionManager::RemoveFromFavoriteCollections  << StandardActionManager::RenameFavoriteCollection
   << StandardActionManager::CutItems << StandardActionManager::CutCollections
   << StandardActionManager::CreateResource << StandardActionManager::DeleteResources
   << StandardActionManager::ResourceProperties << StandardActionManager::SynchronizeResources
   << StandardActionManager::ToggleWorkOffline << StandardActionManager::CopyCollectionToDialog
   << StandardActionManager::MoveCollectionToDialog << StandardActionManager::CopyItemToDialog
   << StandardActionManager::MoveItemToDialog;

  Q_FOREACH( StandardActionManager::Type standardAction, standardActions ) {
    mMailActionManager->createAction( standardAction );
  }

  QList<StandardMailActionManager::Type> mailActions;
  mailActions << StandardMailActionManager::MarkMailAsRead << StandardMailActionManager::MarkMailAsUnread
   << StandardMailActionManager::MarkMailAsImportant << StandardMailActionManager::MarkMailAsActionItem
   << StandardMailActionManager::MarkAllMailAsRead << StandardMailActionManager::MarkAllMailAsUnread
   << StandardMailActionManager::MarkAllMailAsImportant << StandardMailActionManager::MarkAllMailAsActionItem
   << StandardMailActionManager::MoveToTrash << StandardMailActionManager::MoveAllToTrash
   << StandardMailActionManager::RemoveDuplicates << StandardMailActionManager::EmptyAllTrash;

  Q_FOREACH( StandardMailActionManager::Type mailAction, mailActions ) {
    mMailActionManager->createAction( mailAction );
  }

  mMailActionManager->interceptAction( StandardActionManager::CreateResource );

  connect( mMailActionManager->action( StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  mMailActionManager->setActionText( StandardActionManager::SynchronizeResources, ki18np( "Synchronize emails\nin account", "Synchronize emails\nin accounts" ) );
  mMailActionManager->action( StandardActionManager::ResourceProperties )->setText( i18n( "Edit account" ) );
  mMailActionManager->action( StandardActionManager::CreateCollection )->setText( i18n( "Add subfolder" ) );
  mMailActionManager->setActionText( StandardActionManager::DeleteCollections, ki18np( "Delete folder", "Delete folders" ) );
  mMailActionManager->setActionText( StandardActionManager::SynchronizeCollections, ki18np( "Synchronize emails\nin folder", "Synchronize emails\nin folders" ) );
  mMailActionManager->action( StandardActionManager::CollectionProperties )->setText( i18n( "Edit folder" ) );
  mMailActionManager->action( StandardActionManager::MoveCollectionToMenu )->setText( i18n( "Move folder to" ) );
  mMailActionManager->action( StandardActionManager::CopyCollectionToMenu )->setText( i18n( "Copy folder to" ) );
  mMailActionManager->setActionText( StandardActionManager::DeleteItems, ki18np( "Delete email", "Delete emails" ) );
  mMailActionManager->action( StandardActionManager::MoveItemToMenu )->setText( i18n( "Move email\nto folder" ) );
  mMailActionManager->action( StandardActionManager::CopyItemToMenu )->setText( i18n( "Copy email\nto folder" ) );

  actionCollection()->action( "synchronize_all_items" )->setText( i18n( "Synchronize\nall emails" ) );

  connect( collectionSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(folderChanged()));
}

void MainView::setupAgentActionManager( QItemSelectionModel *selectionModel )
{
  AgentActionManager *manager = new AgentActionManager( actionCollection(), this );
  manager->setSelectionModel( selectionModel );

  manager->createAllActions();

  manager->action( AgentActionManager::CreateAgentInstance )->setText( i18n( "Add" ) );
  manager->action( AgentActionManager::DeleteAgentInstance )->setText( i18n( "Delete" ) );
  manager->action( AgentActionManager::ConfigureAgentInstance )->setText( i18n( "Edit" ) );

  manager->interceptAction( AgentActionManager::CreateAgentInstance );

  connect( manager->action( AgentActionManager::CreateAgentInstance ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  manager->setContextText( AgentActionManager::CreateAgentInstance, AgentActionManager::DialogTitle,
                           i18nc( "@title:window", "New Account" ) );
  manager->setContextText( AgentActionManager::CreateAgentInstance, AgentActionManager::ErrorMessageText,
                           i18n( "Could not create account: %1" ) );
  manager->setContextText( AgentActionManager::CreateAgentInstance, AgentActionManager::ErrorMessageTitle,
                           i18n( "Account creation failed" ) );

  manager->setContextText( AgentActionManager::DeleteAgentInstance, AgentActionManager::MessageBoxTitle,
                           i18nc( "@title:window", "Delete Account?" ) );
  manager->setContextText( AgentActionManager::DeleteAgentInstance, AgentActionManager::MessageBoxText,
                           i18n( "Do you really want to delete the selected account?" ) );
}

void MainView::saveMessage()
{
    Item item = currentItem();
    if ( !item.isValid() )
      return;

//See the header file for SaveMailCommand why is it here
    SaveMailCommand *command = new SaveMailCommand( item, this );
    command->execute();
}

void MainView::findInMessage()
{
  QGraphicsObject *root = rootObject();
  MessageViewer::MessageViewItem* item = 0;
  Q_FOREACH(QObject* obj, root->children())
  {
      if (dynamic_cast<MessageViewer::MessageViewItem*>(obj)) {
        item = static_cast<MessageViewer::MessageViewItem*>(obj);
        break;
    }
  }

  if (item) {
    item->viewer()->slotFind();
  }
}

void MainView::preferHTML(bool useHtml)
{
  QGraphicsObject *root = rootObject();
  MessageViewer::MessageViewItem* item = 0;
  Q_FOREACH(QObject* obj, root->children())
  {
      if (dynamic_cast<MessageViewer::MessageViewItem*>(obj)) {
        item = static_cast<MessageViewer::MessageViewItem*>(obj);
        break;
    }
  }

  if (item) {
      QItemSelectionModel* collectionSelectionModel = regularSelectionModel();
      if ( collectionSelectionModel->selection().indexes().isEmpty() )
        return;
      QModelIndexList selectedIndexes = collectionSelectionModel->selection().indexes();
      Q_FOREACH(QModelIndex index, selectedIndexes) {
          Q_ASSERT( index.isValid() );
          const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
          Q_ASSERT( collection.isValid() );

          KSharedConfigPtr config = KSharedConfig::openConfig("kmail-mobilerc");
          KConfigGroup group(config, QString("c%1").arg(collection.id()));
          group.writeEntry("htmlMailOverride", useHtml);
      }
      item->viewer()->setHtmlOverride( useHtml );
  }
}


void MainView::loadExternalReferences(bool load)
{
  QGraphicsObject *root = rootObject();
  MessageViewer::MessageViewItem* item = 0;
  Q_FOREACH(QObject* obj, root->children())
  {
      if (dynamic_cast<MessageViewer::MessageViewItem*>(obj)) {
        item = static_cast<MessageViewer::MessageViewItem*>(obj);
        break;
    }
  }

  if (item) {
      QItemSelectionModel* collectionSelectionModel = regularSelectionModel();
      if ( collectionSelectionModel->selection().indexes().isEmpty() )
        return;
      qDebug() << collectionSelectionModel->selection().indexes().count();
      QModelIndexList selectedIndexes = collectionSelectionModel->selection().indexes();
      Q_FOREACH(QModelIndex index, selectedIndexes) {
          Q_ASSERT( index.isValid() );
          const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
          Q_ASSERT( collection.isValid() );

          KSharedConfigPtr config = KSharedConfig::openConfig("kmail-mobilerc");
          KConfigGroup group(config, QString("c%1").arg(collection.id()));
          group.writeEntry("htmlLoadExternalOverride", load);
      }
      item->viewer()->setHtmlLoadExtOverride( load );
  }
}

void MainView::folderChanged()
{
    QItemSelectionModel* collectionSelectionModel = regularSelectionModel();
    if ( collectionSelectionModel->selection().indexes().isEmpty() )
      return;
    //NOTE: not exactly correct if multiple folders are selected, although I don't know what to do then, as the action is not
    //a tri-state one (checked, unchecked, for some folders checked)
    bool htmlMailOverrideInAll = true;
    bool htmlLoadExternalOverrideInAll = true;

    KSharedConfigPtr config = KSharedConfig::openConfig("kmail-mobilerc");
    Q_FOREACH( QModelIndex index, collectionSelectionModel->selectedRows() ) {
        Q_ASSERT( index.isValid() );
        const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
        Q_ASSERT( collection.isValid() );
        KConfigGroup group(config, QString("c%1").arg(collection.id()));
        if ( group.readEntry("htmlMailOverride", false) == false )
          htmlMailOverrideInAll = false;
        if ( group.readEntry("htmlLoadExternalOverride", false) == false )
          htmlLoadExternalOverrideInAll = false;
    }
    actionCollection()->action("prefer_html_to_plain")->setChecked( htmlMailOverrideInAll );
    actionCollection()->action("load_external_ref")->setChecked( htmlLoadExternalOverrideInAll );
}

// #############################################################

#include "mainview.moc"
