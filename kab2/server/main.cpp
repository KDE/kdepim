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
#include <kuniqueapp.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

// Local includes
#include "KAddressBookInterface.h"

static const char* description=I18N_NOOP("Kab, The KDE addressbook");
static const char* VERSION="0.0.1";

  int
main(int argc, char * argv[])
{
  // Don't do anything if we're being run as root.
  if (getuid() == 0 || geteuid() == 0) {
      cerr << "Please do not run this server as root (uid 0)" << endl;
      return 1;
  }    

  int prev_umask = umask(077);

  KAboutData aboutData(
    "KAddressBook",
    I18N_NOOP("KAddressBook"),
    VERSION,
    description,
    KAboutData::License_GPL,
    "(c) 1999-2000, The KDE-PIM Team",
    0,
    "http://without.netpedia.net",
    "kde-pim@kde.org"
  );

  aboutData.addAuthor(
    "Rik Hemsley",
    I18N_NOOP("Design and coding"),
    "rik@kde.org",
    "http://without.netpedia.net"
  );

  KCmdLineArgs::init(argc, argv, &aboutData);

  if (!KUniqueApplication::start())
    exit(1);
    
  KUniqueApplication app;

  int retval = app.exec();

  umask(prev_umask);

  return retval;
}

