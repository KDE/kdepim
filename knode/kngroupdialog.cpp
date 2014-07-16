/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>

#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>

#include <kdebug.h>
#include <kglobal.h>
#include <kdatepicker.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include "utilities.h"
#include "knnntpaccount.h"
#include "kngroupdialog.h"
#include "knglobals.h"
#include <QPushButton>


KNGroupDialog::KNGroupDialog( QWidget *parent, KNNntpAccount::Ptr a ) :
  KNGroupBrowser(parent, i18n("Subscribe to Newsgroups"),a, User1 | User2, true, i18n("New &List"), i18n("New &Groups...") )
{
  rightLabel->setText(i18n("Current changes:"));
  subView=new Q3ListView(page);
  subView->addColumn(i18n("Subscribe To"));
  unsubView=new Q3ListView(page);
  unsubView->addColumn(i18n("Unsubscribe From"));

  QVBoxLayout *protL=new QVBoxLayout();
  protL->setSpacing(3);
  listL->addLayout(protL, 1,2);
  protL->addWidget(subView);
  protL->addWidget(unsubView);

  dir1=right;
  dir2=left;

  connect(groupView, SIGNAL(selectionChanged(Q3ListViewItem*)),
    this, SLOT(slotItemSelected(Q3ListViewItem*)));
  connect(groupView, SIGNAL(selectionChanged()),
    this, SLOT(slotSelectionChanged()));
  connect(subView, SIGNAL(selectionChanged(Q3ListViewItem*)),
    this, SLOT(slotItemSelected(Q3ListViewItem*)));
  connect(unsubView, SIGNAL(selectionChanged(Q3ListViewItem*)),
    this, SLOT(slotItemSelected(Q3ListViewItem*)));

  connect(arrowBtn1, SIGNAL(clicked()), this, SLOT(slotArrowBtn1()));
  connect(arrowBtn2, SIGNAL(clicked()), this, SLOT(slotArrowBtn2()));

  connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );

  KNHelper::restoreWindowSize("groupDlg", this, QSize(662,393));  // optimized for 800x600

  setHelp("anc-fetch-group-list");
}



KNGroupDialog::~KNGroupDialog()
{
  KNHelper::saveWindowSize("groupDlg", this->size());
}



