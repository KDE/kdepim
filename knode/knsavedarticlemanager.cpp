/***************************************************************************
                          knsavedarticlemanager.cpp  -  description
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

#include <mimelib/datetime.h>
#include <qheader.h>

#include <kurl.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kconfig.h>

#include "kngroupmanager.h"
#include "knjobdata.h"
#include "knnntpaccount.h"
#include "knarticlewidget.h"
#include "knsavedarticle.h"
#include "knode.h"
#include "knuserentry.h"
#include "knhdrviewitem.h"
#include "kngroup.h"
#include "knfetcharticle.h"
#include "knfolder.h"
#include "kncomposer.h"
#include "knsenderrordialog.h"
#include "knsearchdialog.h"
#include "knaccountmanager.h"
#include "knnetaccess.h"
#include "knglobals.h"
#include "utilities.h"
#include "knsavedarticlemanager.h"


KNSavedArticleManager::KNSavedArticleManager(KNListView *v, KNAccountManager *am, QObject * parent, const char * name) :
	QObject(parent,name), KNArticleManager(v), f_older(0), sedlg(0), sDlg(0), accM(am)
{
	//f_ilter=0;

	defaultUser=new KNUserEntry();
	comList=new QList<KNComposer>;
		
	readConfig();
	
  actSendOutbox = new KAction(i18n("Sen&d pending messages"), 0, this, SLOT(slotSendOutbox()),
                              &actionCollection, "net_sendPending");
  actSendOutbox->setEnabled(false);
  actEdit = new KAction(i18n("&Edit"), Key_E , this, SLOT(slotEdit()),
                        &actionCollection, "article_edit");
  actEdit->setEnabled(false);
  actDelete = new KAction(i18n("&Delete"), Key_Delete , this, SLOT(slotDelete()),
                          &actionCollection, "article_delete");
  actDelete->setEnabled(false);
  actSendNow = new KAction(i18n("Send &now"), 0 , this, SLOT(slotSendNow()),
                           &actionCollection, "article_sendNow");
  actSendNow->setEnabled(false);
  actSendLater = new KAction(i18n("Send &later"), 0 , this, SLOT(slotSendLater()),
                             &actionCollection, "article_sendLater");
  actSendLater->setEnabled(false);
}



KNSavedArticleManager::~KNSavedArticleManager()
{
	delete defaultUser;
	delete sDlg;
	delete sedlg;
	delete comList;
}



void KNSavedArticleManager::readConfig()
{
	KConfig *conf=KGlobal::config();
	QCString tmp;
	conf->setGroup("IDENTITY");
	defaultUser->load(conf);
	conf->setGroup("POSTNEWS");
	incSig=conf->readBoolEntry("incSig",true);
	quotSign=conf->readEntry("QuotSign",">").local8Bit();
	intro=conf->readEntry("Intro", "%NAME wrote:").local8Bit();
	charset=conf->readEntry("Charset", "us-ascii").local8Bit();
	enc=(KNArticleBase::encoding)(conf->readNumEntry("Encoding", 0));
	KNArticleBase::setAllow8bitHeaders(conf->readBoolEntry("allow8bitChars", false));
	genMId=conf->readBoolEntry("generateMId", false);
	MIdhost=conf->readEntry("MIdhost").local8Bit();
	KNComposer::readConfig();
	for(KNComposer *c=comList->first(); c; c=comList->next())
		c->setConfig();
}


		
void KNSavedArticleManager::setStandardFolders(KNFolder *d, KNFolder *o, KNFolder *s)
{
  fDrafts=d;
  fOutbox=o;
  actSendOutbox->setEnabled(!fOutbox->isEmpty());
  fSent=s;
}



void KNSavedArticleManager::setFolder(KNFolder *f)
{
	if(f!=0) {
		if(f_older==0) view->header()->setLabel(1, i18n("newsgroups / To"));
		if(sDlg) {
			//if(sDlg->filter()==f_ilter) slotDoSearch(0);
			sDlg->hide();
		}
	}
	f_older=f;
	setCurrentArticle(0);	
}



void KNSavedArticleManager::showHdrs()
{
	KNSavedArticle *art;
	KNHdrViewItem *it;
	//bool filterResult=true;
	if(!f_older) return;
	
	view->clear();
	knGlobals.top->setCursorBusy(true);
	knGlobals.top->setStatusMsg(i18n(" Creating list ..."));
	for(int idx=0; idx<f_older->length(); idx++) {
		art=f_older->at(idx);
		//if(f_ilter) filterResult=f_ilter->applyFilter(art);
		//if(filterResult) {
			it=new KNHdrViewItem(view, art);
			art->setListItem(it);
			art->updateListItem();
		//}
	}
	if(view->firstChild())
	  view->setCurrentItem(view->firstChild());
	
	knGlobals.top->setStatusMsg();
	knGlobals.top->setCursorBusy(false);
	updateStatusString();
}



/*void KNSavedArticleManager::search()
{
	if(!f_older) return;
	if(sDlg) sDlg->show();
	else {
		sDlg=new KNSearchDialog(KNSearchDialog::STfolderSearch);
		connect(sDlg, SIGNAL(dialogDone()), this, SLOT(slotSearchDialogDone()));
		connect(sDlg, SIGNAL(doSearch(KNArticleFilter*)),
			this, SLOT(slotDoSearch(KNArticleFilter*)));
		sDlg->show();
	}
}*/



