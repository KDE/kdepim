/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mainwindow.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KLocalizedString>

static const char description[] = I18N_NOOP( "A test app for embedding calendarviews" );

static const char version[] = "0.1";
#include <KDebug>
int main( int argc, char **argv )
{
    KAboutData about( "viewerapp", 0, ki18n( "ViewerApp" ), version, ki18n( description ),
                     KAboutData::License_GPL,
                     ki18n( "Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net" ), KLocalizedString(), 0, "info@kdab.net");
    about.addAuthor( ki18n( "Kevin Krammer" ), KLocalizedString(), "krake@kdab.com" );
    KCmdLineArgs::init( argc, argv, &about );

    KCmdLineOptions options;
    options.add("+[viewname]", ki18n("Optional list of view names to instantiate"));

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QStringList viewNames;
    for ( int i = 0; i < args->count(); ++i ) {
      viewNames << args->arg( i ).toLower();
    }

    MainWindow *widget = new MainWindow( viewNames ) ;

    widget->show();

    return app.exec();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
