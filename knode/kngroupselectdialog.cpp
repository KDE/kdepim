/***************************************************************************
                     kngroupselectdialog.cpp - description
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

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qheader.h>

#include <klocale.h>

#include "utilities.h"
#include "knstringsplitter.h"
#include "kngroupselectdialog.h"
#include "kngroupmanager.h"


KNGroupSelectDialog::KNGroupSelectDialog(QWidget *parent, KNNntpAccount *a, QCString &act) :
  KNGroupBrowser(parent, i18n("Select Destinations"), a)
{
  selView=new QListView(page);
  selView->addColumn(QString::null);
  selView->header()->hide();
  listL->addWidget(selView, 1,2);
  rightLabel->setText(i18n("Groups for this article:"));
  subCB->setChecked(true);

  if(!act.isEmpty()) {
    KNStringSplitter split;
     QListViewItem *it;
    split.init(act, ",");
    bool splitOk;

    if(!(splitOk=split.first())) {
      it=new QListViewItem(selView, QString(act));
    }
    else {
      do {
        it=new QListViewItem(selView, QString(split.string()));
        splitOk=split.next();
      } while(splitOk);
    }
  }

  connect(selView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));
  connect(groupView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotItemSelected(QListViewItem*)));
  connect(arrowBtn1, SIGNAL(clicked()), this, SLOT(slotArrowBtn1()));
  connect(arrowBtn2, SIGNAL(clicked()), this, SLOT(slotArrowBtn2()));

  restoreWindowSize("groupSelDlg", this, sizeHint());
}



KNGroupSelectDialog::~KNGroupSelectDialog()
{
  saveWindowSize("groupSelDlg", this->size());
}



void KNGroupSelectDialog::itemChangedState(CheckItem *it, bool s)
{
  if(s)
    new QListViewItem(selView, it->text(0));
  else
    removeListItem(selView, it->text(0));
  arrowBtn1->setEnabled(!s);
}



void KNGroupSelectDialog::updateItemState(CheckItem *it)
{
  it->setChecked(itemInListView(selView, it->text(0)));
  if(it->info->subscribed && it->pixmap(0)==0)
    it->setPixmap(0, pmGroup);
}


		
QCString KNGroupSelectDialog::selectedGroups()
{
  QCString ret;
  QListViewItemIterator it(selView);
  ret="";
  bool isFirst=true;
  for(; it.current(); ++it) {
    if(!isFirst)
      ret+=",";
    ret+=it.current()->text(0).local8Bit();
    isFirst=false;
  }

  return ret;
}



void KNGroupSelectDialog::slotItemSelected(QListViewItem *it)
{
  const QObject *s=sender();

  if(s==groupView) {
    selView->clearSelection();
    arrowBtn2->setEnabled(false);
    if(it)
      arrowBtn1->setEnabled(!(static_cast<CheckItem*>(it))->isOn());
    else
      arrowBtn1->setEnabled(false);
  }
  else {
    groupView->clearSelection();
    arrowBtn1->setEnabled(false);
    arrowBtn2->setEnabled((it!=0));
  }
}



void KNGroupSelectDialog::slotArrowBtn1()
{
  QListViewItem *i=groupView->selectedItem();

  if(i) {
    new QListViewItem(selView, i->text(0));
    arrowBtn1->setEnabled(false);
    (static_cast<CheckItem*>(i))->setChecked(true);
  }
}



void KNGroupSelectDialog::slotArrowBtn2()
{

  QListViewItem *i=selView->selectedItem();

  if(i) {
    changeItemState(i->text(0), false);
    delete i;
    arrowBtn2->setEnabled(false);
  }
}


// -----------------------------------------------------------------------------

#include "kngroupselectdialog.moc"

