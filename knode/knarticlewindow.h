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

#include <kmainwindow.h>

class KToggleAction;

class KNArticle;
class KNArticleWidget;
class KNArticleCollection;

class KNArticleWindow : public KMainWindow  {

  Q_OBJECT
  
  public:
    KNArticleWindow(KNArticle *art=0);
    ~KNArticleWindow();
    KNArticleWidget* artWidget()        { return artW; }

    static void closeAllWindowsForCollection(KNArticleCollection *col);
      
  protected:
    KNArticleWidget *artW;
    KToggleAction *a_ctShowToolbar;
    static QList<KNArticleWindow> instances;
    
  protected slots:
    void slotFileClose();
    void slotToggleToolBar();
    void slotConfKeys();
    void slotConfToolbar();
};

#endif
