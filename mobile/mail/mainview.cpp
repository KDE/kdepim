/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#include "acleditor.h"
#include "breadcrumbnavigation.h"
#include "charsetselectiondialog.h"
#include "composerview.h"
#include "configwidget.h"
#include "declarativewidgetbase.h"
#include "filterconfigwidget.h"
#include "filtermodel.h"
#include "emailsexporthandler.h"
#include "emailsfilterproxymodel.h"
#include "emailsguistatemanager.h"
#include "emailsimporthandler.h"
#include "mailactionmanager.h"
#include "mailcommon/collectiongeneralpage.h"
#include "mailcommon/mailkernel.h"
#include "mailcommon/sendmdnhandler.h"
#include "mailthreadgroupercomparator.h"
#include "messagecore/messagehelpers.h"
#include "messagelistproxy.h"
#include "messagelistsettingscontroller.h"
#include "messageviewer/globalsettings.h"
#include "messageviewer/headerstrategy.h"
#include "messageviewer/headerstyle.h"
#include "messageviewer/nodehelper.h"
#include "messageviewer/viewer.h"
#include "messageviewitem.h"
#include "mobilekernel.h"
#include "savemailcommand_p.h"
#include "searchwidget.h"
#include "settings.h"
#include "templateemailmodel.h"
#include "threadgroupermodel.h"
#include "threadmodel.h"
#include "threadselectionmodel.h"
#include "vacationmanager.h"

#include <akonadi/agentactionmanager.h>
#include <akonadi/collection.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionpropertiesdialog.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/kmime/messageflags.h>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/kmime/messagestatus.h>
#include <akonadi/kmime/specialmailcollectionsrequestjob.h>
#include <akonadi/kmime/standardmailactionmanager.h>
#include <akonadi_next/quotacolorproxymodel.h>
#include <akonadibreadcrumbnavigationfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcmdlineargs.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <klinkitemselectionmodel.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kmime/kmime_message.h>
#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>
#include <kselectionproxymodel.h>
#include <kstandarddirs.h>
#include <mailcommon/expirypropertiesdialog.h>
#include <mailcommon/filtermanager.h>
#include <mailcommon/foldercollection.h>
#include <mailcommon/mailutil.h>
#include <mailcommon/mailkernel.h>
#include <mailtransport/transportmanager.h>
#include <messagecomposer/akonadisender.h>
#include <messagecore/stringutil.h>
#include <qmllistselectionmodel.h>
#include <qmlcheckableproxymodel.h>

#include <QtCore/QDir>
#include <QtCore/QSignalMapper>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDeclarative/QDeclarativeContext>
#include <QtGui/QLabel>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QTreeView>

#ifdef _WIN32_WCE
#include <identitypage.h>
#include <kcomponentdata.h>
#endif

#ifdef KDEQMLPLUGIN_STATIC
#include "runtime/qml/kde/kdeintegration.h"
#endif

Q_DECLARE_METATYPE( KMime::Content* )
QML_DECLARE_TYPE( EmailsGuiStateManager )
QML_DECLARE_TYPE( MessageViewer::MessageViewItem )
QML_DECLARE_TYPE( DeclarativeConfigWidget )
QML_DECLARE_TYPE( DeclarativeFilterConfigWidget )
QML_DECLARE_TYPE( DeclarativeSearchWidget )

using namespace Akonadi;

static bool workOffline()
{
  KConfig config( QLatin1String( "akonadikderc" ) );
  const KConfigGroup group( &config, QLatin1String( "Actions" ) );

  return group.readEntry( "WorkOffline", false );
}

MainView::MainView(QWidget* parent)
  : KDeclarativeMainView( QLatin1String( "kmail-mobile" ), new MessageListProxy, parent ),
    mAskingToGoOnline( false ),
    mTransportDialog( 0 ),
    m_grouperComparator( 0 ),
    mQuotaColorProxyModel( new QuotaColorProxyModel( this ) )
{
  qRegisterMetaType<KMime::Content*>();

  updateConfig();

  QDBusConnection::sessionBus().registerService( "org.kde.kmailmobile.composer" );
  QDBusConnection::sessionBus().registerObject( "/composer", this, QDBusConnection::ExportScriptableSlots );

  Akonadi::CollectionPropertiesDialog::registerPage( new MailCommon::CollectionGeneralPageFactory );
}

MainView::~MainView()
{
  delete m_grouperComparator;
}

void MainView::setConfigWidget( ConfigWidget *configWidget )
{
  Q_ASSERT( configWidget );
  connect( configWidget, SIGNAL( configChanged() ), this, SLOT( updateConfig() ) );
}

int MainView::openComposer( const QString &to, const QString &cc, const QString &bcc,
                            const QString &subject, const QString &body )
{
  KMime::Message::Ptr message = KMime::Message::Ptr( new KMime::Message );
  message->to()->fromUnicodeString( to, "utf-8" );
  message->cc()->fromUnicodeString( cc, "utf-8" );
  message->bcc()->fromUnicodeString( bcc, "utf-8" );
  message->date()->setDateTime( KDateTime::currentLocalDateTime() );
  message->subject()->fromUnicodeString( subject, "utf-8" );

  KMime::Content *bodyMessage = message->mainBodyPart();
  bodyMessage->setBody( body.toUtf8() );
  message->assemble();

  ComposerView *composer = new ComposerView;
  composer->setMessage( message );
  composer->show();

  return 0;
}

#define VIEW(model) {                        \
  QTreeView *view = new QTreeView( this );   \
  view->setWindowFlags( Qt::Window );        \
  view->setAttribute(Qt::WA_DeleteOnClose);  \
  view->setModel(model);                     \
  view->setWindowTitle(#model);              \
  view->show();                              \
}                                            \

