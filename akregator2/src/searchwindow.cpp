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

#include "searchwindow.h"
#include "mainwidget.h"
#include "searchdescriptionattribute.h"
#include "searchpatterneditor.h"
#include "searchproxymodel.h"
#include "articlelistview.h"

#include "akregator2config.h"

#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KWindowSystem>

#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QHeaderView>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionRequester>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityTreeView>
#include <Akonadi/SearchCreateJob>
#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/EntityDisplayAttribute>
#include <akonadi/persistentsearchattribute.h>
#include <akonadi/itemfetchscope.h>

#include <KLineEdit>
#include <KPushButton>
#include <KStatusBar>
#include <KJob>
#include <KMessageBox>

#include <krss/item.h>
#include <krss/feeditemmodel.h>

#include <QTimer>

namespace Akregator2 {

SearchWindow::SearchWindow( KRss::FeedItemModel *itemModel, const Akonadi::Collection& collection, MainWidget *parent )
  : KDialog( parent ),
    m_itemModel( itemModel ),
    m_sourceCollection( collection ),
    m_searchProxyModel( 0 ),
    m_renameTimer( 0 ),
    m_closeRequested( false ),
    m_searchJob( 0 ),
    m_config( Settings::self()->config(), "Search")

{
  setCaption( i18n( "Find Article" ));
  setButtons( User1 | User2 | Close );
  setDefaultButton( User1 );
  setButtonGuiItem( User1, KGuiItem( i18nc("@action:button Search for articles", "&Search" ) ) );
  setButtonGuiItem( User2, KStandardGuiItem::stop() );

  KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap( IconSize( KIconLoader::Desktop ),
                                                               IconSize( KIconLoader::Desktop ) ),
                           qApp->windowIcon().pixmap( IconSize( KIconLoader::Small ),
                                                      IconSize( KIconLoader::Small ) ) );

  QWidget *searchWidget = new QWidget( this );
  QVBoxLayout *vbl = new QVBoxLayout( searchWidget );
  vbl->setMargin( 0 );

  QFrame *radioFrame = new QFrame( searchWidget );
  QVBoxLayout *radioLayout = new QVBoxLayout( radioFrame );
  m_allFeeds = new QRadioButton( i18n( "Search in &all feeds" ), searchWidget );

  QHBoxLayout *hbl = new QHBoxLayout( radioFrame );
  m_specificFeed = new QRadioButton( i18n( "Search &only in" ), searchWidget );
  m_specificFeed->setChecked( true );

  m_feedRequester = new Akonadi::CollectionRequester( collection, searchWidget );
  m_feedRequester->setMimeTypeFilter( QStringList() << KRss::Item::mimeType() );
  connect( m_feedRequester, SIGNAL(collectionChanged(Akonadi::Collection)),
           this, SLOT(slotFolderActivated()) );


  m_checkSubfolders = new QCheckBox( i18n( "I&nclude sub-folders" ), searchWidget );
  m_checkSubfolders->setChecked( true );

  radioLayout->addWidget( m_allFeeds );
  hbl->addWidget( m_specificFeed );
  hbl->addWidget( m_feedRequester );
  hbl->addWidget( m_checkSubfolders );
  radioLayout->addLayout( hbl );

  m_searchPattern = new SearchPatternEditor( this );

  QHBoxLayout *hbl3 = new QHBoxLayout( searchWidget );

  m_searchFolderEdit = new KLineEdit( searchWidget );
  m_searchFolderEdit->setClearButtonShown( true );
  connect( m_searchFolderEdit, SIGNAL(textChanged(QString)),
           this, SLOT(scheduleSearchFolderRename(QString)) );

  m_searchFolderLabel = new QLabel( i18n( "Search folder &name: " ), searchWidget );
  m_searchFolderLabel->setBuddy( m_searchFolderEdit );

  m_openSearchFolderBtn = new KPushButton( i18n( "Op&en Search Folder" ), searchWidget );
  m_openSearchFolderBtn->setEnabled( false );
  connect( m_openSearchFolderBtn, SIGNAL(clicked(bool)),
           this, SLOT(slotOpenSearchFolder()) );

  m_openSearchResultBtn = new KPushButton( i18n( "Open &Article" ), searchWidget );
  m_openSearchResultBtn->setEnabled( false );
  connect( m_openSearchResultBtn, SIGNAL(clicked(bool)),
           this, SLOT(slotOpenSearchResult()) );

  hbl3->addWidget( m_searchFolderLabel );
  hbl3->addWidget( m_searchFolderEdit );
  hbl3->addWidget( m_openSearchFolderBtn );
  hbl3->addWidget( m_openSearchResultBtn );

