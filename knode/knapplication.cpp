/***************************************************************************
                        knapplication.cpp  -  description
 copyright            : (C) 2000 by Christian Gebauer
 email                : gebauer@bigfoot.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kwin.h>
#include <kdebug.h>

#include "knode.h"
#include "knapplication.h"


KNApplication::KNApplication()
 : KUniqueApplication()
{
}


KNApplication::~KNApplication()
{
}


int KNApplication::newInstance()
{
  kdDebug(5003) << "KNApplication::newInstance()" << endl;

  if (mainWidget())
    KWin::setActiveWindow(mainWidget()->winId());
  else {
    if (isRestored()) {
      int n = 1;
      while (KNodeApp::canBeRestored(n)){
        if (KNodeApp::classNameOfToplevel(n)=="KNodeApp") {
          (new KNodeApp)->restore(n);
          break;
        }
        n++;
      }
    } else {
      KNodeApp* knode = new KNodeApp;
      knode->show();
    }
  }

  kdDebug(5003) << "KNApplication::newInstance() done" << endl;
  return 0;
}

//--------------------------------

#include "knapplication.moc"