QAbstractItemModel* MainView::createItemModelContext(QDeclarativeContext* context, QAbstractItemModel* model)
{
  m_grouperComparator = new MailThreadGrouperComparator;
  m_threadGrouperModel = new ThreadGrouperModel( m_grouperComparator, this );
  m_threadGrouperModel->setSourceModel( model );

  model = m_threadGrouperModel;

  QAbstractProxyModel *itemFilterModel = createItemFilterModel();
  if ( itemFilterModel ) {
    setItemFilterModel( itemFilterModel );
    itemFilterModel->setSourceModel( model );
    model = itemFilterModel;
  }

  QMLCheckableItemProxyModel *qmlCheckable = new QMLCheckableItemProxyModel( this );
  qmlCheckable->setSourceModel( model );

  QItemSelectionModel *itemActionCheckModel = new QItemSelectionModel( model, this );
  qmlCheckable->setSelectionModel( itemActionCheckModel );

  KSelectionProxyModel *checkedItems = new KSelectionProxyModel( itemActionCheckModel, this );
  checkedItems->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  checkedItems->setSourceModel( model );

  QItemSelectionModel *itemNavigationModel = new QItemSelectionModel( model, this );

  QAbstractProxyModel *_listProxy = listProxy();

  if ( _listProxy ) {
    _listProxy->setParent( this ); // Make sure the proxy gets deleted when this gets deleted.

    _listProxy->setSourceModel( qmlCheckable );
  }
  KLinkItemSelectionModel *itemNavigationSelectionModel = new KLinkItemSelectionModel( _listProxy, itemNavigationModel, this );

  KLinkItemSelectionModel *itemActionSelectionModel = new KLinkItemSelectionModel( _listProxy, itemActionCheckModel, this );
  setItemNaigationAndActionSelectionModels(itemNavigationSelectionModel, itemActionSelectionModel);

  if ( _listProxy ) {
    context->setContextProperty( "itemModel", _listProxy );

    QMLListSelectionModel *qmlItemNavigationSelectionModel = new QMLListSelectionModel( itemNavigationSelectionModel, this );
    QMLListSelectionModel *qmlItemActionSelectionModel = new QMLListSelectionModel( itemActionSelectionModel, this );

    context->setContextProperty( "_itemNavigationModel", QVariant::fromValue( static_cast<QObject*>( qmlItemNavigationSelectionModel ) ) );
    context->setContextProperty( "_itemActionModel", QVariant::fromValue( static_cast<QObject*>( qmlItemActionSelectionModel ) ) );

    Akonadi::BreadcrumbNavigationFactory *bulkActionBnf = new Akonadi::BreadcrumbNavigationFactory( this );
    bulkActionBnf->createCheckableBreadcrumbContext( entityTreeModel(), this );
    context->setContextProperty( "_bulkActionBnf", QVariant::fromValue( static_cast<QObject*>( bulkActionBnf ) ) );
  }

  m_threadsModel = new ThreadModel(_listProxy, this);

  context->setContextProperty( "_threads", m_threadsModel );

  QItemSelectionModel *itemThreadModel = new QItemSelectionModel( model, this );

  m_threadContentsModel = new KSelectionProxyModel(itemThreadModel, this);
  m_threadContentsModel->setSourceModel(_listProxy);
  m_threadContentsModel->setObjectName("threadContentsModel");

  context->setContextProperty( "_threadContents", m_threadContentsModel );

  ThreadSelectionModel *threadSelector = new ThreadSelectionModel(m_threadsModel, itemThreadModel, itemNavigationModel, this);

  QMLListSelectionModel *qmlThreadSelector = new QMLListSelectionModel(threadSelector, this);

  context->setContextProperty("_threadSelector", qmlThreadSelector );

  KLinkItemSelectionModel *threadMailSelector = new KLinkItemSelectionModel(m_threadContentsModel, itemNavigationModel, this);

  QMLListSelectionModel *qmlThreadMailSelector = new QMLListSelectionModel(threadMailSelector, this);

  context->setContextProperty("_threadMailSelector", qmlThreadMailSelector );

  connect( regularSelectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( collectionSelectionChanged() ) );

#if 0
  {
    QTreeView *view = new QTreeView;
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->setModel(m_threadsModel);
    view->setSelectionModel(threadSelector);
    view->setWindowTitle("threads");
    view->show();
  }
#endif

  return model;
}


void MainView::delayedInit()
{
  KDeclarativeMainView::delayedInit();

  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet( "timeit" );
  MobileKernel::self()->setFolderCollectionMonitor( monitor() );

  mCollectionModel = new Akonadi::EntityMimeTypeFilterModel( this );
  mCollectionModel->setSourceModel( entityTreeModel() );
  mCollectionModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  mCollectionModel->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
  mCollectionModel->setDynamicSortFilter( true );
  mCollectionModel->setSortCaseSensitivity( Qt::CaseInsensitive );

  MobileKernel::self()->setCollectionModel( mCollectionModel );
  KSharedConfigPtr config = KSharedConfig::openConfig( "kmail-mobilerc" );
  MessageViewer::GlobalSettings::self()->setSharedConfig( config );
  MessageViewer::GlobalSettings::self()->readConfig();

  mTemplateSelectionModel = new QItemSelectionModel( entityTreeModel(), this );

  mEmailTemplateModel = new TemplateEmailModel( mTemplateSelectionModel, this );
  mEmailTemplateModel->setSourceModel( entityTreeModel() );
  mEmailTemplateModel->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );
  rootContext()->setContextProperty( "_emailTemplateModel", mEmailTemplateModel );

  mFilterModel = new FilterModel;
  rootContext()->setContextProperty( "_filterModel", mFilterModel );

  mAclEditor = new AclEditor( actionCollection(), this );
  rootContext()->setContextProperty( "aclEditor", mAclEditor );
  rootContext()->setContextProperty( "aclModel", mAclEditor->model() );

  VacationManager *vacationManager = new VacationManager( actionCollection(), this, this );
  rootContext()->setContextProperty( "vacationManager", vacationManager );

  mMessageListSettingsController = new MessageListSettingsController( this );
  actionCollection()->addAction( "messagelist_change_settings", mMessageListSettingsController->editAction() );
  rootContext()->setContextProperty( "messageListSettings", mMessageListSettingsController );
  connect( mMessageListSettingsController, SIGNAL( settingsChanged( const MessageListSettings& ) ),
           this, SLOT( messageListSettingsChanged( const MessageListSettings& ) ) );

  QTime time;
  if ( debugTiming ) {
    time.start();
    kWarning() << "Start MainView ctor" << &time << " - " << QDateTime::currentDateTime();
  }

  qmlRegisterType<MessageViewer::MessageViewItem>( "org.kde.messageviewer", 4, 5, "MessageView" );
  qmlRegisterType<DeclarativeConfigWidget>( "org.kde.akonadi.mail", 4, 5, "ConfigWidget" );
  qmlRegisterType<DeclarativeFilterConfigWidget>( "org.kde.akonadi.mail", 4, 5, "FilterConfigWidget" );
  qmlRegisterType<DeclarativeSearchWidget>( "org.kde.akonadi.mail", 4, 5, "SearchWidget" );
  qmlRegisterUncreatableType<EmailsGuiStateManager>( "org.kde.akonadi.mail", 4, 5, "EmailsGuiStateManager", QLatin1String( "This type is only exported for its enums" ) );
