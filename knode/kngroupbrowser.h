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
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNGROUPBROWSER_H
#define KNGROUPBROWSER_H

#include <tqlistview.h>

#include <kdialogbase.h>

#include "kngroupmanager.h"

class KLineEdit;
class TQCheckBox;
class TQLayout;
class TQLabel;
class TQGridLayout;

class KNNntpAccount;


class KNGroupBrowser : public KDialogBase {

  Q_OBJECT

  public:
    class CheckItem : public TQCheckListItem {

      public:
        CheckItem(TQListView *v, const KNGroupInfo &gi, KNGroupBrowser *b);
        CheckItem(TQListViewItem *i, const KNGroupInfo &gi, KNGroupBrowser *b);
        ~CheckItem();
        void setChecked(bool c);

        KNGroupInfo info;

      protected:
        void stateChange(bool s);
        KNGroupBrowser *browser;
    };

    class GroupItem : public TQListViewItem {

      public:
        GroupItem(TQListView *v, const KNGroupInfo &gi);
        GroupItem(TQListViewItem *i, const KNGroupInfo &gi);
        ~GroupItem();

        KNGroupInfo info;
    };

    KNGroupBrowser(TQWidget *parent, const TQString &caption, KNNntpAccount *a, int buttons=0,
                   bool newCBact=false, const TQString &user1=TQString::null, const TQString &user2=TQString::null);
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
    bool itemInListView(TQListView *view, const KNGroupInfo &gi);
    void removeListItem(TQListView *view, const KNGroupInfo &gi);
    void createListItems(TQListViewItem *parent=0);

    TQWidget *page;
    TQListView *groupView;
    int delayedCenter;
    KLineEdit *filterEdit;
    TQCheckBox *noTreeCB, *subCB, *newCB;
    TQPushButton  *arrowBtn1, *arrowBtn2;
    TQPixmap pmGroup, pmNew;
    TQIconSet pmRight, pmLeft;
    TQGridLayout *listL;
    TQLabel *leftLabel, *rightLabel;
    TQTimer *refilterTimer;
    TQString lastFilter;
    bool incrementalFilter;

    KNNntpAccount *a_ccount;
    TQSortedList<KNGroupInfo> *allList, *matchList;

  protected slots:
    void slotLoadList();
    void slotItemExpand(TQListViewItem *it);
    void slotCenterDelayed();
    /** double click checks/unchecks (opens/closes) item */
    void slotItemDoubleClicked(TQListViewItem *it);
    void slotFilter(const TQString &txt);
    void slotTreeCBToggled();
    void slotSubCBToggled();
    void slotNewCBToggled();
    void slotFilterTextChanged(const TQString &txt);
    void slotRefilter();

};

#endif