void KNSavedArticleManager::setCurrentArticle(KNSavedArticle *a)
{
	c_urrentArticle=a;
	if(a) {
		mainArtWidget->setData(a, f_older);
		if(a->hasContent()) showArticle(a);
		else {
			if(a->folder() && a->folder()->loadArticle(a)) showArticle(a);
			else showError(a, i18n("Cannot load the article!"));
		}
	}	else {
    actEdit->setEnabled(false);
    actDelete->setEnabled(false);
    actSendNow->setEnabled(false);
    actSendLater->setEnabled(false);
	}
}



void KNSavedArticleManager::post(KNNntpAccount *acc)
{
	if(!acc) return;
	if(defaultUser->isValid()) {
  	KNSavedArticle *art=newArticle(acc);
  	if(!art) return;
  	openInComposer(art);
  }
  else KMessageBox::information(0, i18n("Please set your name and email first."));	
}



void KNSavedArticleManager::post(KNGroup *g)
{
	if(!g) return;
	if(defaultUser->isValid()) {
  	KNSavedArticle *art=newArticle(g->account());
  	if(!art) return;
  	art->setDestination(g->name().utf8().copy());
  	openInComposer(art);
  }
  else KMessageBox::information(0, i18n("Please set your name and email first."));			
}



void KNSavedArticleManager::reply(KNArticle *a, KNGroup *g)
{
	QCString tmp, refs, introStr;
	int start=0, found=0;
	bool asMail=(g==0);
	KNSavedArticle *art;
	KNMimeContent *body;
	
	if(!a) return;
	if(asMail) art=newArticle();
	else art=newArticle(g->account());
  if(!art) return;
	
	if(asMail) {
		tmp=a->replyToEmail();
		if(tmp.isEmpty()) tmp=a->fromEmail().copy();
		art->setDestination(tmp);
	}
	else {
		tmp=a->headerLine("Followup-To");
		if(tmp.isEmpty()) tmp=a->headerLine("Newsgroups");
		else if(strcasecmp(tmp, "poster")==0) {
			art->setStatus(KNArticleBase::AStoMail);
			art->setServerId(-1);
			tmp=a->replyToEmail();
			if(tmp.isEmpty()) tmp=a->fromEmail().copy();
		}
		art->setDestination(tmp);
		tmp=a->headerLine("References");
		if(tmp.isEmpty()) tmp=a->headerLine("Message-Id");
		else tmp+=" "+a->headerLine("Message-Id");
		art->references().setLine(tmp);
	}
	
		
	if(strncasecmp(a->subject(), "re:", 3)!=0) tmp="Re: "+a->subject();
	else tmp=a->subject().copy();
	art->setSubject(tmp);
	
	
	introStr="";
	while(found!=-1) {
		found=intro.find('%',start);
		if(found>-1) {
			introStr+=intro.mid(start,found-start);
			tmp=intro.mid(found+1,4);	
		  if(tmp=="NAME") {
		  	introStr+=a->fromName();
		  	start=found+5;
		  }		
			else if(tmp=="DATE") {
				introStr+=a->headerLine("Date");
				start=found+5;
			}
			else if(tmp=="MSID") {
				introStr+=a->headerLine("Message-ID");
				start=found+5;
			}
			else {
				introStr+='%';
				start=found+1;
			}
		}		
		else introStr+=intro.mid(start, intro.length());
	}
	art->addBodyLine(introStr);
	art->addBodyLine("");
	body=a->mainContent();
	if(!body->mimeInfo()->isReadable()) body->prepareForDisplay();
	for(char *line=body->firstBodyLine(); line; line=body->nextBodyLine()) {
		if(!incSig && strncmp("-- ", line, 3)==0) break;
		tmp=quotSign+" ";
		tmp+=line;
		art->addBodyLine(tmp);
	}
	openInComposer(art);		
}



