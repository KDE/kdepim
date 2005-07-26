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
#include "kwatchgnupgmainwin.h"

#include <kuniqueapplication.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>

class KWatchGnuPGApplication : public KUniqueApplication {
public:
  KWatchGnuPGApplication();
  ~KWatchGnuPGApplication();
  virtual int newInstance();
private:
  KWatchGnuPGMainWindow* mMainWin;
};

KWatchGnuPGApplication::KWatchGnuPGApplication()
  : KUniqueApplication(), mMainWin(0)
{
}

KWatchGnuPGApplication::~KWatchGnuPGApplication()
{
  delete mMainWin;
}

int KWatchGnuPGApplication::newInstance()
{
  if( !mMainWin ) {
	mMainWin = new KWatchGnuPGMainWindow( 0, "kwatchgnupg mainwin" );
	setMainWidget( mMainWin );
  }
  mMainWin->show();
  return KUniqueApplication::newInstance();
}

int main( int argc, char** argv )
{
  AboutData aboutData;

  KCmdLineArgs::init(argc, argv, &aboutData);
  static const KCmdLineOptions options[] = {
	KCmdLineLastOption// End of options.
  };
  KCmdLineArgs::addCmdLineOptions( options );
  KWatchGnuPGApplication::addCmdLineOptions();

#if 0
  if (!KWatchGnuPGApplication::start()) {
	kdError() << "KWatchGnuPG is already running!" << endl;
	return 0;
  }
#endif
  KWatchGnuPGApplication app;
  return app.exec();
}
