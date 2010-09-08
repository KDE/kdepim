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

#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/KMime/SpecialMailCollectionsRequestJob>

#include <QTimer>
#include <QDir>

Q_DECLARE_METATYPE(KMime::Content*)
QML_DECLARE_TYPE(MessageViewer::MessageViewItem)

MainView::MainView(QWidget* parent) :
  KDeclarativeMainView( QLatin1String( "kmail-mobile" ), new MessageListProxy, parent )
{
  qRegisterMetaType<KMime::Content*>();
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

  addMimeType( KMime::Message::mimeType() );
  itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope );
  setWindowTitle( i18n( "KMail Mobile" ) );

  MailActionManager *mailActionManager = new MailActionManager(actionCollection(), this);
  mailActionManager->setItemSelectionModel(itemSelectionModel());

  connect(actionCollection()->action("mark_message_important"), SIGNAL(triggered(bool)), SLOT(markImportant(bool)));
  connect(actionCollection()->action("mark_message_action_item"), SIGNAL(triggered(bool)), SLOT(markMailTask(bool)));
  connect(actionCollection()->action("write_new_email"), SIGNAL(triggered(bool)), SLOT(startComposer()));
  connect(actionCollection()->action("message_reply"), SIGNAL(triggered(bool)), SLOT(replyToMessage()));
  connect(actionCollection()->action("message_reply_to_all"), SIGNAL(triggered(bool)), SLOT(replyToAll()));
  connect(actionCollection()->action("message_reply_to_author"), SIGNAL(triggered(bool)), SLOT(replyToAuthor()));
  connect(actionCollection()->action("message_reply_to_list"), SIGNAL(triggered(bool)), SLOT(replyToMailingList()));
  connect(actionCollection()->action("message_forward"), SIGNAL(triggered(bool)), SLOT(forwardMessage()));
  connect(actionCollection()->action("message_forward_as_attachment"), SIGNAL(triggered(bool)), SLOT(forwardAsAttachment()));
  connect(actionCollection()->action("message_redirect"), SIGNAL(triggered(bool)), SLOT(redirect()));
  connect(actionCollection()->action("message_send_again"), SIGNAL(triggered(bool)), SLOT(sendAgain()));
  connect(actionCollection()->action("message_edit"), SIGNAL(triggered(bool)), SLOT(sendAgain())); //do the same under a different name
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
    Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
    fetch->fetchScope().fetchFullPayload();
    fetch->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    connect( fetch, SIGNAL(result(KJob*)), SLOT(composeFetchResult(KJob*)) );
}

void MainView::composeFetchResult( KJob *job )
{
  Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() ) {
    kDebug() << "error!!";
    //###: review error string
    KMessageBox::sorry( this,
                        i18n("Could not restore a draft."),
                        i18n("Restore Draft Error"));
    return;
  }

  const Akonadi::Item item = fetch->items().first();
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
  Akonadi::ItemDeleteJob *djob = new Akonadi::ItemDeleteJob( item );
  connect( djob, SIGNAL( result( KJob* ) ), this, SLOT( deleteItemResult( KJob* ) ) );

  // create the composer and fill it with the retrieved message
  ComposerView *composer = new ComposerView;
  composer->setMessage( msg );
  composer->show();
}


void MainView::sendAgain()
{
    Akonadi::Item item = currentItem();
    if ( !item.isValid() )
      return;

    Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( item.id() ), this );
    fetch->fetchScope().fetchFullPayload();
    connect( fetch, SIGNAL(result(KJob*)), SLOT(sendAgainFetchResult(KJob*)) );
}

void MainView::sendAgainFetchResult(KJob* job)
{
  Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Akonadi::Item item = fetch->items().first();
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

void MainView::replyToAuthor()
{
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyAuthor );
}

void MainView::replyToMailingList()
{
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyList );
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

void MainView::forward(quint64 id, ForwardMode mode)
{
  Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
  fetch->fetchScope().fetchFullPayload();
  fetch->setProperty( "forwardMode", QVariant::fromValue( mode ) );
  connect( fetch, SIGNAL(result(KJob*)), SLOT(forwardFetchResult(KJob*)) );
}

void MainView::forwardFetchResult( KJob* job )
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
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
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
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
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

void MainView::replyToMessage()
{
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplySmart );
}

void MainView::replyToAll()
{
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyAll );
}

void MainView::forwardMessage()
{
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), InLine );
}

void MainView::forwardAsAttachment()
{
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), AsAttachment );
}


void MainView::redirect()
{
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), Redirect );
}


