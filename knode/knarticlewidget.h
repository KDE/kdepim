/***************************************************************************
                          knarticlewidget.h  -  description
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


#ifndef KNARTICLEWIDGET_H
#define KNARTICLEWIDGET_H

//bad hack, but we need access to QTextBrowser::anchorAt() !!
#define private protected
#include <qtextbrowser.h>
#undef private

#include <qlist.h>

#include <kaction.h>

class QScrollView;

class KPopupMenu;

class KNArticle;
class KNArticleCollection;
class KNMimeContent;

class KNArticleWidget : public QTextBrowser  {

  Q_OBJECT

  public:
    enum browserType { BTkonqueror=0 , BTnetscape=1 };
    enum anchorType { ATurl, ATauthor, ATreference, ATattachment, ATunknown };

    KNArticleWidget(KActionCollection* actColl, QWidget *parent=0, const char *name=0 );
    ~KNArticleWidget();

//=======================================
    static void readOptions();
    static void saveOptions();
    static void updateInstances();
    static KNArticleWidget* find(KNArticle *a);
    static KNArticleWidget* mainWidget();
    static void showArticle(KNArticle *a);
    static void setFullHeaders(bool b);
    static void toggleFullHeaders();
    static bool fullHeaders();
//=======================================

    bool scrollingDownPossible();       // needed for "read-through"
    void scrollDown();

    void applyConfig();

    void setData(KNArticle *a, KNArticleCollection *c);
    void createHtmlPage();
    void showBlankPage();
    void showErrorMessage(const QString &s);

    void updateContents();

    KNArticle* article()              { return a_rticle; }
    KNArticleCollection* collection() { return c_oll; }


  protected:
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void viewportMousePressEvent(QMouseEvent *e); // RMB for links
    void viewportMouseReleaseEvent(QMouseEvent *e); // automatic copy
    QString toHtmlString(const QString &line, bool parseURLs=true, bool beautification=true);
    void openURL(const QString &url);
    void saveAttachment(int id);
    void openAttachment(int id);
    bool inlinePossible(KNMimeContent *c);
    void setSource(const QString &s); // reimplemented from QTextBrowser
    void anchorClicked(const QString &a, ButtonState button=LeftButton, const QPoint *p=0);

    KNArticle *a_rticle;
    KNArticleCollection *c_oll;
    QList<KNMimeContent> *att;
    bool h_tmlDone;

    KPopupMenu *urlPopup, *attPopup;
    KAction *actSave, *actPrint, *actSelAll, *actCopy;
    KActionCollection *actionCollection;

    static bool showSig, fullHdrs, inlineAtt, openAtt, altAsAtt;
    static QString hexColors[4];
    static QColor txtCol, bgCol, lnkCol;
    static QFont htmlFont;
    static browserType browser;
    static QList<KNArticleWidget> instances;

  protected slots:
    void slotSave();
    void slotPrint();
    void slotSelectAll(); //needed to enable the copy-action

  signals:
    void focusChanged(QFocusEvent*);
    void articleLoaded();              // gets emited when a article is loaded sucessfully

};

#endif