#ifdef KDEQMLPLUGIN_STATIC
  rootContext()->setContextProperty( QLatin1String( "KDE" ), new KDEIntegration( this ) );
#endif

  addMimeType( KMime::Message::mimeType() );
  itemFetchScope().fetchPayloadPart( MessagePart::Envelope );
  setWindowTitle( i18n( "Mail" ) );

  MailActionManager *mailActionManager = new MailActionManager( actionCollection(), this );
  mailActionManager->setItemSelectionModel( itemSelectionModel() );

  connect( actionCollection()->action( "mark_message_important" ), SIGNAL( triggered( bool ) ), SLOT( markImportant( bool ) ) );
  connect( actionCollection()->action( "mark_message_action_item" ), SIGNAL( triggered( bool ) ), SLOT( markMailTask( bool ) ) );
  connect( actionCollection()->action( "send_queued_emails" ), SIGNAL( triggered( bool ) ), SLOT( sendQueued() ) );
  connect( actionCollection()->action( "send_queued_emails_via" ), SIGNAL( triggered( bool ) ), SLOT( sendQueuedVia() ) );
  connect( actionCollection()->action( "message_reply" ), SIGNAL( triggered( bool ) ), SLOT( replyToMessage() ) );
  connect( actionCollection()->action( "message_reply_to_all" ), SIGNAL( triggered( bool ) ), SLOT( replyToAll() ) );
  connect( actionCollection()->action( "message_reply_to_author" ), SIGNAL( triggered( bool ) ), SLOT( replyToAuthor() ) );
  connect( actionCollection()->action( "message_reply_to_list" ), SIGNAL( triggered( bool ) ), SLOT( replyToMailingList() ) );
  connect( actionCollection()->action( "message_reply_without_quoting" ), SIGNAL( triggered( bool ) ), SLOT( replyWithoutQuoting() ) );
  connect( actionCollection()->action( "message_forward" ), SIGNAL( triggered( bool ) ), SLOT( forwardMessage() ) );
  connect( actionCollection()->action( "message_forward_as_attachment" ), SIGNAL( triggered( bool ) ), SLOT( forwardAsAttachment() ) );
  connect( actionCollection()->action( "message_redirect" ), SIGNAL( triggered( bool ) ), SLOT( redirect() ) );
  connect( actionCollection()->action( "message_send_again" ), SIGNAL( triggered( bool ) ), SLOT( sendAgain() ) );
  connect( actionCollection()->action( "message_edit" ), SIGNAL( triggered( bool ) ), SLOT( sendAgain() ) ); //do the same under a different name
  connect( actionCollection()->action( "message_find_in" ), SIGNAL( triggered( bool ) ), SLOT( findInMessage() ) );
  connect( actionCollection()->action( "message_save_as" ), SIGNAL( triggered( bool ) ), SLOT( saveMessage() ) );
  connect( actionCollection()->action( "message_fixed_font" ), SIGNAL( triggered( bool ) ), SLOT( useFixedFont() ) );
  connect( actionCollection()->action( "save_favorite" ), SIGNAL( triggered( bool ) ), SLOT( saveFavorite() ) );
  connect( actionCollection()->action( "prefer_html_to_plain" ), SIGNAL( triggered( bool ) ), SLOT( preferHTML( bool ) ) );
  connect( actionCollection()->action( "load_external_ref" ), SIGNAL( triggered( bool ) ), SLOT( loadExternalReferences( bool ) ) );
  connect( actionCollection()->action( "show_expire_properties" ), SIGNAL( triggered( bool ) ), SLOT( showExpireProperties() ) );
  connect( actionCollection()->action( "move_all_to_trash" ), SIGNAL( triggered( bool ) ), SLOT( moveToOrEmptyTrash() ) );
  connect( actionCollection()->action( "create_todo_reminder" ), SIGNAL( triggered( bool ) ), SLOT( createToDo() ) );
  connect( actionCollection()->action( "create_event" ), SIGNAL( triggered( bool ) ), SLOT( createEvent() ) );
  connect( actionCollection()->action( "apply_filters" ), SIGNAL( triggered( bool ) ), SLOT( applyFilters() ) );

  connect( itemSelectionModel()->model(), SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), SLOT( dataChanged() ) );

  KAction *action = new KAction( i18n( "Identities" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( configureIdentity() ) );
  actionCollection()->addAction( "kmail_mobile_identities", action );

  action = new KAction( i18n( "New Email" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( startComposer() ) );
  actionCollection()->addAction( "add_new_mail", action );

  action = new KAction( i18n( "Import Emails" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( importItems() ) );
  actionCollection()->addAction( QLatin1String( "import_emails" ), action );

  action = new KAction( i18n( "Export Emails" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
  actionCollection()->addAction( QLatin1String( "export_emails" ), action );

  action = new KAction( i18n( "Show Source" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( showMessageSource() ) );
  actionCollection()->addAction( QLatin1String( "show_message_source" ), action );

  action = new KAction( i18n( "Encoding..." ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( selectOverrideEncoding() ) );
  actionCollection()->addAction( QLatin1String( "change_message_encoding" ), action );

  action = new KAction( i18n( "Show all Recipients" ), this );
  action->setCheckable( true );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( toggleShowExtendedHeaders( bool ) ) );
  actionCollection()->addAction( QLatin1String( "show_extended_headers" ), action );

  // lazy load of the default single folders
  QTimer::singleShot( 3000, this, SLOT( initDefaultFolders() ) );

  // Is there messages to recover? Do it if needed.
  recoverAutoSavedMessages();

  if ( debugTiming ) {
    kWarning() << "Finished MainView ctor: " << time.elapsed() << " - "<< &time;
  }

  connect( this, SIGNAL( statusChanged( QDeclarativeView::Status ) ),
           this, SLOT( qmlInitialized( QDeclarativeView::Status ) ) );

  if ( !workOffline() ) {
    const Akonadi::AgentInstance::List lst = MailCommon::Util::agentInstances();
    foreach ( Akonadi::AgentInstance type, lst ) {
      if ( type.identifier().contains( IMAP_RESOURCE_IDENTIFIER ) ||
          type.identifier().contains( POP3_RESOURCE_IDENTIFIER ) ) {
        type.setIsOnline( true );
      }
    }
  }
}

void MainView::qmlInitialized(QDeclarativeView::Status status)
{
  if ( status != Ready )
    return;

  MessageViewer::MessageViewItem* item = messageViewerItem();

  if ( item ) {
    // register the send MDN handler
    item->viewer()->addMessageLoadedHandler( new MailCommon::SendMdnHandler( MobileKernel::self(), this ) );

    bool fixedFont = MessageViewer::GlobalSettings::self()->useFixedFont();
    item->viewer()->setUseFixedFont( fixedFont );
    actionCollection()->action( "message_fixed_font" )->setChecked( fixedFont );
  }
}


void MainView::recoverAutoSavedMessages()
{
  kDebug() << "Any message to recover?";
  QDir autoSaveDir( KStandardDirs::locateLocal( "data", QLatin1String( "kmail2/" ) ) + QLatin1String( "autosave" ) );
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
                          i18n( "Could not recover a saved message." ),
                          i18n( "Recover Message Error" ) );
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
  ItemFetchJob *job = new ItemFetchJob( Item( id ), this );
  job->fetchScope().fetchFullPayload();
  job->fetchScope().setAncestorRetrieval( ItemFetchScope::Parent );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( composeFetchResult( KJob* ) ) );
}

void MainView::composeFetchResult( KJob *job )
{
  const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetchJob->items().isEmpty() ) {
    kDebug() << "error!!";
    //###: review error string
    KMessageBox::sorry( this,
                        i18n( "Could not restore a draft." ),
                        i18n( "Restore Draft Error" ) );
    return;
  }

  const Item item = fetchJob->items().first();
  if ( !item.isValid() && !item.parentCollection().isValid() ) {
    //###: review error string
    KMessageBox::sorry( this,
                        i18n( "Invalid draft message." ),
                        i18n( "Restore Draft Error" ) );
    return;
  }

  const KMime::Message::Ptr msg = MessageCore::Util::message( item );
  if ( !msg ) {
    //###: review error string
    KMessageBox::sorry( this,
                        i18n( "Message content error" ),
                        i18n( "Restore Draft Error" ) );
    return;
  }

  // delete from the drafts folder
  // ###: do we need an option for this?)
  ItemDeleteJob *deleteJob = new ItemDeleteJob( item );
  connect( deleteJob, SIGNAL( result( KJob* ) ), this, SLOT( deleteItemResult( KJob* ) ) );

  // create the composer and fill it with the retrieved message
  ComposerView *composer = new ComposerView;
  composer->setMessage( msg );
  composer->show();
}

void MainView::sendAgain()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  ItemFetchJob *job = new ItemFetchJob( Item( item.id() ), this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL( result( KJob* ) ), SLOT( sendAgainFetchResult( KJob* ) ) );
}

void MainView::sendAgainFetchResult( KJob *job )
{
  const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetchJob->items().isEmpty() )
    return;

  const Item item = fetchJob->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  const KMime::Message::Ptr msg = MessageCore::Util::message( item );
  MessageComposer::MessageFactory factory( msg, item.id() );
  factory.setIdentityManager( MobileKernel::self()->identityManager() );

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
                                i18n( "KMail is currently in offline mode. "
                                      "How do you want to proceed?" ),
                                i18n( "Online/Offline" ),
                                KGuiItem( i18n( "Work Online" ) ),
                                KGuiItem( i18n( "Work Offline" ) ) );

    mAskingToGoOnline = false;
    if ( rc == KMessageBox::No ) {
      return false;
    } else {
      ///emulate turning off offline mode
      QAction *workOffLineAction = mMailActionManager->action( StandardActionManager::ToggleWorkOffline );
      workOffLineAction->setChecked( true );
      workOffLineAction->trigger();
    }
  }

  return true;
}

