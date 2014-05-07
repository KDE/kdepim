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
#include <k4aboutdata.h>
#include <kcmdlineargs.h>
#include <KGlobal>
#include <incidenceeditor-ng/korganizereditorconfig.h>

#include "mainview.h"

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC
#include <QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_addressee)
Q_IMPORT_PLUGIN(akonadi_serializer_contactgroup)
#endif

int main( int argc, char **argv )
{
  const QByteArray& ba = QByteArray( "kaddressbook-mobile" );
  const KLocalizedString name = ki18n( "Kontact Touch Contacts" );

  IncidenceEditorNG::EditorConfig::setEditorConfig( new IncidenceEditorNG::KOrganizerEditorConfig ); //FIXME: use our own config for contacts

  K4AboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "KAddressbook Mobile" ); //has to match the bugzilla product name

  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication<MainView> app;

  return app.exec();
}

