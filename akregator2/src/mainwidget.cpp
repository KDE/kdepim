/*
    This file is part of Akregator2.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "mainwidget.h"
#include "actionmanagerimpl.h"
#include "addfeeddialog.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "akregator2config.h"
#include "akregator2_part.h"
#include "browserframe.h"
#include "createfeedcommand.h"
#include "createfoldercommand.h"
#include "modifycommands.h"
#include "deletesubscriptioncommand.h"
#include "editfeedcommand.h"
#include "feedlistview.h"
#include "importfeedlistcommand.h"
#include "exportfeedlistcommand.h"
#include "framemanager.h"
#include "kernel.h"
#include "openurlrequest.h"
#include <libkdepim/progresswidget/progressmanager.h>

#include "searchbar.h"
#include "selectioncontroller.h"
#include "speechclient.h"
#include "tabwidget.h"
#include "searchwindow.h"
#include "searchdescriptionattribute.h"
#include "types.h"

#include <Akonadi/AgentManager>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Collection>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Session>
#include <Akonadi/Control>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/AttributeFactory>
#include <akonadi/persistentsearchattribute.h>

#include <krss/feedcollection.h>
#include <KRss/Item>
#include <KRss/FeedItemModel>

#include <solid/networking.h>

#include <kaction.h>
#include <kdialog.h>
#include <KDebug>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krandom.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <ktoggleaction.h>
#include <ktoolinvocation.h>
#include <kurl.h>


#include <QClipboard>
#include <QPixmap>
#include <QSplitter>
#include <QTextDocument>
#include <QDomDocument>
#include <QTimer>

#include <algorithm>
#include <memory>
#include <cassert>
#include <boost/shared_ptr.hpp>

using namespace boost;
using namespace Akregator2;
using namespace Solid;
using std::auto_ptr;
using boost::weak_ptr;

class MainWidget::Private {
    MainWidget* const q;
public:
    explicit Private( MainWidget* qq ) : q( qq ) {}
    void setUpAndStart( Command* cmd ) {
        cmd->setParentWidget( q );
        cmd->start();
    }
};

Akregator2::MainWidget::~MainWidget()
{
    // if m_shuttingDown is false, slotOnShutdown was not called. That
     // means that not the whole app is shutdown, only the part. So it
    // should be no risk to do the cleanups now
    if (!m_shuttingDown)
        slotOnShutdown();
    delete d;
}

Akregator2::MainWidget::MainWidget( Part *part, QWidget *parent, ActionManagerImpl* actionManager)
     : QWidget(parent),
     d( new Private( this ) ),
     m_viewMode(NormalView),
     m_actionManager(actionManager)
{
    Akonadi::AttributeFactory::registerAttribute<Akregator2::SearchDescriptionAttribute>();
    Akonadi::AttributeFactory::registerAttribute<Akonadi::PersistentSearchAttribute>();

    m_actionManager->initMainWidget(this);
    m_actionManager->initFrameManager(Kernel::self()->frameManager());
    m_part = part;
    m_shuttingDown = false;
    m_displayingAboutPage = false;
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *lt = new QVBoxLayout( this );
    lt->setMargin(0);

    m_horizontalSplitter = new QSplitter(Qt::Horizontal, this);

    m_horizontalSplitter->setOpaqueResize(true);
    lt->addWidget(m_horizontalSplitter);

    m_feedListView = new KRss::FeedListView( m_horizontalSplitter );
    m_actionManager->initFeedListView( m_feedListView );

#if 0
    connect(m_feedListView, SIGNAL(signalDropped (KUrl::List &, Akregator2::TreeNode*,
            Akregator2::Folder*)),
            this, SLOT(slotFeedUrlDropped (KUrl::List &,
            Akregator2::TreeNode*, Akregator2::Folder*)));
#endif

    m_tabWidget = new TabWidget(m_horizontalSplitter);
    m_actionManager->initTabWidget(m_tabWidget);

    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_tabWidget, SLOT(slotSettingsChanged()));

    connect( m_tabWidget, SIGNAL(signalCurrentFrameChanged(int)),
             Kernel::self()->frameManager(), SLOT(slotChangeFrame(int)));

    connect( m_tabWidget, SIGNAL(signalRemoveFrameRequest(int)),
             Kernel::self()->frameManager(), SLOT(slotRemoveFrame(int)));

    connect( m_tabWidget, SIGNAL(signalOpenUrlRequest(Akregator2::OpenUrlRequest&)),
             Kernel::self()->frameManager(), SLOT(slotOpenUrlRequest(Akregator2::OpenUrlRequest&)));

    connect( Kernel::self()->frameManager(), SIGNAL(signalFrameAdded(Akregator2::Frame*)),
             m_tabWidget, SLOT(slotAddFrame(Akregator2::Frame*)));

    connect( Kernel::self()->frameManager(), SIGNAL(signalSelectFrame(int)),
             m_tabWidget, SLOT(slotSelectFrame(int)) );

    connect( Kernel::self()->frameManager(), SIGNAL(signalFrameRemoved(int)),
             m_tabWidget, SLOT(slotRemoveFrame(int)));

    connect( Kernel::self()->frameManager(), SIGNAL(signalRequestNewFrame(int&)),
             this, SLOT(slotRequestNewFrame(int&)) );

    m_tabWidget->setWhatsThis( i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new QWidget(this);
    m_mainTab->setObjectName("Article Tab");
    m_mainTab->setWhatsThis( i18n("Articles list."));

    QVBoxLayout *mainTabLayout = new QVBoxLayout( m_mainTab);
    mainTabLayout->setMargin(0);

    m_searchBar = new SearchBar(m_mainTab);
    if ( !Settings::showQuickFilter() )
        m_searchBar->hide();

    mainTabLayout->addWidget(m_searchBar);

    m_articleSplitter = new QSplitter(Qt::Vertical, m_mainTab);
    m_articleSplitter->setObjectName("panner2");

    m_articleListView = new ArticleListView( KConfigGroup( Settings::self()->config(), "General" ),  m_articleSplitter );

    m_session = new Akonadi::Session( QByteArray( "Akregator2-" ) + QByteArray::number( qrand() ) );

    Akonadi::ItemFetchScope iscope;
    iscope.fetchPayloadPart( KRss::Item::HeadersPart );
    iscope.fetchAttribute<Akonadi::EntityDisplayAttribute>();
    Akonadi::CollectionFetchScope cscope;
    cscope.setIncludeStatistics( true );
    cscope.setContentMimeTypes( QStringList() << KRss::Item::mimeType() );
    Akonadi::ChangeRecorder* recorder = new Akonadi::ChangeRecorder( this );
    recorder->setSession( m_session );
    recorder->fetchCollection( true );
    recorder->setAllMonitored();
    recorder->setCollectionFetchScope( cscope );
    recorder->setItemFetchScope( iscope );
    recorder->fetchCollectionStatistics( true );
    recorder->setCollectionMonitored( Akonadi::Collection::root() );
    recorder->setMimeTypeMonitored( KRss::Item::mimeType() );
    m_itemModel = new KRss::FeedItemModel( recorder, this );

    m_selectionController = new SelectionController( m_session, m_itemModel, this );
    m_selectionController->setArticleLister( m_articleListView );
    m_selectionController->setFeedSelector( m_feedListView );
    connect( m_selectionController, SIGNAL(totalUnreadCountChanged(int)), this, SIGNAL(signalUnreadCountChanged(int)) );
    connect(m_searchBar, SIGNAL(signalSearch(std::vector<boost::shared_ptr<const Akregator2::Filters::AbstractMatcher> >)),
            m_selectionController, SLOT(setFilters(std::vector<boost::shared_ptr<const Akregator2::Filters::AbstractMatcher> >)) );

#ifdef KRSS_PORT_DISABLED
    FolderExpansionHandler* expansionHandler = new FolderExpansionHandler( this );
    connect( m_feedListView, SIGNAL(expanded(QModelIndex)), expansionHandler, SLOT(itemExpanded(QModelIndex)) );
    connect( m_feedListView, SIGNAL(collapsed(QModelIndex)), expansionHandler, SLOT(itemCollapsed(QModelIndex)) );

    m_selectionController->setFolderExpansionHandler( expansionHandler );
#else
    kWarning() << "Code temporarily disabled (Akonadi port)";
#endif //KRSS_PORT_DISABLED

    connect( m_selectionController, SIGNAL(currentCollectionChanged(Akonadi::Collection)),
             this, SLOT(slotNodeSelected(Akonadi::Collection)) );

    connect( m_selectionController, SIGNAL(currentItemChanged(Akonadi::Item)),
             this, SLOT(slotItemSelected(Akonadi::Item)) );

    connect( m_selectionController, SIGNAL(itemDoubleClicked(Akonadi::Item)),
             this, SLOT(slotOpenItemInBrowser(Akonadi::Item)) );

    m_actionManager->initArticleListView(m_articleListView);

    connect( m_articleListView, SIGNAL(signalMouseButtonPressed(int,KUrl)),
             this, SLOT(slotMouseButtonPressed(int,KUrl)));

/*
    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_articleListView, SLOT(slotPaletteOrFontChanged()));
*/

    m_articleViewer = new ArticleViewer(m_articleSplitter);
    m_actionManager->initArticleViewer(m_articleViewer);
    m_articleListView->setFocusProxy(m_articleViewer);

    connect( m_articleViewer, SIGNAL(signalOpenUrlRequest(Akregator2::OpenUrlRequest&)),
             Kernel::self()->frameManager(), SLOT(slotOpenUrlRequest(Akregator2::OpenUrlRequest&)) );
    connect( m_articleViewer->part()->browserExtension(), SIGNAL(mouseOverInfo(KFileItem)),
             this, SLOT(slotMouseOverInfo(KFileItem)) );
    connect( m_part, SIGNAL(signalSettingsChanged()),
             m_articleViewer, SLOT(slotPaletteOrFontChanged()));
    connect(m_searchBar, SIGNAL(signalSearch(std::vector<boost::shared_ptr<const Akregator2::Filters::AbstractMatcher> >)),
            m_articleViewer, SLOT(setFilters(std::vector<boost::shared_ptr<const Akregator2::Filters::AbstractMatcher> >)) );

    m_articleViewer->part()->widget()->setWhatsThis( i18n("Browsing area."));

    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame = new MainFrame( this, m_part, m_mainTab );
    m_mainFrame->slotSetTitle( i18n("Articles") );
    Kernel::self()->frameManager()->slotAddFrame(m_mainFrame);

    const QList<int> sp1sizes = Settings::splitter1Sizes();
    if ( sp1sizes.count() >= m_horizontalSplitter->count() )
        m_horizontalSplitter->setSizes( sp1sizes );
    const QList<int> sp2sizes = Settings::splitter2Sizes();
    if ( sp2sizes.count() >= m_articleSplitter->count() )
        m_articleSplitter->setSizes( sp2sizes );

    KConfigGroup conf(Settings::self()->config(), "General");

    if(!conf.readEntry("Disable Introduction", false))
    {
        m_articleListView->hide();
        m_searchBar->hide();
        m_articleViewer->displayAboutPage();
        m_mainFrame->slotSetTitle(i18n("About"));
        m_displayingAboutPage = true;
    }

    m_markReadTimer = new QTimer(this);
    m_markReadTimer->setSingleShot(true);
    connect(m_markReadTimer, SIGNAL(timeout()), this, SLOT(slotSetCurrentArticleReadDelayed()) );

    switch (Settings::viewMode())
    {
        case CombinedView:
            slotCombinedView();
            break;
        case WidescreenView:
            slotWidescreenView();
            break;
        default:
            slotNormalView();
    }

    if ( !Settings::resetQuickFilterOnNodeChange() )
    {
        m_searchBar->slotSetStatus( Settings::statusFilter() );
        m_searchBar->slotSetText( Settings::textFilter() );
    }
    const KConfigGroup group( Settings::self()->config(), "General" );
    m_feedListView->setConfigGroup( group );

    //Check network status
    if(Solid::Networking::status() == Solid::Networking::Connected ||Solid::Networking::status() == Solid::Networking::Unknown)
        this->m_networkAvailable=true;
    else if(Solid::Networking::status() == Solid::Networking::Unconnected)
        this->m_networkAvailable=false;

    Akonadi::Control::widgetNeedsAkonadi( this );
}

