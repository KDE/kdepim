/***************************************************************************
                          knarticlewindow.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kstdaction.h>
#include <klocale.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <khtml_part.h>

#include "kngroup.h"
#include "knsavedarticle.h"
#include "knfetcharticle.h"
#include "knarticlewidget.h"
#include "knsavedarticlemanager.h"
#include "utilities.h"
#include "knglobals.h"
#include "knode.h"
#include "knarticlewindow.h"


KNArticleWindow::KNArticleWindow(KNArticle *art, KNArticleCollection *col, const char *name )
  : KMainWindow(0, name)
{
  if(art)
    setCaption(art->subject());
  //setIcon(UserIcon("posting"));

  artW=new KNArticleWidget(this);
  artW->setData(art, col);
  setCentralWidget(artW);

  *actionCollection() += artW->actions();        // include the actions of the article widget

  // file menu
  KStdAction::close(this, SLOT(slotFileClose()),actionCollection());

  // article menu
  new KAction(i18n("Post &reply"),"reply", Key_R , this, SLOT(slotArtReply()),
              actionCollection(), "article_postReply");
  new KAction(i18n("&Mail reply"),"remail", Key_A , this, SLOT(slotArtRemail()),
              actionCollection(), "article_mailReply");
  new KAction(i18n("&Forward"),"fwd", Key_F , this, SLOT(slotArtForward()),
              actionCollection(), "article_forward");
  new KAction(i18n("article","&Cancel"), 0 , this, SLOT(slotArtCancel()),
                   actionCollection(), "article_cancel");
  new KAction(i18n("&Supersede"), 0 , this, SLOT(slotArtSupersede()),
                   actionCollection(), "article_supersede");

  // settings menu
  KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());

  createGUI("knreaderui.rc",false);
  //guiFactory()->addClient(artW->part());
  conserveMemory();

  restoreWindowSize("reader", this, QSize(500,400));
}



KNArticleWindow::~KNArticleWindow()
{
  saveWindowSize("reader", size()); 
}



void KNArticleWindow::slotFileClose()
{
  close();
}



void KNArticleWindow::slotArtReply()
{
  knGlobals.sArtManager->reply(artW->article(),static_cast<KNGroup*>(artW->collection()));
}



void KNArticleWindow::slotArtRemail()
{
  knGlobals.sArtManager->reply(artW->article(), 0);
}



void KNArticleWindow::slotArtForward()
{
  knGlobals.sArtManager->forward(artW->article());
}



void KNArticleWindow::slotArtCancel()
{
  knGlobals.sArtManager->cancel(static_cast<KNFetchArticle*>(artW->article()),static_cast<KNGroup*>(artW->collection()));
}



void KNArticleWindow::slotArtSupersede()
{
  knGlobals.sArtManager->supersede(static_cast<KNFetchArticle*>(artW->article()),static_cast<KNGroup*>(artW->collection()));
}



void KNArticleWindow::slotToggleToolBar()
{
  if(toolBar("mainToolBar")->isVisible())
    toolBar("mainToolBar")->hide();
  else
    toolBar("mainToolBar")->show();
}



void KNArticleWindow::slotConfKeys()
{
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}


    
void KNArticleWindow::slotConfToolbar()
{
  KEditToolbar *dlg = new KEditToolbar(guiFactory(),this);
  if (dlg->exec()) {
    //guiFactory()->removeClient(artW->part());
    createGUI("knreaderui.rc",false);
    //guiFactory()->addClient(artW->part());
    conserveMemory();
  }
  delete dlg;
}


//--------------------------------

#include "knarticlewindow.moc"
