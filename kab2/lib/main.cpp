/*
    KAddressBook version 2
    
    Copyright (C) 1999 The KDE PIM Team <kde-pim@kde.org>
    
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

// Local includes
#include "KAddressBookInterface.h"

  int
main(int argc, char * argv[])
{
  // Don't do anything if we're being run as root.
  if (getuid() == 0 || geteuid() == 0) {
      cerr << "Please do not run this server as root (uid 0)" << endl;
      return 1;
  }    

  int prev_umask = umask(077);

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

  cerr << "Addressbook server `" << *name_it << "' started" << endl;
  
  if (fork() == 0) {
    
    KApplication * app = new KApplication(argc, argv, "kab");
    KAddressBook * ab = new KAddressBook((*name_it).utf8(), *path_it);
    app->exec();
  }
  
  umask(prev_umask);
  return 0;
}