void KNSavedArticleManager::forward(KNArticle *a)
{
	KNSavedArticle *art;
	KNMimeContent *body;
	QCString tmp;
	
	if(!a) return;
	
		
	body=a->mainContent();
	art=newArticle();
	if(!art) return;
	  	
	tmp="Fwd: "+a->subject();
	art->setSubject(tmp);
	art->addBodyLine("");
	art->addBodyLine("======= Forwarded message (begin) =======");
	tmp="Subject: " + a->subject();
	art->addBodyLine(tmp);
	tmp="Date: " + a->headerLine("Date");
	art->addBodyLine(tmp);
	tmp="From: " + a->headerLine("From");
	art->addBodyLine(tmp);
	art->addBodyLine("");
	
  if(body) {
  	for(char *line=body->firstBodyLine(); line; line=body->nextBodyLine())
  		if(strcmp("-- ", line)==0) art->addBodyLine("--");
  		else art->addBodyLine(line);
  }
	
  art->addBodyLine("=======  Forwarded message (end)  =======");
  art->addBodyLine("");
  art->addBodyLine("");
  openInComposer(art);	
}



void KNSavedArticleManager::editArticle(KNSavedArticle *a)
{
	if(!a) a=c_urrentArticle;
	if(!a) return;
	if(a->editable()) openInComposer(a);
	else KMessageBox::information(0, i18n("Sorry this article cannot be edited!"));
}



void KNSavedArticleManager::saveArticle(KNSavedArticle *a)
{
	if(a->id()==-1) {
		if(!fDrafts->addArticle(a)) {
			KMessageBox::error(0, i18n("Cannot save the article!"));
			delete a;
		}
		else if(f_older==fDrafts) showHdrs();
	}
	else {
		if(!a->folder()->saveArticle(a))
			KMessageBox::error(0, i18n("Cannot save the article!"));
	}
	
	a->updateListItem();
	showArticle(a, true);		
}



bool KNSavedArticleManager::deleteArticle(KNSavedArticle *a, bool ask)
{
	KNNntpAccount *acc=0;
	if(!a) a=c_urrentArticle;
	if(!a) return false;
	
	if((!ask)||
		 (KMessageBox::Yes==KMessageBox::questionYesNo(0,i18n("Do you really want to delete\n this article?")))) {
		if(a->id()!=-1) a->folder()->removeArticle(a);
		if(a->serverId()!=-1 && !a->sent()) {
			acc=getAccount(a);
			if(acc) acc->decUnsentCount();
		}
		delete a;
	  if(a==c_urrentArticle)
	    mainArtWidget->showBlankPage();
	  updateStatusString();
	  return true;
	}
	else return false;
}



void KNSavedArticleManager::sendArticle(KNSavedArticle *a, bool now)
{
	KNJobData *job;
	if(!a) a=c_urrentArticle;
	if(!a) return;
	
	if(a->sent()) {
		KMessageBox::information(0, i18n("This article has already been sent."));
		return;
	}	
	
	if(!a->hasContent() && a->folder())
		if(!a->folder()->loadArticle(a)) {
			KMessageBox::error(0, i18n("Cannot load the article"));
			return;
		}
		
	if(now) {
		if(a->locked()) return;
		
		if(a->isMail()) job=new KNJobData(KNJobData::JTmail, accM->smtp(), a);
		else job=new KNJobData(KNJobData::JTpostArticle, getAccount(a), a);
		
	  knGlobals.netAccess->addJob(job);
	}
	else {
		fOutbox->addArticle(a);
	  actSendOutbox->setEnabled(true);
		if(f_older==fOutbox) showHdrs();
	}
	if(a==c_urrentArticle) mainArtWidget->showBlankPage();
}



