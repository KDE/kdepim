/***************************************************************************
                     knodeview.h - description
 copyright            : (C) 1999 by Christian Thurner
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

#ifndef KNODEVIEW_H
#define KNODEVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <qsplitter.h>

#include <kapp.h>

#include "knarticlewidget.h"
#include "knlistview.h"


class KNodeView : public QSplitter
{
  Q_OBJECT

 	friend class KNodeApp;
 	
 	public:
 		 	
		KNodeView(QWidget *parent=0,const char * name=0);
 		~KNodeView();
  	 		
 		void sepPos(QValueList<int> &vert, QValueList<int> &horz);
 		void setSepPos(const QValueList<int> &vert, const QValueList<int> &horz);				

 		bool isZoomed()	{ return is_Zoomed; }
 		void toggleZoom();
 		
 		void headersSize(QStrList *lst);
 		void setHeadersSize(QStrList *lst);
 		 		
 		void nextArticle();
 		void prevArticle();
 		void nextUnreadArticle();
 		void readThrough();
 		void nextUnreadThread();
 		
 		void nextGroup();
 		void prevGroup();
 		 		
 		void toggleThread();
 		
 		
  protected:
 		void initCollectionView();
    void initHdrView();
 		
 		QSplitter *PanHorz;
 	 	KNArticleWidget *artView;
 	 	KNListView *hdrView;
   	KNListView *collectionView;
    int sPos1, sPos2;
    bool is_Zoomed;

};

#endif // KNODEVIEW_H





























































