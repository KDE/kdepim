/***************************************************************************
                          knuserwidget.cpp  -  description
                             -------------------

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

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qmultilineedit.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kuserprofile.h>
#include <kopenwith.h>

#include "knuserentry.h"
#include "knuserwidget.h"


KNUserWidget::KNUserWidget(QWidget *parent, const char *n)
	: QWidget(parent, n), entry(0)
{
  QGridLayout *topL=new QGridLayout(this,  6, 3, 5,5);

  QLabel *l=new QLabel(i18n("Name:"), this);		
  topL->addWidget(l, 0,0);
  name=new QLineEdit(this);
  topL->addMultiCellWidget(name, 0,0, 1,2);
	
  l=new QLabel(i18n("Organization:"), this);
  topL->addWidget(l, 1,0);
  orga=new QLineEdit(this);	
  topL->addMultiCellWidget(orga, 1,1, 1,2);	
	
  l=new QLabel(i18n("Email Address:"), this);
  topL->addWidget(l, 2,0);
  email=new QLineEdit(this);
  topL->addMultiCellWidget(email, 2,2, 1,2);		

  l=new QLabel(i18n("Reply-To Address:"), this);	
  topL->addWidget(l, 3,0);
  replyTo=new QLineEdit(this);
  topL->addMultiCellWidget(replyTo, 3,3, 1,2);

  QButtonGroup *buttonGroup = new QButtonGroup(this);
  connect( buttonGroup, SIGNAL(clicked(int)),
	         this, SLOT(slotSignatureType(int)) );
  buttonGroup->hide();

  sigFile = new QRadioButton( i18n("Use a signature from file"), this );
  buttonGroup->insert(sigFile);
  topL->addMultiCellWidget(sigFile, 4, 4, 0, 2);

  l = new QLabel( i18n("Signature File:"), this);
  topL->addWidget(l, 5, 0 );
  sig = new QLineEdit(this);
  topL->addWidget(sig, 5, 1 );

  chooseBtn = new QPushButton( i18n("C&hoose..."), this);
  connect(chooseBtn, SIGNAL(clicked()),
          this, SLOT(slotSignatureChoose()));
  topL->addWidget(chooseBtn, 5, 2 );
  editBtn = new QPushButton( i18n("&Edit File"), this);
  connect(editBtn, SIGNAL(clicked()),
          this, SLOT(slotSignatureEdit()));
  topL->addWidget(editBtn, 6, 2);

  sigEdit = new QRadioButton( i18n("Specify signature below"), this);
  buttonGroup->insert(sigEdit);
  topL->addMultiCellWidget(sigEdit, 7, 7, 0, 2);

  sigEditor = new QMultiLineEdit(this);
  topL->addMultiCellWidget(sigEditor, 8, 8, 0, 2);

	topL->setColStretch(1,1);
	topL->setRowStretch(5,1);
	topL->setResizeMode(QLayout::Minimum);	
	topL->activate();
}



KNUserWidget::~KNUserWidget()
{
}



void KNUserWidget::setData(KNUserEntry *user)
{
	entry=user;
	
	name->setText(user->name());
	orga->setText(user->orga());	
	email->setText(user->email());
	replyTo->setText(user->replyTo());
	sig->setText(user->sigPath());
	sigEditor->setText(user->sigText());
	slotSignatureType(user->useSigFile()? 0:1);	
}



void KNUserWidget::applyData()
{
	if(entry) {
		entry->setName(name->text().local8Bit());
		entry->setOrga(orga->text().local8Bit());		
		entry->setEmail(email->text().local8Bit());
		entry->setReplyTo(replyTo->text().local8Bit());
		entry->setUseSigFile(sigFile->isChecked());
		entry->setSigPath(sig->text());
		entry->setSigText(sigEditor->text().local8Bit());
	}
}



void KNUserWidget::slotSignatureType(int type)
{
  bool sigFromFile = (type==0);

  sigFile->setChecked(sigFromFile);
  sig->setEnabled(sigFromFile);
  chooseBtn->setEnabled(sigFromFile);
  editBtn->setEnabled(sigFromFile);
  sigEdit->setChecked(!sigFromFile);
  sigEditor->setEnabled(!sigFromFile);
}



void KNUserWidget::slotSignatureChoose()
{
  QString tmp=KFileDialog::getOpenFileName(sig->text(),QString::null,this,i18n("Choose Signature"));
  if(!tmp.isEmpty()) sig->setText(tmp);
}



void KNUserWidget::slotSignatureEdit()
{
  QString fileName = sig->text().stripWhiteSpace();

  if (fileName.isEmpty()) {
    KMessageBox::error(this, i18n("You must specify a filename!"));
    return;
  }

  QFileInfo fileInfo( fileName );
  if (fileInfo.isDir()) {
    KMessageBox::error(this, i18n("You have specified a directory!"));
    return;
  }

  KService::Ptr offer = KServiceTypeProfile::preferredService("text/plain", true);
  KURL::List  lst(fileName);

  if (offer)
    KRun::run(*offer, lst);
  else {
    KFileOpenWithHandler *openhandler = new KFileOpenWithHandler();
    openhandler->displayOpenWithDialog(lst);
  }
}


// -----------------------------------------------------------------------------

#include "knuserwidget.moc"