void Akregator2::MainWidget::slotOnShutdown()
{
    m_shuttingDown = true;

    // close all pageviewers in a controlled way
    // fixes bug 91660, at least when no part loading data
    while ( m_tabWidget->count() > 1 ) { // remove frames until only the main frame remains
        m_tabWidget->setCurrentIndex( m_tabWidget->count() - 1 ); // select last page
        m_tabWidget->slotRemoveCurrentFrame();
    }

    delete m_feedListView; // call delete here, so that the header settings will get saved
    delete m_articleListView; // same for this one

    delete m_mainTab;
    delete m_mainFrame;

    Settings::self()->writeConfig();
}


void Akregator2::MainWidget::saveSettings()
{
    const QList<int> spl1 = m_horizontalSplitter->sizes();
    if ( std::count( spl1.begin(), spl1.end(), 0 ) == 0 )
        Settings::setSplitter1Sizes( spl1 );
    const QList<int> spl2 = m_articleSplitter->sizes();
    if ( std::count( spl2.begin(), spl2.end(), 0 ) == 0 )
        Settings::setSplitter2Sizes( spl2 );
    Settings::setViewMode( m_viewMode );
    Settings::self()->writeConfig();
}


void Akregator2::MainWidget::slotRequestNewFrame(int& frameId)
{
    BrowserFrame* frame = new BrowserFrame(m_tabWidget);

    connect( m_part, SIGNAL(signalSettingsChanged()), frame, SLOT(slotPaletteOrFontChanged()));
    connect( m_tabWidget, SIGNAL(signalZoomInFrame(int)), frame, SLOT(slotZoomIn(int)));
    connect( m_tabWidget, SIGNAL(signalZoomOutFrame(int)), frame, SLOT(slotZoomOut(int)));

    Kernel::self()->frameManager()->slotAddFrame(frame);

    frameId = frame->id();
}

