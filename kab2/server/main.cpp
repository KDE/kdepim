/*
    KAddressBook version 2
    
    Copyright (C) 1999 Rik Hemsley rik@kde.org
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// System includes
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream.h>

// KDE includes
#include <kapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>

// Local includes
#include "KAddressBookInterface.h"

static const char *description=I18N_NOOP("Addressbook Server for KDE");
static const char *version="2.0pre";
static const KCmdLineOptions options[] =
{
        {"+server", I18N_NOOP("Name of addressbook server"), 0},
	{"+path", I18N_NOOP("Addressbook server directory"), 0},
	{0,0,0}
};


  int
main(int argc, char * argv[])
{
  // Don't do anything if we're being run as root.
  if (getuid() == 0 || geteuid() == 0) {
      cerr << "Please do not run this server as root (uid 0)" << endl;
      return 1;
  }    

  int prev_umask = umask(077);

  KAboutData aboutData("kab_server", I18N_NOOP("KAB2 Server"),
    version, description, KAboutData::License_GPL,
    "(c) 1999, Rik Hemsley");
  aboutData.addAuthor("Rik Hemsley",0, "rik@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication::addCmdLineOptions();

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

/*
  KStartParams args(argc, argv);

  QStringList::Iterator name_it;
  QStringList::Iterator path_it;

  bool haveName = args.find("--name", "-n", true, name_it);
  bool havePath = args.find("--path", "-p", true, path_it);

  if (!haveName || !havePath) {
    qDebug("Usage: " + QString(argv[0]) + " --name <name> --path <path>");
    exit(1);
  }

  ++name_it;
  ++path_it;
*/
  if (args->count() != 2) KCmdLineArgs::usage();

  cerr << "Addressbook server `" << args->arg(0) << "' started" << endl;
  
  if (fork() == 0) {
    
    KApplication * app = new KApplication(argc, argv, "kab");
    KAddressBook * ab = new KAddressBook(args->arg(0), args->arg(1));
    app->exec();
  }
  
  umask(prev_umask);
  return 0;
}

