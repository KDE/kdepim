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

#include "kngroupbrowser.h"

#include "knconfigmanager.h"
#include "knglobals.h"
#include "knnntpaccount.h"
#include "scheduler.h"

#include <kseparator.h>
#include <kdebug.h>
#include <klineedit.h>
#include <QCheckBox>
#include <QTimer>
#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>


KNGroupBrowser::KNGroupBrowser(QWidget *parent, const QString &caption, KNNntpAccount::Ptr a,
                               ButtonCodes buttons, bool newCBact, const QString &user1, const QString &user2) :
  KDialog( parent ),
  incrementalFilter(false), a_ccount(a)
{
  setCaption( caption );
  setButtons( buttons | Help | Ok | Cancel );
  setButtonGuiItem( User1, KGuiItem(user1) );
  setButtonGuiItem( User2, KGuiItem(user2) );
  refilterTimer = new QTimer();
  refilterTimer->setSingleShot( true );

  allList=new QList<KNGroupInfo>;
  matchList=new QList<KNGroupInfo>;

  //create Widgets
  page=new QWidget(this);
  setMainWidget(page);

  filterEdit=new KLineEdit(page);
  QLabel *l=new QLabel(i18n("S&earch:"),page);
  l->setBuddy(filterEdit);
  filterEdit->setClearButtonShown( true );
  noTreeCB=new QCheckBox(i18n("Disable &tree view"), page);
  noTreeCB->setChecked(false);
  subCB=new QCheckBox(i18n("&Subscribed only"), page);
  subCB->setChecked(false);
  newCB=new QCheckBox(i18n("&New only"), page);
  if (!newCBact)
    newCB->hide();
  newCB->setChecked(false);
  KSeparator *sep = new KSeparator( Qt::Horizontal, page );

  QFont fnt=font();
  fnt.setBold(true);
  leftLabel=new QLabel(i18n("Loading groups..."),page);
  rightLabel=new QLabel(page);
  leftLabel->setFont(fnt);
  rightLabel->setFont(fnt);

  pmGroup=knGlobals.configManager()->appearance()->icon(KNode::Appearance::group);
  pmNew=knGlobals.configManager()->appearance()->icon(KNode::Appearance::redBall);
  pmRight=KIcon( QApplication::isRightToLeft()? "go-previous": "go-next");
  pmLeft=KIcon(  QApplication::isRightToLeft() ? "go-next" : "go-previous");

  arrowBtn1=new QPushButton(page);
  arrowBtn1->setEnabled(false);
  arrowBtn2=new QPushButton(page);
  arrowBtn2->setEnabled(false);
  arrowBtn1->setIcon(pmRight);
  arrowBtn2->setIcon(pmLeft);
  arrowBtn1->setFixedSize(35,30);
  arrowBtn2->setFixedSize(35,30);

  groupView=new Q3ListView(page);
  groupView->setRootIsDecorated(true);
  groupView->addColumn(i18n("Name"));
  groupView->addColumn(i18n("Description"));
  groupView->setTreeStepSize(15);

  connect(groupView, SIGNAL(doubleClicked(Q3ListViewItem*)),
          this, SLOT(slotItemDoubleClicked(Q3ListViewItem*)));

  //layout
  QGridLayout *topL=new QGridLayout(page);
  topL->setSpacing(5);
  topL->setMargin(0);
  QHBoxLayout *filterL=new QHBoxLayout();
  filterL->setSpacing(10);
  QVBoxLayout *arrL=new QVBoxLayout();
  arrL->setSpacing(10);
  listL=new QGridLayout();
  listL->setSpacing(5);

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
  listL->setColumnStretch(0,5);
  listL->setColumnStretch(2,2);

  arrL->addWidget( arrowBtn1, Qt::AlignCenter );
  arrL->addWidget( arrowBtn2, Qt::AlignCenter );

  //connect
  connect(filterEdit, SIGNAL(textChanged(QString)),
          SLOT(slotFilterTextChanged(QString)));
  connect(groupView, SIGNAL(expanded(Q3ListViewItem*)),
          SLOT(slotItemExpand(Q3ListViewItem*)));

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

  knGlobals.scheduler()->cancelJobs( KNJobData::JTLoadGroups );
  knGlobals.scheduler()->cancelJobs( KNJobData::JTFetchGroups );

  delete matchList;
  delete allList;
  delete refilterTimer;
}


void KNGroupBrowser::slotReceiveList( KNGroupListData::Ptr d )
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
  Q3ListViewItemIterator it(groupView);

  for( ; it.current(); ++it)
    if (it.current()->isSelectable() && (static_cast<CheckItem*>(it.current())->info==gi))
      static_cast<CheckItem*>(it.current())->setChecked(s);
}


bool KNGroupBrowser::itemInListView(Q3ListView *view, const KNGroupInfo &gi)
{
  if(!view) return false;
  Q3ListViewItemIterator it(view);

  for( ; it.current(); ++it)
    if(static_cast<GroupItem*>(it.current())->info==gi)
      return true;

  return false;
}


