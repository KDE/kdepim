/***************************************************************************
                      kngroupbrowser.cpp  -  description
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

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtimer.h>

#include <kseparator.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "knnetaccess.h"
#include "knjobdata.h"
#include "knode.h"
#include "knglobals.h"
#include "knnntpaccount.h"
#include "kngroupmanager.h"
#include "kngroupbrowser.h"


KNGroupBrowser::KNGroupBrowser(QWidget *parent, const QString &caption, KNNntpAccount *a,
                               int buttons, bool newCBact, const QString &user1, const QString &user2) :
  KDialogBase( parent, 0L, true, caption, buttons | Help | Ok | Cancel, Ok, true, user1, user2 ),
  a_ccount(a)
{
  allList=new QSortedList<KNGroupInfo>;
  allList->setAutoDelete(true);
  matchList=new QSortedList<KNGroupInfo>;
  matchList->setAutoDelete(false);

  //create Widgets
  page=new QWidget(this);
  setMainWidget(page);

  filterEdit=new QLineEdit(page);
  QLabel *l=new QLabel(filterEdit,i18n("&Filter:"), page);
  subCB=new QCheckBox(i18n("&subscribed only"), page);
  subCB->setChecked(false);
  newCB=new QCheckBox(i18n("&new only"), page);
  if (!newCBact)
    newCB->hide();
  newCB->setChecked(false);
  KSeparator *sep=new KSeparator(KSeparator::HLine, page);

  QFont fnt=font();
  fnt.setBold(true);
  leftLabel=new QLabel(i18n("Loading groups..."), page);
  rightLabel=new QLabel(page);
  leftLabel->setFont(fnt);
  rightLabel->setFont(fnt);

  pmGroup=UserIcon("group");
  pmNew=UserIcon("redball");
  pmRight=BarIcon("forward");
  pmLeft=BarIcon("back");

  arrowBtn1=new QPushButton(page);
  arrowBtn1->setEnabled(false);
  arrowBtn2=new QPushButton(page);
  arrowBtn2->setEnabled(false);
  arrowBtn1->setPixmap(pmRight);
  arrowBtn2->setPixmap(pmLeft);
  arrowBtn1->setFixedSize(35,30);
  arrowBtn2->setFixedSize(35,30);

  groupView=new QListView(page);
  groupView->setRootIsDecorated(true);
  groupView->addColumn(i18n("Name"));
  groupView->addColumn(i18n("Description"));
  groupView->setTreeStepSize(15);

  //layout
  QGridLayout *topL=new QGridLayout(page,3,1,0,5);
  QHBoxLayout *filterL=new QHBoxLayout(10);
  QVBoxLayout *arrL=new QVBoxLayout(10);
  listL=new QGridLayout(2, 3, 5);

  topL->addLayout(filterL, 0,0);
  topL->addWidget(sep,1,0);
  topL->addLayout(listL, 2,0);

  filterL->addWidget(l);
  filterL->addWidget(filterEdit, 1);
  filterL->addWidget(subCB);
  if (newCBact)
    filterL->addWidget(newCB);

  listL->addWidget(leftLabel, 0,0);
  listL->addWidget(rightLabel, 0,2);
  listL->addWidget(groupView, 1,0);
  listL->addLayout(arrL, 1,1);
  listL->setRowStretch(1,1);
  listL->setColStretch(0,5);
  listL->setColStretch(2,2);

  arrL->addWidget(arrowBtn1, AlignCenter);
  arrL->addWidget(arrowBtn2, AlignCenter);

  //connect
  connect(filterEdit, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotFilter(const QString&)));
  connect(groupView, SIGNAL(expanded(QListViewItem*)),
    this, SLOT(slotItemExpand(QListViewItem*)));

  connect(subCB, SIGNAL(clicked()), this, SLOT(slotRefilter()));
  connect(newCB, SIGNAL(clicked()), this, SLOT(slotRefilter()));

  enableButton(User1,false);
  enableButton(User2,false);

  QTimer::singleShot(2, this, SLOT(slotLoadList()));
}



KNGroupBrowser::~KNGroupBrowser()
{
  knGlobals.netAccess->stopJobsNntp(KNJobData::JTLoadGroups);
  knGlobals.netAccess->stopJobsNntp(KNJobData::JTFetchGroups);
  knGlobals.netAccess->stopJobsNntp(KNJobData::JTCheckNewGroups);

  delete matchList;
  delete allList;
}



void KNGroupBrowser::slotReceiveList(KNGroupListData* d)
{
  leftLabel->setText(i18n("Groups on %1:").arg(a_ccount->name()));
  enableButton(User1,true);
  enableButton(User2,true);

  if (d) {  // d==0 if something has gone wrong...
    delete allList;
    allList = d->extractList();
    slotRefilter();
  }
}



void KNGroupBrowser::changeItemState(const QString &text, bool s)
{
  QListViewItemIterator it(groupView);

  for( ; it.current(); ++it)
    if(it.current()->text(0)==text && it.current()->isSelectable())
      static_cast<CheckItem*>(it.current())->setChecked(s);
}



bool KNGroupBrowser::itemInListView(QListView *view, const QString &text)
{
  if(!view) return false;
  QListViewItemIterator it(view);

  for( ; it.current(); ++it)
    if(it.current()->text(0)==text)
      return true;

  return false;
}



void KNGroupBrowser::createListItems(QListViewItem *parent)
{
  QCString prefix, tlgn;
  QListViewItem *it;
  CheckItem *cit;
  char *compare=0, *colon=0;
  int prefixLen=0, tlgnLen=0;
  bool expandit=false;

  if(parent) {
    QString tmp("");
    QListViewItem *p=parent;
    while(p) {
      tmp.prepend(p->text(0));
      p=p->parent();
    }
    prefix=tmp.local8Bit();
    prefixLen=prefix.length();
  }

  for(KNGroupInfo *gn=matchList->first(); gn; gn=matchList->next()) {

    if(prefixLen>0 && strncasecmp(gn->name.data(), prefix.data(), prefixLen)!=0)
      if(compare!=0)
        break;
      else
        continue;

    compare=&gn->name[prefixLen];

    if(!expandit || strncasecmp(compare, tlgn.data(), tlgnLen)!=0) {
      if((colon=strstr(compare, "."))) {
        tlgnLen=colon-compare+1;
        expandit=true;
      }
      else {
        tlgnLen=strlen(compare);
        expandit=false;
      }

      tlgn.resize(tlgnLen+2);
      strncpy(tlgn.data(), compare, tlgnLen);
      tlgn.data()[tlgnLen]='\0';

      if(expandit) {
        if(parent)
          it=new QListViewItem(parent, QString(tlgn));
        else
          it=new QListViewItem(groupView, QString(tlgn));

        it->setSelectable(false);
        it->setExpandable(true);
      }
      else {
        if(parent)
          cit=new CheckItem(parent, gn, this);
        else
          cit=new CheckItem(groupView, gn, this);
        updateItemState(cit);
      }
    }
  }
}



void KNGroupBrowser::removeListItem(QListView *view, const QString &text)
{
  if(!view) return;
  QListViewItemIterator it(view);

  for( ; it.current(); ++it)
    if(it.current()->text(0)==text) {
      delete it.current();
      break;
    }
}


void KNGroupBrowser::slotLoadList()
{
  emit(loadList(a_ccount));
}


void KNGroupBrowser::slotItemExpand(QListViewItem *it)
{
  if(!it) return;

  if(it->childCount()) {
    kdDebug(5003) << "KNGroupBrowser::slotItemExpand() : has already been expanded, returning" << endl;
    return;
  }

  createListItems(it);
}


#define MIN_FOR_TREE 50
void KNGroupBrowser::slotFilter(const QString &txt)
{
  QCString filtertxt(txt.local8Bit());
  CheckItem *cit=0;

  matchList->clear();
  groupView->clear();

  bool notCheckSub = !subCB->isChecked();
  bool notCheckNew = !newCB->isChecked();
  bool notCheckStr = (filtertxt.length()<=0);

  for(KNGroupInfo *g=allList->first(); g; g=allList->next()) {
    if ((notCheckSub||g->subscribed)&&
        (notCheckNew||g->newGroup)&&
        (notCheckStr||strstr(g->name.data(), filtertxt.data())))
      matchList->append(g);
  }

  if(matchList->count() < MIN_FOR_TREE) {
    for(KNGroupInfo *g=matchList->first(); g; g=matchList->next()) {
      cit=new CheckItem(groupView, g, this);
      updateItemState(cit);
    }
  } else
    createListItems();

  arrowBtn1->setEnabled(false);
  arrowBtn2->setEnabled(false);
}



void KNGroupBrowser::slotRefilter()
{
  slotFilter(filterEdit->text());
}



//=======================================================================================



KNGroupBrowser::CheckItem::CheckItem(QListView *v, const KNGroupInfo *gi, KNGroupBrowser *b) :
  QCheckListItem(v, gi->name, QCheckListItem::CheckBox), info(gi), browser(b)
{
  setText(1,gi->description);
}



KNGroupBrowser::CheckItem::CheckItem(QListViewItem *i, const KNGroupInfo *gi, KNGroupBrowser *b) :
  QCheckListItem(i, gi->name, QCheckListItem::CheckBox), info(gi), browser(b)
{
  setText(1,gi->description);
}



KNGroupBrowser::CheckItem::~CheckItem()
{
}



void KNGroupBrowser::CheckItem::setChecked(bool c)
{
  KNGroupBrowser *b=browser;
  browser=0;
  QCheckListItem::setOn(c);
  browser=b;
}



void KNGroupBrowser::CheckItem::stateChange(bool s)
{
  if(browser) {
    kdDebug(5003) << "KNGroupBrowser::CheckItem::stateChange()" << endl;
    browser->itemChangedState(this, s);
  }
}


//-----------------------------------------

#include "kngroupbrowser.moc"