void Akregator2::MainWidget::sendArticle(bool attach)
{
    QByteArray text;
    QString title;

    Frame* frame = Kernel::self()->frameManager()->currentFrame();

    if (frame && frame->id() > 0) { // are we in some other tab than the articlelist?
        text = frame->url().prettyUrl().toLatin1();
        title = frame->title();
    }
    else { // nah, we're in articlelist..
         const Akonadi::Item aitem = m_selectionController->currentItem();
         if(aitem.isValid()) {
             const KRss::Item item = aitem.payload<KRss::Item>();
             text = KUrl( item.link() ).prettyUrl().toLatin1();
             title = item.title();
         }
    }

    if(text.isEmpty())
        return;

    if(attach)
    {
        KToolInvocation::invokeMailer(QString(),
                           QString(),
                           QString(),
                           title,
                           QString(),
                           QString(),
                           QStringList(text),
                           text);
    }
    else
    {
        KToolInvocation::invokeMailer(QString(),
                           QString(),
                           QString(),
                           title,
                           text,
                           QString(),
                           QStringList(),
                           text);
    }
}

void MainWidget::slotImportFeedList()
{
    KRss::FeedCollection c( m_selectionController->selectedCollection() );
    if ( c.isValid() && !c.isFolder() )
        c = KRss::FeedCollection( c.parentCollection() );
    if ( !c.isValid() ) { //TODO
        KMessageBox::error( this, i18n("Select a parent folder for the import.") );
        return;
    }

    std::auto_ptr<ImportFeedListCommand> cmd( new ImportFeedListCommand );
    cmd->setSession( m_session );
    cmd->setTargetCollection( c );
    d->setUpAndStart( cmd.release() );
}

