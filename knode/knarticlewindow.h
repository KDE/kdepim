/***************************************************************************
                          knarticlewindow.h  -  description
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


#ifndef KNARTICLEWINDOW_H
#define KNARTICLEWINDOW_H

#include <ktmainwindow.h>
#include <kaction.h>

class KNArticle;
class KNArticleWidget;
class KNArticleCollection;

class KNArticleWindow : public KTMainWindow  {

  Q_OBJECT
	
	public:
		KNArticleWindow(KNArticle *art=0, KNArticleCollection *col=0, const char *name=0);
		~KNArticleWindow();
	 	KNArticleWidget* artWidget()				{ return artW; }
			
	protected:
		KAction  *actEditCopy;
		KNArticleWidget *artW;
		
	protected slots:
  	void slotFileSave();
  	void slotFileClose();
  	void slotArtReply();
  	void slotArtRemail();
  	void slotArtForward();
		void slotSelectionChanged();		
};

#endif