  m_matchesView = new ArticleListView( m_config, searchWidget );
  m_matchesView->setAlternatingRowColors( true );
  m_matchesView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  connect( m_matchesView, SIGNAL(doubleClicked(Akonadi::Item)),
           this, SLOT(slotOpenArticle(Akonadi::Item)) );
  connect( m_matchesView, SIGNAL(currentChanged(Akonadi::Item)),
           this, SLOT(slotCurrentChanged(Akonadi::Item)) );

  m_statusBar = new KStatusBar( searchWidget );
  m_statusBar->insertPermanentItem( i18n( "AMiddleLengthText..." ), 0 );
  m_statusBar->changeItem( i18nc( "@info:status finished searching", "Ready." ), 0 );
  m_statusBar->setItemAlignment( 0, Qt::AlignLeft | Qt::AlignVCenter );
  m_statusBar->insertPermanentItem( QString(), 1, 1 );
  m_statusBar->setItemAlignment( 1, Qt::AlignLeft | Qt::AlignVCenter );

  setMainWidget( searchWidget );
  setButtonsOrientation( Qt::Vertical );
  enableButton( User2, false );

  vbl->addWidget( radioFrame );
  vbl->addWidget( m_searchPattern );
  vbl->addWidget( m_matchesView );
  vbl->addLayout( hbl3);
  vbl->addWidget( m_statusBar );

  connect( this, SIGNAL(user1Clicked()), SLOT(slotStartSearch()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotStopSearch()) );
  connect( this, SIGNAL(finished()), SLOT(deleteLater()) );
  connect( this, SIGNAL(closeClicked()), SLOT(slotClose()) );

  if ( collection.hasAttribute<Akonadi::PersistentSearchAttribute>() ) {
    /* Make sure it's our search folder */
    if ( collection.hasAttribute<SearchDescriptionAttribute>() ) {
      SearchDescriptionAttribute *searchDescription = collection.attribute<SearchDescriptionAttribute>();
      // Use the itemModel to retrieve the collection with name
      const QModelIndex index = Akonadi::EntityTreeModel::modelIndexForCollection(itemModel, searchDescription->baseCollection());
      if ( index.isValid() ) {
        m_searchCollection = collection;

        const Akonadi::Collection col = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        m_specificFeed->setChecked( true );
        m_feedRequester->setCollection( col );
        m_checkSubfolders->setChecked( searchDescription->recursive() );
        m_searchPattern->setPattern( searchDescription->searchPattern() );
        m_searchFolderEdit->setText( searchDescription->description() );
      } else {
        m_allFeeds->setChecked( true );
      }
    } else {
      kWarning() << "This search was not created with Akregator2. It cannot be edited within it.";
      m_searchPattern->clear();
    }
  } else {
    m_searchFolderEdit->setPlaceholderText( i18n( "Last Search" ) );
  }

  m_searchPattern->setFocus();
}

SearchWindow::~SearchWindow()
{
}

void SearchWindow::activateFolder( const Akonadi::Collection& collection )
{
  m_feedRequester->setCollection( collection );
}

void SearchWindow::createSearchModel()
{
  if (!m_searchProxyModel) {
    m_searchProxyModel = new SearchProxyModel( this );
    m_searchProxyModel->setSourceModel( m_itemModel );
  }

  m_searchProxyModel->setCollection( m_searchCollection );

  m_matchesView->setModel( m_searchProxyModel );
  m_matchesView->header()->restoreState( m_headerState );
}

void SearchWindow::closeEvent( QCloseEvent *event )
{
  if ( m_searchJob ) {
    m_closeRequested = true;

    m_searchJob->kill( KJob::Quietly );
    m_searchJob->deleteLater();
    m_searchJob = 0;
    QTimer::singleShot( 0, this, SLOT(slotClose()));
  } else {
    KDialog::closeEvent( event );
  }
}

void SearchWindow::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape && m_searchJob ) {
    slotStopSearch();
    return;
  }

  KDialog::keyPressEvent( event );
}

void SearchWindow::enableGUI()
{
  const bool searching = ( m_searchJob != 0 );

  enableButton( KDialog::Close, !searching );
  m_feedRequester->setEnabled( !searching && m_allFeeds->isChecked() );
  m_checkSubfolders->setEnabled( !searching && m_allFeeds->isChecked() );
  m_allFeeds->setEnabled( !searching );
  m_specificFeed->setEnabled( !searching );
  m_searchPattern->setEnabled( !searching );
  m_searchFolderEdit->setEnabled( !searching );

  enableButton( User1, !searching );
  enableButton( User2, searching );
}