void MainWidget::slotExportFeedList()
{
    const Akonadi::Collection c = m_selectionController->selectedCollection();
    if (!c.isValid()) { //TODO
        KMessageBox::error( this, i18n("Select a feed list to export.") );
        return;
    }
    std::auto_ptr<ExportFeedListCommand> cmd( new ExportFeedListCommand );
    cmd->setSession( m_session );
    cmd->setResource( c.resource() );
    d->setUpAndStart( cmd.release() );
}

void MainWidget::slotMetakitImport()
{
#ifdef KRSS_PORT_DISABLED
    std::auto_ptr<MigrateFeedsCommand> cmd( new MigrateFeedsCommand );
    cmd->setOpmlFile( KGlobal::dirs()->saveLocation("data", "akregator2/data") + "/feeds.opml" );
    d->setUpAndStart( cmd.release() );
#endif
}

void Akregator2::MainWidget::addFeedToGroup(const QString& url, const QString& groupName)
{
#ifdef KRSS_PORT_DISABLED
    // Locate the group.
    QList<TreeNode *> namedGroups = m_feedList->findByTitle( groupName );
    Folder* group = 0;
    foreach( TreeNode* const candidate, namedGroups ) {
        if ( candidate->isGroup() ) {
            group =  static_cast<Folder*>( candidate );
            break;
        }
    }

    if (!group)
    {
        Folder* g = new Folder( groupName );
        m_feedList->allFeedsFolder()->appendChild(g);
        group = g;
    }

#endif

    // Invoke the Add Feed dialog with url filled in.
    addFeed(url, true);
}

void Akregator2::MainWidget::slotNormalView()
{
    if (m_viewMode == NormalView)
        return;

    if (m_viewMode == CombinedView)
    {
        m_articleListView->show();

        const Akonadi::Item aitem = m_selectionController->currentItem();

        if ( aitem.isValid() )
            m_articleViewer->showItem( m_selectionController->selectedCollection(), aitem );
        else
            m_articleViewer->slotShowSummary( m_selectionController->selectedCollection(),
                                              m_selectionController->selectedCollectionIndex().data( Akonadi::EntityTreeModel::UnreadCountRole ).toInt() );
    }

    m_articleSplitter->setOrientation(Qt::Vertical);
    m_viewMode = NormalView;

    Settings::setViewMode( m_viewMode );
}

