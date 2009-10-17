#include "krss/feeditemmodel.h"

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

#include <QApplication>
#include <QByteArray>
#include <QSortFilterProxyModel>
#include <QTreeView>

using namespace Akonadi;
using namespace KRss;

int main( int argc, char** argv ) {
    const QByteArray ba = "example_feeditemmodel";
    const KLocalizedString name = ki18n( "KRss feed list model example example" );
    KAboutData aboutData( ba, ba, name, ba, name );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;
    ItemFetchScope scope;
    scope.fetchFullPayload( true );
    scope.fetchAttribute<EntityDisplayAttribute>();

    Session session( QByteArray( "FeedReaderApplication-" ) + QByteArray::number( qrand() ) );

    ChangeRecorder recorder;
    recorder.fetchCollection( true );
    recorder.setItemFetchScope( scope );
    recorder.setCollectionMonitored( Collection::root() );
    recorder.setMimeTypeMonitored( QLatin1String( "application/rss+xml" ) );

    FeedItemModel model( &session, &recorder );
    QSortFilterProxyModel sortModel;
    sortModel.setSourceModel( &model );
    sortModel.setSortRole( FeedItemModel::SortRole );
    QTreeView view;
    view.setSortingEnabled( true );
    view.setModel( &sortModel );
    view.show();

    return app.exec();
}

