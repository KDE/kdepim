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

#include <qlist.h>

#include <kaction.h>

namespace KNConfig {
class FilterListWidget;
}

class KNArticleFilter;
class KNFilterDialog;
class KNFilterSelectAction;

class KNFilterManager : public QObject
{
  Q_OBJECT

  public:
    KNFilterManager(KNFilterSelectAction *a, QObject * parent=0, const char * name=0);
    ~KNFilterManager();
    
    void readOptions();
    void saveOptions();


    KNArticleFilter* currentFilter()        { return currFilter; }    
      
    void startConfig(KNConfig::FilterListWidget *fs);
    void endConfig();
    void commitChanges();
    void newFilter();
    void editFilter(KNArticleFilter *f);
    void copyFilter(KNArticleFilter *f);
    void addFilter(KNArticleFilter *f);
    void deleteFilter(KNArticleFilter *f);
    bool newNameIsOK(KNArticleFilter *f, const QString &newName);
          
  protected:
    void loadFilters();
    void saveFilterLists();
    KNArticleFilter* setFilter(const int id);
    KNArticleFilter* byID(int id);
    void updateMenu();
    
    QList<KNArticleFilter> fList;
    KNConfig::FilterListWidget *fset;
    KNArticleFilter *currFilter;
    KNFilterSelectAction *a_ctFilter;
    QValueList<int> menuOrder;  
    bool commitNeeded;
  
  protected slots:
    void slotMenuActivated(int id);
      
  signals:
    void filterChanged(KNArticleFilter *f);     
    
};

#endif

