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

#ifndef KNGROUPBROWSER_H
#define KNGROUPBROWSER_H

#include <q3listview.h>
//Added by qt3to4:
#include <QIcon>
#include <QPixmap>
#include <QLabel>
#include <QGridLayout>

#include <kdialogbase.h>

#include "kngroupmanager.h"

class KLineEdit;
class QCheckBox;
class QLayout;
class QLabel;
class QGridLayout;

class KNNntpAccount;


/** Base class for group selection dialogs. */
class KNGroupBrowser : public KDialogBase {

  Q_OBJECT

  public:
    /** Checkable list view item with special handling for displaying moderated groups. */
    class CheckItem : public Q3CheckListItem {

      public:
        CheckItem(Q3ListView *v, const KNGroupInfo &gi, KNGroupBrowser *b);
        CheckItem(Q3ListViewItem *i, const KNGroupInfo &gi, KNGroupBrowser *b);
        ~CheckItem();
        void setChecked(bool c);

        KNGroupInfo info;

      protected:
        void stateChange(bool s);
        KNGroupBrowser *browser;
    };

    /** List view item with special handling for displaying moderated groups. */
    class GroupItem : public Q3ListViewItem {

      public:
        GroupItem(Q3ListView *v, const KNGroupInfo &gi);
        GroupItem(Q3ListViewItem *i, const KNGroupInfo &gi);
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
    bool itemInListView(Q3ListView *view, const KNGroupInfo &gi);
    void removeListItem(Q3ListView *view, const KNGroupInfo &gi);
    void createListItems(Q3ListViewItem *parent=0);

    QWidget *page;
    Q3ListView *groupView;
    int delayedCenter;
    KLineEdit *filterEdit;
    QCheckBox *noTreeCB, *subCB, *newCB;
    QPushButton  *arrowBtn1, *arrowBtn2;
    QPixmap pmGroup, pmNew;
    QIcon pmRight, pmLeft;
    QGridLayout *listL;
    QLabel *leftLabel, *rightLabel;
    QTimer *refilterTimer;
    QString lastFilter;
    bool incrementalFilter;

    KNNntpAccount *a_ccount;
    Q3SortedList<KNGroupInfo> *allList, *matchList;

  protected slots:
    void slotLoadList();
    void slotItemExpand(Q3ListViewItem *it);
    void slotCenterDelayed();
    /** double click checks/unchecks (opens/closes) item */
    void slotItemDoubleClicked(Q3ListViewItem *it);
    void slotFilter(const QString &txt);
    void slotTreeCBToggled();
    void slotSubCBToggled();
    void slotNewCBToggled();
    void slotFilterTextChanged(const QString &txt);
    void slotRefilter();

};

#endif
