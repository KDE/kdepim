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
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qlayout.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qapplication.h>

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
#include <qlabel.h>
#include <qpushbutton.h>


KNGroupBrowser::KNGroupBrowser(QWidget *parent, const QString &caption, KNNntpAccount *a,
                               int buttons, bool newCBact, const QString &user1, const QString &user2) :
  KDialogBase( parent, 0L, true, caption, buttons | Help | Ok | Cancel, Ok, true, user1, user2 ),
  incrementalFilter(false), a_ccount(a)
{
  refilterTimer = new QTimer();

  allList=new QSortedList<KNGroupInfo>;
  allList->setAutoDelete(true);
  matchList=new QSortedList<KNGroupInfo>;
  matchList->setAutoDelete(false);

  //create Widgets
  page=new QWidget(this);
  setMainWidget(page);

  filterEdit=new KLineEdit(page);
  QLabel *l=new QLabel(filterEdit,i18n("S&earch:"), page);
  noTreeCB=new QCheckBox(i18n("Disable &tree view"), page);
  noTreeCB->setChecked(false);
  subCB=new QCheckBox(i18n("&Subscribed only"), page);
  subCB->setChecked(false);
  newCB=new QCheckBox(i18n("&New only"), page);
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

  pmGroup=knGlobals.configManager()->appearance()->icon(KNConfig::Appearance::group);
  pmNew=knGlobals.configManager()->appearance()->icon(KNConfig::Appearance::redBall);
  pmRight=BarIconSet( QApplication::reverseLayout()? "back": "forward");
  pmLeft=BarIconSet(  QApplication::reverseLayout() ? "forward" : "back");

  arrowBtn1=new QPushButton(page);
  arrowBtn1->setEnabled(false);
  arrowBtn2=new QPushButton(page);
  arrowBtn2->setEnabled(false);
  arrowBtn1->setIconSet(pmRight);
  arrowBtn2->setIconSet(pmLeft);
  arrowBtn1->setFixedSize(35,30);
  arrowBtn2->setFixedSize(35,30);

  groupView=new QListView(page);
  groupView->setRootIsDecorated(true);
  groupView->addColumn(i18n("Name"));
  groupView->addColumn(i18n("Description"));
  groupView->setTreeStepSize(15);

  connect(groupView, SIGNAL(doubleClicked(QListViewItem*)),
          this, SLOT(slotItemDoubleClicked(QListViewItem*)));

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
  connect(filterEdit, SIGNAL(textChanged(const QString&)),
          SLOT(slotFilterTextChanged(const QString&)));
  connect(groupView, SIGNAL(expanded(QListViewItem*)),
          SLOT(slotItemExpand(QListViewItem*)));

  connect(refilterTimer, SIGNAL(timeout()), SLOT(slotRefilter()));
  connect(noTreeCB, SIGNAL(clicked()), SLOT(slotTreeCBToggled()));
  connect(subCB, SIGNAL(clicked()), SLOT(slotSubCBToggled()));
  connect(newCB, SIGNAL(clicked()), SLOT(slotNewCBToggled()));

  enableButton(User1,false);
  enableButton(User2,false);

  filterEdit->setFocus();

  QTimer::singleShot(2, this, SLOT(slotLoadList()));
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
  QListViewItemIterator it(groupView);

  for( ; it.current(); ++it)
    if (it.current()->isSelectable() && (static_cast<CheckItem*>(it.current())->info==gi))
      static_cast<CheckItem*>(it.current())->setChecked(s);
}


bool KNGroupBrowser::itemInListView(QListView *view, const KNGroupInfo &gi)
{
  if(!view) return false;
  QListViewItemIterator it(view);

  for( ; it.current(); ++it)
    if(static_cast<GroupItem*>(it.current())->info==gi)
      return true;

  return false;
}


void KNGroupBrowser::createListItems(QListViewItem *parent)
{
  QString prefix, tlgn, compare;
  QListViewItem *it;
  CheckItem *cit;
  int colon;
  bool expandit=false;

  if(parent) {
    QListViewItem *p=parent;
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
          it=new QListViewItem(parent, tlgn);
        else
          it=new QListViewItem(groupView, tlgn);

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


void KNGroupBrowser::removeListItem(QListView *view, const KNGroupInfo &gi)
{
  if(!view) return;
  QListViewItemIterator it(view);

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


void KNGroupBrowser::slotItemExpand(QListViewItem *it)
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
    QTimer::singleShot(300, this, SLOT(slotCenterDelayed()));
  }
}


void KNGroupBrowser::slotCenterDelayed()
{
  if (delayedCenter != -1)
    groupView->ensureVisible(groupView->contentsX(), delayedCenter, 0, groupView->visibleHeight()/2);
}


void KNGroupBrowser::slotItemDoubleClicked(QListViewItem *it)
{
  if (it && (it->childCount()==0)) static_cast<CheckItem*>(it)->setOn(!static_cast<CheckItem*>(it)->isOn());
}


#define MIN_FOR_TREE 200
void KNGroupBrowser::slotFilter(const QString &txt)
{
  QString filtertxt = txt.lower();
  QRegExp reg(filtertxt, false, false);
  CheckItem *cit=0;

  bool notCheckSub = !subCB->isChecked();
  bool notCheckNew = !newCB->isChecked();
  bool notCheckStr = (filtertxt.isEmpty());

  bool isRegexp = filtertxt.contains(QRegExp("[^a-z0-9\\-\\+.]"));

  bool doIncrementalUpdate = (!isRegexp && incrementalFilter && (filtertxt.left(lastFilter.length())==lastFilter));

  if (doIncrementalUpdate) {
    QSortedList<KNGroupInfo> *tempList = new QSortedList<KNGroupInfo>();
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


void KNGroupBrowser::slotFilterTextChanged(const QString &)
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


KNGroupBrowser::CheckItem::CheckItem(QListView *v, const KNGroupInfo &gi, KNGroupBrowser *b) :
  QCheckListItem(v, gi.name, QCheckListItem::CheckBox), info(gi), browser(b)
{
  QString des(gi.description);
  if (gi.status == KNGroup::moderated) {
    setText(0,gi.name+" (m)");
    if (!des.upper().contains(i18n("moderated").upper()))
      des+=i18n(" (moderated)");
  }
  setText(1,des);
}


KNGroupBrowser::CheckItem::CheckItem(QListViewItem *i, const KNGroupInfo &gi, KNGroupBrowser *b) :
  QCheckListItem(i, gi.name, QCheckListItem::CheckBox), info(gi), browser(b)
{
  QString des(gi.description);
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


//=======================================================================================


KNGroupBrowser::GroupItem::GroupItem(QListView *v, const KNGroupInfo &gi)
 : QListViewItem(v, gi.name), info(gi)
{
  if (gi.status == KNGroup::moderated)
    setText(0,gi.name+" (m)");
}


KNGroupBrowser::GroupItem::GroupItem(QListViewItem *i, const KNGroupInfo &gi)
 : QListViewItem(i, gi.name), info(gi)
{
}


KNGroupBrowser::GroupItem::~GroupItem()
{
}


//-----------------------------------------

#include "kngroupbrowser.moc"