void MainView::sendQueued()
{
  if ( !askToGoOnline() )
    return;

  KernelIf->msgSender()->sendQueued();
}

void MainView::sendQueuedVia()
{
  if ( !askToGoOnline() )
    return;

  const QStringList availTransports= MailTransport::TransportManager::self()->transportNames();

  delete mTransportDialog;
  mTransportDialog = new QWidget( this, Qt::Dialog ); //not a real dialog though, should be done in QML
  mTransportDialog->setWindowTitle( i18n( "Send Queued Email Via" ) );

  QPalette palette = mTransportDialog->palette();
  palette.setColor( QPalette::Window, Qt::darkGray ); //make sure the label is readable...
  mTransportDialog->setPalette( palette );

  QVBoxLayout *layout = new QVBoxLayout( mTransportDialog );
  QLabel *label = new QLabel( i18n( "Send Queued Email Via" ) );
  layout->addWidget( label );
  QSignalMapper *mapper = new QSignalMapper( mTransportDialog );

  Q_FOREACH( const QString &transport, availTransports ) {
    QPushButton *button = new QPushButton( transport );
    layout->addWidget( button );
    mapper->setMapping( button, transport );
    connect( button, SIGNAL( clicked() ), mapper, SLOT( map() ) );
  }

  connect( mapper, SIGNAL( mapped( QString ) ), this, SLOT( sendQueuedVia( QString ) ));

  QPushButton *button = new QPushButton( i18n( "Discard" ) );
  layout->addWidget( button );
  connect( button, SIGNAL( clicked( bool ) ), mTransportDialog, SLOT( close() ) );

  mTransportDialog->show();
}

