/*
    knsearchdialog.cpp

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

#include <qlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qbitarray.h>

#include <klocale.h>
#include <kseparator.h>
#include <kapp.h>
#include <kiconloader.h>

#include "knfilterconfigwidget.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"
#include "knarticlefilter.h"
#include "knglobals.h"
#include "utilities.h"
#include "knsearchdialog.h"


KNSearchDialog::KNSearchDialog(searchType /*t*/, QWidget *parent)
 : QDialog(parent)
{
  setCaption(kapp->makeStdCaption( i18n("Search for Articles") ));
  setIcon(SmallIcon("knode"));
  QGroupBox *bg=new QGroupBox(this);
  
  startBtn=new QPushButton(i18n("St&art Search"), bg);
  startBtn->setDefault(true); 
  newBtn=new QPushButton(i18n("&New Search"), bg);
  closeBtn=new QPushButton(i18n("&Close"), bg);
  
  fcw=new KNFilterConfigWidget(this);
  fcw->reset();
  
  QHBoxLayout *topL=new QHBoxLayout(this, 5);
  QVBoxLayout *btnL=new QVBoxLayout(bg, 8, 5);
  
  topL->addWidget(fcw, 1);
  topL->addWidget(bg);
  
  btnL->addWidget(startBtn);
  btnL->addWidget(newBtn);
  btnL->addStretch(1);
  btnL->addWidget(closeBtn);  

  connect(startBtn, SIGNAL(clicked()), this, SLOT(slotStartClicked())); 
  connect(newBtn, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));

  f_ilter=new KNArticleFilter();
  f_ilter->setLoaded(true);
  //if(t==STfolderSearch) fcw->setLimited();
  
  setFixedHeight(sizeHint().height());
  restoreWindowSize("searchDlg", this, sizeHint());
}



KNSearchDialog::~KNSearchDialog()
{
  delete f_ilter;
  saveWindowSize("searchDlg", size());
}



void KNSearchDialog::closeEvent(QCloseEvent *e)
{
  e->accept();
  emit dialogDone();
}



void KNSearchDialog::slotStartClicked()
{
  f_ilter->status=fcw->status->filter();
  f_ilter->score=fcw->score->filter();
  f_ilter->age=fcw->age->filter();
  f_ilter->lines=fcw->lines->filter();
  f_ilter->subject=fcw->subject->filter();
  f_ilter->from=fcw->from->filter();
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