void KNSavedArticleManager::sendOutbox()
{
	KNSavedArticle *art=0;
	KNJobData *job;
	
	if(fOutbox->isEmpty()) {
		KMessageBox::information(0, i18n("The outbox is empty"));
		return;
	}
	
	for(int idx=0; idx<fOutbox->length(); idx++) {
		art=fOutbox->at(idx);
		if(art->locked() || art->sent()) continue;
		
		if(art->isMail())
			job=new KNJobData(KNJobData::JTmail, accM->smtp(), art);
		else
			job=new KNJobData(KNJobData::JTpostArticle, getAccount(art), art);
				
		if(!art->hasContent() && art->folder()) {
			if(!art->folder()->loadArticle(art)) {
				job->setErrorString(i18n("Could not load the article"));
				jobDone(job);
			}
		}
		else knGlobals.netAccess->addJob(job);
	}
}



void KNSavedArticleManager::cancel(KNSavedArticle *a)
{
	if (!a) a=c_urrentArticle;
	if (!a) return;
	
  if (cancelAllowed(a))         // check if we can cancel
    if (generateCancel(a,getAccount(a))) {
   		a->setStatus(KNArticleBase::AScanceled);      // the user agreed...
	  	a->setHeader(KNArticleBase::HTxknstatus, "canceled");
  		saveArticle(a);
  		a->updateListItem();
  		a->folder()->setToSync(true);
  	}
}



void KNSavedArticleManager::cancel(KNFetchArticle *a, KNGroup *g)
{
  if (!a || !g) return;

  if (cancelAllowed(a,g))
		generateCancel(a,g->account());
}



void KNSavedArticleManager::supersede(KNSavedArticle *a)
{
	if (!a) a=c_urrentArticle;
	if (!a) return;
	
  if (cancelAllowed(a))         // check if we can cancel
    if (generateSupersede(a,getAccount(a))) {
   		a->setStatus(KNArticleBase::AScanceled);      // the user agreed...
	  	a->setHeader(KNArticleBase::HTxknstatus, "canceled");
  		saveArticle(a);
  		a->updateListItem();
  		a->folder()->setToSync(true);
  	}
}



void KNSavedArticleManager::supersede(KNFetchArticle *a, KNGroup *g)
{
  if (!a || !g) return;
	
  if (cancelAllowed(a,g))
		generateSupersede(a,g->account());
}



KNSavedArticle* KNSavedArticleManager::newArticle(KNNntpAccount *acc)
{
	QCString mid;
	KNSavedArticle *a;
	
	if(genMId) {
		if(MIdhost.isEmpty()) {
			KMessageBox::information(0, i18n("Please set a hostname for the generation\nof the message-id or disable it."));
	  	return 0;
	  }
	  else {
	  	mid="<"+KNArticleBase::uniqueString();
			mid+="@"+MIdhost+">";
		}
	}
	
	if(acc) {
		a=new KNSavedArticle(KNArticleBase::AStoPost);
		a->setServerId(acc->id());
		acc->incUnsentCount();
	}
	else a=new KNSavedArticle(KNArticleBase::AStoMail);
	a->initContent();
	if(genMId) a->setHeader(KNArticleBase::HTmessageId, mid);
	
	//x-headers
	QString dir(KGlobal::dirs()->saveLocation("appdata"));
	if (dir==QString::null)
		displayInternalFileError();
	else {
		KNFile f(dir+"xheaders");
		if(f.open(IO_ReadOnly)) {
			while(!f.atEnd())
				a->addHeaderLine(f.readLine(), true);		
			f.close();
		}		
	}
	
	return a;	
}



KNNntpAccount* KNSavedArticleManager::getAccount(KNSavedArticle *a)
{
	KNNntpAccount *acc=0;
		
	if(!a->isMail()) {
		acc=accM->account(a->serverId());
  }
	return acc;
}



void KNSavedArticleManager::openInComposer(KNSavedArticle *a)
{
	QCString sigPath, sig, tmp;
	KNNntpAccount *acc=getAccount(a);
	
	if(!a->hasContent() && a->folder())
		if(!a->folder()->loadArticle(a)) {
			KMessageBox::error(0, i18n("Cannot load the article"));
			return;
		}
	if(!a->isMail() && a->hasDestination()) {
		KNGroup *g=knGlobals.gManager->group(a->firstDestination(), acc);
		if(g && g->user() && g->user()->hasSigPath()) sigPath=g->user()->sigPath();
	}
	if(sigPath.isEmpty()) sigPath=defaultUser->sigPath();			
	
	if(!sigPath.isEmpty()) {
	  KNFile sigFile(sigPath);	
		if(sigFile.open(IO_ReadOnly)) {
			sig=sigFile.readLineWnewLine();
			while(!sigFile.atEnd())
			  sig += sigFile.readLineWnewLine();
		}
		else KMessageBox::error(0, i18n("Cannot open the signature-file"));
	}	
	
	KNComposer *com=new KNComposer(a, sig, acc);
  com->show();
  connect(com, SIGNAL(composerDone(KNComposer*)),
  	this, SLOT(slotComposerDone(KNComposer*)));
  comList->append(com);
}