void MainView::sendQueuedVia( const QString &transport )
{
  KernelIf->msgSender()->sendQueued( transport );
  delete mTransportDialog;
  mTransportDialog = 0;
}

void MainView::replyToAuthor()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyAuthor );
}

void MainView::replyToMailingList()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyList );
}

void MainView::reply( quint64 id, MessageComposer::ReplyStrategy replyStrategy, bool quoteOriginal )
{
  ItemFetchJob *job = new ItemFetchJob( Item( id ), this );
  job->fetchScope().fetchFullPayload();
  job->setProperty( "replyStrategy", QVariant::fromValue( replyStrategy ) );
  job->setProperty( "quoteOriginal", QVariant::fromValue( quoteOriginal ) );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( replyFetchResult( KJob* ) ) );
}

void MainView::replyFetchResult( KJob *job )
{
  const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetchJob->items().isEmpty() )
    return;

  const Item item = fetchJob->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( MobileKernel::self()->identityManager() );
  factory.setReplyStrategy( fetchJob->property( "replyStrategy" ).value<MessageComposer::ReplyStrategy>() );

  factory.setQuote( fetchJob->property( "quoteOriginal" ).toBool() );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createReply().msg );
  composer->show();

  Item statusItem( item );
  statusItem.setFlag( Akonadi::MessageFlags::Replied );
  Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob( statusItem );
  modifyJob->setIgnorePayload( true );
}

void MainView::forward( quint64 id, ForwardMode mode )
{
  ItemFetchJob *job = new ItemFetchJob( Item( id ), this );
  job->fetchScope().fetchFullPayload();
  job->setProperty( "forwardMode", QVariant::fromValue( mode ) );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( forwardFetchResult( KJob* ) ) );
}

void MainView::forwardFetchResult( KJob* job )
{
  const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetchJob->items().isEmpty() )
    return;

  const Item item = fetchJob->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( MobileKernel::self()->identityManager() );

  ComposerView *composer = new ComposerView;
  const ForwardMode mode = fetchJob->property( "forwardMode" ).value<ForwardMode>();
  switch ( mode ) {
    case InLine:
      composer->setMessage( factory.createForward() );
      break;
    case AsAttachment: {
      QPair< KMime::Message::Ptr, QList< KMime::Content* > > forwardMessage = factory.createAttachedForward( QList< KMime::Message::Ptr >() << item.payload<KMime::Message::Ptr>());
      //the invokeMethods are there to be sure setMessage and addAttachment is called after composer->delayedInit
      QMetaObject::invokeMethod( composer, "setMessage", Qt::QueuedConnection, Q_ARG( KMime::Message::Ptr, forwardMessage.first ) );
      foreach ( KMime::Content* attach, forwardMessage.second )
        QMetaObject::invokeMethod( composer, "addAttachment", Qt::QueuedConnection, Q_ARG( KMime::Content*, attach ) );
      break;
    }
    case Redirect:
      composer->setMessage( factory.createRedirect( "" ) );
      break;
  }

  composer->show();

  Item statusItem( item );
  statusItem.setFlag( Akonadi::MessageFlags::Forwarded );
  Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob( statusItem );
  modifyJob->setIgnorePayload( true );
}

void MainView::markImportant( bool checked )
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  MessageStatus status;
  status.setStatusFromFlags( item.flags() );
  if ( checked && status.isImportant() )
    return;

  if ( checked )
    status.setImportant();
  else
    status.setImportant( false );

  item.setFlags( status.statusFlags() );

  ItemModifyJob *job = new ItemModifyJob( item );
  job->setIgnorePayload( true );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( modifyDone( KJob* ) ) );
}

void MainView::markMailTask( bool checked )
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  MessageStatus status;
  status.setStatusFromFlags( item.flags() );
  if ( checked && status.isToAct() )
    return;

  if ( checked )
    status.setToAct();
  else
    status.setToAct( false );

  item.setFlags( status.statusFlags() );

  ItemModifyJob *job = new ItemModifyJob( item );
  job->setIgnorePayload( true );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( modifyDone( KJob* ) ) );
}

void MainView::replyToMessage()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplySmart );
}

void MainView::replyWithoutQuoting()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplySmart, false);
}

void MainView::replyToAll()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  reply( item.id(), MessageComposer::ReplyAll );
}

void MainView::forwardMessage()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), InLine );
}

void MainView::forwardAsAttachment()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), AsAttachment );
}

void MainView::redirect()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  forward( item.id(), Redirect );
}

Item MainView::currentItem() const
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if ( list.size() != 1 )
    return Item();

  const QModelIndex index = list.first();
  const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return Item();

  return item;
}

void MainView::modifyDone( KJob *job )
{
  if ( job->error() ) {
    kWarning() << "Modify error: " << job->errorString();
    //###: review error string
    //## Use a notification instead?
    KMessageBox::sorry( this,
                        i18n( "Error trying to set item status" ),
                        i18n( "Messages status error" ) );
    return;
  }
}

void MainView::dataChanged()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  MessageStatus status;
  status.setStatusFromFlags( item.flags() );

  actionCollection()->action( "mark_message_important" )->setChecked( status.isImportant() );
  actionCollection()->action( "mark_message_action_item" )->setChecked( status.isToAct() );
}

void MainView::configureIdentity()
{
#ifdef _WIN32_WCE
  KComponentData instance( "kcmkmail_config_identity" ); // keep in sync with kmail for now to reuse kmail translations until after the string freeze
  KMail::IdentityPage *page = new KMail::IdentityPage( instance, this );
  page->setObjectName( "kcm_kpimidentities" );

  KDialog dialog( this );
  dialog.setMainWidget( page );
  dialog.setButtons( KDialog::Ok | KDialog::Cancel );
  dialog.setWindowState( Qt::WindowFullScreen );
  connect( &dialog, SIGNAL( okClicked() ), page, SLOT( save() ) );
  dialog.exec();
#else
  KCMultiDialog dlg;
  dlg.addModule( "kcm_kpimidentities" );
  dlg.currentPage()->setHeader( QLatin1String( "" ) ); // hide header to save space
  dlg.setButtons( KDialog::Ok | KDialog::Cancel );
  dlg.exec();
#endif
}

