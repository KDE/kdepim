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
#include <kcmdlineargs.h>
#include <kurl.h>

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

  // process URLs...
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count()>0) {
    KURL url = args->url(0);    // we take only one URL
    if ((url.protocol()=="news")&&(url.hasHost()))
      static_cast<KNodeApp*>(mainWidget())->openURL(url);
    else
      kdDebug(5003) << "ignoring broken URL" << endl;
  }

  kdDebug(5003) << "KNApplication::newInstance() done" << endl;
  return 0;
}

//--------------------------------

#include "knapplication.moc"
