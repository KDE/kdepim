/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include <libkmobiletools/enginexp.h>
#include <libkmobiletools/ifaces/status.h>
#include <libkmobiletools/errorhandler.h>

#include <KDebug>
#include <KApplication>
#include <KCmdLineArgs>
#include <KLocale>
#include <KAboutData>
#include <KService>
#include <KServiceTypeTrader>
#include <QtCore/QString>

using namespace KMobileTools;
class ErrorPrinter : public QObject {
    Q_OBJECT
public:
    void install() {
        connect( KMobileTools::ErrorHandler::instance(),
                 SIGNAL(errorOccurred(QString,BaseError::Priority)),
                 this,
                 SLOT(printError(QString,BaseError::Priority))
               );
    }

public Q_SLOTS:
    void printError( const QString& errorMessage, BaseError::Priority ) {
        kDebug() <<"Error occurred:" << errorMessage;
    }
};

#include "enginexp.moc"

/**
 * This is a test for the EngineXP class
 */
int main( int argc, char *argv[] ) {
    // init application
    KCmdLineArgs::init( argc, argv, QByteArray("EngineXPTest"), QByteArray("EngineXPTest"), ki18n("EngineXPTest"),
                        QByteArray("1.0") );
    KCmdLineOptions options;
    options.add( "engine <name>", ki18n("The KMobileTools engine to use") );
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication* app = new KApplication( false );
    Q_UNUSED(app);

    // check for args
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QString engineName = args->getOption("engine");

    if( engineName.isEmpty() )
        return -1;

    // install error printer (needed since the error handler will open a gui dialog by default
    // which makes this app crashing)
    ErrorPrinter errorPrinter;
    errorPrinter.install();

    // load lib
    KService::List engineList = KServiceTypeTrader::self()->query( "KMobileTools/EngineXP" );

    int serviceNumber = 0;
    bool engineFound = false;
    for( ; serviceNumber < engineList.size(); serviceNumber++ ) {
        if( engineList.at( serviceNumber )->library() == engineName ) {
            engineFound = true;
            break;
        }
    }

    QStringList arg( "Test device" );
    KMobileTools::EngineXP* fakeEngine = KService::createInstance<KMobileTools::EngineXP>
                                     ( engineList.at( serviceNumber ), (QObject*) 0, arg );

    if( fakeEngine ) {
        kDebug() << engineName <<" engine loaded.";
    } else {
        kDebug() << engineName <<" engine could not be loaded.";
        return -1;
    }

    if( fakeEngine->implements( "Status" ) )
        kDebug() <<"- Implements Status interface";

    if( fakeEngine->implements( "Information" ) ) {
        kDebug() <<"- Implements Information interface";
        KMobileTools::Ifaces::Status* status = qobject_cast<KMobileTools::Ifaces::Status*> ( fakeEngine );
        Q_ASSERT(status != 0);
        status->fetchStatusInformation();
    }

    return 0;
}
