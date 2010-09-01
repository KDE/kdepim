/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "actionmanagerimpl.h"
#include "akregator_part.h"
#include "akregator_run.h"
#include "akregator_view.h"
#include "listtabwidget.h"
#include "addfeeddialog.h"
#include "propertiesdialog.h"
#include "frame.h"
#include "fetchqueue.h"
#include "feedlistview.h"
#include "articlelistview.h"
#include "articleviewer.h"
#include "viewer.h"
#include "feed.h"
#include "tagfolder.h"
#include "folder.h"
#include "feedlist.h"
#include "akregatorconfig.h"
#include "kernel.h"
#include "pageviewer.h"
#include "searchbar.h"
#include "speechclient.h"
#include "storage.h"
#include "tabwidget.h"
#include "tag.h"
#include "tagset.h"
#include "tagnode.h"
#include "tagnodelist.h"
#include "tagpropertiesdialog.h"
#include "treenode.h"
#include "progressmanager.h"
#include "treenodevisitor.h"
#include "notificationmanager.h"

#include <kaction.h>
#include <kapplication.h>
#include <kcharsets.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <kprocess.h>
#include <krun.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kxmlguifactory.h>
#include <kparts/partmanager.h>

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqdatetime.h> // for startup time measure
#include <tqfile.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqmultilineedit.h>
#include <tqpopupmenu.h>
#include <tqptrlist.h>
#include <tqstylesheet.h>
#include <tqtextstream.h>
#include <tqtimer.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <tqvaluevector.h>
#include <tqwhatsthis.h>
#include <tqclipboard.h>

namespace Akregator {

class View::EditNodePropertiesVisitor : public TreeNodeVisitor
{
    public:
        EditNodePropertiesVisitor(View* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            TagPropertiesDialog* dlg = new TagPropertiesDialog(m_view);
            dlg->setTag(node->tag());
            dlg->exec();
            delete dlg;
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            m_view->m_listTabWidget->activeView()->startNodeRenaming(node);
            return true;
        }

        virtual bool visitFeed(Feed* node)
        {
            FeedPropertiesDialog *dlg = new FeedPropertiesDialog( m_view, "edit_feed" );
            dlg->setFeed(node);
            dlg->exec();
            delete dlg;
            return true;
        }
    private:

        View* m_view;
};

class View::DeleteNodeVisitor : public TreeNodeVisitor
{
    public:
        DeleteNodeVisitor(View* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            TQString msg = i18n("<qt>Are you sure you want to delete tag <b>%1</b>? The tag will be removed from all articles.</qt>").arg(node->title());
            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Tag"), KStdGuiItem::del()) == KMessageBox::Continue)
            {
                Tag tag = node->tag();
                TQValueList<Article> articles = m_view->m_feedList->rootNode()->articles(tag.id());
                node->setNotificationMode(false);
                for (TQValueList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
                    (*it).removeTag(tag.id());
                node->setNotificationMode(true);
                Kernel::self()->tagSet()->remove(tag);
                m_view->m_listTabWidget->activeView()->setFocus();
            }
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            TQString msg;
            if (node->title().isEmpty())
                msg = i18n("<qt>Are you sure you want to delete this folder and its feeds and subfolders?</qt>");
            else
                msg = i18n("<qt>Are you sure you want to delete folder <b>%1</b> and its feeds and subfolders?</qt>").arg(node->title());

            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Folder"), KStdGuiItem::del()) == KMessageBox::Continue)
            {
                delete node;
                m_view->m_listTabWidget->activeView()->setFocus();
            }
            return true;
        }

        virtual bool visitFeed(Feed* node)
        {
            TQString msg;
            if (node->title().isEmpty())
                msg = i18n("<qt>Are you sure you want to delete this feed?</qt>");
            else
                msg = i18n("<qt>Are you sure you want to delete feed <b>%1</b>?</qt>").arg(node->title());

            if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Feed"), KStdGuiItem::del()) == KMessageBox::Continue)
            {
                delete node;
                m_view->m_listTabWidget->activeView()->setFocus();
            }
            return true;
        }
    private:

        View* m_view;
};


View::~View()
{
    // if m_shuttingDown is false, slotOnShutdown was not called. That
     // means that not the whole app is shutdown, only the part. So it
    // should be no risk to do the cleanups now
    if (!m_shuttingDown)
    {
        kdDebug() << "View::~View(): slotOnShutdown() wasn't called. Calling it now." << endl;
        slotOnShutdown();
    }
    kdDebug() << "View::~View(): leaving" << endl;
}

View::View( Part *part, TQWidget *parent, ActionManagerImpl* actionManager, const char *name)
 : TQWidget(parent, name), m_viewMode(NormalView), m_actionManager(actionManager)
{
    m_editNodePropertiesVisitor = new EditNodePropertiesVisitor(this);
    m_deleteNodeVisitor = new DeleteNodeVisitor(this);
    m_keepFlagIcon = TQPixmap(locate("data", "akregator/pics/akregator_flag.png"));
    m_part = part;
    m_feedList = new FeedList();
    m_tagNodeList = new TagNodeList(m_feedList, Kernel::self()->tagSet());
    m_shuttingDown = false;
    m_displayingAboutPage = false;
    m_currentFrame = 0L;
    setFocusPolicy(TQWidget::StrongFocus);

    TQVBoxLayout *lt = new TQVBoxLayout( this );

    m_horizontalSplitter = new TQSplitter(TQSplitter::Horizontal, this);

    m_horizontalSplitter->setOpaqueResize(true);
    lt->addWidget(m_horizontalSplitter);

    connect (Kernel::self()->fetchQueue(), TQT_SIGNAL(fetched(Feed*)), this, TQT_SLOT(slotFeedFetched(Feed*)));
    connect (Kernel::self()->fetchQueue(), TQT_SIGNAL(signalStarted()), this, TQT_SLOT(slotFetchingStarted()));
    connect (Kernel::self()->fetchQueue(), TQT_SIGNAL(signalStopped()), this, TQT_SLOT(slotFetchingStopped()));

    connect(Kernel::self()->tagSet(), TQT_SIGNAL(signalTagAdded(const Tag&)), this, TQT_SLOT(slotTagCreated(const Tag&)));
    connect(Kernel::self()->tagSet(), TQT_SIGNAL(signalTagRemoved(const Tag&)), this, TQT_SLOT(slotTagRemoved(const Tag&)));

    m_listTabWidget = new ListTabWidget(m_horizontalSplitter);
    m_actionManager->initListTabWidget(m_listTabWidget);

    connect(m_listTabWidget, TQT_SIGNAL(signalNodeSelected(TreeNode*)), this, TQT_SLOT(slotNodeSelected(TreeNode*)));

    if (!Settings::showTaggingGUI())
        m_listTabWidget->setViewMode(ListTabWidget::single);

    m_feedListView = new NodeListView( this, "feedtree" );
    m_listTabWidget->addView(m_feedListView, i18n("Feeds"), KGlobal::iconLoader()->loadIcon("folder", KIcon::Small));

    connect(m_feedListView, TQT_SIGNAL(signalContextMenu(KListView*, TreeNode*, const TQPoint&)), this, TQT_SLOT(slotFeedTreeContextMenu(KListView*, TreeNode*, const TQPoint&)));

    connect(m_feedListView, TQT_SIGNAL(signalDropped (KURL::List &, TreeNode*,
            Folder*)), this, TQT_SLOT(slotFeedURLDropped (KURL::List &,
            TreeNode*, Folder*)));

    m_tagNodeListView = new NodeListView(this);
    m_listTabWidget->addView(m_tagNodeListView, i18n("Tags"), KGlobal::iconLoader()->loadIcon("rss_tag", KIcon::Small));

    connect(m_tagNodeListView, TQT_SIGNAL(signalContextMenu(KListView*, TreeNode*, const TQPoint&)), this, TQT_SLOT(slotFeedTreeContextMenu(KListView*, TreeNode*, const TQPoint&)));


    ProgressManager::self()->setFeedList(m_feedList);

    m_tabs = new TabWidget(m_horizontalSplitter);
    m_actionManager->initTabWidget(m_tabs);

    connect( m_part, TQT_SIGNAL(signalSettingsChanged()), m_tabs, TQT_SLOT(slotSettingsChanged()));

    connect( m_tabs, TQT_SIGNAL( currentFrameChanged(Frame *) ), this,
            TQT_SLOT( slotFrameChanged(Frame *) ) );

    TQWhatsThis::add(m_tabs, i18n("You can view multiple articles in several open tabs."));

    m_mainTab = new TQWidget(this, "Article Tab");
    TQVBoxLayout *mainTabLayout = new TQVBoxLayout( m_mainTab, 0, 2, "mainTabLayout");

    TQWhatsThis::add(m_mainTab, i18n("Articles list."));

    m_searchBar = new SearchBar(m_mainTab);

    if ( !Settings::showQuickFilter() )
        m_searchBar->hide();

    mainTabLayout->addWidget(m_searchBar);

    m_articleSplitter = new TQSplitter(TQSplitter::Vertical, m_mainTab, "panner2");

    m_articleList = new ArticleListView( m_articleSplitter, "articles" );
    m_actionManager->initArticleListView(m_articleList);

    connect( m_articleList, TQT_SIGNAL(signalMouseButtonPressed(int, const Article&, const TQPoint &, int)), this, TQT_SLOT(slotMouseButtonPressed(int, const Article&, const TQPoint &, int)));

    // use selectionChanged instead of clicked
    connect( m_articleList, TQT_SIGNAL(signalArticleChosen(const Article&)),
                this, TQT_SLOT( slotArticleSelected(const Article&)) );
    connect( m_articleList, TQT_SIGNAL(signalDoubleClicked(const Article&, const TQPoint&, int)),
                this, TQT_SLOT( slotOpenArticleExternal(const Article&, const TQPoint&, int)) );

    m_articleViewer = new ArticleViewer(m_articleSplitter, "article_viewer");
    m_articleViewer->setSafeMode();  // disable JS, Java, etc...

    m_actionManager->initArticleViewer(m_articleViewer);

    connect(m_searchBar, TQT_SIGNAL(signalSearch(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)), m_articleList, TQT_SLOT(slotSetFilter(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)));

    connect(m_searchBar, TQT_SIGNAL(signalSearch(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)), m_articleViewer, TQT_SLOT(slotSetFilter(const Akregator::Filters::ArticleMatcher&, const Akregator::Filters::ArticleMatcher&)));

    connect( m_articleViewer, TQT_SIGNAL(urlClicked(const KURL&, Viewer*, bool, bool)),
             this, TQT_SLOT(slotUrlClickedInViewer(const KURL&, Viewer*, bool, bool)) );

    connect( m_articleViewer->browserExtension(), TQT_SIGNAL(mouseOverInfo(const KFileItem *)),
                                            this, TQT_SLOT(slotMouseOverInfo(const KFileItem *)) );

    connect( m_part, TQT_SIGNAL(signalSettingsChanged()), m_articleViewer, TQT_SLOT(slotPaletteOrFontChanged()));
    TQWhatsThis::add(m_articleViewer->widget(), i18n("Browsing area."));
    mainTabLayout->addWidget( m_articleSplitter );

    m_mainFrame=new Frame(this, m_part, m_mainTab, i18n("Articles"), false);
    connectFrame(m_mainFrame);
    m_tabs->addFrame(m_mainFrame);

    const TQValueList<int> sp1sizes = Settings::splitter1Sizes();
    if ( sp1sizes.count() >= m_horizontalSplitter->sizes().count() )
        m_horizontalSplitter->setSizes( sp1sizes );
    const TQValueList<int> sp2sizes = Settings::splitter2Sizes();
    if ( sp2sizes.count() >= m_articleSplitter->sizes().count() )
        m_articleSplitter->setSizes( sp2sizes );

    KConfig *conf = Settings::self()->config();
    conf->setGroup("General");
    if(!conf->readBoolEntry("Disable Introduction", false))
    {
        m_articleList->hide();
        m_searchBar->hide();
        m_articleViewer->displayAboutPage();
        m_mainFrame->setTitle(i18n("About"));
        m_displayingAboutPage = true;
    }

    m_fetchTimer = new TQTimer(this);
    connect( m_fetchTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotDoIntervalFetches()) );
    m_fetchTimer->start(1000*60);