void KNGroupDialog::itemChangedState(CheckItem *it, bool s)
{
  kDebug(5003) <<"KNGroupDialog::itemChangedState()";
  if(s){
    if(itemInListView(unsubView, it->info)) {
      removeListItem(unsubView, it->info);
      setButtonDirection(btn2, right);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(true);
    }
    else {
      new GroupItem(subView, it->info);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
  else {
    if(itemInListView(subView, it->info)) {
      removeListItem(subView, it->info);
      setButtonDirection(btn1, right);
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
    }
    else {
      new GroupItem(unsubView, it->info);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
}



void KNGroupDialog::updateItemState(CheckItem *it)
{
  it->setChecked( (it->info.subscribed && !itemInListView(unsubView, it->info)) ||
                  (!it->info.subscribed && itemInListView(subView, it->info)) );

  if((it->info.subscribed || it->info.newGroup) && it->pixmap(0)==0)
    it->setPixmap(0, (it->info.newGroup)? pmNew:pmGroup);
}



void KNGroupDialog::toSubscribe(QList<KNGroupInfo> *l)
{
  l->clear();

  bool moderated=false;
  Q3ListViewItemIterator it(subView);
  for(; it.current(); ++it) {
    const KNGroupInfo& info = ((static_cast<GroupItem*>(it.current()))->info);
    l->append(info);
    if (info.status==KNGroup::moderated)
      moderated=true;
  }
  if (moderated)   // warn the user
     KMessageBox::information( knGlobals.topWidget,
       i18n("You have subscribed to a moderated newsgroup.\nYour articles will not appear in the group immediately.\nThey have to go through a moderation process."),
       QString(), "subscribeModeratedWarning" );
}



void KNGroupDialog::toUnsubscribe(QStringList *l)
{
  l->clear();
  Q3ListViewItemIterator it(unsubView);
  for(; it.current(); ++it)
    l->append(((static_cast<GroupItem*>(it.current()))->info).name);
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
      btn->setIcon(pmRight);
    else
      btn->setIcon(pmLeft);
  }
}



void KNGroupDialog::slotItemSelected(Q3ListViewItem *it)
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
    if(!cit->isOn() && !itemInListView(subView, cit->info) && !itemInListView(unsubView, cit->info)) {
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
      setButtonDirection(btn1, right);
    }
    else if(cit->isOn() && !itemInListView(unsubView, cit->info) && !itemInListView(subView, cit->info)) {
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



void KNGroupDialog::slotSelectionChanged()
{
  if (!groupView->selectedItem())
    arrowBtn1->setEnabled(false);
}



void KNGroupDialog::slotArrowBtn1()
{
  if(dir1==right) {
    CheckItem *it=static_cast<CheckItem*>(groupView->selectedItem());
    if (it) {
      new GroupItem(subView, it->info);
      it->setChecked(true);
    }
  }
  else {
    GroupItem *it=static_cast<GroupItem*>(subView->selectedItem());
    if (it) {
      changeItemState(it->info, false);
      delete it;
    }
  }

  arrowBtn1->setEnabled(false);
}


void KNGroupDialog::slotArrowBtn2()
{
  if(dir2==right) {
    CheckItem *it=static_cast<CheckItem*>(groupView->selectedItem());
    if (it) {
      new GroupItem(unsubView, it->info);
      it->setChecked(false);
    }
  }
  else {
    GroupItem *it=static_cast<GroupItem*>(unsubView->selectedItem());
    if (it) {
      changeItemState(it->info, true);
      delete it;
    }
  }

  arrowBtn2->setEnabled(false);
}


// new list
void KNGroupDialog::slotUser1()
{
  leftLabel->setText(i18n("Downloading groups..."));
  enableButton(User1,false);
  enableButton(User2,false);
  emit(fetchList(a_ccount));
}


// new groups
void KNGroupDialog::slotUser2()
{
  QDate lastDate = a_ccount->lastNewFetch();
  KDialog *dlg = new KDialog( this );
  dlg->setCaption( i18n("New Groups") );
  dlg->setButtons( Ok | Cancel );

  QGroupBox *btnGrp = new QGroupBox( i18n("Check for New Groups"), dlg );
  dlg->setMainWidget(btnGrp);
  QGridLayout *topL = new QGridLayout( btnGrp );

  QRadioButton *takeLast = new QRadioButton( i18n("Created since last check:"), btnGrp );
  topL->addWidget(takeLast, 0, 0, 1, 2 );

  QLabel *l = new QLabel(KGlobal::locale()->formatDate(lastDate, KLocale::LongDate),btnGrp);
  topL->addWidget(l, 1, 1, Qt::AlignLeft);

  connect(takeLast, SIGNAL(toggled(bool)), l, SLOT(setEnabled(bool)));

  QRadioButton *takeCustom = new QRadioButton( i18n("Created since this date:"), btnGrp );
  topL->addWidget(takeCustom, 2, 0, 1, 2 );

  dateSel = new KDatePicker( lastDate, btnGrp );
  dateSel->setMinimumSize(dateSel->sizeHint());
  topL->addWidget(dateSel, 3, 1, Qt::AlignLeft);

  connect(takeCustom, SIGNAL(toggled(bool)), this, SLOT(slotDatePickerEnabled(bool)));

  takeLast->setChecked(true);
  dateSel->setEnabled(false);

  topL->addItem( new QSpacerItem(30, 0 ), 0, 0 );

  if (dlg->exec()) {
    if (takeCustom->isChecked())
      lastDate = dateSel->date();
    a_ccount->setLastNewFetch(QDate::currentDate());
    leftLabel->setText(i18n("Checking for new groups..."));
    enableButton(User1,false);
    enableButton(User2,false);
    filterEdit->clear();
    subCB->setChecked(false);
    newCB->setChecked(true);
    emit(checkNew(a_ccount,lastDate));
    incrementalFilter=false;
    slotRefilter();
  }

  delete dlg;
}

void KNGroupDialog::slotDatePickerEnabled( bool b )
{
  dateSel->setEnabled( b );
}


//--------------------------------