void KNGroupBrowser::createListItems(Q3ListViewItem *parent)
{
  QString prefix, tlgn, compare;
  Q3ListViewItem *it;
  CheckItem *cit;
  int colon;
  bool expandit=false;

  if(parent) {
    Q3ListViewItem *p=parent;
    while(p) {
      prefix.prepend(p->text(0));
      p=p->parent();
    }
  }

  qSort(*matchList);
  Q_FOREACH(const KNGroupInfo& gn, *matchList) {

    if(!prefix.isEmpty() && !gn.name.startsWith(prefix)) {
      if(!compare.isNull())
        break;
      else
        continue;
    }

    compare=gn.name.mid(prefix.length());

    if(!expandit || !compare.startsWith(tlgn)) {
     if( ( colon = compare.indexOf('.') ) != -1 ) {
        colon++;
        expandit=true;
      } else {
        colon=compare.length();
        expandit=false;
      }

      tlgn = compare.left(colon);

      if(expandit) {
        if(parent)
          it=new Q3ListViewItem(parent, tlgn);
        else
          it=new Q3ListViewItem(groupView, tlgn);
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


void KNGroupBrowser::removeListItem(Q3ListView *view, const KNGroupInfo &gi)
{
  if(!view) return;
  Q3ListViewItemIterator it(view);

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


void KNGroupBrowser::slotItemExpand(Q3ListViewItem *it)
{
  if(!it) return;

  if(it->childCount()) {
    kDebug(5003) <<"KNGroupBrowser::slotItemExpand() : has already been expanded, returning";
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


void KNGroupBrowser::slotItemDoubleClicked(Q3ListViewItem *it)
{
  if (it && (it->childCount()==0)) static_cast<CheckItem*>(it)->setOn(!static_cast<CheckItem*>(it)->isOn());
}


#define MIN_FOR_TREE 200
void KNGroupBrowser::slotFilter(const QString &txt)
{
  QString filtertxt = txt.toLower();
  QRegExp reg(filtertxt, Qt::CaseInsensitive, QRegExp::RegExp);
  CheckItem *cit=0;

  bool notCheckSub = !subCB->isChecked();
  bool notCheckNew = !newCB->isChecked();
  bool notCheckStr = (filtertxt.isEmpty());

  bool isRegexp = filtertxt.contains(QRegExp("[^a-z0-9\\-\\+.]"));

  bool doIncrementalUpdate = (!isRegexp && incrementalFilter && (filtertxt.left(lastFilter.length())==lastFilter));

    kDebug() << "Populating view, incremental is " << doIncrementalUpdate;
  if (doIncrementalUpdate) {
    QList<KNGroupInfo> *tempList = new QList<KNGroupInfo>();

    Q_FOREACH(const KNGroupInfo& g, *matchList) {
      if ((notCheckSub||g.subscribed)&&
          (notCheckNew||g.newGroup)&&
          ( notCheckStr || ( g.name.indexOf(filtertxt) != -1 ) ) )
      tempList->append(g);
    }

    delete matchList;
    matchList=tempList;
  } else {
    matchList->clear();

    Q_FOREACH(const KNGroupInfo& g, *allList) {
      if ((notCheckSub||g.subscribed)&&
          (notCheckNew||g.newGroup)&&
          (notCheckStr||(isRegexp? (reg.indexIn(g.name,0) != -1) : ( g.name.indexOf( filtertxt ) != -1 ) )))
        matchList->append(g);
    }
  }

  groupView->clear();

  if((matchList->count() < MIN_FOR_TREE) || noTreeCB->isChecked()) {
    Q_FOREACH(const KNGroupInfo& g, *matchList) {
      cit=new CheckItem(groupView, g, this);
      updateItemState(cit);
    }
  } else {
    createListItems();
  }

  lastFilter = filtertxt;
  incrementalFilter = !isRegexp;

  leftLabel->setText(i18n("Groups on %1: (%2 displayed)", a_ccount->name(), matchList->count()));

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
    refilterTimer->start(200);
}


void KNGroupBrowser::slotRefilter()
{
  refilterTimer->stop();
  slotFilter(filterEdit->text());
}


//=======================================================================================


KNGroupBrowser::CheckItem::CheckItem(Q3ListView *v, const KNGroupInfo &gi, KNGroupBrowser *b) :
  Q3CheckListItem(v, gi.name, Q3CheckListItem::CheckBox), info(gi), browser(b)
{
  QString des(gi.description);
  if (gi.status == KNGroup::moderated) {
    setText(0,gi.name+" (m)");
    if (!des.toUpper().contains(i18n("moderated").toUpper()))
      des+=i18n(" (moderated)");
  }
  setText(1,des);
}


KNGroupBrowser::CheckItem::CheckItem(Q3ListViewItem *i, const KNGroupInfo &gi, KNGroupBrowser *b) :
  Q3CheckListItem(i, gi.name, Q3CheckListItem::CheckBox), info(gi), browser(b)
{
  QString des(gi.description);
  if (gi.status == KNGroup::moderated) {
    setText(0,gi.name+" (m)");
    if (!des.toUpper().contains(i18n("moderated").toUpper()))
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
  Q3CheckListItem::setOn(c);
  browser=b;
}


void KNGroupBrowser::CheckItem::stateChange(bool s)
{
  if(browser) {
    kDebug(5003) <<"KNGroupBrowser::CheckItem::stateChange()";
    browser->itemChangedState(this, s);
  }
}


//=======================================================================================


KNGroupBrowser::GroupItem::GroupItem(Q3ListView *v, const KNGroupInfo &gi)
 : Q3ListViewItem(v, gi.name), info(gi)
{
  if (gi.status == KNGroup::moderated)
    setText(0,gi.name+" (m)");
}


KNGroupBrowser::GroupItem::GroupItem(Q3ListViewItem *i, const KNGroupInfo &gi)
 : Q3ListViewItem(i, gi.name), info(gi)
{
}


KNGroupBrowser::GroupItem::~GroupItem()
{
}


//-----------------------------------------

