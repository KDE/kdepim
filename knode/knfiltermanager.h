/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNFILTERMANAGER_H
#define KNFILTERMANAGER_H

#include <qglobal.h>
#include <qvaluelist.h>

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
    KNFilterManager(QObject * parent = 0, const char * name = 0);
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

    // Allow to delay the setup of UI elements, since the knode part may not
    // be available when the config dialog is called
    void setMenuAction(KNFilterSelectAction *a, KAction *keybA);

  protected:
    void loadFilters();
    void saveFilterLists();
    KNArticleFilter* setFilter(const int id);
    KNArticleFilter* byID(int id);
    void updateMenu();

    QValueList<KNArticleFilter*> mFilterList;
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

