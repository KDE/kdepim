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
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KMessageBox>

#include <QApplication>
#include <QByteArray>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTreeView>

using namespace Akonadi;
using namespace KRss;

MainWidget::MainWidget( QWidget* parent ) : QSplitter( parent ), m_feedModel( 0 ) {
    m_feedView = new QTreeView;
    addWidget( m_feedView );
    m_itemView = new QTreeView;
    addWidget( m_itemView );

    ItemFetchScope scope;
    scope.fetchFullPayload( true );
    scope.fetchAttribute<EntityDisplayAttribute>();

    Session* session = new Session( QByteArray( "FeedReaderApplication-" ) + QByteArray::number( qrand() ) );

    ChangeRecorder* recorder = new ChangeRecorder;
    recorder->fetchCollection( true );
    recorder->setItemFetchScope( scope );
    recorder->setCollectionMonitored( Collection::root() );
    recorder->setMimeTypeMonitored( QLatin1String( "application/rss+xml" ) );

    m_itemModel = new FeedItemModel( session, recorder );
    QSortFilterProxyModel* sortModel = new QSortFilterProxyModel;
    sortModel->setSourceModel( m_itemModel );
    sortModel->setSortRole( FeedItemModel::SortRole );
    m_itemView->setSortingEnabled( true );
    m_itemView->setModel( sortModel );

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
    m_feedModel = new FeedListModel( m_feedList, m_tagProvider );
    m_feedView->setModel( m_feedModel );
}

int main( int argc, char** argv ) {
    const QByteArray ba = "example_feeditemmodel";
    const KLocalizedString name = ki18n( "KRss feed list model example example" );
    KAboutData aboutData( ba, ba, name, ba, name );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    ResourceManager::registerAttributes();

    MainWidget mainWidget;
    mainWidget.setOrientation( Qt::Horizontal );
    mainWidget.show();
    RetrieveFeedListJob * const fjob = new RetrieveFeedListJob;
    fjob->setResources( ResourceManager::self()->resources() );
    fjob->connect( fjob, SIGNAL(result(KJob*)), &mainWidget, SLOT(feedListRetrieved(KJob*)) );
    fjob->start();

    TagProviderRetrieveJob * const tjob = new TagProviderRetrieveJob;
    tjob->connect( tjob, SIGNAL(result(KJob*)), &mainWidget, SLOT(tagProviderRetrieved(KJob*)) );
    tjob->start();
    return app.exec();
}

#include "feeditemmodel_example.moc"
