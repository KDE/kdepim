/*
    kngroupbrowser.h

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

#ifndef KNGROUPBROWSER_H
#define KNGROUPBROWSER_H

#include <qlistview.h>

#include <kdialogbase.h>

#include "kngroupmanager.h"

class KLineEdit;
class QCheckBox;
class QLayout;
class QLabel;
class QGridLayout;

class KNNntpAccount;


class KNGroupBrowser : public KDialogBase {

  Q_OBJECT

  public:
    class CheckItem : public QCheckListItem {

      public:
        CheckItem(QListView *v, const KNGroupInfo &gi, KNGroupBrowser *b);
        CheckItem(QListViewItem *i, const KNGroupInfo &gi, KNGroupBrowser *b);
        ~CheckItem();
        void setChecked(bool c);

        KNGroupInfo info;

      protected:
        void stateChange(bool s);
        KNGroupBrowser *browser;
    };

    class GroupItem : public QListViewItem {

      public:
        GroupItem(QListView *v, const KNGroupInfo &gi);
        GroupItem(QListViewItem *i, const KNGroupInfo &gi);
        ~GroupItem();

        KNGroupInfo info;
    };

    KNGroupBrowser(QWidget *parent, const QString &caption, KNNntpAccount *a, int buttons=0,
                   bool newCBact=false, const QString &user1=QString::null, const QString &user2=QString::null);
    ~KNGroupBrowser();

    KNNntpAccount* account()const      { return a_ccount; }
    virtual void itemChangedState(CheckItem *it, bool s)=0;

  public slots:
    void slotReceiveList(KNGroupListData* d);

  signals:
    void loadList(KNNntpAccount *a);

  protected:
    virtual void updateItemState(CheckItem *it)=0;
    void changeItemState(const KNGroupInfo &gi, bool s);
    bool itemInListView(QListView *view, const KNGroupInfo &gi);
    void removeListItem(QListView *view, const KNGroupInfo &gi);
    void createListItems(QListViewItem *parent=0);

    QWidget *page;
    QListView *groupView;
    int delayedCenter;
    KLineEdit *filterEdit;
    QCheckBox *noTreeCB, *subCB, *newCB;
    QPushButton  *arrowBtn1, *arrowBtn2;
    QPixmap pmGroup, pmNew;
    QIconSet pmRight, pmLeft;
    QGridLayout *listL;
    QLabel *leftLabel, *rightLabel;
    QTimer *refilterTimer;
    QString lastFilter;
    bool incrementalFilter;

    KNNntpAccount *a_ccount;
    QSortedList<KNGroupInfo> *allList, *matchList;

  protected slots:
    void slotLoadList();
    void slotItemExpand(QListViewItem *it);
    void slotCenterDelayed();
    /** double click checks/unchecks (opens/closes) item */
    void slotItemDoubleClicked(QListViewItem *it);
    void slotFilter(const QString &txt);
    void slotTreeCBToggled();
    void slotSubCBToggled();
    void slotNewCBToggled();
    void slotFilterTextChanged(const QString &txt);
    void slotRefilter();

};

#endif
