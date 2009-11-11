/*
 * mailreader.cpp
 *
 * Copyright (C) 2008 Andras Mantia <amantia@kde.org>
 */
#include "mailreader.h"

#include <akonadi/changerecorder.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/session.h>
#include <akonadi/statisticsproxymodel.h>
#include <akonadi/kmime/messageparts.h>

#include <KDE/KAction>
#include <KDE/KConfigDialog>
#include <KDE/KActionCollection>
#include <KDE/KLocale>
#include <KDE/KStandardAction>
#include <KDE/KStatusBar>

#include <QtGui/QDockWidget>
#include <QtGui/QSortFilterProxyModel>

#include <messagelist/pane.h>


#include "mailreaderview.h"
#include "settings.h"


mailreader::mailreader()
    : KXmlGuiWindow(),
      m_view(new mailreaderView(this))
{
    // accept dnd
    setAcceptDrops(true);

    // tell the KXmlGuiWindow that this is indeed the main widget
    setCentralWidget(m_view);

    setupDocks();

    // then, setup our actions
    setupActions();

    // add a status bar
    statusBar()->show();

    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();
    
}

mailreader::~mailreader()
{
}

void mailreader::setupDocks()
{
  // Setup the core model
  Akonadi::Session *session = new Akonadi::Session( "AkonadiMailReader", this );

  Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );
  monitor->setCollectionMonitored( Akonadi::Collection::root() );
  monitor->fetchCollection( true );
  monitor->setMimeTypeMonitored( "message/rfc822", true );
  monitor->itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Header );

  Akonadi::EntityTreeModel *entityModel = new Akonadi::EntityTreeModel( session, monitor, this );
  entityModel->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );

  // Create the collection view
  Akonadi::EntityTreeView *collectionView = new Akonadi::EntityTreeView( 0, this );
  collectionView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  // Setup the message folders collection...
  Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel( this );
  collectionFilter->setSourceModel( entityModel );
  collectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  collectionFilter->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );

  // ... with statistics...
  Akonadi::StatisticsProxyModel *statisticsProxyModel = new Akonadi::StatisticsProxyModel( this );
  statisticsProxyModel->setToolTipEnabled( true );
  statisticsProxyModel->setExtraColumnsEnabled( false );
  statisticsProxyModel->setSourceModel( collectionFilter );

  // ... and sortable
  QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( this );
  sortModel->setDynamicSortFilter( true );
  sortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  sortModel->setSourceModel( statisticsProxyModel );

  // Use the model
  collectionView->setModel( sortModel );

  // Now make the message list multi-tab pane
  m_messagePane = new MessageList::Pane( entityModel, collectionView->selectionModel(), this );
  connect( m_messagePane, SIGNAL(messageSelected(Akonadi::Item)),
           this, SLOT(slotMessageSelected(Akonadi::Item)) );

  // Dock the message list view
  QDockWidget *messageListDock = new QDockWidget( i18n("Messages"), this );
  messageListDock->setObjectName( "Messages" );
  messageListDock->setWidget( m_messagePane );
  messageListDock->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
  addDockWidget( Qt::TopDockWidgetArea, messageListDock );

  // Dock the folder tree view
  QDockWidget *folderDock = new QDockWidget( i18n("Folders"), this );
  folderDock->setObjectName( "Folders" );
  folderDock->setWidget( collectionView );
  folderDock->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
  addDockWidget( Qt::LeftDockWidgetArea, folderDock );

  // Fine tuning on the dock policy (nesting + corners)
  setDockNestingEnabled( true );
  setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
  setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
}

void mailreader::setupActions()
{
    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());
    KStandardAction::preferences(m_view, SLOT(slotConfigure()), actionCollection());

    KAction *createTab = new KAction(KIcon("tab-new"),
                                      i18n("Open a new tab"),
                                      this);
    actionCollection()->addAction("new_tab", createTab);
    connect(createTab, SIGNAL(triggered(bool)),
            m_messagePane, SLOT(createNewTab()));


    m_previousMessage = new KAction(i18n("Previous Message"), this);
    actionCollection()->addAction("previous_message", m_previousMessage);
    connect(m_previousMessage, SIGNAL(triggered( bool )), SLOT(slotPreviousMessage()));
    m_nextMessage = new KAction(i18n("Next Message"), this);
    actionCollection()->addAction("next_message", m_nextMessage);
    connect(m_nextMessage, SIGNAL(triggered( bool )), SLOT(slotNextMessage()));
}

void mailreader::slotMessageSelected( const Akonadi::Item &item )
{
  // TODO: Use ETM/PartFetcher to cache fetched items.

  Akonadi::ItemFetchJob *itemFetchJob = new Akonadi::ItemFetchJob( item, this );
  itemFetchJob->fetchScope().fetchFullPayload( true );

  connect( itemFetchJob, SIGNAL(itemsReceived(Akonadi::Item::List)), SLOT(itemsReceived(Akonadi::Item::List)) );
  connect( itemFetchJob, SIGNAL(result(KJob *)), SLOT(itemFetchDone(KJob *)) );

  m_view->showItem( item );
}

void mailreader::itemsReceived(const Akonadi::Item::List &list )
{
  Q_ASSERT( list.size() == 1 );

  Akonadi::Item item = list.first();

  kDebug() << item.payloadData() << item.hasPayload<KMime::Message::Ptr>();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  m_view->showItem( item );
}

void mailreader::itemFetchDone(KJob *job)
{
  if (job->error())
    kDebug() << job->errorString();
}


void mailreader::slotPreviousMessage()
{
  m_messagePane->selectPreviousMessageItem( MessageList::Core::MessageTypeAny,
                                            MessageList::Core::ClearExistingSelection,
                                            true, true );
}

void mailreader::slotNextMessage()
{
  m_messagePane->selectNextMessageItem( MessageList::Core::MessageTypeAny,
                                        MessageList::Core::ClearExistingSelection,
                                        true, true );
}

#include "mailreader.moc"
