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

#ifndef AKREGATOR2_MAINWIDGET_H
#define AKREGATOR2_MAINWIDGET_H

#include "akregator2_export.h"

//#include <krss/ui/feedlistview.h>

#include <kurl.h>
#include <solid/networking.h>

#include <QPixmap>
#include <QPointer>
#include <QWidget>

class KConfig;
class KFileItem;
class KConfigGroup;
class KJob;

class QDomDocument;
class QSplitter;

namespace KRss {
    class FeedListView;
    class Item;
}

namespace Akonadi {
    class Collection;
    class Session;
}

namespace Akregator2 {

class ActionManagerImpl;
class ArticleListView;
class ArticleViewer;
class Frame;
class FrameManager;
class Part;
class SearchBar;
class SelectionController;
class TabWidget;

}

namespace Akregator2 {

/**
 * This is the main widget of the view, containing tree view, article list, viewer etc.
 */
class AKREGATOR2PART_EXPORT MainWidget : public QWidget
{
    Q_OBJECT
    public:

        /** constructor
        @param part the Akregator2::Part which contains this widget
        @param parent parent widget
        @param name the name of the widget (@ref QWidget )
        */
        MainWidget(Akregator2::Part *part, QWidget *parent, ActionManagerImpl* actionManager);

        /** destructor.  Note that cleanups should be done in
        slotOnShutdown(), so we don't risk accessing self-deleting objects after deletion. */
        ~MainWidget();

        /** saves settings. Make sure that the Settings singleton is not destroyed yet when saveSettings is called */
        void saveSettings();

        FrameManager* frameManager() const;

        /**
         * Add a feed to a group.
         * @param url The URL of the feed to add.
         * @param group The name of the folder into which the feed is added.
         * If the group does not exist, it is created.
         * The feed is added as the last member of the group.
         */
        void addFeedToGroup(const QString& url, const QString& group);

        /** session management **/
        void readProperties(const KConfigGroup & config);
        void saveProperties(KConfigGroup & config);

        //Returns true if networking is available
        bool isNetworkAvailable() const;

        enum ViewMode { NormalView=0,
                        WidescreenView,
                        CombinedView };

        ViewMode viewMode() const { return m_viewMode; }


    signals:
        /** emitted when the unread count of "All Feeds" was changed */
        void signalUnreadCountChanged(int);

	void signalItemsSelected( const QList<KRss::Item>& );

    public slots:

        void slotImportFeedList();
        void slotExportFeedList();

        void slotMetakitImport();

        /** opens the current article (currentItem) in external browser
        TODO: use selected instead of current? */
        void slotOpenSelectedArticlesInBrowser();

        /** opens current article in new tab, background/foreground depends on settings TODO: use selected instead of current? */
        void slotOpenSelectedArticles();
        void slotOpenSelectedArticlesInBackground();

        void slotOnShutdown();

        /** selected tree node has changed */
        void slotNodeSelected( const Akonadi::Collection& c );

        /** the item selection has changed */
        void slotItemSelected( const KRss::Item& item );

        /** copies the link of current article to clipboard
        */
        void slotCopyLinkAddress();

        void slotRequestNewFrame(int& frameId);

        /** displays a URL in the status bar when the user moves the mouse over a link */
        void slotMouseOverInfo(const KFileItem& kifi);

        /** adds a new feed to the feed tree */
        void slotFeedAdd();
        /** adds a new folder to the feed tree */
        void slotFolderAdd();
        /** removes the currently selected feed (ask for confirmation)*/
        void slotFeedRemove();
        /** calls the properties dialog for feeds, starts renaming for feed groups */
        void slotFeedModify();
        /** fetches the currently selected feed */
        void slotFetchCurrentFeed();
        /** starts fetching of all feeds in the tree */
        void slotFetchAllFeeds();
        /** aborts all running fetch operations */
        void slotAbortFetches();
        /** marks all articles in the currently selected feed as read */
        void slotMarkFeedRead();
        /** marks all articles in all feeds in the tree as read */
        void slotMarkAllFeedsRead();
        /** opens the homepage of the currently selected feed */
        void slotOpenHomepage();

        /** reloads all open tabs */
        void slotReloadAllTabs();


        /** toggles the keep flag of the currently selected article */
        void slotArticleToggleKeepFlag(bool enabled);
        /** deletes the currently selected article */
        void slotArticleDelete();
        /** marks the currently selected article as read */
        void slotSetSelectedArticleRead();
        /** marks the currently selected article as unread */
        void slotSetSelectedArticleUnread();
        /** marks the currenctly selected article as read after a user-set delay */
        void slotSetCurrentArticleReadDelayed();

        /** reads the currently selected articles using KTTSD */
        void slotTextToSpeechRequest();

        /** switches view mode to normal view */
        void slotNormalView();
        /** switches view mode to widescreen view */
        void slotWidescreenView();
        /** switches view mode to combined view */
        void slotCombinedView();
        /** toggles the visibility of the filter bar */
        void slotToggleShowQuickFilter();

        /** selects the previous unread article in the article list */
        void slotPrevUnreadArticle();
        /** selects the next unread article in the article list */
        void slotNextUnreadArticle();

        void slotSendLink() { sendArticle(); }
        void slotSendFile() { sendArticle(true); }

        void slotNetworkStatusChanged(Solid::Networking::Status status);
    protected:

        void sendArticle(bool attach=false);

    protected slots:

        /** special behaviour in article list view (TODO: move code there?) */
        void slotMouseButtonPressed(int button, const KUrl&);

        /** opens the link of an item in the external browser */
        void slotOpenItemInBrowser( const KRss::Item& item );

        void slotFetchQueueStarted();
        void slotFetchQueueFinished();
        void slotJobFinished( KJob *job );

    private:
        class Private;
        Private* const d;

        void addFeed(const QString& url, bool autoExec );
        /** opens current article in new tab, background/foreground depends on settings TODO: use selected instead of current? */
        void openSelectedArticles(bool openInBackground);

        SelectionController* m_selectionController;

        KRss::FeedListView* m_feedListView;
        ArticleListView* m_articleListView;
        Akonadi::Session* m_session;

        ArticleViewer *m_articleViewer;
        TabWidget* m_tabWidget;

        QWidget *m_mainTab;
        Frame *m_mainFrame;

        SearchBar* m_searchBar;

        QSplitter *m_articleSplitter;
        QSplitter *m_horizontalSplitter;

        Akregator2::Part *m_part;
        ViewMode m_viewMode;

        QTimer *m_markReadTimer;

        bool m_shuttingDown;
        bool m_displayingAboutPage;
        bool m_networkAvailable;

        ActionManagerImpl* m_actionManager;
        FrameManager* m_frameManager;
};

} // namespace Akregator2

#endif // AKREGATOR2_MAINWIDGET_H
