/*
    kngroupbrowser.cpp

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

#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqtimer.h>
#include <tqapplication.h>

#include <kseparator.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>

#include "knnetaccess.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knnntpaccount.h"
#include "kngroupbrowser.h"
#include <tqlabel.h>
#include <tqpushbutton.h>


KNGroupBrowser::KNGroupBrowser(TQWidget *parent, const TQString &caption, KNNntpAccount *a,
                               int buttons, bool newCBact, const TQString &user1, const TQString &user2) :
  KDialogBase( parent, 0L, true, caption, buttons | Help | Ok | Cancel, Ok, true, user1, user2 ),
  incrementalFilter(false), a_ccount(a)
{
  refilterTimer = new TQTimer();

  allList=new TQSortedList<KNGroupInfo>;
  allList->setAutoDelete(true);
  matchList=new TQSortedList<KNGroupInfo>;
  matchList->setAutoDelete(false);

  //create Widgets
  page=new TQWidget(this);
  setMainWidget(page);

  filterEdit=new KLineEdit(page);
  TQLabel *l=new TQLabel(filterEdit,i18n("S&earch:"), page);
  noTreeCB=new TQCheckBox(i18n("Disable &tree view"), page);
  noTreeCB->setChecked(false);
  subCB=new TQCheckBox(i18n("&Subscribed only"), page);
  subCB->setChecked(false);
  newCB=new TQCheckBox(i18n("&New only"), page);
  if (!newCBact)
    newCB->hide();
  newCB->setChecked(false);
  KSeparator *sep=new KSeparator(KSeparator::HLine, page);

  TQFont fnt=font();
  fnt.setBold(true);
  leftLabel=new TQLabel(i18n("Loading groups..."), page);
  rightLabel=new TQLabel(page);
  leftLabel->setFont(fnt);
  rightLabel->setFont(fnt);

  pmGroup=knGlobals.configManager()->appearance()->icon(KNConfig::Appearance::group);
  pmNew=knGlobals.configManager()->appearance()->icon(KNConfig::Appearance::redBall);
  pmRight=BarIconSet( TQApplication::reverseLayout()? "back": "forward");
  pmLeft=BarIconSet(  TQApplication::reverseLayout() ? "forward" : "back");

  arrowBtn1=new TQPushButton(page);
  arrowBtn1->setEnabled(false);
  arrowBtn2=new TQPushButton(page);
  arrowBtn2->setEnabled(false);
  arrowBtn1->setIconSet(pmRight);
  arrowBtn2->setIconSet(pmLeft);
  arrowBtn1->setFixedSize(35,30);
  arrowBtn2->setFixedSize(35,30);

  groupView=new TQListView(page);
  groupView->setRootIsDecorated(true);
  groupView->addColumn(i18n("Name"));
  groupView->addColumn(i18n("Description"));
  groupView->setTreeStepSize(15);

  connect(groupView, TQT_SIGNAL(doubleClicked(TQListViewItem*)),
          this, TQT_SLOT(slotItemDoubleClicked(TQListViewItem*)));

  //layout
  TQGridLayout *topL=new TQGridLayout(page,3,1,0,5);
  TQHBoxLayout *filterL=new TQHBoxLayout(10);
  TQVBoxLayout *arrL=new TQVBoxLayout(10);
  listL=new TQGridLayout(2, 3, 5);

  topL->addLayout(filterL, 0,0);
  topL->addWidget(sep,1,0);
  topL->addLayout(listL, 2,0);

  filterL->addWidget(l);
  filterL->addWidget(filterEdit, 1);
  filterL->addWidget(noTreeCB);
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
  connect(filterEdit, TQT_SIGNAL(textChanged(const TQString&)),
          TQT_SLOT(slotFilterTextChanged(const TQString&)));
  connect(groupView, TQT_SIGNAL(expanded(TQListViewItem*)),
          TQT_SLOT(slotItemExpand(TQListViewItem*)));

  connect(refilterTimer, TQT_SIGNAL(timeout()), TQT_SLOT(slotRefilter()));
  connect(noTreeCB, TQT_SIGNAL(clicked()), TQT_SLOT(slotTreeCBToggled()));
  connect(subCB, TQT_SIGNAL(clicked()), TQT_SLOT(slotSubCBToggled()));
  connect(newCB, TQT_SIGNAL(clicked()), TQT_SLOT(slotNewCBToggled()));

  enableButton(User1,false);
  enableButton(User2,false);

  filterEdit->setFocus();

  TQTimer::singleShot(2, this, TQT_SLOT(slotLoadList()));
}


KNGroupBrowser::~KNGroupBrowser()
{

  knGlobals.netAccess()->stopJobsNntp(KNJobData::JTLoadGroups);
  knGlobals.netAccess()->stopJobsNntp(KNJobData::JTFetchGroups);
  knGlobals.netAccess()->stopJobsNntp(KNJobData::JTCheckNewGroups);

  delete matchList;
  delete allList;
  delete refilterTimer;
}


void KNGroupBrowser::slotReceiveList(KNGroupListData* d)
{
  enableButton(User1,true);
  enableButton(User2,true);

  if (d) {  // d==0 if something has gone wrong...
    delete allList;
    allList = d->extractList();
    incrementalFilter=false;
    slotRefilter();
  }
}


void KNGroupBrowser::changeItemState(const KNGroupInfo &gi, bool s)
{
  TQListViewItemIterator it(groupView);

  for( ; it.current(); ++it)
    if (it.current()->isSelectable() && (static_cast<CheckItem*>(it.current())->info==gi))
      static_cast<CheckItem*>(it.current())->setChecked(s);
}


bool KNGroupBrowser::itemInListView(TQListView *view, const KNGroupInfo &gi)
{
  if(!view) return false;
  TQListViewItemIterator it(view);

  for( ; it.current(); ++it)
    if(static_cast<GroupItem*>(it.current())->info==gi)
      return true;

  return false;
}


void KNGroupBrowser::createListItems(TQListViewItem *parent)
{
  TQString prefix, tlgn, compare;
  TQListViewItem *it;
  CheckItem *cit;
  int colon;
  bool expandit=false;

  if(parent) {
    TQListViewItem *p=parent;
    while(p) {
      prefix.prepend(p->text(0));
      p=p->parent();
    }
  }

  for(KNGroupInfo *gn=matchList->first(); gn; gn=matchList->next()) {

    if(!prefix.isEmpty() && !gn->name.startsWith(prefix))
      if(!compare.isNull())
        break;
      else
        continue;

    compare=gn->name.mid(prefix.length());

    if(!expandit || !compare.startsWith(tlgn)) {
     if((colon=compare.find('.'))!=-1) {
        colon++;
        expandit=true;
      } else {
        colon=compare.length();
        expandit=false;
      }

      tlgn = compare.left(colon);

      if(expandit) {
        if(parent)
          it=new TQListViewItem(parent, tlgn);
        else
          it=new TQListViewItem(groupView, tlgn);

        it->setSelectable(false);
        it->setExpandable(true);
      }
      else {
        if(parent)
          cit=new CheckItem(parent, *gn, this);
        else
          cit=new CheckItem(groupView, *gn, this);
        updateItemState(cit);
      }
    }
  }
}


void KNGroupBrowser::removeListItem(TQListView *view, const KNGroupInfo &gi)
{
  if(!view) return;
  TQListViewItemIterator it(view);

  for( ; it.current(); ++it)
    if(static_cast<GroupItem*>(it.current())->info==gi) {
      delete it.current();
      break;
    }
}


void KNGroupBrowser::slotLoadList()
{
  emit(loadList(a_ccount));
}


void KNGroupBrowser::slotItemExpand(TQListViewItem *it)
{
  if(!it) return;

  if(it->childCount()) {
    kdDebug(5003) << "KNGroupBrowser::slotItemExpand() : has already been expanded, returning" << endl;
    return;
  }

  createListItems(it);

  // center the item - smart scrolling
  delayedCenter = -1;
  int y = groupView->itemPos(it);
  int h = it->height();

  if ( (y+h*4+5) >= (groupView->contentsY()+groupView->visibleHeight()) )
  {
    groupView->ensureVisible(groupView->contentsX(), y+h/2, 0, h/2);
    delayedCenter = y+h/2;
    TQTimer::singleShot(300, this, TQT_SLOT(slotCenterDelayed()));
  }
}


void KNGroupBrowser::slotCenterDelayed()
{
  if (delayedCenter != -1)
    groupView->ensureVisible(groupView->contentsX(), delayedCenter, 0, groupView->visibleHeight()/2);
}


void KNGroupBrowser::slotItemDoubleClicked(TQListViewItem *it)
{
  if (it && (it->childCount()==0)) static_cast<CheckItem*>(it)->setOn(!static_cast<CheckItem*>(it)->isOn());
}


#define MIN_FOR_TREE 200
void KNGroupBrowser::slotFilter(const TQString &txt)
{
  TQString filtertxt = txt.lower();
  TQRegExp reg(filtertxt, false, false);
  CheckItem *cit=0;

  bool notCheckSub = !subCB->isChecked();
  bool notCheckNew = !newCB->isChecked();
  bool notCheckStr = (filtertxt.isEmpty());

  bool isRegexp = filtertxt.contains(TQRegExp("[^a-z0-9\\-\\+.]"));

  bool doIncrementalUpdate = (!isRegexp && incrementalFilter && (filtertxt.left(lastFilter.length())==lastFilter));

  if (doIncrementalUpdate) {
    TQSortedList<KNGroupInfo> *tempList = new TQSortedList<KNGroupInfo>();
    tempList->setAutoDelete(false);

    for(KNGroupInfo *g=matchList->first(); g; g=matchList->next()) {
      if ((notCheckSub||g->subscribed)&&
          (notCheckNew||g->newGroup)&&
          (notCheckStr||(g->name.find(filtertxt)!=-1)))
      tempList->append(g);
    }

    delete matchList;
    matchList=tempList;
  } else {
    matchList->clear();

    for(KNGroupInfo *g=allList->first(); g; g=allList->next()) {
      if ((notCheckSub||g->subscribed)&&
          (notCheckNew||g->newGroup)&&
          (notCheckStr||(isRegexp? (reg.search(g->name,0) != -1):(g->name.find(filtertxt)!=-1))))
        matchList->append(g);
    }
  }

  groupView->clear();

  if((matchList->count() < MIN_FOR_TREE) || noTreeCB->isChecked()) {
    for(KNGroupInfo *g=matchList->first(); g; g=matchList->next()) {
      cit=new CheckItem(groupView, *g, this);
      updateItemState(cit);
    }
  } else {
    createListItems();
  }

  lastFilter = filtertxt;
  incrementalFilter = !isRegexp;

  leftLabel->setText(i18n("Groups on %1: (%2 displayed)").arg(a_ccount->name()).arg(matchList->count()));

  arrowBtn1->setEnabled(false);
  arrowBtn2->setEnabled(false);
}


void KNGroupBrowser::slotTreeCBToggled()
{
  incrementalFilter=false;
  slotRefilter();
}


void KNGroupBrowser::slotSubCBToggled()
{
  incrementalFilter=subCB->isChecked();
  slotRefilter();
}


void KNGroupBrowser::slotNewCBToggled()
{
  incrementalFilter=newCB->isChecked();
  slotRefilter();
}


void KNGroupBrowser::slotFilterTextChanged(const TQString &)
{
  if (subCB->isChecked() || newCB->isChecked())
    slotRefilter();
  else
    refilterTimer->start(200,true);
}


void KNGroupBrowser::slotRefilter()
{
  refilterTimer->stop();
  slotFilter(filterEdit->text());
}


//=======================================================================================


KNGroupBrowser::CheckItem::CheckItem(TQListView *v, const KNGroupInfo &gi, KNGroupBrowser *b) :
  TQCheckListItem(v, gi.name, TQCheckListItem::CheckBox), info(gi), browser(b)
{
  TQString des(gi.description);
  if (gi.status == KNGroup::moderated) {
    setText(0,gi.name+" (m)");
    if (!des.upper().contains(i18n("moderated").upper()))
      des+=i18n(" (moderated)");
  }
  setText(1,des);
}


KNGroupBrowser::CheckItem::CheckItem(TQListViewItem *i, const KNGroupInfo &gi, KNGroupBrowser *b) :
  TQCheckListItem(i, gi.name, TQCheckListItem::CheckBox), info(gi), browser(b)
{
  TQString des(gi.description);
  if (gi.status == KNGroup::moderated) {
    setText(0,gi.name+" (m)");
    if (!des.upper().contains(i18n("moderated").upper()))
      des+=i18n(" (moderated)");
  }
  setText(1,des);
}


KNGroupBrowser::CheckItem::~CheckItem()
{
}


void KNGroupBrowser::CheckItem::setChecked(bool c)
{
  KNGroupBrowser *b=browser;
  browser=0;
  TQCheckListItem::setOn(c);
  browser=b;
}


void KNGroupBrowser::CheckItem::stateChange(bool s)
{
  if(browser) {
    kdDebug(5003) << "KNGroupBrowser::CheckItem::stateChange()" << endl;
    browser->itemChangedState(this, s);
  }
}


//=======================================================================================


KNGroupBrowser::GroupItem::GroupItem(TQListView *v, const KNGroupInfo &gi)
 : TQListViewItem(v, gi.name), info(gi)
{
  if (gi.status == KNGroup::moderated)
    setText(0,gi.name+" (m)");
}


KNGroupBrowser::GroupItem::GroupItem(TQListViewItem *i, const KNGroupInfo &gi)
 : TQListViewItem(i, gi.name), info(gi)
{
}


KNGroupBrowser::GroupItem::~GroupItem()
{
}


//-----------------------------------------

#include "kngroupbrowser.moc"
