/*
    knfiltermanager.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNFILTERMANAGER_H
#define KNFILTERMANAGER_H

#include <qlist.h>

#include <kaction.h>

namespace KNConfig {
class FilterListWidget;
}

class KNArticleFilter;
class KNFilterDialog;


class KNFilterSelectAction : public KActionMenu
{
  Q_OBJECT

  public:
    KNFilterSelectAction( const QString& text, const QString& pix,
                          QObject* parent, const char *name );
    ~KNFilterSelectAction();

    void setCurrentItem(int id);

  protected slots:
    void slotMenuActivated(int id);

  signals:
    void activated(int id);

  private:
    int currentItem;
};


class KNFilterManager : public QObject
{
  Q_OBJECT

  public:
    KNFilterManager(KNFilterSelectAction *a, KAction *keybA, QObject * parent=0, const char * name=0);
    ~KNFilterManager();
    
    void readOptions();
    void saveOptions();

    void prepareShutdown();

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
    void slotShowFilterChooser();
      
  signals:
    void filterChanged(KNArticleFilter *f);     
    
};

#endif