bool KNSavedArticleManager::getComposerData(KNComposer *c)
{
	KNSavedArticle *art=c->article();
	KNUserEntry *guser=0, *usr=0;
	KNGroup *g=0;
	DwDateTime dt;
	QCString tmp;

	if(!c->hasValidData()) {
		KMessageBox::information(0, i18n("Please enter a subject and at least one\nnewsgroup or mail-address!"));
		return false;
	}
	
	//set and assemble data
	if(c->textChanged()) c->bodyContent(art);
	art->setSubject(c->subject());
	art->setDestination(c->destination());
  art->setTimeT(dt.AsUnixTime());
  art->mimeInfo()->setIsReadable(true);
	art->mimeInfo()->setCTEncoding(enc);
	art->mimeInfo()->setCTMediaType(KNArticleBase::MTtext);
	art->mimeInfo()->setCTSubType(KNArticleBase::STplain);	
	art->mimeInfo()->setCharsetParameter(charset);
	art->assemble();
	
	
	//set additional headers
	if(!art->isMail()) {
		g=knGlobals.gManager->group(art->firstDestination(), getAccount(art));
		if(g) guser=g->user();
	}
	
	//UserAgent
	art->setHeader(KNArticleBase::HTuserAgent, "KNode " KNODE_VERSION);
	
	//Organization
	if(guser && guser->hasOrga()) usr=guser;
	else usr=defaultUser;
	if(usr->hasOrga()) art->setHeader(KNArticleBase::HTorga, usr->orga(), true);
	else art->removeHeader("Organization");
	
	//Lines
	tmp.setNum(c->lines());
	art->setHeader(KNArticleBase::HTlines, tmp);
		
	//Fup2
	if(art->isMail())
		art->removeHeader("Followup-To");
	else {
		tmp=c->followUp2();
		if(!tmp.isEmpty()) art->setHeader(KNArticleBase::HTfup2, tmp);
		else art->removeHeader("Followup-To");
	}

	//Reply-To
  if(guser && guser->hasReplyTo())
		usr=guser;
	else
		usr=defaultUser;
	if(usr->hasReplyTo())
		art->setHeader(KNArticleBase::HTreplyTo, usr->replyTo(), true);
	else
		art->removeHeader("Reply-To");
	
	//From
	if(guser && guser->hasName()) usr=guser;
	else usr=defaultUser;
	tmp=usr->name().copy()+" <";
	if(guser && guser->hasEmail()) usr=guser;
	else usr=defaultUser;
	tmp+=usr->email()+">";			
	art->setHeader(KNArticleBase::HTfrom, tmp, true);
		
	return true;	
}



void KNSavedArticleManager::showArticle(KNArticle *a, bool force)
{
	KNArticleManager::showArticle(a, force);
  actEdit->setEnabled(true);
  actDelete->setEnabled(true);
  actSendNow->setEnabled(true);
  actSendLater->setEnabled(true);
}



void KNSavedArticleManager::showError(KNArticle *a, const QString &error)
{
  KNArticleManager::showError(a, error);
  actEdit->setEnabled(false);
  actDelete->setEnabled(false);
  actSendNow->setEnabled(false);
  actSendLater->setEnabled(false);
}



void KNSavedArticleManager::slotComposerDone(KNComposer *com)
{
	KNSavedArticle *art=com->article();
	bool delCom=true;
		
	switch(com->result()) {
		case KNComposer::CRsendNow:
			delCom=getComposerData(com);
			if(delCom) sendArticle(art, true);
			  else com->setDoneSuccess(false);			
		break;
		case KNComposer::CRsendLater:
			delCom=getComposerData(com);
			if (delCom) sendArticle(art, false);
			  else com->setDoneSuccess(false);
		break;
		case KNComposer::CRsave :
			delCom=getComposerData(com);
			if(delCom) saveArticle(art);
			  else com->setDoneSuccess(false);			
		break;
		case KNComposer::CRdelAsk:
			delCom=deleteArticle(art, true);
		break;
		case KNComposer::CRdel:
			delCom=deleteArticle(art, false);
		break;
		default: break;
	};
	if(delCom) {
		comList->removeRef(com);
		delete com;
	}	
}



