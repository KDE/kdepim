/***************************************************************************
                          kncomposer.cpp  -  description
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

#define HAVE_SGI_STL
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kabapi.h>

#include "kncomposer.h"
#include "kngroupselectdialog.h"
#include "knstringsplitter.h"
#include "utilities.h"


KNComposer::KNComposer(KNSavedArticle *a, const QCString &sig, KNNntpAccount *n)
    :	KTMainWindow(0)
{
	r_esult=CRsave;
	attChanged=false;
	a_rticle=a;
	nntp=n;
	if(!sig.isEmpty()) s_ignature=sig.copy();	
	
	//init GUI
	KMenuBar *mb=menuBar();
	QPopupMenu *fileMenu=new QPopupMenu();
	fileMenu->insertItem(i18n("&Send"), FILE_SEND);
	fileMenu->insertItem(i18n("Send &later"), FILE_SEND_LATER);
	fileMenu->insertItem(i18n("Sa&ve"), FILE_SAVE);
	fileMenu->insertSeparator();
	fileMenu->insertItem(i18n("&Delete"), FILE_DELETE);
	fileMenu->insertItem(i18n("&Close"), FILE_CLOSE);
	editMenu=new QPopupMenu();
	editMenu->insertItem(i18n("&Cut"), EDIT_CUT);
	editMenu->insertItem(i18n("Co&py"), EDIT_COPY);
	editMenu->insertItem(i18n("Pas&te"), EDIT_PASTE);
	editMenu->insertItem(i18n("&Select all"), EDIT_SEL_ALL);
	editMenu->insertSeparator();
	editMenu->insertItem(i18n("&Find"), EDIT_FIND);
	editMenu->insertItem(i18n("&Replace"), EDIT_REPLACE);
	appendMenu=new QPopupMenu();
	appendMenu->insertItem(i18n("Insert &signature"), APP_SIG);
	appendMenu->insertItem(i18n("Insert &file"), APP_FILE);
	appendMenu->insertItem(i18n("&Attach file"), APP_ATT_FILE);	
	mb->insertItem(i18n("&File"), fileMenu);
	mb->insertItem(i18n("&Edit"), editMenu);
	mb->insertItem(i18n("&Append"), appendMenu);
	connect(mb, SIGNAL(activated(int)), this, SLOT(slotCallback(int)));
	
	KToolBar* tb=new KToolBar(this,0,32);
  addToolBar(tb);
  tb->insertButton(UserIcon("send.xpm"), FILE_SEND, true, i18n("send now"));
  tb->insertButton(UserIcon("save.xpm"), FILE_SAVE, true, i18n("save"));
  tb->insertButton(UserIcon("signature.xpm"), APP_SIG, true, i18n("append signature"));
  tb->insertButton(UserIcon("delete.xpm"), FILE_DELETE, true, i18n("delete"));
  connect(tb, SIGNAL(clicked(int)), this, SLOT(slotCallback(int)));

  //init view
  view=new ComposerView(this, a_rticle->isMail());
	setView(view);
	connect(view->subject, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotSubjectChanged(const QString&)));
	if(!a_rticle->isMail()) {
		connect(view->fupCheck, SIGNAL(toggled(bool)),
			this, SLOT(slotFupCheckToggled(bool)));
	}
	connect(view->dest, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotDestinationChanged(const QString&)));
	connect(view->destButton, SIGNAL(clicked()),
		this, SLOT(slotDestButtonClicked()));

	//init data	
	initData();
	if(appSig) appendSignature();
	setConfig();
	setDialogSize("composer", this);	
}



KNComposer::~KNComposer()
{
	saveDialogSize("composer", this->size());	
}



void KNComposer::setConfig()
{
	view->edit->setWordWrap(QMultiLineEdit::FixedColumnWidth);
  view->edit->setWrapColumnOrWidth(lineLen);
	QFont fnt=KGlobal::generalFont();
	if(useViewFnt) fnt.setFamily(fntFam);
	view->edit->setFont(fnt);
}



void KNComposer::closeEvent(QCloseEvent *e)
{
	if(a_rticle->id()==-1) r_esult=CRdel;
	else r_esult=CRcancel;
	
	e->accept();
	emit composerDone(this);
}



void KNComposer::initData()
{
	KNMimeContent *body=a_rticle->mainContent();

	d_estination=a_rticle->destination().copy();
	view->dest->setText(d_estination);
	if(!a_rticle->isMail()) slotDestinationChanged(d_estination);
	
	if(body) {
		for(char *line=body->firstBodyLine(); line; line=body->nextBodyLine())
			view->edit->insertLine(line);
	}
		
	if(a_rticle->subject().isEmpty()) slotSubjectChanged(QString::null);
	else view->subject->setText(a_rticle->subject());
//	view->edit->toggleModified(false);		
}



bool KNComposer::hasValidData()
{
	return ( (!view->subject->text().isEmpty()) && (!d_estination.isEmpty()) );
}



void KNComposer::bodyContent(KNMimeContent *b)
{
	b->clearBody();
	for(int idx=0; idx < view->edit->numLines(); idx++)
			b->addBodyLine(view->edit->textLine(idx).local8Bit());
}



QCString KNComposer::followUp2()
{
	QCString ret;
	if(view->fupCheck->isChecked() && view->fup2->count()!=0)
		ret=view->fup2->currentText().local8Bit();
	return ret;
}



void KNComposer::appendSignature()
{
	int pos=-1, cnt=0;
	if(!s_ignature.isEmpty()) {
		for(int i=view->edit->numLines()-1; i>=0; i--) {
			cnt++;
			if(view->edit->textLine(i).left(3) == "-- ") {
				pos=i;
				break;
			}
		}
					
		if(pos!=-1)
			for(int i=0; i<cnt; i++) view->edit->removeLine(pos);
		
		view->edit->insertLine("-- ");
  	view->edit->insertLine(s_ignature);
  	view->edit->setModified(true);		
	}
}



void KNComposer::attachFile()
{
	snyimpl();
}



void KNComposer::slotDestinationChanged(const QString &t)
{
	KNStringSplitter split;
	bool splitOk;
		
	d_estination=t.local8Bit();
 	
 	if(!a_rticle->isMail()) {
   	view->fup2->clear();
   	split.init(d_estination, ",");
    splitOk=split.first();
    view->fupCheck->setEnabled(splitOk);
  	view->fupCheck->setChecked(splitOk);
  	view->fup2->setEnabled(splitOk);
    while(splitOk) {
     	view->fup2->insertItem(QString(split.string()));
     	splitOk=split.next();
    }
	}
}



void KNComposer::slotCallback(int id)
{
	switch(id) {
		case FILE_SEND:
			r_esult=CRsendNow;
			emit composerDone(this);
		break;
		case FILE_SEND_LATER:
			r_esult=CRsendLater;
			emit composerDone(this);
		break;
		case FILE_SAVE:
			r_esult=CRsave;
			emit composerDone(this);
		break;
		case FILE_DELETE:
			r_esult=CRdelAsk;
			emit composerDone(this);
		break;
		case FILE_CLOSE:
			close();
		break;
		case EDIT_CUT:
			view->edit->cut();
		break;
		case EDIT_COPY:
			view->edit->copyText();
		break;
		case EDIT_PASTE:
			view->edit->paste();
		break;
		case EDIT_SEL_ALL:
			view->edit->selectAll();
		break;
		case EDIT_FIND:
			view->edit->search();
		break;
		case EDIT_REPLACE:
			view->edit->replace();
		break;
		case APP_SIG:
			appendSignature();
		break;
		case APP_FILE:
			snyimpl();
		break;
		case APP_ATT_FILE:
			attachFile();
		break;		
	}
}



/*void KNComposer::slotDestComboActivated(int idx)
{
	if(idx==0 && s_tatus!=KNArticleBase::AStoPost) {
		s_tatus=KNArticleBase::AStoPost;
		view->dest->clear();
		view->fupCheck->setEnabled(true);
		view->fup2->setEnabled(view->fupCheck->isChecked());
	}
	else if(idx==1 && s_tatus!=KNArticleBase::AStoMail) {
		s_tatus=KNArticleBase::AStoMail;
		view->dest->clear();
		view->fup2->clear();
		view->fupCheck->setChecked(false);
		view->fupCheck->setEnabled(false);
	}				
} */



