/*  -*- mode: C++; c-file-style: "gnu" -*-
    main.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include <cryptplugfactory.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>

int main( int argc, char** argv )
{
  AboutData aboutData;

  KCmdLineArgs::init(argc, argv, &aboutData);
  static const KCmdLineOptions options[] = {
            { "+name", I18N_NOOP("The name of the plugin"), 0 },
            { "+lib" , I18N_NOOP("The library of the plugin"), 0 },
            { "external" , I18N_NOOP("Search for external certificates initially"), 0 },
            { "query " , I18N_NOOP("Initial query string"), 0 },
	    { "import-certificate ", I18N_NOOP("Name of certificte file to import"), 0 },
	    KCmdLineLastOption// End of options.
  };
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if( !Kleo::CryptPlugFactory::instance()->smime() ) {
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

  QObject::connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
  return app.exec();
}
