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

#include <kseparator.h>

#include "kngroupbrowser.h"
#include "knglobals.h"
#include "knarticlecollection.h" //KNFile
#include "knnntpaccount.h"


KNGroupBrowser::KNGroupBrowser(QWidget *parent, KNNntpAccount *a) :
  QDialog(parent, 0, true), a_ccount(a)
{
  allList=new QStrList();
  subList=new QStrList();
  xTop->gManager()->getSubscribed(a, subList);
  matchList=new QStrList(false);
  matchList->setAutoDelete(false);
  listPtr=allList;


  //create Widgets
  pmRight=UserIcon("arrow_right");
  pmLeft=UserIcon("arrow_left");
  pmGroup=UserIcon("group");
  QLabel *l1, *l2;
  KSeparator *sep1, *sep2;
  QFont fnt=font();
  fnt.setBold(true);

  l1=new QLabel(i18n("Filter:"), this);
  l2=new QLabel(QString(i18n("Grouplist for %1:")).arg(a->name()), this);
  rightLabel=new QLabel(this);
  l2->setFont(fnt);
  rightLabel->setFont(fnt);

  filterEdit=new QLineEdit(this);

  subCB=new QCheckBox(i18n("subscribed only"), this);
  /*treeCB=new QCheckBox(i18n("hierarchical view"), this);
  treeCB->setChecked(true);*/

  sep1=new KSeparator(KSeparator::HLine, this);
  sep2=new KSeparator(KSeparator::HLine, this);

  groupView=new QListView(this);
  groupView->setRootIsDecorated(true);
  groupView->addColumn(i18n("available groups"));

  arrowBtn1=new QPushButton(this);
  arrowBtn1->setEnabled(false);
  arrowBtn2=new QPushButton(this);
  arrowBtn2->setEnabled(false);
  arrowBtn1->setPixmap(pmRight);
  arrowBtn2->setPixmap(pmLeft);
  arrowBtn1->setFixedSize(35,30);
  arrowBtn2->setFixedSize(35,30);

  helpBtn=new QPushButton(i18n("Help"), this);
  cancelBtn=new QPushButton(i18n("Cancel"), this);
  okBtn=new QPushButton(i18n("Ok"), this);

  //layout
  QVBoxLayout *topL, *arrL;
  QHBoxLayout *filterL;

  topL=new QVBoxLayout(this, 10);

  arrL=new QVBoxLayout(10);
  filterL=new QHBoxLayout(10);
  listL=new QGridLayout(2, 3, 5);
  btnL=new QHBoxLayout(10);

  topL->addLayout(filterL);
  topL->addWidget(sep1);
  topL->addLayout(listL, 1);
  topL->addWidget(sep2);
  topL->addLayout(btnL);

  filterL->addWidget(l1);
  filterL->addWidget(filterEdit, 1);
  filterL->addWidget(subCB);

  listL->addWidget(l2, 0,0);
  listL->addWidget(rightLabel, 0,2);
  listL->addWidget(groupView, 1,0);
  listL->addLayout(arrL, 1,1);
  arrL->addWidget(arrowBtn1, AlignCenter);
  arrL->addWidget(arrowBtn2, AlignCenter);


  //listL->addWidget(treeCB, 2,0);
  listL->setRowStretch(1,1);
  listL->setColStretch(0,1);
  listL->setColStretch(2,1);

  btnL->addWidget(helpBtn);
  btnL->addStretch(1);
  btnL->addWidget(cancelBtn);
  btnL->addWidget(okBtn);

  //topL->activate();

  //connect
  connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

  connect(filterEdit, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotFilter(const QString&)));
  connect(groupView, SIGNAL(expanded(QListViewItem*)),
    this, SLOT(slotItemExpand(QListViewItem*)));

  connect(subCB, SIGNAL(clicked()), this, SLOT(slotSubCBClicked()));

  QTimer::singleShot(2, this, SLOT(slotLoadList()));
}




KNGroupBrowser::~KNGroupBrowser()
{
  delete subList;
  delete matchList;
  delete allList;
}



