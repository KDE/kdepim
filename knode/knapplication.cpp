/*
    knapplication.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <kwin.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kurl.h>
#include <kconfig.h>

#include "knode.h"
#include "knapplication.h"
#include "knconvert.h"

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

  KConfig *conf=KGlobal::config();
  conf->setGroup("GENERAL");
  QString ver=conf->readEntry("Version");

  if(!ver.isEmpty() && ver!=KNODE_VERSION) { //new version installed
    if(KNConvert::needToConvert(ver)) { //we need to convert
      kdDebug(5003) << "KNApplication::newInstance() : conversion needed" << endl;
      KNConvert *convDlg=new KNConvert(ver);
      if(!convDlg->exec()) { //reject()
        if(convDlg->conversionDone()) //conversion has already happened but the user has canceled afterwards
          conf->writeEntry("Version", KNODE_VERSION);
        exit(0);
      }
      else //conversion done
        conf->writeEntry("Version", KNODE_VERSION);
      delete convDlg;
    }
    else //new version but no need to convert anything => just save the new version
      conf->writeEntry("Version", KNODE_VERSION);
  }

  if (mainWidget())
    KWin::setActiveWindow(mainWidget()->winId());
  else {
    if (isRestored()) {
      int n = 1;
      while (KNMainWindow::canBeRestored(n)){
        if (KNMainWindow::classNameOfToplevel(n)=="KNMainWindow") {
          (new KNMainWindow)->restore(n);
          break;
        }
        n++;
      }
    }

    if (!mainWidget()) {
      KNMainWindow* knode = new KNMainWindow;
      knode->show();
    }
  }

  // process URLs...
  KNMainWindow *mainWin = static_cast<KNMainWindow*>(mainWidget());
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count()>0) {
    KURL url=args->url(0);    // we take only one URL
    mainWin->openURL(url);
  }

  kdDebug(5003) << "KNApplication::newInstance() done" << endl;
  return 0;
}

//--------------------------------

#include "knapplication.moc"