void Akregator2::MainWidget::slotWidescreenView()
{
    if (m_viewMode == WidescreenView)
        return;

    if (m_viewMode == CombinedView)
    {
        m_articleListView->show();

        const Akonadi::Item item = m_selectionController->currentItem();

        if ( item.isValid() )
            m_articleViewer->showItem( m_selectionController->selectedCollection(), item );
        else
            m_articleViewer->slotShowSummary( m_selectionController->selectedCollection(),
                                              m_selectionController->selectedCollectionIndex().data( Akonadi::EntityTreeModel::UnreadCountRole ).toInt() );
    }
    m_articleSplitter->setOrientation(Qt::Horizontal);
    m_viewMode = WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void Akregator2::MainWidget::slotCombinedView()
{
    if (m_viewMode == CombinedView)
        return;

    m_articleListView->slotClear();
    m_articleListView->hide();
    m_viewMode = CombinedView;

    Settings::setViewMode( m_viewMode );
}


void Akregator2::MainWidget::slotNodeSelected(const Akonadi::Collection& c)
{
    m_markReadTimer->stop();

    const int unread = m_selectionController->selectedCollectionIndex().data( Akonadi::EntityTreeModel::UnreadCountRole ).toInt();
    if (m_displayingAboutPage)
    {
        m_mainFrame->slotSetTitle(i18n("Articles"));
        if (m_viewMode != CombinedView)
            m_articleListView->show();
        if (Settings::showQuickFilter())
            m_searchBar->show();
        m_displayingAboutPage = false;
    }

    m_tabWidget->setCurrentWidget( m_mainFrame );
    if ( Settings::resetQuickFilterOnNodeChange() )
        m_searchBar->slotClearSearch();

    if (m_viewMode == CombinedView)
    {
        m_articleViewer->showNode( m_articleListView->model() );
    }
    else
    {
        m_articleViewer->slotShowSummary( c, unread );
    }

    if ( c.isValid() )
        m_mainFrame->setWindowTitle( KRss::FeedCollection( c ).title() );

    m_actionManager->slotNodeSelected(c);
}


void Akregator2::MainWidget::slotFeedAdd()
{
    addFeed(QString(), false);
}

static Akonadi::Collection findParentFolder( const Akonadi::Collection& c ) {
    if ( !c.isValid() || KRss::FeedCollection( c ).isFolder() )
        return c;
    else
        return c.parentCollection();
}

void Akregator2::MainWidget::addFeed(const QString& url, bool autoExec)
{
    std::auto_ptr<CreateFeedCommand> cmd( new CreateFeedCommand( m_session, this ) );
    cmd->setAutoExecute( autoExec );
    cmd->setUrl( url );
    const Akonadi::Collection parentCollection = findParentFolder( m_selectionController->selectedCollection() );
    cmd->setParentCollection( parentCollection );
    // FIXME: keep a shared pointer to the default resource in MainWidget
    d->setUpAndStart( cmd.release() );
}

void Akregator2::MainWidget::slotFolderAdd()
{
    bool ok;
    const QString name = KInputDialog::getText( i18n( "Add Folder" ),
                                                i18n( "Folder name:" ),
                                                QString(),
                                                &ok,
                                                this );
    if ( !ok )
    {
        return;
    }
    const Akonadi::Collection c = m_selectionController->selectedCollection();
    KRss::FeedCollection fc( c );
    if ( !fc.isFolder() )
        fc = KRss::FeedCollection( fc.parentCollection() );
    std::auto_ptr<CreateFolderCommand> cmd( new CreateFolderCommand( c, name, this ) );
    cmd->setSession( m_session );
    cmd->setFeedListView( m_feedListView );
    d->setUpAndStart( cmd.release() );
}

void Akregator2::MainWidget::slotFeedRemove()
{
    const Akonadi::Collection c = m_selectionController->selectedCollection();
    if ( !c.isValid() )
        return;

    if ( c.parentCollection() == Akonadi::Collection::root() )
        return;

    //PENDING(frank) make this a command and the message more sophisticated (feed vs. foldre etc.)

    if ( KMessageBox::questionYesNo( this,
                                     i18n("<qt>Are you sure you want to delete <strong>%1</strong>?</qt>", Qt::escape( KRss::FeedCollection( c ).title() ) ),
                                     i18n("Delete Feed"),
                                     KStandardGuiItem::del(),
                                     KStandardGuiItem::cancel() ) == KMessageBox::No )
        return;

    Akonadi::CollectionDeleteJob* job = new Akonadi::CollectionDeleteJob( c, m_session );
    job->start();
}

void Akregator2::MainWidget::slotFeedModify()
{
    const Akonadi::Collection c = m_selectionController->selectedCollection();
    if ( !c.isValid() )
        return;

    std::auto_ptr<EditFeedCommand> cmd( new EditFeedCommand );
    cmd->setCollection( c );
    cmd->setSession( m_session );
    d->setUpAndStart( cmd.release() );
}

void Akregator2::MainWidget::slotNextUnreadArticle()
{
    if (m_viewMode == CombinedView)
    {
        m_feedListView->slotNextUnreadFeed();
        return;
    }
    const QModelIndex c = m_selectionController->selectedCollectionIndex();

    if ( c.data( Akonadi::EntityTreeModel::UnreadCountRole ).toInt() > 0 )
        m_articleListView->slotNextUnreadArticle();
    else
        m_feedListView->slotNextUnreadFeed();
}

void Akregator2::MainWidget::slotPrevUnreadArticle()
{
    if (m_viewMode == CombinedView)
    {
        m_feedListView->slotPrevUnreadFeed();
        return;
    }
    const QModelIndex c = m_selectionController->selectedCollectionIndex();

    if ( c.data( Akonadi::EntityTreeModel::UnreadCountRole ).toInt() > 0 )
        m_articleListView->slotPreviousUnreadArticle();
    else
        m_feedListView->slotPrevUnreadFeed();
}

void Akregator2::MainWidget::slotMarkAllFeedsRead()
{
    const Akonadi::Collection::List l = m_selectionController->resourceRootCollections();
    if ( l.isEmpty() )
        return;

    MarkAsReadCommand* cmd = new MarkAsReadCommand( this );
    cmd->setCollections( l );
    cmd->setSession( m_session );
    d->setUpAndStart( cmd );
}

void Akregator2::MainWidget::slotMarkFeedRead()
{
    const Akonadi::Collection c = m_selectionController->selectedCollection();
    if ( !c.isValid() )
        return;

    MarkAsReadCommand* cmd = new MarkAsReadCommand( this );
    cmd->setCollections( Akonadi::Collection::List() << c );
    cmd->setSession( m_session );
    d->setUpAndStart( cmd );
}

void Akregator2::MainWidget::slotFetchCurrentFeed()
{
    if(isNetworkAvailable()) {
        const Akonadi::Collection c = m_selectionController->selectedCollection();
        if ( !c.isValid() )
            return;

        Akonadi::AgentManager::self()->synchronizeCollection( c );
    } else {
        m_mainFrame->slotSetStatusText(i18n("Networking is not available."));
    }
}

void Akregator2::MainWidget::slotFetchAllFeeds()
{
    if(isNetworkAvailable()) {
        const Akonadi::Collection::List l = m_selectionController->resourceRootCollections();
        if ( l.isEmpty() )
            return;
        Q_FOREACH( const Akonadi::Collection& i, l )
            Akonadi::AgentManager::self()->synchronizeCollection( i, true );
    } else {
        m_mainFrame->slotSetStatusText(i18n("Networking is not available."));
    }

}

void Akregator2::MainWidget::slotAbortFetches() {
    KPIM::ProgressManager::instance()->slotAbortAll();
}

void Akregator2::MainWidget::slotFetchQueueStarted()
{
    m_mainFrame->slotSetState(Frame::Started);
    m_actionManager->action("feed_stop")->setEnabled(true);
    m_mainFrame->slotSetStatusText(i18n("Fetching Feeds..."));
    m_actionManager->action( "feed_fetch_all" )->setEnabled( false );
}

void Akregator2::MainWidget::slotFetchQueueFinished()
{
    m_mainFrame->slotSetState(Frame::Completed);
    m_actionManager->action("feed_stop")->setEnabled(false);
    m_mainFrame->slotSetStatusText(QString());
    m_actionManager->action( "feed_fetch_all" )->setEnabled( true );
}

void Akregator2::MainWidget::slotItemSelected( const Akonadi::Item& item )
{
    if (m_viewMode == CombinedView)
        return;

    m_markReadTimer->stop();

    KToggleAction* const maai = qobject_cast<KToggleAction*>( m_actionManager->action( "article_set_status_important" ) );
    assert( maai );
    maai->setChecked( KRss::Item::isImportant( item ) );


    const Akonadi::Item::List items = m_selectionController->selectedItems();
    emit signalItemsSelected( items );

    m_articleViewer->showItem( m_selectionController->selectedCollection(), item );

    if ( !item.isValid() || KRss::Item::isRead( item ) )
        return;

    if ( !Settings::useMarkReadDelay() )
        return;

    const int delay = Settings::markReadDelay();

    if ( delay > 0 )
    {
        m_markReadTimer->start( delay * 1000 );
    }
    else
    {
        Akonadi::Item modifiedItem = item;
        modifiedItem.setFlag( KRss::Item::flagRead() );
        Akonadi::ItemModifyJob* job = new Akonadi::ItemModifyJob( modifiedItem, m_session );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(slotJobFinished(KJob*)) );
        job->setIgnorePayload( true );
        job->start();
    }
}

