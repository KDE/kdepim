/***************************************************************************
                          knfiltermanager.h  -  description
                             -------------------
    
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


#ifndef KNFILTERMANAGER_H
#define KNFILTERMANAGER_H

#include <qobject.h>
#include <qpushbutton.h>
#include <qsemimodal.h>

#include <kaction.h>

#include "knarticlefilter.h"
#include "knlistbox.h"


class KNFilterSettings;


class KNFilterSelectAction : public KAction
{
  Q_OBJECT

  public:
    KNFilterSelectAction( const QString& text, const QString& pix,
                          int accel, QObject* parent, const char* name );
    ~KNFilterSelectAction();

    virtual int plug( QWidget* widget, int index = -1 );
    KPopupMenu* popupMenu()    { return p_opup; }
    void setCurrentItem(int id);

  protected slots:
    void slotMenuActivated(int id);

  signals:
    void activated(int id);

  private:
    QPixmap p_ixmap;
    KPopupMenu *p_opup;
    int currentItem;
};



class KNFilterManager : public QObject
{
	Q_OBJECT

	public:
	  KNFilterManager(KNFilterSelectAction *filterMenu);
		KNFilterManager();
		~KNFilterManager();
		
		KNArticleFilter* setFilter(const int id);
		KNArticleFilter* currentFilter()	{ return currFilter; }
			
		void startConfig(KNFilterSettings *fs);
		void endConfig();
		void newFilter();
		void editFilter(KNArticleFilter *f);
		void addFilter(KNArticleFilter *f);
		void deleteFilter(KNArticleFilter *f);
					
	protected:
		void loadFilters();
		void saveFilterLists();
		KNArticleFilter* byID(int id);
		bool nameIsOK(KNArticleFilter *f);
		void updateMenu();
		
		QList<KNArticleFilter> fList;
		KNFilterSettings *fset;
	  KNArticleFilter *currFilter;
		KNFilterSelectAction *menu;
		QValueList<int> menuOrder;		
	
	protected slots:
		void slotMenuActivated(int id);
	  void slotEditDone(KNFilterDialog *fd);
			
	signals:
		void filterChanged(KNArticleFilter *f); 		
		
};

#endif

















