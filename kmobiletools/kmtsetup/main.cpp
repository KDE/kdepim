/***************************************************************************
   Copyright (C) 2007
   by Davide Bettio <davide.bettio@kdemail.net>
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/


#include "kmtsetupwidget.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <krun.h>
#include <kuser.h>
#include <k3process.h>
#include <kdebug.h>
#include <kstandarddirs.h>

static const char description[] =
    I18N_NOOP("KMobileTools Permission Wizard");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
    KCmdLineLastOption
};

bool sudoRun(KCmdLineArgs *args)
{
    QString thisAppName=KStandardDirs::findExe(args->appName());
    kDebug() << "Trying to start " << thisAppName << endl;
    K3Process process;
    process << "kdesu" << "-t" << K3Process::quote(thisAppName);
    if(process.start( K3Process::Block ) && process.exitStatus()==0 ) return true;
    process.clearArguments();
    process << "gksu" << "-d" << K3Process::quote(thisAppName);
    if(process.start( K3Process::Block ) && process.exitStatus()==0 ) return true;
    return false;
}

int main(int argc, char **argv)
{
    KAboutData about("kmtsetup", I18N_NOOP("KMTSetup"), version, description,
                     KAboutData::License_GPL, "(C) 2006 Davide Bettio 'WindowsUninstall'", 0, 0, "davbet@aliceposta.it");
    about.addAuthor( "Davide Bettio 'WindowsUninstall'", 0, "davbet@aliceposta.it" );
    about.addAuthor( "Marco Gulino", 0, "marco@kmobiletools.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;
    KMTSetupWidget *mainWin = 0;

//     if (app.isSessionRestored())
//     {
//         RESTORE(KMTSetupWidget);
//     }
//     else
//     {
        // no session.. just start up normally
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if( /*false && */! KUser(getuid() ).isSuperUser() )
    {
        if (sudoRun(args)) return 0; else return -1;
    }
    else kDebug() << "We're ROOT! Yuppie!\n";
    mainWin = new KMTSetupWidget();
    app.setMainWidget( mainWin );
    mainWin->show();

    args->clear();
//     }

    // mainWin has WDestructiveClose flag by default, so it will delete itself.
    return app.exec();
}