    // delete expired articles once per hour
    m_expiryTimer = new TQTimer(this);
    connect(m_expiryTimer, TQT_SIGNAL(timeout()), this,
            TQT_SLOT(slotDeleteExpiredArticles()) );
    m_expiryTimer->start(3600*1000);

    m_markReadTimer = new TQTimer(this);
    connect(m_markReadTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotSetCurrentArticleReadDelayed()) );

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

    if (!Settings::resetQuickFilterOnNodeChange())
    {
        m_searchBar->slotSetStatus(Settings::statusFilter());
        m_searchBar->slotSetText(Settings::textFilter());
    }

    TQTimer::singleShot(1000, this, TQT_SLOT(slotDeleteExpiredArticles()) );
    m_part->mergePart(m_articleViewer);
}

void View::slotSettingsChanged()
{
    // if tagging is hidden, show only feed list
    m_listTabWidget->setViewMode(Settings::showTaggingGUI() ? ListTabWidget::verticalTabs : ListTabWidget::single);

}

void View::slotOnShutdown()
{
    m_shuttingDown = true; // prevents slotFrameChanged from crashing

    m_articleList->slotShowNode(0);
    m_articleViewer->slotShowNode(0);

    Kernel::self()->fetchQueue()->slotAbort();

    m_feedListView->setNodeList(0);
    ProgressManager::self()->setFeedList(0);

    delete m_feedList;
    delete m_tagNodeList;

    // close all pageviewers in a controlled way
    // fixes bug 91660, at least when no part loading data
    m_tabs->setCurrentPage(m_tabs->count()-1); // select last page
    while (m_tabs->count() > 1) // remove frames until only the main frame remains
        m_tabs->slotRemoveCurrentFrame();

    delete m_mainTab;
    delete m_mainFrame;
    delete m_editNodePropertiesVisitor;
    delete m_deleteNodeVisitor;
}

void View::saveSettings()
{
    const TQValueList<int> spl1 = m_horizontalSplitter->sizes();
    if ( spl1.contains( 0 ) == 0 )
        Settings::setSplitter1Sizes( spl1 );
    const TQValueList<int> spl2 = m_articleSplitter->sizes();
    if ( spl2.contains( 0 ) == 0 )
        Settings::setSplitter2Sizes( spl2 );
    Settings::setViewMode( m_viewMode );
    Settings::writeConfig();
}

void View::slotOpenNewTab(const KURL& url, bool background)
{
    PageViewer* page = new PageViewer(this, "page");

    connect( m_part, TQT_SIGNAL(signalSettingsChanged()), page, TQT_SLOT(slotPaletteOrFontChanged()));

    connect( page, TQT_SIGNAL(setTabIcon(const TQPixmap&)),
            this, TQT_SLOT(setTabIcon(const TQPixmap&)));
    connect( page, TQT_SIGNAL(urlClicked(const KURL &, Viewer*, bool, bool)),
            this, TQT_SLOT(slotUrlClickedInViewer(const KURL &, Viewer*, bool, bool)) );

    Frame* frame = new Frame(this, page, page->widget(), i18n("Untitled"));
    frame->setAutoDeletePart(true); // delete page viewer when removing the tab

    connect(page, TQT_SIGNAL(setWindowCaption (const TQString &)), frame, TQT_SLOT(setTitle (const TQString &)));
    connectFrame(frame);
    m_tabs->addFrame(frame);

    if(!background)
        m_tabs->showPage(page->widget());
    else
        setFocus();

    page->openURL(url);
}


void View::setTabIcon(const TQPixmap& icon)
{
    const PageViewer *s = dynamic_cast<const PageViewer*>(sender());
    if (s) {
        m_tabs->setTabIconSet(const_cast<PageViewer*>(s)->widget(), icon);
    }
}

void View::connectFrame(Frame *f)
{
    connect(f, TQT_SIGNAL(statusText(const TQString &)), this, TQT_SLOT(slotStatusText(const TQString&)));
    connect(f, TQT_SIGNAL(captionChanged (const TQString &)), this, TQT_SLOT(slotCaptionChanged (const TQString &)));
    connect(f, TQT_SIGNAL(loadingProgress(int)), this, TQT_SLOT(slotLoadingProgress(int)) );
    connect(f, TQT_SIGNAL(started()), this, TQT_SLOT(slotStarted()));
    connect(f, TQT_SIGNAL(completed()), this, TQT_SLOT(slotCompleted()));
    connect(f, TQT_SIGNAL(canceled(const TQString &)), this, TQT_SLOT(slotCanceled(const TQString&)));
}

void View::slotStatusText(const TQString &c)
{
    if (sender() == m_currentFrame)
        emit setStatusBarText(c);
}

void View::slotCaptionChanged(const TQString &c)
{
    if (sender() == m_currentFrame)
        emit setWindowCaption(c);
}

