/***************************************************************************
                          knsenderrordialog.cpp  -  description
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

#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpushbutton.h>

#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>

#include "knjobdata.h"
#include "knsavedarticle.h"
#include "utilities.h"
#include "knlistbox.h"
#include "knglobals.h"
#include "knsenderrordialog.h"


KNSendErrorDialog::KNSendErrorDialog() : QSemiModal(knGlobals.topWidget, 0, true)
{
  jobList.setAutoDelete(true);
  jobs=new KNListBox(this);
  jobs->setMinimumHeight(jobs->sizeHint().height());
  error=new QLabel(this);
  error->setFrameStyle(QFrame::Box | QFrame::Sunken);
  closeBtn=new QPushButton(i18n("&Close"), this);
  closeBtn->setDefault(true);
    
  connect(jobs, SIGNAL(highlighted(int)), this, SLOT(slotJobHighlighted(int)));
  connect(closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseBtnClicked()));
  
  QVBoxLayout *topL=new QVBoxLayout(this, 5,5);
  QHBoxLayout *btnL=new QHBoxLayout();
  topL->addWidget(jobs, 1);
  topL->addWidget(error);
  topL->addSpacing(5);
  topL->addLayout(btnL);
  btnL->addStretch(1);
  btnL->addWidget(closeBtn);

  setCaption(kapp->makeStdCaption(i18n("Errors while sending")));
  restoreWindowSize("sendDlg", this, sizeHint());
}



KNSendErrorDialog::~KNSendErrorDialog()
{
  saveWindowSize("sendDlg", size());
}



void KNSendErrorDialog::appendJob(KNJobData *job)
{
  static QPixmap pm=UserIcon("snderr");
  KNLBoxItem *it;
  KNSavedArticle *art=(KNSavedArticle*)job->data();
  jobList.append(job);
  it=new KNLBoxItem(art->subject(), job, &pm);
  jobs->insertItem(it); 
  jobs->setCurrentItem(it);
}



void KNSendErrorDialog::slotJobHighlighted(int idx)
{
  KNLBoxItem *it=jobs->itemAt(idx);
  error->setText(((KNJobData*)it->data())->errorString());  
}



void KNSendErrorDialog::slotCloseBtnClicked()
{
  emit dialogDone();
}



//--------------------------------

#include "knsenderrordialog.moc"
