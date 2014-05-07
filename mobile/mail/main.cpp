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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include "mainview.h"
#include "kmailmobileoptions.h"

#include <k4aboutdata.h>
#include <kdeclarativeapplication.h>
#include <KGlobal>

#include <QtCore/QDateTime>

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC
#include <QtCore/QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_mail)
Q_IMPORT_PLUGIN(akonadi_serializer_addressee)
Q_IMPORT_PLUGIN(akonadi_serializer_contactgroup)
Q_IMPORT_PLUGIN(akonadi_serializer_kcalcore)
#endif

#ifdef KDEPIM_STATIC_LIBS
extern bool ___MailTransport____INIT();
#endif

class KMailMobileApplication : public KDeclarativeApplication<MainView>
{
public:
  KMailMobileApplication();
  explicit KMailMobileApplication( const KCmdLineOptions &applicationOptions );
  virtual int newInstance();
};

KMailMobileApplication::KMailMobileApplication(): KDeclarativeApplication<MainView>()
{
}

KMailMobileApplication::KMailMobileApplication( const KCmdLineOptions &applicationOptions ): KDeclarativeApplication<MainView>( applicationOptions )
{
}

int KMailMobileApplication::newInstance()
{
  KDeclarativeApplication<MainView>::newInstance();
  if ( m_mainView ) {
    m_mainView->handleCommandLine();
  }
  return 0;
}

int main( int argc, char **argv )
{
  qWarning() << "Starting main function" << QDateTime::currentDateTime();

#ifdef KDEPIM_STATIC_LIBS
    ___MailTransport____INIT();
#endif

  const QByteArray& ba = QByteArray( "kmail-mobile" );
  const KLocalizedString name = ki18n( "Kontact Touch Mail" );
  K4AboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "KMail Mobile" ); //has to match the bugzilla product name

  KCmdLineArgs::init( argc, argv, &aboutData );
  KMailMobileApplication app( kmailMobileOptions() );

  if ( !KMailMobileApplication::start() ) {
     return 0;
  }

  return app.exec();
}
