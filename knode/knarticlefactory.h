/*
    knarticlefactory.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNARTICLEFACTORY_H
#define KNARTICLEFACTORY_H

#include <qsemimodal.h>
#include <qlist.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "knjobdata.h"
#include "knmime.h"
#include "knlistbox.h"

class KNGroup;
class KNFolder;
class KNComposer;
class KNSendErrorDialog;
class KNGroupManager;
class KNFolderManager;
class KNNntpAccount;

namespace KNConfig {
  class Identity;
};


class KNArticleFactory : public QObject , public KNJobConsumer {

  Q_OBJECT

  public:
    enum replyType { RTgroup, RTmail, RTboth };

    KNArticleFactory(KNFolderManager *fm, KNGroupManager *gm, QObject *p=0, const char *n=0);
    ~KNArticleFactory();

    //factory methods
    void createPosting(KNNntpAccount *a);
    void createPosting(KNGroup *g);
    void createReply(KNRemoteArticle *a, bool post=true, bool mail=false);
    void createForward(KNArticle *a);
    void createCancel(KNArticle *a);
    void createSupersede(KNArticle *a);
    void createMail(KNHeaders::AddressField *address);

    //article handling
    void edit(KNLocalArticle *a);
    void saveArticles(KNLocalArticle::List *l, KNFolder *f);
    bool deleteArticles(KNLocalArticle::List *l, bool ask=true);
    void sendArticles(KNLocalArticle::List *l, bool now=true);
    void sendOutbox();

    //composer handling
    bool closeComposeWindows();    // try to close all composers, return false if user objects
    void deleteComposersForFolder(KNFolder *f);
    void deleteComposerForArticle(KNLocalArticle *a);
    KNComposer* findComposer(KNLocalArticle *a);
    void configChanged();

  protected:
    //job handling
    void processJob(KNJobData *j); //reimplemented from KNJobConsumer

    //article generation
    KNLocalArticle* newArticle(KNGroup *g, QString &sig, bool withXHeaders=true);

    //cancel & supersede
    bool cancelAllowed(KNArticle *a);

    //rewrap procedure
    int findBreakPos(const QString &text, int start);
    void appendTextWPrefix(QString &result, const QString &text, const QString &prefix);

    //send-errors
    void showSendErrorDialog();

    QList<KNComposer> c_ompList;
    KNFolderManager *f_olManager;
    KNGroupManager *g_rpManager;
    KNSendErrorDialog *s_endErrDlg;

  protected slots:
    void slotComposerDone(KNComposer *com);
    void slotSendErrorDialogDone();

};


class KNSendErrorDialog : public QSemiModal  {

  Q_OBJECT

  public:
    KNSendErrorDialog();
    ~KNSendErrorDialog();

    void append(const QString &subject, const QString &error);

  protected:
    class LBoxItem : public KNListBoxItem {
      public:
        LBoxItem(const QString &e, const QString &t, QPixmap *p=0)
          : KNListBoxItem(t, p) , error(e)  {}
        ~LBoxItem() {}

        QString error;
    };

    QListBox *j_obs;
    QLabel *e_rror;
    QPushButton *c_loseBtn;
    QPixmap p_ixmap;

  protected slots:
    void slotHighlighted(int idx);
    void slotCloseBtnClicked();

  signals:
    void dialogDone();

};

#endif //KNARTICLEFACTORY_H