void View::slotStarted()
{
    if (sender() == m_currentFrame)
        emit signalStarted(0);
}

void View::slotCanceled(const TQString &s)
{
    if (sender() == m_currentFrame)
        emit signalCanceled(s);
}

void View::slotCompleted()
{
    if (sender() == m_currentFrame)
        emit signalCompleted();
}

void View::slotLoadingProgress(int percent)
{
    if (sender() == m_currentFrame)
        emit setProgress(percent);
}

bool View::importFeeds(const TQDomDocument& doc)
{
    FeedList* feedList = new FeedList();
    bool parsed = feedList->readFromXML(doc);

    // FIXME: parsing error, print some message
    if (!parsed)
    {
        delete feedList;
        return false;
    }
    TQString title = feedList->title();

    if (title.isEmpty())
        title = i18n("Imported Folder");

    bool ok;
    title = KInputDialog::getText(i18n("Add Imported Folder"), i18n("Imported folder name:"), title, &ok);

    if (!ok)
    {
        delete feedList;
        return false;
    }

    Folder* fg = new Folder(title);
    m_feedList->rootNode()->appendChild(fg);
    m_feedList->append(feedList, fg);

    return true;
}

bool View::loadFeeds(const TQDomDocument& doc, Folder* parent)
{
    FeedList* feedList = new FeedList();
    bool parsed = feedList->readFromXML(doc);

    // parsing went wrong
    if (!parsed)
    {
        delete feedList;
        return false;
    }
    m_feedListView->setUpdatesEnabled(false);
    m_tagNodeListView->setUpdatesEnabled(false);
    if (!parent)
    {
        TagSet* tagSet = Kernel::self()->tagSet();

        Kernel::self()->setFeedList(feedList);
        ProgressManager::self()->setFeedList(feedList);
        disconnectFromFeedList(m_feedList);
        delete m_feedList;
        delete m_tagNodeList;
        m_feedList = feedList;
        connectToFeedList(m_feedList);

        m_tagNodeList = new TagNodeList(m_feedList, tagSet);
        m_feedListView->setNodeList(m_feedList);
        m_tagNodeListView->setNodeList(m_tagNodeList);

        TQStringList tagIDs = m_feedList->rootNode()->tags();
        TQStringList::ConstIterator end = tagIDs.end();
        for (TQStringList::ConstIterator it = tagIDs.begin(); it != end; ++it)
        {
            kdDebug() << *it << endl;
            // create a tag for every tag ID in the archive that is not part of the tagset
            // this is a fallback in case the tagset was corrupted,
            // so the tagging information from archive does not get lost.
            if (!tagSet->containsID(*it))
            {
                Tag tag(*it, *it);
                tagSet->insert(tag);
            }
        }
    }
    else
        m_feedList->append(feedList, parent);

    m_feedListView->setUpdatesEnabled(true);
    m_feedListView->triggerUpdate();
    m_tagNodeListView->setUpdatesEnabled(true);
    m_tagNodeListView->triggerUpdate();
    return true;
}

void View::slotDeleteExpiredArticles()
{
    TreeNode* rootNode = m_feedList->rootNode();
    if (rootNode)
        rootNode->slotDeleteExpiredArticles();
}

TQDomDocument View::feedListToOPML()
{
    return m_feedList->toXML();
}

void View::addFeedToGroup(const TQString& url, const TQString& groupName)
{

    // Locate the group.
    TreeNode* node = m_feedListView->findNodeByTitle(groupName);

    Folder* group = 0;
    if (!node || !node->isGroup())
    {
        Folder* g = new Folder( groupName );
        m_feedList->rootNode()->appendChild(g);
        group = g;
    }
    else
        group = static_cast<Folder*>(node);

    // Invoke the Add Feed dialog with url filled in.
    if (group)
        addFeed(url, 0, group, true);
}

void View::slotNormalView()
{
    if (m_viewMode == NormalView)
    return;

    if (m_viewMode == CombinedView)
    {
        m_articleList->slotShowNode(m_listTabWidget->activeView()->selectedNode());
        m_articleList->show();

        Article article = m_articleList->currentArticle();

        if (!article.isNull())
            m_articleViewer->slotShowArticle(article);
        else
            m_articleViewer->slotShowSummary(m_listTabWidget->activeView()->selectedNode());
    }

    m_articleSplitter->setOrientation(TQSplitter::Vertical);
    m_viewMode = NormalView;

    Settings::setViewMode( m_viewMode );
}

void View::slotWidescreenView()
{
    if (m_viewMode == WidescreenView)
    return;

    if (m_viewMode == CombinedView)
    {
        m_articleList->slotShowNode(m_listTabWidget->activeView()->selectedNode());
        m_articleList->show();

        Article article = m_articleList->currentArticle();

        if (!article.isNull())
            m_articleViewer->slotShowArticle(article);
        else
            m_articleViewer->slotShowSummary(m_listTabWidget->activeView()->selectedNode());
    }

    m_articleSplitter->setOrientation(TQSplitter::Horizontal);
    m_viewMode = WidescreenView;

    Settings::setViewMode( m_viewMode );
}

void View::slotCombinedView()
{
    if (m_viewMode == CombinedView)
        return;

    m_articleList->slotClear();
    m_articleList->hide();
    m_viewMode = CombinedView;

    slotNodeSelected(m_listTabWidget->activeView()->selectedNode());
    Settings::setViewMode( m_viewMode );
}

