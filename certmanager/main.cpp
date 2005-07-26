/*
    main.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�lvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "aboutdata.h"
#include "certmanager.h"

#include <kleo/cryptobackendfactory.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

int main( int argc, char** argv )
{
  AboutData aboutData;

  KCmdLineArgs::init(argc, argv, &aboutData);
  static const KCmdLineOptions options[] = {
            { "external" , I18N_NOOP("Search for external certificates initially"), 0 },
            { "query " , I18N_NOOP("Initial query string"), 0 },
	    { "import-certificate ", I18N_NOOP("Name of certificate file to import"), 0 },
	    KCmdLineLastOption// End of options.
  };
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KGlobal::locale()->insertCatalogue( "libkleopatra" );
  KGlobal::iconLoader()->addAppDir( "libkleopatra" );

  if( !Kleo::CryptoBackendFactory::instance()->smime() ) {
    KMessageBox::error(0,
			i18n( "<qt>The crypto plugin could not be initialized.<br>"
			      "Certificate Manager will terminate now.</qt>") );
    return -2;
  }

  CertManager* manager = new CertManager( args->isSet("external"),
					  QString::fromLocal8Bit(args->getOption("query")),
					  QString::fromLocal8Bit(args->getOption("import-certificate")) );

  args->clear();
  manager->show();

  return app.exec();
}