void SearchWindow::updateCollectionStatistics( const Akonadi::Entity::Id& id, const Akonadi::CollectionStatistics& statistics )
{
  QString msg;
  if ( id == m_searchCollection.id() ) {
    msg = i18np( "% match", "%1 matches", statistics.count() );
  }
  m_statusBar->changeItem( msg, 0 );
}

void SearchWindow::scheduleSearchFolderRename(const QString& name)
{
  if ( !name.isEmpty() ) {
    if ( !m_renameTimer ) {
      m_renameTimer = new QTimer( this );
      m_renameTimer->setInterval( 250 );
      m_renameTimer->setSingleShot( true );
      connect( m_renameTimer, SIGNAL(timeout()),
               this, SLOT(slotDoRenameSearchFolder()) );
    }
    m_renameTimer->start();
    m_openSearchFolderBtn->setEnabled( false );
  } else {
    if ( m_renameTimer ) {
      m_renameTimer->stop();
    }
    m_openSearchFolderBtn->setEnabled( !name.isEmpty() );
  }
}

void SearchWindow::slotDoRenameSearchFolder()
{
  const QString name = m_searchFolderEdit->text();
  if ( m_searchCollection.isValid() ) {
    if ( m_searchCollection.name() != name ) {
      m_searchCollection.setName( name );
      Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob( m_searchCollection, this );
      connect( job, SIGNAL(finished(KJob*)),
               this, SLOT(slotSearchFolderRenameDone(KJob*)) );
      return;
    }
  }

  m_openSearchFolderBtn->setEnabled( true );
}

void SearchWindow::slotSearchFolderRenameDone(KJob* job)
{
  if ( job->error() ) {
    kWarning() << "Job failed:" << job->errorText();
    KMessageBox::information( this, i18n( "There was a problem renaming your serach folder. "
                                          "A common reason for this is that another search folder "
                                          "with the same name already exists.") );
  }

  m_openSearchFolderBtn->setEnabled( true );
}

void SearchWindow::slotClose()
{
  accept();
}

void SearchWindow::slotFolderActivated()
{
  m_specificFeed->setChecked( true );
}

void SearchWindow::slotOpenArticle( const Akonadi::Item &item )
{
  MainWidget* widget = qobject_cast<Akregator2::MainWidget*>( parent() );
  widget->slotOpenItemInBrowser( item );
}

void SearchWindow::slotOpenSearchFolder()
{
  MainWidget* widget = qobject_cast<Akregator2::MainWidget*>( parent() );
  slotDoRenameSearchFolder();
  widget->slotNodeSelected( m_searchCollection );
}

void SearchWindow::slotOpenSearchResult()
{
  Akonadi::Item item = m_matchesView->currentIndex().data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  if (!item.isValid()) {
    return;
  }

  MainWidget* widget = qobject_cast<Akregator2::MainWidget*>( parent() );
  widget->slotItemSelected( item );
}

void SearchWindow::slotCurrentChanged( const Akonadi::Item& item )
{
  m_openSearchResultBtn->setEnabled( item.isValid() );
}

void SearchWindow::slotStopSearch()
{
  if ( m_searchJob ) {
    m_searchJob->kill( KJob::Quietly );
    m_searchJob->deleteLater();
    m_searchJob = 0;
  }

  enableButton( User2, false );
}

void SearchWindow::slotStartSearch()
{
  m_lastFocus = focusWidget();
  setButtonFocus( User1 );

  if ( m_searchFolderEdit->text().isEmpty() ) {
    m_searchFolderEdit->setText( i18n( "Last Search" ) );
  }
  enableButton( User1, false );
  enableButton( User2, true );
  if ( m_itemModel ) {
    m_headerState = m_matchesView->header()->saveState();
  }
  m_matchesView->setModel( 0 );

  m_sortColumn = m_matchesView->header()->sortIndicatorSection();
  m_sortOrder = m_matchesView->header()->sortIndicatorOrder();
  m_matchesView->setSortingEnabled( false );

  if ( m_searchJob ) {
    m_searchJob->kill( KJob::Quietly );
    m_searchJob->deleteLater();
    m_searchJob = 0;
  }

  m_searchFolderEdit->setEnabled( false );

  KUrl::List urls;
  if ( !m_allFeeds->isChecked() ) {
    const Akonadi::Collection col = m_feedRequester->collection();
    urls << col.url( Akonadi::Collection::UrlShort );
    if ( m_checkSubfolders->isChecked() ) {
      childCollectionsFromSelectedCollection( col, urls );
    }
  }

  enableGUI();

  const QString query = m_searchPattern->nepomukQuery(urls);
  const QString queryLanguage = "SPARQL";

  qDebug() << queryLanguage;
  qDebug() << query;
  if ( query.isEmpty() ) {
    return;
  }
  m_openSearchFolderBtn->setEnabled( true );

  if ( !m_searchCollection.isValid() ) {
    m_searchJob = new Akonadi::SearchCreateJob( m_searchFolderEdit->text(), query, this);
  } else {
    Akonadi::PersistentSearchAttribute *attribute = m_searchCollection.attribute<Akonadi::PersistentSearchAttribute>(Akonadi::Entity::AddIfMissing);
    attribute->setQueryLanguage( queryLanguage );
    attribute->setQueryString( query );
    m_searchJob = new Akonadi::CollectionModifyJob( m_searchCollection, this );
  }

  connect( m_searchJob, SIGNAL(result(KJob*)), this, SLOT(slotSearchDone(KJob*)) );
}