void View::slotFrameChanged(Frame *f)
{
    if (m_shuttingDown)
        return;

    m_currentFrame=f;

    emit setWindowCaption(f->caption());
    emit setProgress(f->progress());
    emit setStatusBarText(f->statusText());

    if (f->part() == m_part)
        m_part->mergePart(m_articleViewer);
    else
        m_part->mergePart(f->part());

    f->widget()->setFocus();

    switch (f->state())
    {
        case Frame::Started:
            emit signalStarted(0);
            break;
        case Frame::Canceled:
            emit signalCanceled(TQString::null);
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            emit signalCompleted();
    }
}

void View::slotFeedTreeContextMenu(KListView*, TreeNode* /*node*/, const TQPoint& /*p*/)
{
    m_tabs->showPage(m_mainTab);
}

void View::slotMoveCurrentNodeUp()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current)
        return;
    TreeNode* prev = current->prevSibling();
    Folder* parent = current->parent();

    if (!prev || !parent)
        return;

    parent->removeChild(prev);
    parent->insertChild(prev, current);
    m_listTabWidget->activeView()->ensureNodeVisible(current);
}

void View::slotMoveCurrentNodeDown()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current)
        return;
    TreeNode* next = current->nextSibling();
    Folder* parent = current->parent();

    if (!next || !parent)
        return;

    parent->removeChild(current);
    parent->insertChild(current, next);
    m_listTabWidget->activeView()->ensureNodeVisible(current);
}

void View::slotMoveCurrentNodeLeft()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current || !current->parent() || !current->parent()->parent())
        return;

    Folder* parent = current->parent();
    Folder* grandparent = current->parent()->parent();

    parent->removeChild(current);
    grandparent->insertChild(current, parent);
    m_listTabWidget->activeView()->ensureNodeVisible(current);
}

void View::slotMoveCurrentNodeRight()
{
    TreeNode* current = m_listTabWidget->activeView()->selectedNode();
    if (!current || !current->parent())
        return;
    TreeNode* prev = current->prevSibling();

    if ( prev && prev->isGroup() )
    {
        Folder* fg = static_cast<Folder*>(prev);
        current->parent()->removeChild(current);
        fg->appendChild(current);
        m_listTabWidget->activeView()->ensureNodeVisible(current);
    }
}

void View::slotNodeSelected(TreeNode* node)
{
    m_markReadTimer->stop();

    if (node)
    {
        kdDebug() << "node selected: " << node->title() << endl;
        kdDebug() << "unread: " << node->unread() << endl;
        kdDebug() << "total: " << node->totalCount() << endl;
    }

    if (m_displayingAboutPage)
    {
        m_mainFrame->setTitle(i18n("Articles"));
        if (m_viewMode != CombinedView)
            m_articleList->show();
        if (Settings::showQuickFilter())
            m_searchBar->show();
        m_displayingAboutPage = false;
    }

    m_tabs->showPage(m_mainTab);

    if (Settings::resetQuickFilterOnNodeChange())
        m_searchBar->slotClearSearch();

    if (m_viewMode == CombinedView)
        m_articleViewer->slotShowNode(node);
    else
    {
        m_articleList->slotShowNode(node);
        m_articleViewer->slotShowSummary(node);
    }

    if (node)
        m_mainFrame->setCaption(node->title());

    m_actionManager->slotNodeSelected(node);

    updateTagActions();
}

void View::slotOpenURL(const KURL& url, Viewer* currentViewer, BrowserRun::OpeningMode mode)
{
    if (mode == BrowserRun::EXTERNAL)
        Viewer::displayInExternalBrowser(url);
    else
    {
         KParts::URLArgs args = currentViewer ? currentViewer->browserExtension()->urlArgs() : KParts::URLArgs();

        BrowserRun* r = new BrowserRun(this, currentViewer, url, args, mode);
        connect(r, TQT_SIGNAL(signalOpenInViewer(const KURL&, Akregator::Viewer*, Akregator::BrowserRun::OpeningMode)),
            this, TQT_SLOT(slotOpenURLReply(const KURL&, Akregator::Viewer*, Akregator::BrowserRun::OpeningMode)));
    }
}

//TODO: KDE4 remove this ugly ugly hack
void View::slotUrlClickedInViewer(const KURL& url, Viewer* viewer, bool newTab, bool background)
{

    if (!newTab)
    {
        slotOpenURL(url, viewer, BrowserRun::CURRENT_TAB);
    }
    else
    {
        slotOpenURL(url, 0L, background ? BrowserRun::NEW_TAB_BACKGROUND : BrowserRun::NEW_TAB_FOREGROUND);
    }
}

//TODO: KDE4 remove this ugly ugly hack
void View::slotOpenURLReply(const KURL& url, Viewer* currentViewer, BrowserRun::OpeningMode mode)
{
    switch (mode)
    {
        case BrowserRun::CURRENT_TAB:
            currentViewer->openURL(url);
            break;
        case BrowserRun::NEW_TAB_FOREGROUND:
        case BrowserRun::NEW_TAB_BACKGROUND:
            slotOpenNewTab(url, mode == BrowserRun::NEW_TAB_BACKGROUND);
            break;
        case BrowserRun::EXTERNAL:
            Viewer::displayInExternalBrowser(url);
            break;
    }
}

void View::slotFeedAdd()
{
    Folder* group = 0;
    if (!m_feedListView->selectedNode())
        group = m_feedList->rootNode(); // all feeds
    else
    {
        //TODO: tag nodes need rework
        if ( m_feedListView->selectedNode()->isGroup())
            group = static_cast<Folder*>(m_feedListView->selectedNode());
        else
            group= m_feedListView->selectedNode()->parent();

    }

    TreeNode* lastChild = group->children().last();

    addFeed(TQString::null, lastChild, group, false);
}

