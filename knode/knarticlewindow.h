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

#include <qwidget.h>

class KMenuBar;

class KNArticle;
class KNArticleWidget;
class KNArticleCollection;

class KNArticleWindow : public QWidget  {

  Q_OBJECT
	
	public:
		KNArticleWindow(KNArticle *art=0, KNArticleCollection *col=0, QWidget *parent=0, const char *name=0);
		~KNArticleWindow();
	 	KNArticleWidget* artWidget()				{ return artW; }
			
	protected:
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
		KNArticleWidget *artW;
		KMenuBar *menu;
		
	protected slots:
		void slotMenu(int ID);
		
};

#endif
