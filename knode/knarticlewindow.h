/*
    knarticlewindow.h

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

#ifndef KNARTICLEWINDOW_H
#define KNARTICLEWINDOW_H

#include <kmainwindow.h>

class KNArticle;
class KNArticleWidget;
class KNArticleCollection;


class KNArticleWindow : public KMainWindow  {

  Q_OBJECT

  public:
    KNArticleWindow(KNArticle *art);
    ~KNArticleWindow();
    KNArticleWidget* artWidget()const        { return artW; }

    static bool closeAllWindowsForCollection(KNArticleCollection *col, bool force=true);
    static bool closeAllWindowsForArticle(KNArticle *art, bool force=true);
    static bool raiseWindowForArticle(KNArticle *art);   // false: no window found
    static bool raiseWindowForArticle(const QCString &mid);

  protected:
    KAccel *a_ccel;
    KNArticleWidget *artW;
    static QPtrList<KNArticleWindow> instances;

  protected slots:
    void slotFileClose();
    void slotConfKeys();
    void slotConfToolbar();
    void slotNewToolbarConfig();
};

#endif
