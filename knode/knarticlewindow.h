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

#ifndef KNARTICLEWINDOW_H
#define KNARTICLEWINDOW_H

#include <kmainwindow.h>

class KNArticle;
class KNArticleCollection;

namespace KNode {
  class ArticleWidget;
}

class KNArticleWindow : public KMainWindow  {

  Q_OBJECT

  public:
    KNArticleWindow(KNArticle *art);
    ~KNArticleWindow();
    KNode::ArticleWidget* artWidget()const        { return artW; }

    static bool closeAllWindowsForCollection(KNArticleCollection *col, bool force=true);
    static bool closeAllWindowsForArticle(KNArticle *art, bool force=true);
    static bool raiseWindowForArticle(KNArticle *art);   // false: no window found
    static bool raiseWindowForArticle(const QCString &mid);

  protected:
    KNode::ArticleWidget *artW;
    static QValueList<KNArticleWindow*> mInstances;

};

#endif