void View::addFeed(const TQString& url, TreeNode *after, Folder* parent, bool autoExec)
{

    AddFeedDialog *afd = new AddFeedDialog( 0, "add_feed" );

    afd->setURL(KURL::decode_string(url));

    if (autoExec)
        afd->slotOk();
    else
    {
        if (afd->exec() != TQDialog::Accepted)
        {
            delete afd;
            return;
        }
    }

    Feed* feed = afd->feed;
    delete afd;

    FeedPropertiesDialog *dlg = new FeedPropertiesDialog( 0, "edit_feed" );
    dlg->setFeed(feed);

    dlg->selectFeedName();

    if (!autoExec)
        if (dlg->exec() != TQDialog::Accepted)
        {
            delete feed;
            delete dlg;
            return;
        }

    if (!parent)
        parent = m_feedList->rootNode();

    parent->insertChild(feed, after);

    m_feedListView->ensureNodeVisible(feed);


    delete dlg;
}

void View::slotFeedAddGroup()
{
    TreeNode* node = m_feedListView->selectedNode();
    TreeNode* after = 0;

    if (!node)
        node = m_feedListView->rootNode();

    // if a feed is selected, add group next to it
    //TODO: tag nodes need rework
    if (!node->isGroup())
    {
        after = node;
        node = node->parent();
    }

    Folder* currentGroup = static_cast<Folder*> (node);

    bool Ok;

    TQString text = KInputDialog::getText(i18n("Add Folder"), i18n("Folder name:"), "", &Ok);

    if (Ok)
    {
        Folder* newGroup = new Folder(text);
        if (!after)
            currentGroup->appendChild(newGroup);
        else
            currentGroup->insertChild(newGroup, after);

        m_feedListView->ensureNodeVisible(newGroup);
    }
}

void View::slotFeedRemove()
{
    TreeNode* selectedNode = m_listTabWidget->activeView()->selectedNode();

    // don't delete root element! (safety valve)
    if (!selectedNode || selectedNode == m_feedList->rootNode())
        return;

    m_deleteNodeVisitor->visit(selectedNode);
}

void View::slotFeedModify()
{
    TreeNode* node = m_listTabWidget->activeView()->selectedNode();
    if (node)
        m_editNodePropertiesVisitor->visit(node);

}

void View::slotNextUnreadArticle()
{
    if (m_viewMode == CombinedView)
        m_listTabWidget->activeView()->slotNextUnreadFeed();

    TreeNode* sel = m_listTabWidget->activeView()->selectedNode();
    if (sel && sel->unread() > 0)
        m_articleList->slotNextUnreadArticle();
    else
        m_listTabWidget->activeView()->slotNextUnreadFeed();
}

void View::slotPrevUnreadArticle()
{
    if (m_viewMode == CombinedView)
        m_listTabWidget->activeView()->slotPrevUnreadFeed();

    TreeNode* sel = m_listTabWidget->activeView()->selectedNode();
    if (sel && sel->unread() > 0)
        m_articleList->slotPreviousUnreadArticle();
    else
        m_listTabWidget->activeView()->slotPrevUnreadFeed();
}

void View::slotMarkAllFeedsRead()
{
    m_feedList->rootNode()->slotMarkAllArticlesAsRead();
}

void View::slotMarkAllRead()
{
    if(!m_listTabWidget->activeView()->selectedNode()) return;
    m_listTabWidget->activeView()->selectedNode()->slotMarkAllArticlesAsRead();
}

void View::slotOpenHomepage()
{
    Feed* feed = dynamic_cast<Feed *>(m_listTabWidget->activeView()->selectedNode());

    if (!feed)
        return;

    KURL url = KURL(feed->htmlUrl())
;
    switch (Settings::lMBBehaviour())
    {
        case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
            slotOpenURL(url, 0, BrowserRun::EXTERNAL);
            break;
        case Settings::EnumLMBBehaviour::OpenInBackground:
            slotOpenURL(url, 0, BrowserRun::NEW_TAB_BACKGROUND);
            break;
        default:
            slotOpenURL(url, 0, BrowserRun::NEW_TAB_FOREGROUND);
    }
}

void View::slotSetTotalUnread()
{
    emit signalUnreadCountChanged( m_feedList->rootNode()->unread() );
}

void View::slotDoIntervalFetches()
{
    m_feedList->rootNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue(), true);
}

