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

#include <klocale.h>

#include "utilities.h"
#include "knstringsplitter.h"
#include "kngroupselectdialog.h"


KNGroupSelectDialog::KNGroupSelectDialog(QWidget *parent, KNNntpAccount *a, QCString &act) :
  KNGroupBrowser(parent, a)
{
  selView=new QListView(this);
  selView->addColumn(i18n("selected groups"));
  listL->addWidget(selView, 1,2);
  rightLabel->setText(i18n("Groups for this article:"));

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



void KNGroupSelectDialog::updateItemState(CheckItem *it, bool isSub)
{
  it->setChecked(itemInListView(selView, it->text(0)));
  if(isSub && it->pixmap(0)==0)
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



/*#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

#include <kseparator.h>

#include "kngrouplistwidget.h"

#include "knglobals.h"
#include "knstringsplitter.h"
#include "utilities.h"

KNGroupSelectDialog::KNGroupSelectDialog(KNNntpAccount *a, QCString &groups, QWidget *parent)
	: QDialog(parent,0,true)
{
	KNStringSplitter split;
	bool splitOk;
	
	gb1=new QGroupBox(i18n("all groups"), this);
	gb2=new QGroupBox(i18n("selected groups"), this);
		
	glw=new KNGroupListWidget(a, gb1);
		
	add=new QPushButton(QString::null,this);
	add->setPixmap(UserIcon("arrow_right"));
	add->setFixedSize(50,35);
		
	del=new QPushButton(QString::null,this);
	del->setPixmap(UserIcon("arrow_left"));
	del->setFixedSize(50,35);
	
	lb=new QListBox(gb2);
		
	ok=new QPushButton(i18n("OK"), this);
	cancel=new QPushButton(i18n("Cancel"), this);
	KSeparator *sep=new KSeparator(this);
		
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QHBoxLayout *listL=new QHBoxLayout(10);
	QVBoxLayout *btnL1=new QVBoxLayout(10);
	QHBoxLayout *btnL2=new QHBoxLayout(10);
	
	topL->addLayout(listL, 1);
	topL->addWidget(sep);
	topL->addLayout(btnL2);
	listL->addWidget(gb1, 1);
	listL->addLayout(btnL1);
	listL->addWidget(gb2, 1);
	btnL1->addStretch(1);
	btnL1->addWidget(add);
	btnL1->addStretch(1);
	btnL1->addWidget(del);
	btnL1->addStretch(1);
	btnL2->addStretch(1);
	btnL2->addWidget(ok);
	btnL2->addWidget(cancel);
	topL->activate();
		
	connect(glw, SIGNAL(itemSelected(const QString&)), this, SLOT(slotAdd(const QString&)));
	connect(add, SIGNAL(clicked()), this, SLOT(slotAddBtn()));
	connect(del, SIGNAL(clicked()), this, SLOT(slotRemoveBtn()));
	connect(lb, SIGNAL(selected(int)), this, SLOT(slotRemove(int)));
	connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	
	if(!groups.isEmpty()) {
		split.init(groups, ",");
		splitOk=split.first();
		if(!splitOk) lb->insertItem(QString(groups));
		else {
  		while(splitOk) {
  			lb->insertItem(QString(split.string()));
  			splitOk=split.next();
  		}
  	}
	}
	
	setCaption(i18n("select newsgroups"));
	restoreWindowSize("gSelectDLG", this, sizeHint());
}



KNGroupSelectDialog::~KNGroupSelectDialog()
{
	saveWindowSize( "gSelectDLG", size());
}



void KNGroupSelectDialog::resizeEvent(QResizeEvent *)
{
	glw->setGeometry(20,30, gb1->width()-40, gb1->height()-50);
	lb->setGeometry(20,30, gb2->width()-40, gb2->height()-50);
}



void KNGroupSelectDialog::addToSelected(const QString &text)

{
	bool inList=false;
	for(uint idx=0; idx<lb->count(); idx++)
		if(lb->text(idx) == text) inList=true;
	if(!inList) lb->insertItem(text);
}



void KNGroupSelectDialog::slotAddBtn()
{
  if(!glw->currentText().isEmpty()) addToSelected(glw->currentText());
}


				
void KNGroupSelectDialog::slotRemoveBtn()
{
	if(lb->currentItem()>-1) lb->removeItem(lb->currentItem());
}


			
void KNGroupSelectDialog::slotAdd(const QString &text)
{
	addToSelected(text);
}



void KNGroupSelectDialog::slotRemove(int i)
{
	lb->removeItem(i);
}



QCString& KNGroupSelectDialog::selectedGroups()
{
	selGroups="";
	if(lb->count()>0) {
		selGroups=lb->text(0).local8Bit();
		if(lb->count()>1) {
  		for(uint i=1; i<lb->count(); i++){
  			selGroups+=",";
  			selGroups+=lb->text(i).local8Bit();
  		}
  	}
	}
	return selGroups;
}


*/
