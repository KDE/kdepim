/***************************************************************************
                       kngroupbrowser.h  -  description
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


#ifndef KNGROUPBROWSER_H
#define KNGROUPBROWSER_H

#include <qlistview.h>
#include <qsortedlist.h>
#include <qcheckbox.h>

#include <kdialogbase.h>

class QLineEdit;
class QCheckBox;
class QLayout;
class QLabel;

class KNNntpAccount;
class KNGroupInfo;
class KNGroupListData;

class KNGroupBrowser : public KDialogBase {

  Q_OBJECT

  public:
    class CheckItem : public QCheckListItem {

      public:
        CheckItem(QListView *v, const KNGroupInfo *gi, KNGroupBrowser *b);
        CheckItem(QListViewItem *i, const KNGroupInfo *gi, KNGroupBrowser *b);
        ~CheckItem();
        void setChecked(bool c);

        const KNGroupInfo *info;

      protected:
        void stateChange(bool s);
        KNGroupBrowser *browser;
    };

    KNGroupBrowser(QWidget *parent, const QString &caption, KNNntpAccount *a, int buttons=0,
                   bool newCBact=false, const QString &user1=QString::null, const QString &user2=QString::null);
    ~KNGroupBrowser();

    KNNntpAccount* account()      { return a_ccount; }
    virtual void itemChangedState(CheckItem *it, bool s)=0;

  public slots:
    void slotReceiveList(KNGroupListData* d);

  signals:
    void loadList(KNNntpAccount *a);

  protected:
    virtual void updateItemState(CheckItem *it)=0;
    void changeItemState(const QString &text, bool s);
    bool itemInListView(QListView *view, const QString &text);
    void removeListItem(QListView *view, const QString &text);
    void createListItems(QListViewItem *parent=0);

    QWidget *page;
    QListView *groupView;
    QLineEdit *filterEdit;
    QCheckBox *subCB, *newCB;
    QPushButton  *arrowBtn1, *arrowBtn2;
    QPixmap pmGroup, pmNew,
            pmRight, pmLeft;
    QGridLayout *listL;
    QLabel *leftLabel, *rightLabel;

    KNNntpAccount *a_ccount;
    QSortedList<KNGroupInfo> *allList, *matchList;

  protected slots:
    void slotLoadList();
    void slotItemExpand(QListViewItem *it);
    void slotFilter(const QString &txt);
    void slotRefilter();

};


#endif