bool MainView::isDraft( int row )
{
  static const int column = 0;
  const QModelIndex index = itemSelectionModel()->model()->index( row, column );
  kDebug() << "itemSelectionModel " << itemSelectionModel() << " model" << itemSelectionModel()->model() << " idx->model()" << index.model();
  itemSelectionModel()->select( QItemSelection( index, index ), QItemSelectionModel::ClearAndSelect );

  const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  return folderIsDrafts( item.parentCollection() );
}

bool MainView::isDraftThreadContent( int row )
{
  static const int column = 0;
  const QModelIndex index = m_threadContentsModel->index( row, column );

  const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  return folderIsDrafts( item.parentCollection() );
}


bool MainView::isDraftThreadRoot( int row )
{
  static const int column = 0;
  const QModelIndex index = m_threadsModel->index( row, column );

  const int threadSize = index.data(ThreadModel::ThreadSizeRole).toInt();
  if (threadSize != 1)
    return false;

  const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  return folderIsDrafts( item.parentCollection() );
}

bool MainView::isSingleMessage(int row)
{
  static const int column = 0;
  const QModelIndex index = m_threadsModel->index( row, column );

  const int threadSize = index.data(ThreadModel::ThreadSizeRole).toInt();
  return threadSize == 1;
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
  findCreateDefaultCollection( SpecialMailCollections::Templates );

}

void MainView::findCreateDefaultCollection( SpecialMailCollections::Type type )
{
  if ( SpecialMailCollections::self()->hasDefaultCollection( type ) ) {
    const Collection collection = SpecialMailCollections::self()->defaultCollection( type );
    if ( !( collection.rights() & Collection::AllRights ) )
      kDebug() << "You do not have read/write permission to your inbox folder";
  } else {
    SpecialMailCollectionsRequestJob *job =
        new SpecialMailCollectionsRequestJob( this );

    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( createDefaultCollectionDone( KJob* ) ) );
    job->requestDefaultCollection( type );
    job->setProperty("TYPE", (int) type );
  }
}

void MainView::createDefaultCollectionDone( KJob *job )
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

  if ( (SpecialMailCollections::Type)( job->property("TYPE").toInt() ) == SpecialMailCollections::Templates ) {
    mTemplateSelectionModel->select( EntityTreeModel::modelIndexForCollection( entityTreeModel(), CommonKernel->templatesCollectionFolder() ),  QItemSelectionModel::Select   );
  }

  SpecialMailCollectionsRequestJob *requestJob =
      qobject_cast<SpecialMailCollectionsRequestJob*>( job );

  const Collection collection = requestJob->collection();
  if ( !( collection.rights() & Collection::AllRights ) )
    kDebug() << "You do not have read/write permission to your inbox folder.";

  connect( SpecialMailCollections::self(), SIGNAL( defaultCollectionsChanged() ),
           this, SLOT( initDefaultFolders() ), Qt::UniqueConnection );

  folderChanged(); //call here, as e.g trash folders cannot be detected before the special collections are set up
}

bool MainView::folderIsDrafts( const Collection &collection )
{
  const Collection defaultDraftCollection = SpecialMailCollections::self()->defaultCollection( SpecialMailCollections::Drafts );

  // check if this is the default draft folder
  if ( collection == defaultDraftCollection )
    return true;

  // check for invalid collection
  const QString idString = QString::number( collection.id() );
  if ( idString.isEmpty() )
    return false;

  // search the identities if the folder matches the drafts-folder
  const KPIMIdentities::IdentityManager *im = MobileKernel::self()->identityManager();
  for ( KPIMIdentities::IdentityManager::ConstIterator it = im->begin(); it != im->end(); ++it ) {
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
                        i18n( "Cannot delete draft." ),
                        i18n( "Delete Draft Error" ) );
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
   << StandardMailActionManager::RemoveDuplicates << StandardMailActionManager::EmptyAllTrash << StandardMailActionManager::EmptyTrash;

  Q_FOREACH( StandardMailActionManager::Type mailAction, mailActions ) {
    mMailActionManager->createAction( mailAction );
  }

  mMailActionManager->interceptAction( StandardActionManager::CreateResource );

  connect( mMailActionManager->action( StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  const QStringList pages = QStringList() << QLatin1String( "MailCommon::CollectionGeneralPage" )
                                          << QLatin1String( "Akonadi::CachePolicyPage" );

  mMailActionManager->setCollectionPropertiesPageNames( pages );

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

  connect( collectionSelectionModel, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( folderChanged() ) );
  connect( itemActionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( itemActionModelChanged() ) );
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

QAbstractProxyModel* MainView::createMainProxyModel() const
{
  return mQuotaColorProxyModel;
}

QAbstractProxyModel* MainView::createItemFilterModel() const
{
  return new EmailsFilterProxyModel();
}

ImportHandlerBase* MainView::importHandler() const
{
  return new EmailsImportHandler();
}

ExportHandlerBase* MainView::exportHandler() const
{
  return new EmailsExportHandler();
}

GuiStateManager* MainView::createGuiStateManager() const
{
  return new EmailsGuiStateManager();
}

void MainView::saveMessage()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

//See the header file for SaveMailCommand why is it here
  SaveMailCommand *command = new SaveMailCommand( item, this );
  command->execute();
}

void MainView::itemSelectionChanged()
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1) {
    // TODO Clear messageViewerItem
    return;
  }

  const QModelIndex itemIdx = list.first();
  const Akonadi::Collection parentCol = itemIdx.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
  Q_ASSERT(parentCol.isValid());
  QModelIndex index = EntityTreeModel::modelIndexForCollection(entityTreeModel(), parentCol);
  Q_ASSERT(index.isValid());

  QString path;
  while ( index.isValid() ) {
    path.prepend( index.data().toString() );
    index = index.parent();
    if ( index.isValid() )
      path.prepend( " / " );
  }

  if (messageViewerItem()) {
    const Akonadi::Item item = itemIdx.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();
    messageViewerItem()->setItem(item);
    messageViewerItem()->setMessagePath(path);
  }
}

void MainView::collectionSelectionChanged()
{
  const QModelIndexList indexes = regularSelectionModel()->selectedIndexes();
  if ( indexes.isEmpty() )
    return;

  const QModelIndex index = indexes.first();
  const Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
  if ( collection.isValid() ) {
    mAclEditor->setCollection( collection );
    mMessageListSettingsController->setCollection( collection );
  }
}