void SearchWindow::slotSearchDone(KJob* job)
{
  Q_ASSERT( job == m_searchJob );
  if ( job->error() ) {
    KMessageBox::sorry( this, i18n( "Can't get search result. %1", job->errorString() ) );
    m_searchJob = 0;
    return;
  }

  /* Stop monitoring the old collection */
  if (Akonadi::SearchCreateJob *searchJob = qobject_cast<Akonadi::SearchCreateJob*>( m_searchJob ) ) {
    m_searchCollection = searchJob->createdCollection();
  } else if (Akonadi::CollectionModifyJob *modifyJob = qobject_cast<Akonadi::CollectionModifyJob*>( m_searchJob ) ) {
    m_searchCollection = modifyJob->collection();
  }

  Q_ASSERT( m_searchCollection.isValid() );
  Q_ASSERT( m_searchCollection.hasAttribute<Akonadi::PersistentSearchAttribute>() );

  Settings::self()->setLastSearchCollectionId( m_searchCollection.id() );

  Akonadi::Collection c( m_searchCollection.id() );
  SearchDescriptionAttribute *searchDescription = c.attribute<SearchDescriptionAttribute>( Akonadi::Entity::AddIfMissing );
  if ( m_allFeeds->isChecked() ) {
    searchDescription->setBaseCollection( Akonadi::Collection() );
  } else {
    searchDescription->setBaseCollection( m_feedRequester->collection() );
  }

  searchDescription->setRecursive( m_checkSubfolders->isChecked() );
  searchDescription->setDescription( m_searchCollection.name() );
  searchDescription->setSearchPattern( m_searchPattern->pattern() );

  Akonadi::Job *j = new Akonadi::CollectionModifyJob( c, this );
  connect(j, SIGNAL(finished(KJob*)), SLOT(modifyJobFinished(KJob*)));
  m_searchJob = 0;

  QTimer::singleShot( 100, this, SLOT(createSearchModel()) );
  QTimer::singleShot( 0, this, SLOT(enableGUI()) );

  if ( m_lastFocus ) {
    m_lastFocus->setFocus();
  }

  if ( m_closeRequested ) {
    close();
  }

  m_matchesView->setSortingEnabled( true );
  m_matchesView->header()->setSortIndicator( m_sortColumn, m_sortOrder );
  m_searchFolderEdit->setEnabled( true );
}

void SearchWindow::modifyJobFinished(KJob* job)
{
  if (job->error()) {
    kDebug() << job->error() << job->errorString() << job->errorText();
  }
}


void SearchWindow::childCollectionsFromSelectedCollection( const Akonadi::Collection& collection, KUrl::List&lstUrlCollection )
{
  if ( collection.isValid() )  {
    QModelIndex idx = Akonadi::EntityTreeModel::modelIndexForCollection( m_itemModel, collection );
    if ( idx.isValid() ) {
      getChildren( m_itemModel, idx, lstUrlCollection );
    }
  }
}

void SearchWindow::getChildren( const QAbstractItemModel *model,
                                const QModelIndex &parentIndex,
                                KUrl::List &list )
{
  const int rowCount = model->rowCount( parentIndex );
  for ( int row = 0; row < rowCount; ++row ) {
    const QModelIndex index = model->index( row, 0, parentIndex );
    if ( model->rowCount( index ) > 0 ) {
      getChildren( model, index, list );
    }
    Akonadi::Collection c = model->data(index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( c.isValid() )
      list << c.url( Akonadi::Collection::UrlShort );
  }
}

} /* namespace Akregator2 */