void KNComposer::slotDestButtonClicked()
{
	KNGroupSelectDialog *gsdlg=0;
	KabAPI *kab;
	AddressBook::Entry entry;
	KabKey key;
		
	if(!a_rticle->isMail()) {
		gsdlg=new KNGroupSelectDialog(nntp, d_estination, this);
		if(gsdlg->exec()) {
	 		d_estination=gsdlg->selectedGroups();
	 		view->dest->setText(d_estination);
	 	}
		delete gsdlg;
	}
	else {
		kab=new KabAPI(this);
		if(kab->init()!=AddressBook::NoError) {
			KMessageBox::error(0, i18n("Cannot initialize the adressbook"));
			delete kab;
			return;
		}
		if(kab->exec()) {
			if(kab->getEntry(entry, key)!=AddressBook::NoError) {
				KMessageBox::error(0, i18n("Cannot read adressbook-entry"));
			}
			else {
				if(!d_estination.isEmpty()) d_estination+=",";
				d_estination+=entry.emails.first().local8Bit();
				view->dest->setText(d_estination);
				//qDebug("KabApi::getEntry() : name: %s, key: %s", entry.lastname.latin1(), ());
			}
    }
    delete kab;			
	}
}



void KNComposer::slotFupCheckToggled(bool b)
{
	view->fup2->setEnabled(b);
}