void KNSavedArticleManager::slotSendErrorDialogDone()
{
	delete sedlg;
	sedlg=0;
}


void KNSavedArticleManager::jobDone(KNJobData *job)
{
	KNSavedArticle *art;
	KNNntpAccount *acc;
	art=(KNSavedArticle*)job->data();
	
	if(!job->success()) {
		if(!sedlg) {
			sedlg=new KNSendErrorDialog();
			connect(sedlg, SIGNAL(dialogDone()), this, SLOT(slotSendErrorDialogDone()));
			sedlg->show();
		}
		sedlg->appendJob(job);
		fOutbox->addArticle(art);
		if(f_older==fOutbox) showHdrs();
    actSendOutbox->setEnabled(true);
	}
  else {
		if(art->isMail()) {
			art->setHeader(KNArticleBase::HTxknstatus, "mailed");
			art->setStatus(KNArticleBase::ASmailed);
		}
		else {
			art->setHeader(KNArticleBase::HTxknstatus, "posted");
			art->setStatus(KNArticleBase::ASposted);
			acc=(KNNntpAccount*)job->account();
			acc->decUnsentCount();
		}
		fSent->addArticle(art);
		if(f_older==fSent) showHdrs();
	  actSendOutbox->setEnabled(!fOutbox->isEmpty());
	}
	delete job;
}



/*void KNSavedArticleManager::slotSearchDialogDone()
{
	sDlg->hide();
	slotDoSearch(0);
}



void KNSavedArticleManager::slotDoSearch(KNArticleFilter *f)
{
	f_ilter=f;
	showHdrs();
}*/



void KNSavedArticleManager::mailToClicked(KNArticleWidget *aw)
{
	QCString tmp;
	KNSavedArticle *art;
	art=newArticle();
  if(!art) return;
	tmp=aw->article()->replyToEmail();
	if(tmp.isEmpty()) tmp=aw->article()->fromEmail();
	art->setDestination(tmp.copy());
	openInComposer(art);
}



// try to close all composers, return false if user objects
bool KNSavedArticleManager::closeComposeWindows()
{
  KNComposer *comp;
  QList<KNComposer> comList2 = *comList;  // copy it, because the close() call will modifiy the original

  for ( comp=comList2.first(); comp != 0; comp=comList2.next() ) {
    if (!comp->close())
      return false;
  }

  return true;
}



void KNSavedArticleManager::updateStatusString()
{
	if(f_older) {
	  knGlobals.top->setStatusMsg(i18n(" %1 : %2 messages").arg(f_older->name()).arg(f_older->length()), SB_GROUP);
	  knGlobals.top->setCaption(f_older->name());
	}
}



bool KNSavedArticleManager::cancelAllowed(KNSavedArticle *a)
{
  if (!a)
    return false;
  if (a->isMail()) {
		KMessageBox::information(0, i18n("Emails cannot be canceled or superseded!"));
		return false;
  }
  if ((a->type()==KNArticleBase::ATcontrol) && (static_cast<KNControlArticle*>(a)->ctlType()==KNArticleBase::CTcancel)) {
		KMessageBox::information(0, i18n("Cancel messages cannot be canceled or superseded!"));
		return false;
  }
	if (!a->sent()) {
		KMessageBox::information(0, i18n("Only sent articles can be canceled or superseded!"));
		return false;
  }
	if (a->canceled()) {
		KMessageBox::information(0, i18n("This article has already been canceled or superseded!"));	
		return false;
  }		
	if(a->headerLine("Message-ID").isEmpty()) {
		KMessageBox::information(0, i18n("This article cannot be canceled or superseded,\nbecause it's message-id has not been created by KNode!\nBut you can look for your article in the newsgroup\nand cancel (or supersede) it there."));
		return false;
	}
	return true;
}