void KNGroupBrowser::slotLoadList()
{
  KNFile f;
  f.setName(a_ccount->path()+"groups");
  QCString line;

  if(f.open(IO_ReadOnly)) {
    xTop->setStatusMsg(i18n("loading list of groups ..."));
    allList->clear();			

    while(!f.atEnd()) {
      line = f.readLine();

      allList->append(line.data());

      if((allList->count() & 1023)==0)
        kapp->processEvents();
    }

    f.close();
    xTop->setStatusMsg();
    filterEdit->setText("");
    slotFilter("");
  }	
  else
    qWarning("unable to open %s, reason %d", f.name().latin1(), f.status());
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



void KNGroupBrowser::slotItemExpand(QListViewItem *it)
{
  if(!it) return;

  if(it->childCount()) {
    qDebug("KNGroupBrowser::slotItemExpand() : has already been expanded, returning");
    return;
  }

  QCString gn(it->text(0).latin1());
  int gnlen = gn.length();

  for(char *var=listPtr->first(); var; var=listPtr->next()) {
    if(!strnicmp(var, gn.data(), gnlen)) {
      CheckItem* item = new CheckItem(it, QString(var), this);
      if(listPtr==subList)
        updateItemState(item, true);
      else
        updateItemState(item, subList->contains(var));
    }
    else
      if(it->childCount())
        break;
  }
}


#define MIN_FOR_TREE 50
void KNGroupBrowser::slotFilter(const QString &txt)
{
  qDebug("KNGroupBrowser::slotFilter()");
  QCString tlgroupname("");
  QCString filtertxt(txt.local8Bit());
  int groupnamelen = tlgroupname.length();
  bool expandit = false;
  QListViewItem* previous = 0;
  CheckItem *item=0;

  matchList->clear();
  groupView->clear();

  if(subCB->isChecked()) listPtr=subList;
  else listPtr=allList;

  if(!txt.isEmpty()) {
    for(char *g=listPtr->first(); g; g=listPtr->next()) {
      if(strstr(g, filtertxt.data()))
        matchList->append(g);
    }
    listPtr=matchList;
  }

  //treeCB->setChecked( (listPtr->count() >= MIN_FOR_TREE) );

  if(listPtr->count() < MIN_FOR_TREE) {
  //if(!treeCB->isChecked()) {
    for(char *g=listPtr->first(); g; g=listPtr->next()) {
      item=new CheckItem(groupView, QString(g), this);
      if(listPtr==subList)
        updateItemState(item, true);
      else
        updateItemState(item, subList->contains(g));
    }
  }
  else {
    for(char *g=listPtr->first(); g; g=listPtr->next()) {
      char* p;

      if(!expandit || strnicmp(g, tlgroupname.data(), groupnamelen)) {
        if((p = strstr(g, "."))) {
          groupnamelen = p - g + 1;
          expandit = true;
        }
        else {
          groupnamelen = strlen(g);
          expandit = false;
        }

        tlgroupname = g;
        tlgroupname.truncate(groupnamelen);

        QListViewItem* i = new QListViewItem(groupView, previous);
        i->setExpandable(expandit);
        i->setText(0, tlgroupname);
        i->setSelectable(false);

        previous = i;
      }
    }
  }

  arrowBtn1->setEnabled(false);
  arrowBtn2->setEnabled(false);
}



void KNGroupBrowser::slotSubCBClicked()
{
  slotFilter(filterEdit->text());
}



//=======================================================================================



KNGroupBrowser::CheckItem::CheckItem(QListView *v, const QString &t, KNGroupBrowser *b) :
  QCheckListItem(v, t, QCheckListItem::CheckBox), browser(b)
{
}



KNGroupBrowser::CheckItem::CheckItem(QListViewItem *i, const QString &t, KNGroupBrowser *b) :
  QCheckListItem(i, t, QCheckListItem::CheckBox), browser(b)
{
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
    qDebug("KNGroupBrowser::CheckItem::stateChange()");
    browser->itemChangedState(this, s);
  }
}


//-----------------------------------------

#include "kngroupbrowser.moc"
