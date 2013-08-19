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

#ifndef KNODE_KNARTICLEWINDOW_H
#define KNODE_KNARTICLEWINDOW_H

#include "knarticle.h"
#include "knarticlecollection.h"

#include <kxmlguiwindow.h>

#include <QByteArray>
#include <QList>

namespace KNode {

class ArticleWidget;

/** A stand-alone article viewer window. */
class ArticleWindow : public KXmlGuiWindow  {

  Q_OBJECT

  public:
    /** Create a new article window.
     * @param art The article to show in this window.
     */
    explicit ArticleWindow( KNArticle::Ptr art );
    /// Destroy this article viewer window.
    ~ArticleWindow();
    /** Returns the article widget of this window. */
    ArticleWidget* articleWidget() const { return mArticleWidget; }

    /// List of article windows.
    typedef QList<KNode::ArticleWindow*> List;

    /** Close all article windows showing articles from the given collection.
     * @param col The article collection (folder/group).
     * @param force Really close the windows.
     * @return true if all windows have been closed.
     */
    static bool closeAllWindowsForCollection( KNArticleCollection::Ptr col, bool force = true );
    /** Clise all windows showing the given article.
     * @param art Close all windows showing this article.
     * @param force Really close the windows.
     * @return true if all windows have been closed.
     */
    static bool closeAllWindowsForArticle( KNArticle::Ptr art, bool force = true );
    /** Raise the article window for the given article.
     * @param art The article.
     * @return false if no article window was found.
     */
    static bool raiseWindowForArticle( KNArticle::Ptr art );
    /** Raise the article window showing a specific article.
     * @param mid Message-ID of the article.
     * @return false if no article was found.
     */
    static bool raiseWindowForArticle(const QByteArray &mid);

  protected:
    /// The article widget of this window.
    ArticleWidget *mArticleWidget;
    /// List of all article windows.
    static List mInstances;
};

}

#endif // KNODE_KNARTICLEWINDOW_H
