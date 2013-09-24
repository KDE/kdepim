/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifdef Q_OS_WINCE
# include <windows.h>
# include <winuser.h>
#endif

#include <incidenceeditor-ng/korganizereditorconfig.h>

#include "mainview.h"

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC
#include <QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_addressee)
Q_IMPORT_PLUGIN(akonadi_serializer_contactgroup)
#endif

int main( int argc, char **argv )
{
#ifdef Q_OS_WINCE
  SetCursor( LoadCursor( NULL, IDC_WAIT ) );
#endif
  const QByteArray& ba = QByteArray( "kaddressbook-mobile" );
  const KLocalizedString name = ki18n( "Kontact Touch Contacts" );

  IncidenceEditorNG::EditorConfig::setEditorConfig( new IncidenceEditorNG::KOrganizerEditorConfig ); //FIXME: use our own config for contacts

  KAboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "KAddressbook Mobile" ); //has to match the bugzilla product name

  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication<MainView> app;

  KGlobal::locale()->insertCatalog( QLatin1String("kabc") );
  KGlobal::locale()->insertCatalog( QLatin1String("akonadicontact") );
  KGlobal::locale()->insertCatalog( QLatin1String("libkdepim") );
  KGlobal::locale()->insertCatalog( QLatin1String("libkldap") ); // for ldap server dialog
  KGlobal::locale()->insertCatalog( QLatin1String("calendarsupport") ); // for categories

#ifdef Q_OS_WINCE
  SetCursor( LoadCursor( NULL, NULL ) );
#endif

  return app.exec();
}