bool KNSavedArticleManager::cancelAllowed(KNFetchArticle *a, KNGroup *g)
{
  if (!a || !g)
    return false;
	KNUserEntry *user = defaultUser;
	if (g->user())
	  user = g->user();
  if (user->name()!=a->fromName()||user->email()!=a->fromEmail()) {
		KMessageBox::information(0, i18n("This article does not appear to be from you.\nYou can only cancel or supersede you own articles."));	
    return false;
  }
  if (!a->hasContent())  {
  	KMessageBox::information(0, i18n("You have to download the article body\nbefore you can cancel or supersede the article."));	
    return false;
  }
  return true;
}



// returns false if aborted by the user
bool KNSavedArticleManager::generateCancel(KNArticle *a, KNNntpAccount *acc)
{
	if(KMessageBox::No==KMessageBox::questionYesNo(0, i18n("Do you really want to cancel this article?")))
	  return false;
  bool sendNow = (KMessageBox::Yes==KMessageBox::questionYesNo(0, i18n("Do you want to send the cancel\nmessage now or later?"),QString::null,i18n("&Now"),i18n("&Later")));

	QCString mid;
	if (genMId) {
		if (MIdhost.isEmpty()) {
			KMessageBox::information(0, i18n("Please set a hostname for the generation\nof the message-id or disable it."));
	  	return false;
	  } else {
	  	mid="<"+KNArticleBase::uniqueString();
			mid+="@"+MIdhost+">";
		}
	}
	
	KNControlArticle *ca=new KNControlArticle(KNArticleBase::CTcancel, KNArticleBase::AStoPost);
  ca->setServerId(acc->id());
	acc->incUnsentCount();
	ca->initContent();
	if(genMId) ca->setHeader(KNArticleBase::HTmessageId, mid);
	
	QCString id=a->headerLine("Message-ID");
	ca->setSubject("cancel of "+id);
	ca->setHeader(KNArticleBase::HTfrom, a->headerLine("From"));
	ca->setDestination(a->headerLine("Newsgroups"));
	ca->setTimeT(time(0));
	ca->setHeader(KNArticleBase::HTcontrol,"cancel "+id);
	ca->addBodyLine("cancel by original author");
	
	ca->mimeInfo()->setCTMediaType(KNArticleBase::MTtext);
	ca->mimeInfo()->setCTSubType(KNArticleBase::STplain);
	
	ca->assemble();
	
	sendArticle(ca, sendNow);

  return true;
}

	

// returns false if aborted by the user
bool KNSavedArticleManager::generateSupersede(KNArticle *a, KNNntpAccount *acc)
{
	if(KMessageBox::No==KMessageBox::questionYesNo(0, i18n("Do you really want to supersede this article?")))
	  return false;

	QCString mid;
	if (genMId) {
		if (MIdhost.isEmpty()) {
			KMessageBox::information(0, i18n("Please set a hostname for the generation\nof the message-id or disable it."));
	  	return false;
	  } else {
	  	mid="<"+KNArticleBase::uniqueString();
			mid+="@"+MIdhost+">";
		}
	}
	
	KNControlArticle *ca=new KNControlArticle(KNArticleBase::CTsupersede, KNArticleBase::AStoPost);	
  ca->setServerId(acc->id());
	acc->incUnsentCount();
	
	ca->copyContent(a);
	ca->setHeader(KNArticleBase::HTsupersedes, a->headerLine("Message-ID"));
		
	if(genMId)
	  ca->setHeader(KNArticleBase::HTmessageId, mid);
	else
	  ca->removeHeader("Message-Id");
	
	ca->parse();
 	/*/x-headers
	QString dir(KGlobal::dirs()->saveLocation("appdata"));
	if (dir==QString::null)
		displayInternalFileError();
	else {
		KNFile f(dir+"xheaders");
		if(f.open(IO_ReadOnly)) {
			while(!f.atEnd())
				a->addHeaderLine(f.readLine(), true);		
			f.close();
		}		
	}*/

  /*ca->setSubject(a->headerLine("Subject"));
	ca->setDestination(a->headerLine("Newsgroups"));
	QCString ref = a->headerLine("References");
	if (!ref.isEmpty())
  	ca->references()->setLine(ref);	

	
  	
  KNMimeContent *body=a->mainContent();
	if(!body->mimeInfo()->isReadable()) body->prepareForDisplay();
	for(char *line=body->firstBodyLine(); line; line=body->nextBodyLine()) {
		if(!incSig && strncmp("-- ", line, 3)==0) break;
		ca->addBodyLine(line);
	}*/
		
	
	openInComposer(ca);
			
  return true;
}



// -----------------------------------------------------------------------------

#include "knsavedarticlemanager.moc"
