/***************************************************************************
                     kngroupdialog.cpp - description
 copyright            : (C) 1999 by Christian Thurner
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

#include <klocale.h> //i18n

#include "kngroupdialog.h"
#include "utilities.h"


KNGroupDialog::KNGroupDialog(QWidget *parent, KNNntpAccount *a) :
  KNGroupBrowser(parent, a)
{
  newListBtn=new QPushButton(i18n("New list"), this);
  btnL->insertWidget(2, newListBtn);

  rightLabel->setText(i18n("Current changes:"));
  subView=new QListView(this);
  subView->addColumn(i18n("subscribe to"));
  unsubView=new QListView(this);
  unsubView->addColumn(i18n("unsubscribe from"));

  QVBoxLayout *protL=new QVBoxLayout(3);
  listL->addLayout(protL, 1,2);
  protL->addWidget(subView);
  protL->addWidget(unsubView);

  dir1=right;
  dir2=left;

  connect(groupView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));
  connect(subView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));
  connect(unsubView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));

  connect(arrowBtn1, SIGNAL(clicked()), this, SLOT(slotArrowBtn1()));
  connect(arrowBtn2, SIGNAL(clicked()), this, SLOT(slotArrowBtn2()));
  connect(newListBtn, SIGNAL(clicked()), this, SLOT(slotNewListBtn()));

  setDialogSize("groupDlg", this);
}



KNGroupDialog::~KNGroupDialog()
{
  saveDialogSize("groupDlg", this->size());
}



void KNGroupDialog::itemChangedState(CheckItem *it, bool s)
{
  qDebug("KNGroupDialog::itemChangedState()");
  if(s){
    if(itemInListView(unsubView, it->text(0))) {
      removeListItem(unsubView, it->text(0));
      setButtonDirection(btn2, right);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(true);
    }
    else {
      new QListViewItem(subView, it->text(0));
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
  else {
    if(itemInListView(subView, it->text(0))) {
      removeListItem(subView, it->text(0));
      setButtonDirection(btn1, right);
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
    }
    else {
      new QListViewItem(unsubView, it->text(0));
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
}



void KNGroupDialog::updateItemState(CheckItem *it, bool isSub)
{
  it->setChecked( (isSub && !itemInListView(unsubView, it->text(0))) ||
                  (!isSub && itemInListView(subView, it->text(0)))  );

  if(isSub && it->pixmap(0)==0)
    it->setPixmap(0, pmGroup);
}



void KNGroupDialog::toSubscribe(QStrList *l)
{
  l->clear();
  QListViewItemIterator it(subView);
  for(; it.current(); ++it)
    l->append(it.current()->text(0).latin1());
}



void KNGroupDialog::toUnsubscribe(QStrList *l)
{
  l->clear();
  QListViewItemIterator it(unsubView);
  for(; it.current(); ++it)
    l->append(it.current()->text(0).latin1());
}



void KNGroupDialog::setButtonDirection(arrowButton b, arrowDirection d)
{
  QPushButton *btn=0;
  if(b==btn1 && dir1!=d) {
    btn=arrowBtn1;
    dir1=d;
  }
  else if(b==btn2 && dir2!=d) {
    btn=arrowBtn2;
    dir2=d;
  }

  if(btn) {
    if(d==right)
      btn->setPixmap(pmRight);
    else
      btn->setPixmap(pmLeft);
  }
}



void KNGroupDialog::slotItemSelected(QListViewItem *it)
{
  const QObject *s=sender();


  if(s==subView) {
    unsubView->clearSelection();
    groupView->clearSelection();
    arrowBtn2->setEnabled(false);
    arrowBtn1->setEnabled(true);
    setButtonDirection(btn1, left);
  }
  else if(s==unsubView) {
    subView->clearSelection();
    groupView->clearSelection();
    arrowBtn1->setEnabled(false);
    arrowBtn2->setEnabled(true);
    setButtonDirection(btn2, left);
  }
  else {
    CheckItem *cit;
    subView->clearSelection();
    unsubView->clearSelection();
    cit=static_cast<CheckItem*>(it);
    if(!cit->isOn() && !itemInListView(subView, cit->text(0)) && !itemInListView(unsubView, cit->text(0))) {
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
      setButtonDirection(btn1, right);
    }
    else if(cit->isOn() && !itemInListView(unsubView, cit->text(0)) && !itemInListView(subView, cit->text(0))) {
      arrowBtn2->setEnabled(true);
      arrowBtn1->setEnabled(false);
      setButtonDirection(btn2, right);
    }
    else {
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
}



void KNGroupDialog::slotArrowBtn1()
{
  QListViewItem *it=0;

  if(dir1==right) {
    it=groupView->selectedItem();
    new QListViewItem(subView, it->text(0));
    (static_cast<CheckItem*>(it))->setChecked(true);
  }
  else {
    it=subView->selectedItem();
    changeItemState(it->text(0), false);
    delete it;
  }

  arrowBtn1->setEnabled(false);
}



void KNGroupDialog::slotArrowBtn2()
{
  QListViewItem *it=0;

  if(dir2==right) {
    it=groupView->selectedItem();
    new QListViewItem(unsubView, it->text(0));
    (static_cast<CheckItem*>(it))->setChecked(false);
  }
  else {
    it=unsubView->selectedItem();
    changeItemState(it->text(0), true);
    delete it;
  }

  arrowBtn2->setEnabled(false);
}



void KNGroupDialog::slotNewListBtn()
{
  emit newList(a_ccount);
}



//--------------------------------

#include "kngroupdialog.moc"
