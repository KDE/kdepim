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

  // file menu
  KStdAction::save(this, SLOT(slotFileSave()),actionCollection());
  KStdAction::print(artW, SLOT(print()),actionCollection());
  KStdAction::close(this, SLOT(slotFileClose()),actionCollection());

  // edit menu
  actEditCopy = KStdAction::copy(artW, SLOT(copySelection()),actionCollection());
  actEditCopy->setEnabled(false);
  connect(artW->part(),SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
  KStdAction::find(artW, SLOT(findText()),actionCollection());

  // article menu
  new KAction(i18n("Post &reply"),"reply", Key_R , this, SLOT(slotArtReply()),
              actionCollection(), "article_postReply");
  new KAction(i18n("&Mail reply"),"remail", Key_A , this, SLOT(slotArtRemail()),
              actionCollection(), "article_mailReply");
  new KAction(i18n("&Forward"),"fwd", Key_F , this, SLOT(slotArtForward()),
              actionCollection(), "article_forward");

  createGUI( "knreaderui.rc" );

  resize(500,400);                      // default value
	setDialogSize("reader", this);	
}



KNArticleWindow::~KNArticleWindow()
{
	saveDialogSize("reader", this->size());	
}



void KNArticleWindow::slotFileSave()
{
  if(artW->article())
		KNArticleManager::saveArticleToFile(artW->article());
}



void KNArticleWindow::slotFileClose()
{
  delete this;
}



void KNArticleWindow::slotArtReply()
{
	xTop->sArtManager()->reply(artW->article(), (KNGroup*)artW->collection());
}



void KNArticleWindow::slotArtRemail()
{
	xTop->sArtManager()->reply(artW->article(), 0);
}



void KNArticleWindow::slotArtForward()
{
	xTop->sArtManager()->forward(artW->article());
}



void KNArticleWindow::slotSelectionChanged()
{
  actEditCopy->setEnabled(artW->part()->hasSelection());		
}


//--------------------------------

#include "knarticlewindow.moc"
