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

#include "knarticlewindow.h"
#include "knarticlewidget.h"
#include "knglobals.h"
#include "knsavedarticlemanager.h"
#include "utilities.h"


KNArticleWindow::KNArticleWindow(KNArticle *art, KNArticleCollection *col, const char *name )
	: KTMainWindow(name)
{
	if(art)
		setCaption(art->subject());
  //setIcon(UserIcon("posting"));

	artW=new KNArticleWidget(this);
	artW->setData(art, col);
  setView(artW);

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

  new KAction(i18n("&Cancel"), 0 , this, SLOT(slotArtCancel()),
                   actionCollection(), "article_cancel");
  new KAction(i18n("&Supersede"), 0 , this, SLOT(slotArtSupersede()),
                   actionCollection(), "article_supersede");

  createGUI( "knreaderui.rc",false);
  guiFactory()->addClient(artW->part());
  conserveMemory();

	restoreWindowSize("reader", this, QSize(500,400));
}



KNArticleWindow::~KNArticleWindow()
{
	saveWindowSize("reader", size());	
}



void KNArticleWindow::slotFileClose()
{
  delete this;
}



void KNArticleWindow::slotArtReply()
{
	xTop->sArtManager()->reply(artW->article(),static_cast<KNGroup*>(artW->collection()));
}



void KNArticleWindow::slotArtRemail()
{
	xTop->sArtManager()->reply(artW->article(), 0);
}



void KNArticleWindow::slotArtForward()
{
	xTop->sArtManager()->forward(artW->article());
}



void KNArticleWindow::slotArtCancel()
{
	xTop->sArtManager()->cancel(static_cast<KNFetchArticle*>(artW->article()),static_cast<KNGroup*>(artW->collection()));
}



void KNArticleWindow::slotArtSupersede()
{
	xTop->sArtManager()->supersede(static_cast<KNFetchArticle*>(artW->article()),static_cast<KNGroup*>(artW->collection()));
}


//--------------------------------

#include "knarticlewindow.moc"
