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
#include <qtimer.h>
#include "knjobdata.h"


class KPopupMenu;

class KNArticle;
class KNArticleCollection;
class KNMimeContent;


class KNArticleWidget : public QTextBrowser, public KNJobConsumer {

  Q_OBJECT

  public:
    enum browserType  { BTkonqueror=0 , BTnetscape=1 };
    enum anchorType   { ATurl, ATauthor, ATreference, ATattachment, ATunknown };

    KNArticleWidget(KActionCollection* actColl, QWidget *parent=0, const char *name=0 );
    ~KNArticleWidget();

    bool scrollingDownPossible();       // needed for "read-through"
    void scrollDown();

    void applyConfig();

    void setArticle(KNArticle *a);
		void createHtmlPage();
    void showBlankPage();
    void showErrorMessage(const QString &s);

    void updateContents();

    KNArticle* article()              { return a_rticle; }


  protected:
		void processJob(KNJobData *j);
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
    QList<KNMimeContent> *a_tt;
    bool h_tmlDone, f_ullHdrs;
   	QTimer *t_imer;
    	
    KPopupMenu *u_rlPopup, *a_ttPopup;


  //-------------------------- <Actions> ---------------------------

    KActionCollection *a_ctions;

    KAction *a_ctSave,
            *a_ctPrint,
            *a_ctSelAll,
            *a_ctCopy,
            *a_ctReply,
            *a_ctRemail,
            *a_ctForward,
            *a_ctCancel,
            *a_ctSupersede,
            *a_ctEdit;
    KToggleAction *a_ctToggleFullHdrs;


  protected slots:
    void slotSave();
    void slotPrint();
    void slotSelectAll(); //needed to enable the copy-action
    void slotReply();
    void slotRemail();
    void slotForward();
    void slotCancel();
    void slotSupersede();
    void slotEdit();
    void slotToggleFullHdrs();

  //-------------------------- </Actions> --------------------------

    void slotTimeout();

  signals:
    void focusChanged(QFocusEvent*);


  //----------------------- Static members -------------------------
  public:
    static void configChanged();
    static void articleRemoved(KNArticle *a);
    static void articleChanged(KNArticle *a);
    static void collectionRemoved(KNArticleCollection *c);

  protected:
    static QList<KNArticleWidget> i_nstances;
};

#endif
