/*
    knapplication.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
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

#include "knode.h"
#include "knapplication.h"
#include "knglobals.h"
#include "kngroupmanager.h"
#include "knaccountmanager.h"
#include "knodeview.h"
#include "knlistview.h"
#include "knnntpaccount.h"
#include "kngroup.h"
#include "kncollectionviewitem.h"


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
      while (KNMainWindow::canBeRestored(n)){
        if (KNMainWindow::classNameOfToplevel(n)=="KNMainWindow") {
          (new KNMainWindow)->restore(n);
          break;
        }
        n++;
      }
    } else {
      KNMainWindow* knode = new KNMainWindow;
      knode->show();
    }
  }

  // process URLs...
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count()>0) {
    KURL url=args->url(0);    // we take only one URL
    if( (url.protocol()=="news") && (url.hasHost()) )
      openURL(url);
    else
      kdDebug(5003) << "KNApplication::newInstance() : ignoring broken URL" << endl;
  }

  kdDebug(5003) << "KNApplication::newInstance() done" << endl;
  return 0;
}


void KNApplication::openURL(const KURL &url)
{
  QString host = url.host();
  unsigned short int port = url.port();
  KNNntpAccount *acc;

  // lets see if we already have an account for this host...
  for(acc=knGlobals.accManager->first(); acc; acc=knGlobals.accManager->next())
    if( acc->server()==host && (port==0 || acc->port()==port) )
      break;

  if(!acc) {
    acc=new KNNntpAccount();
    acc->setName(host);
    acc->setServer(host);

    if(port!=0)
      acc->setPort(port);

    if(url.hasUser() && url.hasPass()) {
      acc->setNeedsLogon(true);
      acc->setUser(url.user());
      acc->setPass(url.pass());
    }

    if(!knGlobals.accManager->newAccount(acc))
      return;
  }

  QString groupname=url.path(-1);
  while(groupname.startsWith("/"))
    groupname.remove(0,1);

  QListViewItem *item=0;
  if(groupname.isEmpty())
    item=acc->listItem();
  else {
    KNGroup *grp= knGlobals.grpManager->group(groupname, acc);

    if(!grp) {
      KNGroupInfo inf(groupname, "");
      knGlobals.grpManager->subscribeGroup(&inf, acc);
      grp=knGlobals.grpManager->group(groupname, acc);
      if(grp)
        item=grp->listItem();
    }
    else
      item=grp->listItem();

  }

  if(item) {
    knGlobals.view->collectionView()->setActive(item, true);
    knGlobals.view->collectionView()->setCurrentItem(item);
    knGlobals.view->collectionView()->ensureItemVisible(item);
  }
}


//--------------------------------

#include "knapplication.moc"
