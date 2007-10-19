#include "../utils/filenamerequester.h"

#include <QApplication>
#include <KComponentData>

int main( int argc, char * argv[] ) {
    KComponentData cd( "test_filenamerequester" );
    QApplication app( argc, argv );

    Kleo::FileNameRequester requester;
    requester.show();

    return app.exec();
}