void Akregator2::MainWidget::slotMouseButtonPressed(int button, const KUrl& url)
{
    if (button != Qt::MidButton)
        return;

    if (!url.isValid())
        return;

    OpenUrlRequest req(url);

    switch (Settings::mMBBehaviour())
    {
        case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenUrlRequest::ExternalBrowser);
            break;
        case Settings::EnumMMBBehaviour::OpenInBackground:
            req.setOptions(OpenUrlRequest::NewTab);
            req.setOpenInBackground(true);
            break;
        default:
            req.setOptions(OpenUrlRequest::NewTab);
            req.setOpenInBackground(false);
    }

    Kernel::self()->frameManager()->slotOpenUrlRequest(req);
}

void Akregator2::MainWidget::slotOpenHomepage()
{
    const Akonadi::Collection c = m_selectionController->selectedCollection();
    if ( !c.isValid() )
        return;
    const KRss::FeedCollection fc( c );

    KUrl url( fc.htmlUrl() );
    if (url.isValid()) {
        OpenUrlRequest req( url );
        req.setOptions(OpenUrlRequest::ExternalBrowser);
        Kernel::self()->frameManager()->slotOpenUrlRequest(req);
    }
}

void Akregator2::MainWidget::slotOpenSelectedArticlesInBrowser()
{
    const Akonadi::Item::List items = m_selectionController->selectedItems();

    Q_FOREACH( const Akonadi::Item& i, items )
        slotOpenItemInBrowser( i );
}

void Akregator2::MainWidget::slotOpenItemInBrowser( const Akonadi::Item& aitem )
{
    if ( !aitem.isValid() )
        return;
    const KRss::Item item = aitem.payload<KRss::Item>();

    const KUrl link( item.link() );
    if ( !link.isValid() )
        return;

    OpenUrlRequest req( link );
    req.setOptions( OpenUrlRequest::ExternalBrowser );
    Kernel::self()->frameManager()->slotOpenUrlRequest( req );
}


void Akregator2::MainWidget::openSelectedArticles(bool openInBackground)
{
    const Akonadi::Item::List aitems = m_selectionController->selectedItems();

    Q_FOREACH( const Akonadi::Item& aitem, aitems )
    {
        Q_ASSERT( aitem.isValid() );
        if ( !aitem.isValid() )
            continue;

        const KRss::Item item = aitem.payload<KRss::Item>();

        const KUrl url( item.link() );
        if ( !url.isValid() )
          continue;

        OpenUrlRequest req( url );
        req.setOptions( OpenUrlRequest::NewTab );
        if( openInBackground ) {
            req.setOpenInBackground( true );
            Kernel::self()->frameManager()->slotOpenUrlRequest( req, false /*don't use settings for open in background*/ );
        } else {
            Kernel::self()->frameManager()->slotOpenUrlRequest( req );
        }
    }
}

