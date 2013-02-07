/*
    This file is part of Akregator2.

    Copyright (C) 2013 Dan Vrátil <dvratil@redhat.com>

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

#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H


#include <KDialog>

#include <Akonadi/Item>
#include <Akonadi/Collection>

#include <QModelIndex>

namespace KRss {
class FeedItemModel;
}

class KJob;
class KLineEdit;
class KPushButton;
class KStatusBar;

class QAbstractItemModel;
class QCheckBox;
class QCloseEvent;
class QKeyEvent;
class QLabel;
class QRadioButton;
class QTimer;

namespace Akonadi {
class ChangeRecorder;
class CollectionRequester;
class Session;
}

namespace Akregator2 {

class ArticleListView;
class SearchPatternEditor;
class SearchProxyModel;
class MainWidget;

class SearchWindow : public KDialog
{
    Q_OBJECT

  public:
    explicit SearchWindow( KRss::FeedItemModel *itemModel, const Akonadi::Collection &collection, Akregator2::MainWidget* parent );
    virtual ~SearchWindow();

    void activateFolder( const Akonadi::Collection &collection );

    Akonadi::Item::List selectedItems() const;

  protected:
    virtual void keyPressEvent( QKeyEvent* );
    virtual void closeEvent( QCloseEvent* );

  private Q_SLOTS:
    void createSearchModel();

    void slotFolderActivated();
    void slotStartSearch();
    void slotStopSearch();
    void slotClose();
    void slotOpenSearchFolder();
    void slotOpenSearchResult();
    void slotOpenArticle( const Akonadi::Item &item );
    void slotCurrentChanged( const Akonadi::Item &item );

    void slotSearchDone( KJob *job );
    void slotSearchFolderRenameDone( KJob *job );

    void slotDoRenameSearchFolder();
    void scheduleSearchFolderRename( const QString &name );

    void enableGUI();

  private:
    void updateCollectionStatistics( const Akonadi::Collection::Id &id, const Akonadi::CollectionStatistics &statistics);

    void childCollectionsFromSelectedCollection( const Akonadi::Collection& collection,
                                                 KUrl::List&lstUrlCollection );
    void getChildren( const QAbstractItemModel *model,
                      const QModelIndex &parentIndex,
                      KUrl::List &list );


    KRss::FeedItemModel* m_itemModel;
    Akonadi::Collection m_sourceCollection;
    Akonadi::Collection m_searchCollection;
    SearchProxyModel* m_searchProxyModel;

    QRadioButton* m_allFeeds;
    QRadioButton* m_specificFeed;
    Akonadi::CollectionRequester* m_feedRequester;
    QCheckBox* m_checkSubfolders;
    SearchPatternEditor *m_searchPattern;
    ArticleListView* m_matchesView;
    QLabel* m_searchFolderLabel;
    KLineEdit* m_searchFolderEdit;
    KPushButton* m_openSearchFolderBtn;
    KPushButton* m_openSearchResultBtn;
    KStatusBar* m_statusBar;
    QWidget* m_lastFocus;

    QTimer* m_renameTimer;
    bool m_closeRequested;
    KJob* m_searchJob;

    QByteArray m_headerState;
    int m_sortColumn;
    Qt::SortOrder m_sortOrder;

    KConfigGroup m_config;

public slots:
    void modifyJobFinished(KJob*);
};

} /* namespace Akregator2 */

#endif // SEARCHWINDOW_H
;
