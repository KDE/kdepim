/*
    knarticlewidget.h

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

#ifndef KNARTICLEWIDGET_H
#define KNARTICLEWIDGET_H

#include <qbitarray.h>

//bad hack, but we need access to QTextBrowser::anchorAt(). Obsolete with Qt 3.0.
#define private protected
#include <ktextbrowser.h>
#undef private

#include "knjobdata.h"

class QWidget;
class QPopupMenu;

class KAction;
class KActionCollection;
class KToggleAction;
class KSelectAction;
class KPopupMenu;

class KNArticle;
class KNArticleCollection;
class KNMimeContent;


class KNSourceViewWindow : public KTextBrowser {

  Q_OBJECT

  public:
    KNSourceViewWindow(const QString &htmlCode);
    ~KNSourceViewWindow();

  protected:
    void viewportMouseReleaseEvent(QMouseEvent *e); // automatic copy

};


//=============================================================================================================


class KNMimeSource : public QMimeSource {

  public:
    KNMimeSource(QByteArray data, QCString mimeType);
    ~KNMimeSource();

    const char* format(int n = 0) const;
    QByteArray encodedData (const char *) const;

  protected:
    QByteArray d_ata;
    QCString m_imeType;
};


//=============================================================================================================

class KNArticleWidget : public KTextBrowser, public KNJobConsumer {

  Q_OBJECT

  public:
    enum browserType  { BTkonqueror=0 , BTnetscape=1 };
    enum anchorType   { ATurl, ATauthor, ATattachment, ATnews, ATmsgid, ATmailto, ATunknown };

    KNArticleWidget(KActionCollection* actColl, QWidget *parent=0, const char *name=0 );
    ~KNArticleWidget();

    bool scrollingDownPossible();       // needed for "read-through"
    void scrollDown();

    void applyConfig();
    void setBodyPopup(QPopupMenu *popup)  { b_odyPopup = popup; };

    void setArticle(KNArticle *a);
    void createHtmlPage();
    void showBlankPage();
    void showErrorMessage(const QString &s);
    void updateContents();

    KNArticle* article()                { return a_rticle; }

    KSelectAction* setCharsetAction()   { return a_ctSetCharset; }
    KAction* setCharsetKeyboardAction() { return a_ctSetCharsetKeyb; }

  protected:
    void processJob(KNJobData *j);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void viewportMousePressEvent(QMouseEvent *e); // RMB for links
    void viewportMouseReleaseEvent(QMouseEvent *e); // automatic copy
    bool canDecode8BitText(const QCString &charset);
    QString toHtmlString(const QString &line, bool parseURLs=false, bool beautification=false, bool allowRot13=false);
    void openURL(const QString &url);
    void saveAttachment(int id);
    void openAttachment(int id);
    bool inlinePossible(KNMimeContent *c);
    void setSource(const QString &s); // reimplemented from QTextBrowser
    void anchorClicked(const QString &a, ButtonState button=LeftButton, const QPoint *p=0);

    KNArticle *a_rticle;
    QList<KNMimeContent> *a_tt;
    QMimeSourceFactory *f_actory;
    bool h_tmlDone, r_ot13;
    QTimer *t_imer;
    QCString o_verrideCS;
    bool f_orceCS;
      
    KPopupMenu *u_rlPopup, *a_ttPopup;
    QPopupMenu *b_odyPopup;

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
            *a_ctVerify,
            *a_ctSupersede,
            *a_ctSetCharsetKeyb,
            *a_ctViewSource;

    KToggleAction *a_ctToggleFullHdrs,
                  *a_ctToggleRot13,
                  *a_ctToggleFixedFont;
    KSelectAction *a_ctSetCharset;

  protected slots:
    void slotSave();
    void slotPrint();
    void slotSelectAll(); //needed to enable the copy-action
    void slotReply();
    void slotRemail();
    void slotForward();
    void slotCancel();
    void slotSupersede();
    void slotVerify();
    void slotToggleFullHdrs();
    void slotToggleRot13();
    void slotToggleFixedFont();
    void slotSetCharset(const QString&);
    void slotSetCharsetKeyboard();
    void slotViewSource();

  //-------------------------- </Actions> --------------------------

    void slotTimeout();

  signals:
    void focusChanged(QFocusEvent*);

  //----------------------- Static members -------------------------
  public:
    static void configChanged();
    static bool articleVisible(KNArticle *a);
    static void articleRemoved(KNArticle *a);
    static void articleChanged(KNArticle *a);
    static void articleLoadError(KNArticle *a, const QString &error);
    static void collectionRemoved(KNArticleCollection *c);
    static void cleanup();

  protected:
    static QList<KNArticleWidget> i_nstances;

};


//============================================================================================


class KNDisplayedHeader {

  public:
    KNDisplayedHeader();
    ~KNDisplayedHeader();

    //some common headers
    static const char** predefs();

    //name
    const QString& name()               { return n_ame; }
    void setName(const QString &s)      { n_ame = s; }
    bool hasName()                      { return !n_ame.isEmpty(); }

    //translated name
    QString translatedName();                     // *trys* to translate the name
    void setTranslatedName(const QString &s);     // *trys* to retranslate the name to english
    void setTranslateName(bool b)       { t_ranslateName=b; }
    bool translateName()                { return t_ranslateName; }

    //header
    const QString& header()             { return h_eader; }
    void setHeader(const QString &s)    { h_eader = s; }

    //flags
    bool flag(int i)                    { return f_lags.at(i); }
    void setFlag(int i, bool b)         { f_lags.setBit(i, b); }

    //HTML-tags
    void createTags();
    const QString& nameOpenTag()        { return t_ags[0]; }
    const QString& nameCloseTag()       { return t_ags[1]; }
    const QString& headerOpenTag()      { return t_ags[2]; }
    const QString& headerCloseTag()     { return t_ags[3]; }

  protected:
    bool t_ranslateName;
    QString n_ame, h_eader, t_ags[4];
    QBitArray f_lags;

};

#endif