Akonadi::Item MainView::currentItem()
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1)
    return Akonadi::Item();
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return Akonadi::Item();

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
  Akonadi::Item item = currentItem();
  if ( !item.isValid() )
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
  Q_ASSERT(idx.isValid());
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
    //###: review error string
    KMessageBox::sorry( this,
                        i18n("Error creating default collection."),
                        i18n("Internal Error"));
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
  Akonadi::StandardMailActionManager *manager = new Akonadi::StandardMailActionManager( actionCollection(), this );
  manager->setCollectionSelectionModel( collectionSelectionModel );
  manager->setItemSelectionModel( itemSelectionModel );

  manager->createAllActions();

  manager->interceptAction( Akonadi::StandardActionManager::CreateResource );

  connect( manager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  manager->setActionText( Akonadi::StandardActionManager::SynchronizeResources, ki18np( "Synchronize emails\nin account", "Synchronize emails\nin accounts" ) );
  manager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Edit account" ) );
  manager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add subfolder" ) );
  manager->setActionText( Akonadi::StandardActionManager::DeleteCollections, ki18np( "Delete folder", "Delete folders" ) );
  manager->setActionText( Akonadi::StandardActionManager::SynchronizeCollections, ki18np( "Synchronize emails\nin folder", "Synchronize emails\nin folders" ) );
  manager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Edit folder" ) );
  manager->action( Akonadi::StandardActionManager::MoveCollectionToMenu )->setText( i18n( "Move folder to" ) );
  manager->action( Akonadi::StandardActionManager::CopyCollectionToMenu )->setText( i18n( "Copy folder to" ) );
  manager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete email", "Delete emails" ) );
  manager->action( Akonadi::StandardActionManager::MoveItemToMenu )->setText( i18n( "Move email\nto folder" ) );
  manager->action( Akonadi::StandardActionManager::CopyItemToMenu )->setText( i18n( "Copy email\nto folder" ) );

  actionCollection()->action( "synchronize_all_items" )->setText( i18n( "Synchronize\nall emails" ) );

  connect( collectionSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(folderChanged()));
}

void MainView::setupAgentActionManager( QItemSelectionModel *selectionModel )
{
  Akonadi::AgentActionManager *manager = new Akonadi::AgentActionManager( actionCollection(), this );
  manager->setSelectionModel( selectionModel );
  manager->createAllActions();

  manager->action( Akonadi::AgentActionManager::CreateAgentInstance )->setText( i18n( "Add" ) );
  manager->action( Akonadi::AgentActionManager::DeleteAgentInstance )->setText( i18n( "Delete" ) );
  manager->action( Akonadi::AgentActionManager::ConfigureAgentInstance )->setText( i18n( "Edit" ) );

  manager->interceptAction( Akonadi::AgentActionManager::CreateAgentInstance );

  connect( manager->action( Akonadi::AgentActionManager::CreateAgentInstance ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::DialogTitle,
                           i18nc( "@title:window", "New Account" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageText,
                           i18n( "Could not create account: %1" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageTitle,
                           i18n( "Account creation failed" ) );

  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxTitle,
                           i18nc( "@title:window", "Delete Account?" ) );
  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxText,
                           i18n( "Do you really want to delete the selected account?" ) );
}

void MainView::saveMessage()
{
    Akonadi::Item item = currentItem();
    if ( !item.isValid() )
      return;

//See the header file for SaveMailCommand why is it here
    SaveMailCommand *command = new SaveMailCommand( item, this );
    command->execute();
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
      const QModelIndex index = collectionSelectionModel->selection().indexes().at( 0 );
      Q_ASSERT( index.isValid() );
      const Akonadi::Collection collection = index.data( Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
      Q_ASSERT( collection.isValid() );

      KSharedConfigPtr config = KSharedConfig::openConfig("kmail-mobilerc");
      KConfigGroup group(config, QString("c%1").arg(collection.id()));
      group.writeEntry("htmlMailOverride", useHtml);

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
      const QModelIndex index = collectionSelectionModel->selection().indexes().at( 0 );
      Q_ASSERT( index.isValid() );
      const Akonadi::Collection collection = index.data( Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
      Q_ASSERT( collection.isValid() );

      KSharedConfigPtr config = KSharedConfig::openConfig("kmail-mobilerc");
      KConfigGroup group(config, QString("c%1").arg(collection.id()));
      group.writeEntry("htmlLoadExternalOverride", load);

      item->viewer()->setHtmlLoadExtOverride( load );
  }
}

void MainView::folderChanged()
{
    QItemSelectionModel* collectionSelectionModel = regularSelectionModel();
    if ( collectionSelectionModel->selection().indexes().isEmpty() )
      return;
    const QModelIndex index = collectionSelectionModel->selection().indexes().at( 0 );
    Q_ASSERT( index.isValid() );
    const Akonadi::Collection collection = index.data( Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
    Q_ASSERT( collection.isValid() );

    KSharedConfigPtr config = KSharedConfig::openConfig("kmail-mobilerc");
    KConfigGroup group(config, QString("c%1").arg(collection.id()));
    actionCollection()->action("prefer_html_to_plain")->setChecked(group.readEntry("htmlMailOverride", false));
    actionCollection()->action("load_external_ref")->setChecked(group.readEntry("htmlLoadExternalOverride", false));
}

// #############################################################

#include "mainview.moc"
