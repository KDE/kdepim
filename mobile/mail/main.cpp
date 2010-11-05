/*
* This file is part of Akonadi
*
* Copyright 2010 Stephen Kelly <steveire@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include <kdeclarativeapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#ifdef Q_OS_WINCE
# include <windows.h>
# include <winuser.h>
#endif

#include <QDateTime>

#include "mainview.h"

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC
#include <QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_mail)
Q_IMPORT_PLUGIN(akonadi_serializer_addressee)
Q_IMPORT_PLUGIN(akonadi_serializer_contactgroup)
#endif

int main( int argc, char **argv )
{
  kWarning() << "Starting main function" << QDateTime::currentDateTime();
#ifdef Q_OS_WINCE
  SetCursor( LoadCursor( NULL, IDC_WAIT ) );
#endif
  const QByteArray& ba = QByteArray( "kmail-mobile" );
  const KLocalizedString name = ki18n( "KMail Mobile" );
  KAboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "KMail Mobile" ); //has to match the bugzilla product name

  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication app;

  KGlobal::locale()->insertCatalog( "libmessagecore" );
  KGlobal::locale()->insertCatalog( "libmessagecomposer" );
  KGlobal::locale()->insertCatalog( "libmessageviewer" );
  KGlobal::locale()->insertCatalog( "libtemplateparser" );

  MainView view;
  view.show();
#ifdef Q_OS_WINCE
  SetCursor( LoadCursor( NULL, NULL ) );
#endif

  return app.exec();
}