void View::slotFetchCurrentFeed()
{
    if ( !m_listTabWidget->activeView()->selectedNode() )
        return;
    m_listTabWidget->activeView()->selectedNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void View::slotFetchAllFeeds()
{
    m_feedList->rootNode()->slotAddToFetchQueue(Kernel::self()->fetchQueue());
}

void View::slotFetchingStarted()
{
    m_mainFrame->setState(Frame::Started);
    m_actionManager->action("feed_stop")->setEnabled(true);
    m_mainFrame->setStatusText(i18n("Fetching Feeds..."));
}

void View::slotFetchingStopped()
{
    m_mainFrame->setState(Frame::Completed);
    m_actionManager->action("feed_stop")->setEnabled(false);
    m_mainFrame->setStatusText(TQString::null);
}

void View::slotFeedFetched(Feed *feed)
{
    // iterate through the articles (once again) to do notifications properly
    if (feed->articles().count() > 0)
    {
        TQValueList<Article> articles = feed->articles();
        TQValueList<Article>::ConstIterator it;
        TQValueList<Article>::ConstIterator end = articles.end();
        for (it = articles.begin(); it != end; ++it)
        {
            if ((*it).status()==Article::New && ((*it).feed()->useNotification() || Settings::useNotifications()))
            {
                NotificationManager::self()->slotNotifyArticle(*it);
            }
        }
    }
}

void View::slotMouseButtonPressed(int button, const Article& article, const TQPoint &, int)
{
    if (button == Qt::MidButton)
    {
        KURL link = article.link();
        switch (Settings::mMBBehaviour())
        {
            case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
                slotOpenURL(link, 0L, BrowserRun::EXTERNAL);
                break;
            case Settings::EnumMMBBehaviour::OpenInBackground:
                slotOpenURL(link, 0L, BrowserRun::NEW_TAB_BACKGROUND);
                break;
            default:
                slotOpenURL(link, 0L, BrowserRun::NEW_TAB_FOREGROUND);
        }
    }
}

void View::slotAssignTag(const Tag& tag, bool assign)
{
    kdDebug() << (assign ? "assigned" : "removed") << " tag \"" << tag.id() << "\"" << endl;
    TQValueList<Article> selectedArticles = m_articleList->selectedArticles();
    for (TQValueList<Article>::Iterator it = selectedArticles.begin(); it != selectedArticles.end(); ++it)
    {
        if (assign)
            (*it).addTag(tag.id());
        else
            (*it).removeTag(tag.id());
    }
    updateTagActions();
}
/*
void View::slotRemoveTag(const Tag& tag)
{
    kdDebug() << "remove tag \"" << tag.id() << "\" from selected articles" << endl;
    TQValueList<Article> selectedArticles = m_articleList->selectedArticles();
    for (TQValueList<Article>::Iterator it = selectedArticles.begin(); it != selectedArticles.end(); ++it)
        (*it).removeTag(tag.id());

    updateTagActions();
}
*/
void View::slotNewTag()
{
    Tag tag(KApplication::randomString(8), "New Tag");
    Kernel::self()->tagSet()->insert(tag);
    TagNode* node = m_tagNodeList->findByTagID(tag.id());
    if (node)
        m_tagNodeListView->startNodeRenaming(node);
}

void View::slotTagCreated(const Tag& tag)
{
    if (m_tagNodeList && !m_tagNodeList->containsTagId(tag.id()))
    {
        TagNode* tagNode = new TagNode(tag, m_feedList->rootNode());
        m_tagNodeList->rootNode()->appendChild(tagNode);
    }
}

void View::slotTagRemoved(const Tag& /*tag*/)
{
}

void View::slotArticleSelected(const Article& article)
{
    if (m_viewMode == CombinedView)
        return;

    m_markReadTimer->stop();

    Feed *feed = article.feed();
    if (!feed)
        return;

    Article a(article);
    if (a.status() != Article::Read)
    {
        int delay;

        if ( Settings::useMarkReadDelay() )
        {
            delay = Settings::markReadDelay();

            if (delay > 0)
                m_markReadTimer->start( delay*1000, true );
            else
                a.setStatus(Article::Read);
        }
    }

    KToggleAction*  maai = dynamic_cast<KToggleAction*>(m_actionManager->action("article_set_status_important"));
    maai->setChecked(a.keep());

    kdDebug() << "selected: " << a.guid() << endl;

    updateTagActions();

    m_articleViewer->slotShowArticle(a);
}

void View::slotOpenArticleExternal(const Article& article, const TQPoint&, int)
{
    if (!article.isNull())
        Viewer::displayInExternalBrowser(article.link());
}


void View::slotOpenCurrentArticle()
{
    Article article = m_articleList->currentArticle();

    if (article.isNull())
        return;

    KURL link;
    if (article.link().isValid())
        link = article.link();
    else if (article.guidIsPermaLink())
        link = KURL(article.guid());

    if (link.isValid())
    {
        slotOpenURL(link, 0L, BrowserRun::NEW_TAB_FOREGROUND);
    }
}

void View::slotOpenCurrentArticleExternal()
{
    slotOpenArticleExternal(m_articleList->currentArticle(), TQPoint(), 0);
}

void View::slotOpenCurrentArticleBackgroundTab()
{
    Article article = m_articleList->currentArticle();

    if (article.isNull())
        return;

    KURL link;

    if (article.link().isValid())
        link = article.link();
    else if (article.guidIsPermaLink())
        link = KURL(article.guid());

    if (link.isValid())
    {
        slotOpenURL(link, 0L, BrowserRun::NEW_TAB_BACKGROUND);
    }
}

void View::slotCopyLinkAddress()
{
    Article article = m_articleList->currentArticle();

    if(article.isNull())
       return;

    TQString link;
    if (article.link().isValid() || (article.guidIsPermaLink() && KURL(article.guid()).isValid()))
    {
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
            link = article.link().url();
        else
            link = article.guid();
        QClipboard *cb = TQApplication::clipboard();
        cb->setText(link, QClipboard::Clipboard);
        cb->setText(link, QClipboard::Selection);
    }
}

void View::slotFeedURLDropped(KURL::List &urls, TreeNode* after, Folder* parent)
{
    KURL::List::iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it )
    {
        addFeed((*it).prettyURL(), after, parent, false);
    }
}

void View::slotToggleShowQuickFilter()
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

