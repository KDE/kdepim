/*
    kngroupselectdialog.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

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
  connect(groupView, SIGNAL(selectionChanged()),
    this, SLOT(slotSelectionChanged()));
  connect(arrowBtn1, SIGNAL(clicked()), this, SLOT(slotArrowBtn1()));
  connect(arrowBtn2, SIGNAL(clicked()), this, SLOT(slotArrowBtn2()));

  restoreWindowSize("groupSelDlg", this, QSize(659,364));  // optimized for 800x600
}



KNGroupSelectDialog::~KNGroupSelectDialog()
{
  saveWindowSize("groupSelDlg", this->size());
}



void KNGroupSelectDialog::itemChangedState(CheckItem *it, bool s)
{
  if(s)
    new GroupItem(selView, it->info);
  else
    removeListItem(selView, it->info);
  arrowBtn1->setEnabled(!s);
}



void KNGroupSelectDialog::updateItemState(CheckItem *it)
{
  it->setChecked(itemInListView(selView, it->info));
  if(it->info.subscribed && it->pixmap(0)==0)
    it->setPixmap(0, pmGroup);
}


    
QString KNGroupSelectDialog::selectedGroups()
{
  QString ret;
  QListViewItemIterator it(selView);

  bool isFirst=true;
  for(; it.current(); ++it) {
    if(!isFirst)
      ret+=",";
    ret+=it.current()->text(0);
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



void KNGroupSelectDialog::slotSelectionChanged()
{
  if (!groupView->selectedItem())
    arrowBtn1->setEnabled(false);
}



void KNGroupSelectDialog::slotArrowBtn1()
{
  CheckItem *i=static_cast<CheckItem*>(groupView->selectedItem());

  if(i) {
    new GroupItem(selView, i->info);
    arrowBtn1->setEnabled(false);
    i->setChecked(true);
  }
}



void KNGroupSelectDialog::slotArrowBtn2()
{
  GroupItem *i=static_cast<GroupItem*>(selView->selectedItem());

  if(i) {
    changeItemState(i->info, false);
    delete i;
    arrowBtn2->setEnabled(false);
  }
}


// -----------------------------------------------------------------------------

#include "kngroupselectdialog.moc"

