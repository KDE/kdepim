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

//#define HAVE_SGI_STL
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kabapi.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <keditcl.h>

#include "knsavedarticle.h"
#include "knmimecontent.h"
#include "kngroupselectdialog.h"
#include "knstringsplitter.h"
#include "utilities.h"
#include "kncomposer.h"


KNComposer::KNComposer(KNSavedArticle *a, const QCString &sig, KNNntpAccount *n)
    :	KTMainWindow(0), r_esult(CRsave), a_rticle(a), nntp(n), attChanged(false)
{
	if(!sig.isEmpty()) s_ignature=sig.copy();	
	
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
		
  // file menu
  new KAction(i18n("&Send Now"),"sendnow", 0 , this, SLOT(slotSendNow()),
              actionCollection(), "send_now");
  new KAction(i18n("Send &Later"), 0, this, SLOT(slotSendLater()),
              actionCollection(), "send_later");
  new KAction(i18n("Save As &Draft"),"save", 0 , this, SLOT(slotSaveAsDraft()),
              actionCollection(), "save_as_draft");
  new KAction(i18n("D&elete"),"delete", 0 , this, SLOT(slotArtDelete()),
              actionCollection(), "art_delete");
  KStdAction::close(this, SLOT(slotFileClose()),actionCollection());

  // edit menu
  KStdAction::undo(view->edit, SLOT(undo()), actionCollection());
  KStdAction::redo(view->edit, SLOT(redo()), actionCollection());
  KStdAction::cut(view->edit, SLOT(cut()), actionCollection());
  KStdAction::copy(view->edit, SLOT(copy()), actionCollection());
  KStdAction::paste(view->edit, SLOT(paste()), actionCollection());
  KStdAction::selectAll(view->edit, SLOT(selectAll()), actionCollection());
  KStdAction::find(this, SLOT(slotFind()), actionCollection());
  KStdAction::findNext(this, SLOT(slotFindNext()), actionCollection());
  KStdAction::replace(this, SLOT(slotReplace()), actionCollection());
  KStdAction::spelling (this, SLOT(slotSpellcheck()), actionCollection(), "spellcheck");

  // attach menu
  new KAction(i18n("Append &Signature"), "signature", 0 , this, SLOT(slotAppendSig()),
                   actionCollection(), "append_signature");
  new KAction(i18n("&Insert File"), 0, this, SLOT(slotInsertFile()),
                   actionCollection(), "insert_file");
  new KAction(i18n("Attach &File"), 0, this, SLOT(slotAttachFile()),
                   actionCollection(), "attach_file");

  // settings menu
  KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  new KAction(i18n("Configure &Spellchecker"), 0, this, SLOT(slotConfSpellchecker()),
                   actionCollection(), "setup_spellchecker");

  createGUI("kncomposerui.rc");

	//init data	
	initData();
	if(appSig) slotAppendSig();
	setConfig();
	restoreWindowSize("composer", this, sizeHint());
}


KNComposer::~KNComposer()
{
	saveWindowSize("composer", size());	
}


void KNComposer::setConfig()
{
	view->edit->setWordWrap(QMultiLineEdit::FixedColumnWidth);
  view->edit->setWrapColumnOrWidth(lineLen);
	QFont fnt=KGlobalSettings::generalFont();
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
	QString path;
		
	if(!a_rticle->isMail()) {
		gsdlg=new KNGroupSelectDialog(this, nntp, d_estination);
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


void KNComposer::slotSendNow()
{
	r_esult=CRsendNow;
	emit composerDone(this);
}


void KNComposer::slotSendLater()
{
  r_esult=CRsendLater;
  emit composerDone(this);
}


void KNComposer::slotSaveAsDraft()
{			
  r_esult=CRsave;
  emit composerDone(this);
}


void KNComposer::slotArtDelete()
{
  r_esult=CRdelAsk;
  emit composerDone(this);
}


void KNComposer::slotFileClose()
{
	close();
}


void KNComposer::slotFind()
{
	view->edit->search();
}


void KNComposer::slotFindNext()
{
	view->edit->repeatSearch();
}


void KNComposer::slotReplace()
{
  view->edit->replace();
}


void KNComposer::slotSpellcheck()
{
  #warning stub - spellcheck
}


void KNComposer::slotAppendSig()
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


void KNComposer::slotInsertFile()
{
  KURL url = KFileDialog::getOpenURL(QString::null, QString::null, this, i18n("Insert File"));
  QString fileName;

  if (url.isEmpty())
    return;

  if (url.isLocalFile())
    fileName = url.path();
  else
    if (!KIO::NetAccess::download(url, fileName))
      return;

  unsigned int len = QFileInfo(fileName).size();
  unsigned int readLen;
  QCString temp;
  QFile file(fileName);

  if (!file.open(IO_Raw|IO_ReadOnly)) {
    displayExternalFileError();
  } else {
    temp.resize(len + 2);
    readLen = file.readBlock(temp.data(), len);
    if (temp[len-1]!='\n')
      temp[len++] = '\n';
    temp[len] = '\0';
  }

  if (!url.isLocalFile()) {
    KIO::NetAccess::removeTempFile(fileName);
  }

  int editLine,editCol;
  view->edit->getCursorPosition(&editLine, &editCol);
  view->edit->insertAt(temp, editLine, editCol);
}


void KNComposer::slotAttachFile()
{
  #warning stub - attachFile
}


void KNComposer::slotToggleToolBar()
{
  if(toolBar()->isVisible())
    toolBar()->hide();
  else
    toolBar()->show();
}

  	
void KNComposer::slotConfKeys()
{
  KKeyDialog::configureKeys(actionCollection(),xmlFile());
}
 	
  	
void KNComposer::slotConfToolbar()
{
  KEditToolbar dlg(actionCollection());
  if (dlg.exec()) {
    createGUI();
  }
}


void KNComposer::slotConfSpellchecker()
{
  #warning stub - setup Spellchecker
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
	destButton=new QPushButton(i18n("Browse..."), fr1);
	if(!mail) {
		fupCheck=new QCheckBox(i18n("Followup-To:"), fr1);
		fupCheck->setMinimumSize(fupCheck->sizeHint());
		fup2=new QComboBox(true, fr1);
		fup2->setMinimumSize(fup2->sizeHint());
		frameLines=3;
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
		attList=new QListView(this);
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
	KConfig *conf=KGlobal::config();
	conf->setGroup("POSTNEWS");
	lineLen=conf->readNumEntry("maxLength", 72);
	appSig=conf->readBoolEntry("appSig", true);
	useViewFnt=conf->readBoolEntry("useViewFont", false);
	conf->setGroup("FONTS-COLORS");
	fntFam=conf->readEntry("family", "helvetica");
}




//--------------------------------

#include "kncomposer.moc"
