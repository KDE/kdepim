/*
    kngroupselectdialog.cpp

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
#include <tqlabel.h>
#include <tqheader.h>
#include <tqcheckbox.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "utilities.h"
#include "kngroupselectdialog.h"
#include <tqpushbutton.h>


KNGroupSelectDialog::KNGroupSelectDialog(TQWidget *parent, KNNntpAccount *a, const TQString &act) :
  KNGroupBrowser(parent, i18n("Select Destinations"), a)
{
  selView=new TQListView(page);
  selView->addColumn(TQString::null);
  selView->header()->hide();
  listL->addWidget(selView, 1,2);
  rightLabel->setText(i18n("Groups for this article:"));
  subCB->setChecked(true);

  KNGroupInfo info;
  TQStringList actGroups = TQStringList::split(',',act);
  for ( TQStringList::Iterator it = actGroups.begin(); it != actGroups.end(); ++it ) {
    info.name = *it;
    new GroupItem(selView, info);
  }

  connect(selView, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
    this, TQT_SLOT(slotItemSelected(TQListViewItem*)));
  connect(groupView, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
    this, TQT_SLOT(slotItemSelected(TQListViewItem*)));
  connect(groupView, TQT_SIGNAL(selectionChanged()),
    this, TQT_SLOT(slotSelectionChanged()));
  connect(arrowBtn1, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotArrowBtn1()));
  connect(arrowBtn2, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotArrowBtn2()));

  KNHelper::restoreWindowSize("groupSelDlg", this, TQSize(659,364));  // optimized for 800x600
}



KNGroupSelectDialog::~KNGroupSelectDialog()
{
  KNHelper::saveWindowSize("groupSelDlg", this->size());
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



TQString KNGroupSelectDialog::selectedGroups()const
{
  TQString ret;
  TQListViewItemIterator it(selView);
  bool moderated=false;
  int count=0;
  bool isFirst=true;

  for(; it.current(); ++it) {
    if(!isFirst)
      ret+=",";
    ret+=(static_cast<GroupItem*>(it.current()))->info.name;
    isFirst=false;
    count++;
    if ((static_cast<GroupItem*>(it.current()))->info.status == KNGroup::moderated)
      moderated=true;
  }

  if (moderated && (count>=2))   // warn the user
     KMessageBox::information(parentWidget(),i18n("You are crossposting to a moderated newsgroup.\nPlease be aware that your article will not appear in any group\nuntil it has been approved by the moderators of the moderated group."),
                              TQString::null,"crosspostModeratedWarning");

  return ret;
}



void KNGroupSelectDialog::slotItemSelected(TQListViewItem *it)
{
  const TQObject *s=sender();

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