void Akregator2::MainWidget::slotCopyLinkAddress()
{
    const Akonadi::Item aitem = m_selectionController->currentItem();

    if( !aitem.isValid() )
       return;

    const KRss::Item item = aitem.payload<KRss::Item>();

    if ( KUrl( item.link() ).isValid() )
    {
        const QString link = KUrl( item.link() ).url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(link, QClipboard::Clipboard);
        // don't set url to selection as it's a no-no according to a fd.o spec
        //cb->setText(link, QClipboard::Selection);
    }
}

#if 0
void Akregator2::MainWidget::slotFeedUrlDropped(KUrl::List &urls, TreeNode* after, Folder* parent)
{
    Q_FOREACH ( const KUrl& i, urls )
        addFeed( i.prettyUrl(), false );
}
#endif


void Akregator2::MainWidget::slotToggleShowQuickFilter()
{
    if ( Settings::showQuickFilter() )
    {
        Settings::setShowQuickFilter(false);
        m_searchBar->slotClearSearch();
        m_searchBar->hide();
    }
    else
    {
        Settings::setShowQuickFilter(true);
        if (!m_displayingAboutPage)
            m_searchBar->show();
    }

}

void Akregator2::MainWidget::slotArticleDelete()
{

    if ( m_viewMode == CombinedView )
        return;

    const Akonadi::Item::List items = m_selectionController->selectedItems();

    QString msg;
    switch (items.count())
    {
        case 0:
            return;
        case 1:
            msg = i18n("<qt>Are you sure you want to delete article <b>%1</b>?</qt>", Qt::escape(items.first().payload<KRss::Item>().title()));
            break;
        default:
            msg = i18np("<qt>Are you sure you want to delete the selected article?</qt>", "<qt>Are you sure you want to delete the %1 selected articles?</qt>", items.count());
    }

    if ( KMessageBox::warningContinueCancel( this,
                                             msg, i18n( "Delete Article" ),
                                             KStandardGuiItem::del(),
                                             KStandardGuiItem::cancel(),
                                             "Disable delete article confirmation" ) != KMessageBox::Continue )
        return;

    Q_FOREACH( const Akonadi::Item& i, m_selectionController->selectedItems() )
    {
        Akonadi::Item modifiedItem = i;
        modifiedItem.setFlag( KRss::Item::flagRead() );
        modifiedItem.setFlag( KRss::Item::flagDeleted() );
        Akonadi::ItemModifyJob* job = new Akonadi::ItemModifyJob( modifiedItem, m_session );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(slotJobFinished(KJob*)) );
        job->setIgnorePayload( true );
        job->start();
    }

    //Select the next item
    const QModelIndexList indexes = m_articleListView->selectionModel()->selectedRows();
    if ( indexes.size() == 1 ) {
        const QModelIndex nextItem = indexes[0].sibling( indexes[0].row()+1, 0 );
        m_articleListView->selectionModel()->select( nextItem, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }
}


void Akregator2::MainWidget::slotArticleToggleKeepFlag( bool )
{
    const Akonadi::Item::List items = m_selectionController->selectedItems();

    if ( items.isEmpty() )
        return;

    bool allFlagsSet = true;
    Q_FOREACH ( const Akonadi::Item& i, items )
    {
        allFlagsSet = allFlagsSet && KRss::Item::isImportant( i );
        if ( !allFlagsSet )
            break;
    }


    QList<Akonadi::Item> aitems;

    Q_FOREACH ( const Akonadi::Item& i, items )
    {
        if ( !allFlagsSet && !KRss::Item::isImportant( i ) ) {
            Akonadi::Item modifiedItem = i;
            modifiedItem.setFlag( KRss::Item::flagImportant() );
            aitems.append( modifiedItem );
        } else if ( allFlagsSet && KRss::Item::isImportant( i ) ) {
            Akonadi::Item modifiedItem = i;
            modifiedItem.clearFlag( KRss::Item::flagImportant() );
            aitems.append( modifiedItem );
        }
    }

    if ( aitems.isEmpty() )
        return;

    Akonadi::ItemModifyJob* job = new Akonadi::ItemModifyJob( aitems, m_session );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(slotJobFinished(KJob*)) );
    job->setIgnorePayload( true );
    job->start();

}

