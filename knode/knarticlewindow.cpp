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
#include <kconfig.h>

#include "kngroup.h"
#include "knsavedarticle.h"
#include "knfetcharticle.h"
#include "knarticlewidget.h"
#include "knsavedarticlemanager.h"
#include "utilities.h"
#include "knglobals.h"
#include "knode.h"
#include "knarticlewindow.h"


KNArticleWindow::KNArticleWindow(KNArticle *art, KNArticleCollection *col)
  : KMainWindow(0, "articleWindow")
{
  if(art)
    setCaption(art->subject());
  //setIcon(UserIcon("posting"));

  artW=new KNArticleWidget(this);
  artW->setData(art, col);
  setCentralWidget(artW);
  connect(artW, SIGNAL(articleLoaded()), SLOT(slotArticleLoaded()));

  *actionCollection() += artW->actions();        // include the actions of the article widget

  // file menu
  KStdAction::close(this, SLOT(slotFileClose()),actionCollection());

  // article menu
  actPostReply = new KAction(i18n("Post &reply"),"message_reply", Key_R , this, SLOT(slotArtReply()),
                             actionCollection(), "article_postReply");
  actPostReply->setEnabled(false);
  actMailReply = new KAction(i18n("&Mail reply"),"mail_reply", Key_A , this, SLOT(slotArtRemail()),
                             actionCollection(), "article_mailReply");
  actMailReply->setEnabled(false);
  actForward = new KAction(i18n("&Forward"), "mail_forward", Key_F , this, SLOT(slotArtForward()),
                           actionCollection(), "article_forward");
  actForward->setEnabled(false);
  actCancel = new KAction(i18n("article","&Cancel"), 0 , this, SLOT(slotArtCancel()),
                          actionCollection(), "article_cancel");
  actCancel->setEnabled(false);
  actSupersede = new KAction(i18n("&Supersede"), 0 , this, SLOT(slotArtSupersede()),
                             actionCollection(), "article_supersede");
  actSupersede->setEnabled(false);

  // settings menu
  actShowToolbar = KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  KStdAction::saveOptions(this, SLOT(slotSaveOptions()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());

  createGUI("knreaderui.rc");

  KConfig *conf = KGlobal::config();
  conf->setGroup("articleWindow_options");
  applyMainWindowSettings(conf);
  actShowToolbar->setChecked(!toolBar()->isHidden());
}



KNArticleWindow::~KNArticleWindow()
{
}



QSize KNArticleWindow::sizeHint() const
{
  return QSize(500,400);    // default optimized for 800x600
}



void KNArticleWindow::slotArticleLoaded()
{
  actPostReply->setEnabled(true);
  actMailReply->setEnabled(true);
  actForward->setEnabled(true);
  actCancel->setEnabled(true);
  actSupersede->setEnabled(true);
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
  if(toolBar()->isVisible())
    toolBar()->hide();
  else
    toolBar()->show();
}



void KNArticleWindow::slotSaveOptions()
{
  KConfig *conf = KGlobal::config();
  conf->setGroup("articleWindow_options");
  saveMainWindowSettings(conf);
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