void KNComposer::slotSubjectChanged(const QString &t)
{
	if(!t.isEmpty()) setCaption(t);
	else setCaption(i18n("No subject"));
}




//=====================================================================================


KNComposer::ComposerView::ComposerView(QWidget *parent, bool mail)
	: QSplitter(QSplitter::Vertical, parent, 0)
{
	QLabel *l1, *l2;
	int frameLines=2;
	QWidget *editW=new QWidget(this);
	QFrame *fr1=new QFrame(editW);
	fr1->setFrameStyle(QFrame::Box | QFrame::Sunken);
	l1=new QLabel(i18n("Subject:"), fr1);
	subject=new QLineEdit(fr1);
	if(mail) l2=new QLabel(i18n("To:"), fr1);
	else l2=new QLabel(i18n("Groups:"), fr1);
	dest=new QLineEdit(fr1);
	destButton=new QPushButton("...", fr1);
	if(!mail) {
		fupCheck=new QCheckBox(i18n("Followup-To:"), fr1);
		fup2=new QComboBox(true, fr1);
		frameLines=3;
		SIZE(fupCheck);
		SIZE(fup2);
	}
	
	//SIZE(l1); SIZE(l2); SIZE(destButton);
	//SIZE(subject); SIZE(dest); 	

	edit=new KEdit(editW);
	edit->setMinimumHeight(150);
	
	QGridLayout *frameL1=new QGridLayout(fr1, frameLines,3, 10,10);
	frameL1->addWidget(l1, 0,0);
	frameL1->addMultiCellWidget(subject, 0,0,1,2);
	frameL1->addWidget(l2, 1,0);
	frameL1->addWidget(dest, 1,1);
	frameL1->addWidget(destButton, 1,2);
	if(!mail) {
		frameL1->addWidget(fupCheck, 2,0);
		frameL1->addMultiCellWidget(fup2, 2,2,1,2);
	}
	frameL1->setColStretch(1,1);
		
	QVBoxLayout *topL=new QVBoxLayout(editW, 5,5);
	topL->addWidget(fr1);
	topL->addWidget(edit, 1);
	topL->activate();
	
	attList=0;
}



KNComposer::ComposerView::~ComposerView()
{
}



void KNComposer::ComposerView::showAttachementList()
{
	if(!attList) {
		attList=new KNListBox(this);
		attList->show();
	}
}



//===============================================================
bool KNComposer::appSig;
bool KNComposer::useViewFnt;
int KNComposer::lineLen;
QString KNComposer::fntFam;

void KNComposer::readConfig()
{
	KConfig *conf=CONF();
	conf->setGroup("POSTNEWS");
	lineLen=conf->readNumEntry("maxLength", 72);
	appSig=conf->readBoolEntry("appSig", true);
	useViewFnt=conf->readBoolEntry("useViewFont", false);
	conf->setGroup("FONTS-COLORS");
	fntFam=conf->readEntry("family", "helvetica");
}

