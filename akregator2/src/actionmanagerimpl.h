/*
    This file is part of Akregator2.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR2_ACTIONMANAGERIMPL_H
#define AKREGATOR2_ACTIONMANAGERIMPL_H

#include "actionmanager.h"

class KAction;
class KActionCollection;

class QWidget;

namespace boost {
template <typename T> class shared_ptr;
}

namespace Akonadi {
    class Collection;
}

namespace KRss {
    class FeedListView;
}

namespace Akregator2 {

class ArticleListView;
class ArticleViewer;
class FrameManager;
class MainWidget;
class Part;
class TabWidget;
class TrayIcon;

/**
 * Akregator2-specific implementation of the ActionManager interface
 */
class ActionManagerImpl : public ActionManager
{
    Q_OBJECT

    public:
        explicit ActionManagerImpl(Part* part, QObject* parent=0);
        virtual ~ActionManagerImpl();

        virtual QAction* action(const char* name);
        virtual QWidget* container(const char* name);

        void initMainWidget(MainWidget* mainWidget);
	void setTrayIcon(TrayIcon* trayIcon);
        void initArticleViewer(ArticleViewer* articleViewer);
        void initArticleListView(ArticleListView* articleList);
        void initFeedListView( KRss::FeedListView* feedListView );
        void initTabWidget(TabWidget* tabWidget);
        void initFrameManager(FrameManager* frameManager);

        void setArticleActionsEnabled( bool enabled );

    public slots:

        void slotNodeSelected( const Akonadi::Collection& );

    protected:

        KActionCollection* actionCollection();

    private Q_SLOTS:
        void progressItemsChanged();

    private:

        void initPart();

        class ActionManagerImplPrivate;
        ActionManagerImplPrivate* d;
};

} // namespace Akregator2

#endif // AKREGATOR2_ACTIONMANAGERIMPL_H