MessageViewer::MessageViewItem* MainView::messageViewerItem()
{
  MessageViewer::MessageViewItem* item = 0;

  QList<MessageViewer::MessageViewItem*> items = rootObject()->findChildren<MessageViewer::MessageViewItem* >();
  if ( !items.isEmpty() )
    item = items.first();

  return item;
}


void MainView::findInMessage()
{
  MessageViewer::MessageViewItem* item = messageViewerItem();
  if ( item ) {
    item->viewer()->slotFind();
  }
}

void MainView::preferHTML(bool useHtml)
{
  MessageViewer::MessageViewItem* item = messageViewerItem();

  if ( item ) {
    const QItemSelectionModel *collectionSelectionModel = regularSelectionModel();
    if ( collectionSelectionModel->selection().indexes().isEmpty() )
      return;

    const QModelIndexList selectedIndexes = collectionSelectionModel->selection().indexes();
    Q_FOREACH( const QModelIndex &index, selectedIndexes ) {
      Q_ASSERT( index.isValid() );

      const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
      Q_ASSERT( collection.isValid() );

      KSharedConfigPtr config = KSharedConfig::openConfig( "kmail-mobilerc" );
      KConfigGroup group( config, QString( "c%1" ).arg( collection.id() ) );
      group.writeEntry( "htmlMailOverride", useHtml );
    }

    item->viewer()->setHtmlOverride( useHtml );
  }
}

void MainView::loadExternalReferences(bool load)
{
  MessageViewer::MessageViewItem* item = messageViewerItem();

  if ( item ) {
    const QItemSelectionModel *collectionSelectionModel = regularSelectionModel();
    if ( collectionSelectionModel->selection().indexes().isEmpty() )
      return;

    const QModelIndexList selectedIndexes = collectionSelectionModel->selection().indexes();
    Q_FOREACH( const QModelIndex &index, selectedIndexes ) {
      Q_ASSERT( index.isValid() );

      const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
      Q_ASSERT( collection.isValid() );

      KSharedConfigPtr config = KSharedConfig::openConfig( "kmail-mobilerc" );
      KConfigGroup group( config, QString( "c%1" ).arg( collection.id() ) );
      group.writeEntry( "htmlLoadExternalOverride", load );
    }

    item->viewer()->setHtmlLoadExtOverride( load );
  }
}

void MainView::folderChanged()
{
  const QItemSelectionModel* collectionSelectionModel = regularSelectionModel();
  const QModelIndexList indexes = collectionSelectionModel->selection().indexes();
  if ( indexes.isEmpty() )
    return;

  //NOTE: not exactly correct if multiple folders are selected, although I don't know what to do then, as the action is not
  //a tri-state one (checked, unchecked, for some folders checked)
  bool htmlMailOverrideInAll = true;
  bool htmlLoadExternalOverrideInAll = true;

  KSharedConfigPtr config = KSharedConfig::openConfig( "kmail-mobilerc" );
  Q_FOREACH( const QModelIndex &index, collectionSelectionModel->selectedRows() ) {
    Q_ASSERT( index.isValid() );

    const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
    Q_ASSERT( collection.isValid() );

    KConfigGroup group( config, QString( "c%1" ).arg( collection.id() ) );
    if ( group.readEntry( "htmlMailOverride", false ) == false )
      htmlMailOverrideInAll = false;

    if ( group.readEntry( "htmlLoadExternalOverride", false ) == false )
      htmlLoadExternalOverrideInAll = false;
  }
  actionCollection()->action( "prefer_html_to_plain" )->setChecked( htmlMailOverrideInAll );
  actionCollection()->action( "load_external_ref" )->setChecked( htmlLoadExternalOverrideInAll );

  actionCollection()->action( "move_all_to_trash" )->setText( i18n( "Move All to Trash" ) );
  if ( indexes.count() == 1 ) {
    const QModelIndex index = collectionSelectionModel->selection().indexes().first();
    const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
    Q_ASSERT( collection.isValid() );

    if ( CommonKernel->folderIsTrash( collection ) )
      actionCollection()->action( "move_all_to_trash" )->setText( i18n( "Empty Trash" ) );
  }
}

void MainView::showExpireProperties()
{
  const QItemSelectionModel *collectionSelectionModel = regularSelectionModel();
  if ( collectionSelectionModel->selection().indexes().isEmpty() )
    return;

  const QModelIndex index = collectionSelectionModel->selection().indexes().first();
  const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
  Q_ASSERT( collection.isValid() );

  MailCommon::ExpiryPropertiesDialog *dlg = new MailCommon::ExpiryPropertiesDialog( this, MailCommon::FolderCollection::forCollection( collection ) );
  dlg->show();
}

void MainView::moveToOrEmptyTrash()
{
  const QItemSelectionModel *collectionSelectionModel = regularSelectionModel();
  const QModelIndexList indexes = collectionSelectionModel->selection().indexes();
  if ( indexes.isEmpty() )
    return;

  const QModelIndex index = indexes.first();
  const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
  Q_ASSERT( collection.isValid() );

  if ( indexes.count() == 1 && CommonKernel->folderIsTrash( collection ) ) {
    //empty trash
    kDebug() << "EMPTY TRASH";
    mMailActionManager->action( Akonadi::StandardMailActionManager::EmptyTrash )->trigger();
  } else {
    mMailActionManager->action( Akonadi::StandardMailActionManager::MoveAllToTrash )->trigger();
  }
}

void MainView::createToDo()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  MailCommon::Util::createTodoFromMail( item );
}

void MainView::createEvent()
{
  const Item item = currentItem();
  if ( !item.isValid() )
    return;

  MailCommon::Util::createEventFromMail( item );
}

void MainView::useFixedFont()
{
  MessageViewer::MessageViewItem* item = messageViewerItem();

  if ( item ) {
    bool fixedFont = MessageViewer::GlobalSettings::self()->useFixedFont();
    item->viewer()->setUseFixedFont( !fixedFont );
    item->viewer()->update( MessageViewer::Viewer::Force );
    MessageViewer::GlobalSettings::self()->setUseFixedFont( !fixedFont );
    MessageViewer::GlobalSettings::self()->writeConfig();
  }
}

