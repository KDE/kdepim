/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <kdeclarativeapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "mainview.h"

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC
#include <QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_mail)
#endif

int main( int argc, char **argv )
{
 const QByteArray& ba = QByteArray( "notes-mobile" );
  const KLocalizedString name = ki18n( "Kontact Touch Notes" );

  KAboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "KJots Mobile" ); //has to match the bugzilla product name

  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication<MainView> app;

  return app.exec();
}

