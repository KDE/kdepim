/*
    This file is part of Akregator2.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "aboutdata.h"
#include "mainwindow.h"
#include "akregator2_options.h"

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kontactinterface/pimuniqueapplication.h>
#include <QtDBus/QtDBus>

#include <QDateTime>
#include <QStringList>
#include <KDebug>

namespace Akregator2 {

class Application : public KontactInterface::PimUniqueApplication {
  public:
    Application() : mMainWindow(0) {}
    ~Application() {}

    int newInstance();

  private:
    Akregator2::MainWindow *mMainWindow;
};

int Application::newInstance()
{
  if (!isSessionRestored())
  {
    QDBusInterface akr("org.kde.akregator2", "/Akregator2", "org.kde.akregator2.part");

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( !mMainWindow ) {
      mMainWindow = new Akregator2::MainWindow();
      mMainWindow->loadPart();
      mMainWindow->setupProgressWidgets();
      if (!args->isSet("hide-mainwindow"))
        mMainWindow->show();
      akr.call( "openStandardFeedList");
    }

    akr.call( "handleCommandLine" );
    args->clear();
  }
  return KUniqueApplication::newInstance();
}

} // namespace Akregator2

int main(int argc, char **argv)
{
    qsrand( QDateTime::currentDateTime().toTime_t() );
    Akregator2::AboutData about;
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( Akregator2::akregator2_options() );
    KUniqueApplication::addCmdLineOptions();
    if ( !Akregator2::Application::start() ) {
        kWarning() << "akregator2 is already running, exiting.";
        exit( 0 );
    }

    Akregator2::Application app;

    // start knotifyclient if not already started. makes it work for people who doesn't use full kde, according to kmail devels
    //KNotifyClient::startDaemon();

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
#undef RESTORE
#define RESTORE(type) { int n = 1;\
    while (KMainWindow::canBeRestored(n)){\
        (new type)->restore(n, false);\
            n++;}}

        RESTORE(Akregator2::MainWindow);
    }

    return app.exec();
}

