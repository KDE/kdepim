/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "mainwindow.h"
#include <kuniqueapplication.h>
#include "global.h"
#include "constants.h"


#include <k4aboutdata.h>
#include <kcmdlineargs.h>

static const char description[] =
    I18N_NOOP( "A KDE Blogging Client" );

int main( int argc, char *argv[] )
{
    KLocalizedString::setApplicationDomain("blogilo");
    K4AboutData about( "blogilo", 0, ki18n( APPNAME ), VERSION, ki18n( description ),
                      K4AboutData::License_GPL_V2, ki18n( "Copyright © 2008–2014 Blogilo authors" ),
                      KLocalizedString(), "http://blogilo.gnufolks.org" );
    about.addAuthor( ki18n( "Mehrdad Momeny" ), ki18n( "Core Developer" ), "mehrdad.momeny@gmail.com" );
    about.addAuthor( ki18n( "Golnaz Nilieh" ), ki18n( "Core Developer" ), "g382nilieh@gmail.com" );
    about.addAuthor( ki18n( "Laurent Montel" ), ki18n( "Core Developer" ), "montel@kde.org" );
    about.addCredit( ki18n( "Roozbeh Shafiee" ), ki18n( "Icon designer" ), "roozbeh@roozbehonline.com");
    about.addCredit( ki18n( "Sajjad Baroodkoo" ), ki18n( "Icon designer" ), "sajjad@graphit.ir");

    about.setTranslator( ki18nc("NAME OF TRANSLATORS", "Your names"),
                         ki18nc("EMAIL OF TRANSLATORS", "Your emails"));
    KCmdLineArgs::init( argc, argv, &about );
//     KCmdLineOptions options;

    KUniqueApplication app;
    global_init();

    MainWindow *bilbo = new MainWindow;

    bilbo->show();
    int r = app.exec();

    global_end();
    return r;
}