namespace {

static void setSelectedArticleStatus( Akonadi::Session* session, QObject* rec, const Akregator2::AbstractSelectionController* controller, Akregator2::ArticleStatus status )
{
    Akonadi::Item::List items = controller->selectedItems();

    if ( items.isEmpty() )
        return;

    Akonadi::Item::List changedItems;

    Q_FOREACH ( const Akonadi::Item& i, items )
    {
        switch ( status ) {
        case Akregator2::Read:
            if ( KRss::Item::isUnread( i ) ) {
                Akonadi::Item modifiedItem = i;
                modifiedItem.setFlag( KRss::Item::flagRead() );
                changedItems.append( modifiedItem );
            }
            break;
        case Akregator2::Unread:
            if ( KRss::Item::isRead( i ) ) {
                Akonadi::Item modifiedItem = i;
                modifiedItem.clearFlag( KRss::Item::flagRead() );
                changedItems.append( modifiedItem );
            }
            break;
        }
    }

    if ( changedItems.isEmpty() )
        return;

    Akonadi::ItemModifyJob* job = new Akonadi::ItemModifyJob( changedItems, session );
    rec->connect( job, SIGNAL(finished(KJob*)), rec, SLOT(slotJobFinished(KJob*)) );
    job->setIgnorePayload( true );
    job->start();
}

}

void Akregator2::MainWidget::slotSetSelectedArticleRead()
{
    ::setSelectedArticleStatus( m_session, this, m_selectionController, Akregator2::Read );
}

void Akregator2::MainWidget::slotTextToSpeechRequest()
{

    if (Kernel::self()->frameManager()->currentFrame() == m_mainFrame)
    {
        if (m_viewMode != CombinedView)
        {
            // in non-combined view, read selected articles
            SpeechClient::self()->slotSpeak(m_selectionController->selectedItems());
            // TODO: if article viewer has a selection, read only the selected text?
        }
        else
        {
            if (m_selectionController->selectedCollection().isValid())
            {
                //TODO: read articles in current node, respecting quick filter!
            }
        }
    }
    else
    {
        // TODO: read selected page viewer
    }
}

void Akregator2::MainWidget::slotSetSelectedArticleUnread()
{
    ::setSelectedArticleStatus( m_session, this, m_selectionController, Akregator2::Unread );
}

void Akregator2::MainWidget::slotSetCurrentArticleReadDelayed()
{
    Akonadi::Item item = m_selectionController->currentItem();

    if ( !item.isValid() )
        return;

    item.setFlag( KRss::Item::flagRead() );
    Akonadi::ItemModifyJob* job = new Akonadi::ItemModifyJob( item, m_session );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(slotJobFinished(KJob*)) );
    job->setIgnorePayload( true );
    job->start();
}

void Akregator2::MainWidget::slotMouseOverInfo(const KFileItem& kifi)
{
    m_mainFrame->slotSetStatusText( kifi.isNull() ? QString() : kifi.url().prettyUrl() );
}

void Akregator2::MainWidget::readProperties(const KConfigGroup &config)
{
    if ( !Settings::resetQuickFilterOnNodeChange() )
    {
        // read filter settings
        m_searchBar->slotSetText(config.readEntry("searchLine"));
        m_searchBar->slotSetStatus(config.readEntry("searchCombo").toInt());
    }
    // Reopen tabs
    QStringList childList = config.readEntry( QString::fromLatin1( "Children" ),
        QStringList() );
    Q_FOREACH( const QString& framePrefix, childList )
    {
        BrowserFrame* const frame = new BrowserFrame(m_tabWidget);
        frame->loadConfig( config, framePrefix + QLatin1Char( '_' ) );

        connect( m_part, SIGNAL(signalSettingsChanged()), frame, SLOT(slotPaletteOrFontChanged()));
        connect( m_tabWidget, SIGNAL(signalZoomInFrame(int)), frame, SLOT(slotZoomIn(int)));
        connect( m_tabWidget, SIGNAL(signalZoomOutFrame(int)), frame, SLOT(slotZoomOut(int)));

        Kernel::self()->frameManager()->slotAddFrame(frame);

    }
}

void Akregator2::MainWidget::saveProperties(KConfigGroup & config)
{
    // save filter settings
    config.writeEntry("searchLine", m_searchBar->text());
    config.writeEntry("searchCombo", m_searchBar->status());

    Kernel::self()->frameManager()->saveProperties(config);
}

void Akregator2::MainWidget::slotJobFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        KMessageBox::error( this, job->errorString() );
    }
}

void Akregator2::MainWidget::slotReloadAllTabs()
{
    m_tabWidget->slotReloadAllTabs();
}

void Akregator2::MainWidget::slotSearch()
{
    SearchWindow *window = new SearchWindow( m_itemModel, m_selectionController->selectedCollection(), this );
    window->show();
}

bool Akregator2::MainWidget::isNetworkAvailable() const
{
  return m_networkAvailable;
}

void Akregator2::MainWidget::slotNetworkStatusChanged(Solid::Networking::Status status)
{
  if(status==Solid::Networking::Connected || Solid::Networking::status() == Solid::Networking::Unknown)
  {
    m_networkAvailable=true;
    m_mainFrame->slotSetStatusText(i18n("Networking is available now."));
    this->slotFetchAllFeeds();
  }
  else if(Solid::Networking::Unconnected)
  {
    m_networkAvailable=false;
    m_mainFrame->slotSetStatusText(i18n("Networking is not available."));
  }
}

void Akregator2::MainWidget::slotOpenSelectedArticles()
{
    openSelectedArticles( false );
}


void Akregator2::MainWidget::slotOpenSelectedArticlesInBackground()
{
    openSelectedArticles( true );
}

