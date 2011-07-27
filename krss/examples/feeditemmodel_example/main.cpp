/*
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "feeditemmodel_example.h"
#include "krss/feeditemmodel.h"
#include "krss/feedlist.h"
#include "krss/resourcemanager.h"

#include <Akonadi/ChangeRecorder>
#include <Akonadi/Control>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/entitytreeview.h>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KMessageBox>
#include <kselectionproxymodel.h>

#include <QApplication>
#include <QByteArray>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QVBoxLayout>

using namespace Akonadi;
using namespace KRss;

MainWidget::MainWidget( QWidget* parent ) : QWidget( parent ) /*, m_feedModel( 0 ) */ {
    m_feedView = new EntityTreeView( this );
    m_feedView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_feedView->setSelectionBehavior( QAbstractItemView::SelectRows );
    QVBoxLayout* layout = new QVBoxLayout( this );
    QSplitter* splitter = new QSplitter;
    layout->addWidget( splitter );
    splitter->setOrientation( Qt::Horizontal );
    splitter->addWidget( m_feedView );
    Control::widgetNeedsAkonadi( splitter );

    ItemFetchScope scope;
    scope.fetchFullPayload( true );
    scope.fetchAttribute<EntityDisplayAttribute>();

    Session* session = new Session( QByteArray( "FeedReaderApplication-" ) + QByteArray::number( qrand() ) );
    ChangeRecorder* recorder = new ChangeRecorder;
    recorder->setSession( session );
    recorder->fetchCollection( true );
    recorder->setItemFetchScope( scope );
    recorder->setCollectionMonitored( Collection::root() );
    recorder->setMimeTypeMonitored( QLatin1String( "application/rss+xml" ) );
    m_itemModel = new FeedItemModel( recorder );

    m_itemView = new EntityTreeView( this );
    splitter->addWidget( m_itemView );
    m_itemView->setSortingEnabled( true );
}

void MainWidget::feedListRetrieved( KJob* j ) {
    if ( j->error() ) {
        KMessageBox::error( this, j->errorString() );
        return;
    }
    m_feedList = static_cast<RetrieveFeedListJob*>( j )->feedList();
    init();
}

void MainWidget::tagProviderRetrieved( KJob* j ) {
    if ( j->error() ) {
        KMessageBox::error( this, j->errorString() );
        return;
    }
    m_tagProvider = static_cast<TagProviderRetrieveJob*>( j )->tagProvider();
    init();
}

void MainWidget::init() {
    if ( !m_tagProvider || !m_feedList )
        return;
#if 0
    m_feedModel = new FeedListModel( m_feedList, m_tagProvider );
    m_feedView->setModel( m_feedModel );
#endif
    EntityMimeTypeFilterModel* filterProxy = new EntityMimeTypeFilterModel;
    filterProxy->setHeaderGroup( EntityTreeModel::CollectionTreeHeaders );
    filterProxy->setSourceModel( m_itemModel );
    m_feedView->setModel( m_itemModel );

    KSelectionProxyModel* selectionProxy = new KSelectionProxyModel( m_feedView->selectionModel() );
    selectionProxy->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );
    selectionProxy->setSourceModel( m_itemModel );
    EntityMimeTypeFilterModel* filterProxy2 = new EntityMimeTypeFilterModel;

    filterProxy2->setHeaderGroup( EntityTreeModel::ItemListHeaders );
    filterProxy2->setSourceModel( selectionProxy );
    filterProxy2->setSortRole( FeedItemModel::SortRole );
    m_itemView->setModel( filterProxy2 );

}

int main( int argc, char** argv ) {
    const QByteArray ba = "example_feeditemmodel";
    const KLocalizedString name = ki18n( "KRss feed list model example example" );
    KAboutData aboutData( ba, ba, name, ba, name );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    ResourceManager::registerAttributes();

    QWidget window;
    QVBoxLayout layout( &window );
    MainWidget mainWidget( &window );
    layout.addWidget( &mainWidget );

    RetrieveFeedListJob * const fjob = new RetrieveFeedListJob;
    fjob->setResources( ResourceManager::self()->resources() );
    fjob->connect( fjob, SIGNAL(result(KJob*)), &mainWidget, SLOT(feedListRetrieved(KJob*)) );
    fjob->start();

    TagProviderRetrieveJob * const tjob = new TagProviderRetrieveJob;
    tjob->connect( tjob, SIGNAL(result(KJob*)), &mainWidget, SLOT(tagProviderRetrieved(KJob*)) );
    tjob->start();
    window.show();
    return app.exec();
}

#include "feeditemmodel_example.moc"
