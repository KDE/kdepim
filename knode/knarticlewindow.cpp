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

#include <kmenubar.h>

#include <klocale.h>
#include <qpopupmenu.h>
#include "knarticlewindow.h"
#include "knarticlewidget.h"
#include "knglobals.h"
#include "knsavedarticlemanager.h"

#define ID_SAVE	10
#define ID_PRINT 20
#define ID_CLOSE 30
#define ID_PREPLY 40
#define ID_MREPLY 50
#define ID_FWD 60


KNArticleWindow::KNArticleWindow(KNArticle *art, KNArticleCollection *col, QWidget *parent, const char *name )
	: QWidget(parent,name)
{
	if(art){
		QCString title="KNode: "+art->subject();
		setCaption(title);
	}
	artW=new KNArticleWidget(this);
	artW->setData(art, col);
	
	menu=new KMenuBar(this);
	//menu->setLineWidth(1);
	//menu->setFrameStyle(QFrame::Panel | QFrame::Raised);
		
	QPopupMenu *file=new QPopupMenu();
	file->insertItem(i18n("&Save"), ID_SAVE);
	file->insertItem(i18n("&Print"), ID_PRINT);
	file->insertItem(i18n("&Close"), ID_CLOSE);
	
	QPopupMenu *article=new QPopupMenu();
	article->insertItem(i18n("&Post reply"), ID_PREPLY);
	article->insertItem(i18n("&Mail reply"), ID_MREPLY);
	article->insertItem(i18n("&Forward"), ID_FWD);
		
	menu->insertItem(i18n("&File"), file);
	menu->insertItem(i18n("&Article"), article);
		
	connect(menu, SIGNAL(activated(int)), this, SLOT(slotMenu(int)));	
	setIcon(UserIcon("posting.xpm"));
}



KNArticleWindow::~KNArticleWindow()
{
}



void KNArticleWindow::slotMenu(int ID)
{
	switch(ID) {
	
		case ID_SAVE :
			if(artW->article()) {
				KNArticleManager::saveArticleToFile(artW->article());
			}
		break;
		
		case ID_PRINT :
#warning FIXME printing not yet implemented in khtml
//			artW->view()->print();
		break;
		
		case ID_CLOSE :
			delete this;
		break;
	
		case ID_PREPLY :
			xTop->sArtManager()->reply(artW->article(), (KNGroup*)artW->collection());
		break;
		
		case ID_MREPLY :
			xTop->sArtManager()->reply(artW->article(), 0);
		break;
		
		case ID_FWD :
			xTop->sArtManager()->forward(artW->article());
		break;	
	}	
}



void KNArticleWindow::resizeEvent(QResizeEvent *)
{
	menu->setGeometry(0,0, width(), menu->height());
	artW->setGeometry(0, menu->height()+1, width(), height()-menu->height()-1);
}



void KNArticleWindow::closeEvent(QCloseEvent *)
{
	delete this;
}
