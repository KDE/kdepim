/*
    knsearchdialog.cpp

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

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include "knfilterconfigwidget.h"
#include "knarticlefilter.h"
#include "utilities.h"
#include "knsearchdialog.h"


KNSearchDialog::KNSearchDialog(searchType /*t*/, QWidget *parent)
  : QDialog(parent)
{
  setCaption(kapp->makeStdCaption( i18n("Search for Articles") ));
  setIcon(SmallIcon("knode"));
  QGroupBox *bg=new QGroupBox(this);

  startBtn=new QPushButton(SmallIcon("mail_find"),i18n("Sea&rch"), bg);
  startBtn->setDefault(true);
  newBtn=new QPushButton(SmallIcon("editclear"),i18n("C&lear"), bg);
  closeBtn=new KPushButton(KStdGuiItem::close(), bg);

  completeThreads=new QCheckBox(i18n("Sho&w complete threads"),this);
  fcw=new KNFilterConfigWidget(this);
  fcw->reset();

  QHBoxLayout *topL=new QHBoxLayout(this, 5);
  QVBoxLayout *filterL=new QVBoxLayout(this, 0, 5);
  QVBoxLayout *btnL=new QVBoxLayout(bg, 8, 5);

  filterL->addWidget(completeThreads);
  filterL->addWidget(fcw,1);

  btnL->addWidget(startBtn);
  btnL->addWidget(newBtn);
  btnL->addStretch(1);
  btnL->addWidget(closeBtn);

  topL->addLayout(filterL, 1);
  topL->addWidget(bg);

  connect(startBtn, SIGNAL(clicked()), this, SLOT(slotStartClicked()));
  connect(newBtn, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));

  f_ilter=new KNArticleFilter();
  f_ilter->setLoaded(true);
  f_ilter->setSearchFilter(true);

  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("searchDlg", this, sizeHint());
  fcw->setStartFocus();
}



KNSearchDialog::~KNSearchDialog()
{
  delete f_ilter;
  KNHelper::saveWindowSize("searchDlg", size());
}


void KNSearchDialog::slotStartClicked()
{
  f_ilter->status=fcw->status->filter();
  f_ilter->score=fcw->score->filter();
  f_ilter->age=fcw->age->filter();
  f_ilter->lines=fcw->lines->filter();
  f_ilter->subject=fcw->subject->filter();
  f_ilter->from=fcw->from->filter();
  f_ilter->messageId=fcw->messageId->filter();
  f_ilter->references=fcw->references->filter();
  f_ilter->setApplyOn(completeThreads->isChecked()? 1:0);
  emit doSearch(f_ilter);
}



void KNSearchDialog::slotNewClicked()
{
  fcw->reset();
}



void KNSearchDialog::slotCloseClicked()
{
  emit dialogDone();
}


//--------------------------------

#include "knsearchdialog.moc"