void View::slotArticleDelete()
{

    if ( m_viewMode == CombinedView )
        return;

    TQValueList<Article> articles = m_articleList->selectedArticles();

    TQString msg;
    switch (articles.count())
    {
        case 0:
            return;
        case 1:
            msg = i18n("<qt>Are you sure you want to delete article <b>%1</b>?</qt>").arg(TQStyleSheet::escape(articles.first().title()));
            break;
        default:
            msg = i18n("<qt>Are you sure you want to delete the selected article?</qt>",
		"<qt>Are you sure you want to delete the %n selected articles?</qt>",
		articles.count());
    }

    if (KMessageBox::warningContinueCancel(0, msg, i18n("Delete Article"), KStdGuiItem::del()) == KMessageBox::Continue)
    {
        if (m_listTabWidget->activeView()->selectedNode())
            m_listTabWidget->activeView()->selectedNode()->setNotificationMode(false);

        TQValueList<Feed*> feeds;
        for (TQValueList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        {
            Feed* feed = (*it).feed();
            if (!feeds.contains(feed))
                feeds.append(feed);
            feed->setNotificationMode(false);
            (*it).setDeleted();
        }

        for (TQValueList<Feed*>::Iterator it = feeds.begin(); it != feeds.end(); ++it)
        {
            (*it)->setNotificationMode(true);
        }

        if (m_listTabWidget->activeView()->selectedNode())
            m_listTabWidget->activeView()->selectedNode()->setNotificationMode(true);
    }
}


void View::slotArticleToggleKeepFlag(bool /*enabled*/)
{
    TQValueList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    bool allFlagsSet = true;
    for (TQValueList<Article>::Iterator it = articles.begin(); allFlagsSet && it != articles.end(); ++it)
        if (!(*it).keep())
            allFlagsSet = false;

    for (TQValueList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setKeep(!allFlagsSet);
}

void View::slotSetSelectedArticleRead()
{
    TQValueList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    for (TQValueList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setStatus(Article::Read);
}

void View::slotTextToSpeechRequest()
{
    if (m_currentFrame == m_mainFrame)
    {
        if (m_viewMode != CombinedView)
        {
            // in non-combined view, read selected articles
            SpeechClient::self()->slotSpeak(m_articleList->selectedArticles());
            // TODO: if article viewer has a selection, read only the selected text?
        }
        else
        {
            if (m_listTabWidget->activeView()->selectedNode())
            {
                //TODO: read articles in current node, respecting quick filter!
            }
        }
    }
    else
    {
        TQString selectedText = static_cast<PageViewer *>(m_currentFrame->part())->selectedText();

        if (!selectedText.isEmpty())
            SpeechClient::self()->slotSpeak(selectedText, "en");
    }
}

void View::slotSetSelectedArticleUnread()
{
    TQValueList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    for (TQValueList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setStatus(Article::Unread);
}

void View::slotSetSelectedArticleNew()
{
    TQValueList<Article> articles = m_articleList->selectedArticles();

    if (articles.isEmpty())
        return;

    for (TQValueList<Article>::Iterator it = articles.begin(); it != articles.end(); ++it)
        (*it).setStatus(Article::New);
}

void View::slotSetCurrentArticleReadDelayed()
{
    Article article = m_articleList->currentArticle();

    if (article.isNull())
        return;

    article.setStatus(Article::Read);
}

void View::slotMouseOverInfo(const KFileItem *kifi)
{
    if (kifi)
    {
        KFileItem *k=(KFileItem*)kifi;
        m_mainFrame->setStatusText(k->url().prettyURL());//getStatusBarInfo());
    }
    else
    {
        m_mainFrame->setStatusText(TQString::null);
    }
}

void View::readProperties(KConfig* config)
{

    if (!Settings::resetQuickFilterOnNodeChange())
    {
        m_searchBar->slotSetText(config->readEntry("searchLine"));
        int statusfilter = config->readNumEntry("searchCombo", -1);
        if (statusfilter != -1)
            m_searchBar->slotSetStatus(statusfilter);
    }

    int selectedID = config->readNumEntry("selectedNodeID", -1);
    if (selectedID != -1)
    {
        TreeNode* selNode = m_feedList->findByID(selectedID);
        if (selNode)
            m_listTabWidget->activeView()->setSelectedNode(selNode);
    }

    TQStringList urls = config->readListEntry("FeedBrowserURLs");
    TQStringList::ConstIterator it = urls.begin();
    for (; it != urls.end(); ++it)
    {
        KURL url = KURL::fromPathOrURL(*it);
        if (url.isValid())
            slotOpenNewTab(url, true); // open in background
    }
}

void View::saveProperties(KConfig* config)
{
    // save filter settings
    config->writeEntry("searchLine", m_searchBar->text());
    config->writeEntry("searchCombo", m_searchBar->status());

    TreeNode* sel = m_listTabWidget->activeView()->selectedNode();

    if (sel)
    {
        config->writeEntry("selectedNodeID", sel->id() );
    }

    // save browser URLs
    TQStringList urls;
    TQPtrList<Frame> frames = m_tabs->frames();
    TQPtrList<Frame>::ConstIterator it = frames.begin();
    for (; it != frames.end(); ++it)
    {
        Frame *frame = *it;
        KParts::ReadOnlyPart *part = frame->part();
        PageViewer *pageViewer = dynamic_cast<PageViewer*>(part); // don't save the ArticleViewer
        if (pageViewer)
        {
            KURL url = pageViewer->url();
            if (url.isValid())
                urls.append(url.prettyURL());
        }
    }

    config->writeEntry("FeedBrowserURLs", urls);
}

void View::connectToFeedList(FeedList* feedList)
{
    connect(feedList->rootNode(), TQT_SIGNAL(signalChanged(TreeNode*)), this, TQT_SLOT(slotSetTotalUnread()));
    slotSetTotalUnread();
}

void View::disconnectFromFeedList(FeedList* feedList)
{
    disconnect(feedList->rootNode(), TQT_SIGNAL(signalChanged(TreeNode*)), this, TQT_SLOT(slotSetTotalUnread()));
}

void View::updateTagActions()
{
    TQStringList tags;

    TQValueList<Article> selectedArticles = m_articleList->selectedArticles();

    for (TQValueList<Article>::ConstIterator it = selectedArticles.begin(); it != selectedArticles.end(); ++it)
    {
        TQStringList atags = (*it).tags();
        for (TQStringList::ConstIterator it2 = atags.begin(); it2 != atags.end(); ++it2)
        {
            if (!tags.contains(*it2))
                tags += *it2;
        }
    }
    m_actionManager->slotUpdateTagActions(!selectedArticles.isEmpty(), tags);
}

} // namespace Akregator

#include "akregator_view.moc"