int MainView::emailTemplateCount()
{
  return mEmailTemplateModel ? mEmailTemplateModel->rowCount() : 0;
}

void MainView::newMessageFromTemplate( int index )
{
  Akonadi::Item item = mEmailTemplateModel->index( index, 0 ).data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item );
  job->fetchScope().fetchFullPayload( true );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( templateFetchResult( KJob* ) ) );

}

void MainView::templateFetchResult( KJob* job)
{
  const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( job->error() || fetchJob->items().isEmpty() ) {
    kDebug() << "error!!";
    //###: review error string
    KMessageBox::sorry( this,
                        i18n( "Could not fetch template." ),
                        i18n( "Template Fetching Error" ) );
    return;
  }

  const Item item = fetchJob->items().first();

  KMime::Message::Ptr message = MessageCore::Util::message( item );
  KMime::Message::Ptr newMsg(new KMime::Message);
  newMsg->setContent( message->encodedContent() );
  newMsg->parse();
  // these fields need to be regenerated for the new message
  newMsg->removeHeader("Date");
  newMsg->removeHeader("Message-ID");
  ComposerView *composer = new ComposerView;
  composer->setMessage( newMsg );
  composer->show();
}

void MainView::updateConfig()
{
  mQuotaColorProxyModel->setWarningThreshold( Settings::self()->miscQuotaWarningThreshold() );
  mQuotaColorProxyModel->setWarningColor( Settings::self()->miscQuotaWarningColor() );

  MessageViewer::MessageViewItem *item = messageViewerItem();
  if ( item ) {
    item->viewer()->writeConfig();
    item->viewer()->readConfig(); // let CSS parser reread its config
  }
}

void MainView::applyFilters()
{
  Item::List items;

  foreach ( const QModelIndex &index, itemActionModel()->selectedRows() ) {
    const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();
    if ( item.isValid() )
      items << item;
  }

  FilterIf->filterManager()->applyFilters( items );
}

void MainView::itemActionModelChanged()
{
  const QModelIndexList indexes = itemActionModel()->selectedRows();
  actionCollection()->action( "apply_filters" )->setEnabled( !indexes.isEmpty() );
}

bool MainView::selectNextUnreadMessageInCurrentFolder()
{
  const QAbstractItemModel *model = itemModel();
  const QModelIndexList list = itemSelectionModel()->selectedRows();

  const QModelIndex currentIndex = (list.isEmpty() ? model->index( 0, 0 ) : list.first());

  const int rowCount = model->rowCount( QModelIndex() );

  // start from current message
  for ( int row = currentIndex.row() + 1; row < rowCount; ++row ) {
    const QModelIndex itemIndex = model->index( row, 0 );
    const Akonadi::Item item = itemIndex.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if ( !item.hasFlag( Akonadi::MessageFlags::Seen ) ) {
      messageViewerItem()->setItem( item );
      return true;
    }
  }

  // no unread message found, try from begin of folder
  for ( int row = 0; row < currentIndex.row(); ++row ) {
    const QModelIndex itemIndex = model->index( row, 0 );
    const Akonadi::Item item = itemIndex.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if ( !item.hasFlag( Akonadi::MessageFlags::Seen ) ) {
      messageViewerItem()->setItem( item );
      return true;
    }
  }

  return false; // no unread message in folder
}

void MainView::selectNextUnreadMessage()
{
  if ( selectNextUnreadMessageInCurrentFolder() )
    return;

  // since we passed a custom model in createMainProxyModel(), we have to use it here as well
  QAbstractItemModel *model = mQuotaColorProxyModel;

  // since there is no unread message left in current folder, try the next one
  QModelIndex next = MailCommon::Util::nextUnreadCollection( model, model->index( 0, 0 ), MailCommon::Util::ForwardSearch );
  if ( next.isValid() ) {
    regularSelectionModel()->setCurrentIndex( next, QItemSelectionModel::ClearAndSelect );
    selectNextUnreadMessageInCurrentFolder();
  }
}

void MainView::showMessageSource()
{
  MessageViewer::MessageViewItem *item = messageViewerItem();
  if ( item ) {
    item->viewer()->slotShowMessageSource();
  }
}

void MainView::selectOverrideEncoding()
{
  MessageViewer::MessageViewItem *item = messageViewerItem();
  if ( item ) {
    CharsetSelectionDialog dlg( this );
    dlg.setCharset( item->viewer()->overrideEncoding() );

    if ( dlg.exec() )
      item->viewer()->setOverrideEncoding( dlg.charset() );
  }
}

void MainView::toggleShowExtendedHeaders( bool value )
{
  MessageViewer::MessageViewItem *item = messageViewerItem();
  if ( item ) {
    if ( value )
      item->viewer()->setHeaderStyleAndStrategy( MessageViewer::HeaderStyle::mobileExtended(), MessageViewer::HeaderStrategy::all() );
    else
      item->viewer()->setHeaderStyleAndStrategy( MessageViewer::HeaderStyle::mobile(), MessageViewer::HeaderStrategy::all() );
  }
}

void MainView::messageListSettingsChanged( const MessageListSettings &settings )
{
  switch ( settings.sortingOption() ) {
    case MessageListSettings::SortByDateTime:
      m_grouperComparator->setSortingOption( MailThreadGrouperComparator::SortByDateTime );
      break;
    case MessageListSettings::SortByDateTimeMostRecent:
      m_grouperComparator->setSortingOption( MailThreadGrouperComparator::SortByDateTimeMostRecent );
      break;
    case MessageListSettings::SortBySenderReceiver:
      m_grouperComparator->setSortingOption( MailThreadGrouperComparator::SortBySenderReceiver );
      break;
    case MessageListSettings::SortBySubject:
      m_grouperComparator->setSortingOption( MailThreadGrouperComparator::SortBySubject );
      break;
    case MessageListSettings::SortBySize:
      m_grouperComparator->setSortingOption( MailThreadGrouperComparator::SortBySize );
      break;
  }

  m_threadGrouperModel->sort( 0, settings.sortingOrder() );
}

// #############################################################

#include "mainview.moc"
