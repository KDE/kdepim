/*
    knarticlefactory.cpp

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
#include <qframe.h>

#include <klocale.h>
#include <kapp.h>
#include <kmessagebox.h>
#include <kwin.h>
#include <kseparator.h>

#include "knarticlefactory.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "kngroupmanager.h"
#include "knaccountmanager.h"
#include "kngroup.h"
#include "knfoldermanager.h"
#include "knfolder.h"
#include "kncomposer.h"
#include "knnntpaccount.h"
#include "knlistbox.h"
#include "utilities.h"
#include "resource.h"


KNArticleFactory::KNArticleFactory(KNFolderManager *fm, KNGroupManager *gm, QObject *p, const char *n)
  : QObject(p, n), KNJobConsumer(), f_olManager(fm), g_rpManager(gm), s_endErrDlg(0)
{
  c_ompList.setAutoDelete(true);
}


KNArticleFactory::~KNArticleFactory()
{
  delete s_endErrDlg;
}


void KNArticleFactory::createPosting(KNNntpAccount *a)
{
  if(!a)
    return;

  QString sig;
  KNLocalArticle *art=newArticle(0, sig);
  if(!art)
    return;

  art->setServerId(a->id());
  art->setDoPost(true);
  art->setDoMail(false);

  KNComposer *c=new KNComposer(art, QString::null, sig, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createPosting(KNGroup *g)
{
  if(!g)
    return;

  QString sig;
  KNLocalArticle *art=newArticle(g, sig);

  if(!art)
    return;

  art->setServerId(g->account()->id());
  art->setDoPost(true);
  art->setDoMail(false);
  art->newsgroups()->from7BitString(g->groupname());

  KNComposer *c=new KNComposer(art, QString::null, sig, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createReply(KNRemoteArticle *a, bool post, bool mail)
{
  if(!a)
    return;

  //create new article
  KNGroup *g=static_cast<KNGroup*>(a->collection());
  QString sig;
  KNLocalArticle *art=newArticle(g, sig);
  if(!art)
    return;

  art->setServerId(g->account()->id());
  art->setDoPost(post);
  art->setDoMail(mail);

  //------------------------- <Headers> ----------------------------

  //subject
  QString subject=a->subject()->asUnicodeString();
  if(subject.left(3).upper()!="RE:")
    subject.prepend("Re: ");
  art->subject()->fromUnicodeString(subject, a->subject()->rfc2047Charset());

  //newsgroups
  KNHeaders::FollowUpTo *fup2=a->followUpTo(false);
  if(fup2 && !fup2->isEmpty()) {
    if(fup2->as7BitString(false).upper()=="POSTER") { //Followup-To: poster
      art->setDoPost(false);
      art->setDoMail(true);
    }
    else
      art->newsgroups()->from7BitString(fup2->as7BitString(false));
  }
  else
    art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

  //To
  KNHeaders::ReplyTo *replyTo=a->replyTo(false);
  KNHeaders::AddressField address;
  if(replyTo && !replyTo->isEmpty()) {
    if(replyTo->hasName())
      address.setName(replyTo->name().copy());
    if(replyTo->hasEmail())
      address.setEmail(replyTo->email().copy());
  }
  else {
    KNHeaders::From *from=a->from();
    if(from->hasName())
      address.setName(from->name().copy());
    if(from->hasEmail())
      address.setEmail(from->email().copy());
  }
  art->to()->addAddress(address);

  //References
  KNHeaders::References *references=a->references(false);
  QCString refs;
  if(references && !references->isEmpty())
    refs=references->as7BitString(false).copy()+" "+a->messageID()->as7BitString(false);
  else
    refs=a->messageID()->as7BitString(false).copy();
  art->references()->from7BitString(refs);

  //------------------------- </Headers> ---------------------------

  //--------------------------- <Body> -----------------------------

  // attribution line
  QString attribution=knGlobals.cfgManager->postNewsComposer()->intro();
  attribution.replace(QRegExp("%NAME"),a->from()->name());
  attribution.replace(QRegExp("%EMAIL"),QString::fromLatin1(a->from()->email()));
  attribution.replace(QRegExp("%DATE"),KGlobal::locale()->formatDateTime(a->date()->qdt(),false));
  attribution.replace(QRegExp("%MSID"),a->messageID()->asUnicodeString());
  attribution+="\n\n";

  QString quoted;
  QStringList text;
  QStringList::Iterator line;
  bool incSig=knGlobals.cfgManager->postNewsComposer()->includeSignature();

  a->decodedText(text);

  if(knGlobals.cfgManager->postNewsComposer()->rewrap()) {  //rewrap original article

    int wrapAt=knGlobals.cfgManager->postNewsComposer()->maxLineLength(), idx=0, breakPos=0;
    QString lastPrefix, thisPrefix, leftover, thisLine;
    char c;

    for(line=text.begin(); line!=text.end(); ++line) {

      if(!incSig && (*line)=="-- ")
        break;

      thisPrefix=QString::null;
      idx=0;
      for(QChar uc=(*line).at(idx); uc!=(char)(0); uc=(*line).at(++idx)) {
        c=uc.latin1();
        if( (c=='>') || (c=='|') || (c==' ') || (c==':') || (c=='#') || (c=='[') )
          thisPrefix.append(uc);
        else
          break;
      }

      thisLine=(*line);
      thisLine.remove(0,thisPrefix.length());
      thisLine = thisLine.stripWhiteSpace();

      if(!leftover.isEmpty()) {   // don't break paragraphs, tables and quote levels
        if(thisLine.isEmpty() || (thisPrefix!=lastPrefix) || thisLine.contains("  ") || thisLine.contains('\t'))
          appendTextWPrefix(quoted, leftover, "> "+lastPrefix);
        else
          thisLine.prepend(leftover+" ");
        leftover=QString::null;
      }

      if((int)(thisPrefix.length()+thisLine.length()) > wrapAt-2) {
        breakPos=findBreakPos(thisLine,wrapAt-thisPrefix.length()-2);
        if(breakPos < (int)(thisLine.length())) {
          leftover=thisLine.right(thisLine.length()-breakPos-1);
          thisLine.truncate(breakPos);
        }
      }

      quoted+=("> "+thisPrefix+thisLine+"\n");
      lastPrefix=thisPrefix;
    }

    if (!leftover.isEmpty())
      appendTextWPrefix(quoted, leftover, "> "+lastPrefix);

  }
  else {
    for(line=text.begin(); line!=text.end(); ++line) {
      if(!incSig && (*line)=="-- ")
        break;
      quoted+="> "+(*line)+"\n";
    }
  }

  //-------------------------- </Body> -----------------------------


  //open composer
  KNComposer *c=new KNComposer(art, attribution+quoted, sig, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createForward(KNArticle *a)
{
  if(!a)
    return;

  //create new article
  QString sig;
  KNLocalArticle *art=newArticle(0, sig);
  if(!art)
    return;

  art->setDoPost(false);
  art->setDoMail(true);


  //------------------------- <Headers> ----------------------------

  //subject
  QString subject=("Fwd: "+a->subject()->asUnicodeString());
  art->subject()->fromUnicodeString(subject, a->subject()->rfc2047Charset());

  //------------------------- </Headers> ---------------------------

  //--------------------------- <Body> -----------------------------

  QString fwd=QString("\n,-------------- %1\n\n").arg(i18n("Forwarded message (begin)"));

  fwd+=( i18n("Subject")+": "+a->subject()->asUnicodeString()+"\n");
  fwd+=( i18n("From")+": "+a->from()->asUnicodeString()+"\n");
  fwd+=( i18n("Date")+": "+a->date()->asUnicodeString()+"\n\n");

  KNMimeContent *text=a->textContent();
  if(text) {
    QString decoded;
    text->decodedText(decoded);
    fwd+=decoded;
  }

  fwd+=QString("\n`-------------- %1\n").arg(i18n("Forwarded message (end)"));

  //--------------------------- </Body> ----------------------------


  //open composer
  KNComposer *c=new KNComposer(art, fwd, sig, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createCancel(KNArticle *a)
{

  if(KMessageBox::No==KMessageBox::questionYesNo(knGlobals.topWidget,
    i18n("Do you really want to cancel this article?")))
    return;

  if(!cancelAllowed(a))
    return;


    bool sendNow;
    switch (KMessageBox::warningYesNoCancel(knGlobals.topWidget, i18n("Do you want to send the cancel\nmessage now or later?"), i18n("Question"),i18n("&Now"),i18n("&Later"))) {
      case KMessageBox::Yes : sendNow = true; break;
      case KMessageBox::No :  sendNow = false; break;
      default :               return;
    }

    KNGroup *grp;
    KNNntpAccount *nntp;

    if(a->type()==KNMimeBase::ATremote)
      nntp=(static_cast<KNGroup*>(a->collection()))->account();
    else {
      KNLocalArticle *la=static_cast<KNLocalArticle*>(a);
      la->setCanceled(true);
      la->updateListItem();
      nntp=knGlobals.accManager->account(la->serverId());
      if(!nntp)
        nntp=knGlobals.accManager->first();
      if(!nntp) {
        KMessageBox::error(knGlobals.topWidget, i18n("You have no valid news-account configured!"));
        return;
      }
    }

    grp=knGlobals.grpManager->group(a->newsgroups()->firstGroup(), nntp);

    QString sig;
    KNLocalArticle *art=newArticle(grp, sig, false);
    if(!art)
      return;

    //init
    art->contentType()->setCharset("US-ASCII");
    art->contentTransferEncoding()->setCte(KNHeaders::CE7Bit);
    art->setDoPost(true);
    art->setDoMail(false);

    //server
    art->setServerId(nntp->id());

    //subject
    KNHeaders::MessageID *msgId=a->messageID();
    QCString tmp;
    tmp="cancel of "+msgId->as7BitString(false);
    art->subject()->from7BitString(tmp);

    //newsgroups
    art->newsgroups()->from7BitString( a->newsgroups()->as7BitString(false) );

    //control
    tmp="cancel "+msgId->as7BitString(false);
    art->control()->from7BitString(tmp);

    //Lines
    art->lines()->setNumberOfLines(1);

    //body
    art->fromUnicodeString(QString::fromLatin1("cancel by original author\n"));

    //assemble
    art->assemble();

    //send
    KNLocalArticle::List lst;
    lst.append(art);
    sendArticles(&lst, sendNow);
}


void KNArticleFactory::createSupersede(KNArticle *a)
{
  if(KMessageBox::No==KMessageBox::questionYesNo(knGlobals.topWidget,
    i18n("Do you really want to supersede this article?")))
    return;

  KNGroup *grp;
  KNNntpAccount *nntp;

  if(a->type()==KNMimeBase::ATremote)
    nntp=(static_cast<KNGroup*>(a->collection()))->account();
  else {
    KNLocalArticle *la=static_cast<KNLocalArticle*>(a);
    la->setCanceled(true);
    la->updateListItem();
    nntp=knGlobals.accManager->account(la->serverId());
    if(!nntp)
      nntp=knGlobals.accManager->first();
    if(!nntp) {
      KMessageBox::error(knGlobals.topWidget, i18n("You have no valid news-account configured!"));
      return;
    }
  }

  grp=knGlobals.grpManager->group(a->newsgroups()->firstGroup(), nntp);

  //new article
  QString sig;
  KNLocalArticle *art=newArticle(grp, sig);
  if(!art)
    return;

  art->setDoPost(true);
  art->setDoMail(false);

  //server
  art->setServerId(nntp->id());

  //subject
  art->subject()->fromUnicodeString(a->subject()->asUnicodeString(), QFont::AnyCharSet);

  //to
  art->to()->from7BitString(a->to()->as7BitString(false));

  //newsgroups
  art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

  //References
  art->references()->from7BitString(a->references()->as7BitString(false));

  //Supersedes
  art->supersedes()->from7BitString(a->messageID()->as7BitString(false));

  //Body
  QString text;
  KNMimeContent *textContent=a->textContent();
  if(textContent)
    textContent->decodedText(text);


  //open composer
  KNComposer *c=new KNComposer(art, text, sig);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createMail(KNHeaders::AddressField *address)
{
  //create new article
  QString sig;
  KNLocalArticle *art=newArticle(0, sig);
  if(!art)
    return;

  art->setDoMail(true);
  art->setDoPost(false);
  art->to()->addAddress((*address));

  //open composer
  KNComposer *c=new KNComposer(art, QString::null, sig, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::edit(KNLocalArticle *a)
{
  if(!a)
    return;

  KNComposer *com=findComposer(a);
  if(com) {
    KWin::setActiveWindow(com->winId());
    return;
  }

  if(a->editDisabled()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("This article cannot be edited."));
    return;
  }

  //find signature
  KNConfig::Identity *id=knGlobals.cfgManager->identity();

  if(a->doPost()) {
    KNNntpAccount *acc=knGlobals.accManager->account(a->serverId());
    if(acc) {
      KNHeaders::Newsgroups *grps=a->newsgroups();
      KNGroup *grp=g_rpManager->group(grps->firstGroup(), acc);
      if(grp && grp->identity() && grp->identity()->hasSignature())
        id=grp->identity();
    }
  }

  //open composer
  com=new KNComposer(a, QString::null, id->getSignature(), false);
  c_ompList.append(com);
  connect(com, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  com->show();
}


void KNArticleFactory::saveArticles(KNLocalArticle::List *l, KNFolder *f)
{
  if(!f->saveArticles(l)) {
    displayInternalFileError();
    for(KNLocalArticle *a=l->first(); a; a=l->next())
      if(a->id()==-1)
        delete a; //ok, this is ugly - we simply delete orphant articles
  }
}


bool KNArticleFactory::deleteArticles(KNLocalArticle::List *l, bool ask)
{
  if(l->isEmpty())
    return true;

  if(ask) {
    QStringList lst;
    for(KNLocalArticle *a=l->first(); a; a=l->next()) {
      if(a->isLocked())
        continue;
      if(a->subject()->isEmpty())
        lst << i18n("no subject");
      else
        lst << a->subject()->asUnicodeString();
    }
    if( KMessageBox::No == KMessageBox::questionYesNoList(
      knGlobals.topWidget, i18n("Do you really want to delete these articles?"), lst) )
      return false;
  }

  KNFolder *f=static_cast<KNFolder*>(l->first()->collection());
  if(f)
    f->removeArticles(l, true);
  else {
    for(KNLocalArticle *a=l->first(); a; a=l->next())
      delete a;
  }
  return true;
}


void KNArticleFactory::sendArticles(KNLocalArticle::List *l, bool now)
{
  KNJobData *job=0;
  KNServerInfo *ser=0;

  KNLocalArticle::List unsent, sent;
  for(KNLocalArticle *a=l->first(); a; a=l->next()) {
    if(a->pending())
      unsent.append(a);
    else
      sent.append(a);
  }

  if(!sent.isEmpty()) {
    showSendErrorDialog();
    for(KNLocalArticle *a=sent.first(); a; a=sent.next())
      s_endErrDlg->append(a->subject()->asUnicodeString(), i18n("Article has been sent already"));
  }

  if(!now) {
    saveArticles(&unsent, f_olManager->outbox());
    return;
  }


  for(KNLocalArticle *a=unsent.first(); a; a=unsent.next()) {

    if(a->isLocked())
      continue;

    if(!a->hasContent()) {
      KNFolder *f=static_cast<KNFolder*>(a->collection());
      if(!f->loadArticle(a)) {
        showSendErrorDialog();
        s_endErrDlg->append(a->subject()->asUnicodeString(), i18n("Could not load article"));
        continue;
      }
    }

    if(a->doPost() && !a->posted()) {
      ser=knGlobals.accManager->account(a->serverId());
      job=new KNJobData(KNJobData::JTpostArticle, this, ser, a);
      emitJob(job);
    }
    else if(a->doMail() && !a->mailed()) {
      ser=knGlobals.accManager->smtp();
      job=new KNJobData(KNJobData::JTmail, this, ser, a);
      emitJob(job);
    }
  }
}


void KNArticleFactory::sendOutbox()
{
  KNLocalArticle::List lst;
  KNFolder *ob=f_olManager->outbox();

  if(!ob->loadHdrs()) {
    KMessageBox::error(knGlobals.topWidget, i18n("Could not load the outbox!"));
    return;
  }

  for(int i=0; i< ob->length(); i++)
    lst.append(ob->at(i));

  sendArticles(&lst, true);
}


bool KNArticleFactory::closeComposeWindows()
{
  KNComposer *comp;

  while((comp=c_ompList.first()))
    if(!comp->close())
      return false;

  return true;
}


void KNArticleFactory::deleteComposersForFolder(KNFolder *f)
{
  QList<KNComposer> list=c_ompList;

  for(KNComposer *i=list.first(); i; i=list.next())
    for(int x=0; x<f->count(); x++)
      if(i->article()==f->at(x)) {
        c_ompList.removeRef(i); //auto delete
        continue;
      }
}


void KNArticleFactory::deleteComposerForArticle(KNLocalArticle *a)
{
  KNComposer *com=findComposer(a);
  if(com)
    c_ompList.removeRef(com); //auto delete
}


KNComposer* KNArticleFactory::findComposer(KNLocalArticle *a)
{
  for(KNComposer *i=c_ompList.first(); i; i=c_ompList.next())
    if(i->article()==a)
      return i;
  return 0;
}


void KNArticleFactory::configChanged()
{
  for(KNComposer *c=c_ompList.first(); c; c=c_ompList.next())
    c->setConfig();
}


void KNArticleFactory::processJob(KNJobData *j)
{
  if(j->canceled()) {
    delete j;
    return;
  }

  KNLocalArticle *art=static_cast<KNLocalArticle*>(j->data());
  KNLocalArticle::List lst;
  lst.append(art);

  if(!j->success()) {
    //sending of this article failed => move it to the "Outbox-Folder"
    if(!f_olManager->outbox()->saveArticles(&lst)) {
      displayInternalFileError();
      if(art->collection()==0)
        delete art;
      return;
    }
    showSendErrorDialog();
    s_endErrDlg->append(art->subject()->asUnicodeString(), j->errorString());
    delete j; //unlock article
  }
  else {

    //disable edit
    art->setEditDisabled(true);

    switch(j->type()) {

      case KNJobData::JTpostArticle:
        delete j; //unlock article
        art->setPosted(true);
        if(art->doMail() && !art->mailed()) { //article has been posted, now mail it
          sendArticles(&lst, true);
          return;
        }
      break;

      case KNJobData::JTmail:
        delete j; //unlock article
        art->setMailed(true);
      break;

      default: break;
    };

    //article has been sent successfully => move it to the "Sent-folder"
    if(!f_olManager->sent()->saveArticles(&lst)) {
      displayInternalFileError();
      if(art->collection()==0)
        delete art;
    }
  }
}


KNLocalArticle* KNArticleFactory::newArticle(KNGroup *g, QString &sig, bool withXHeaders)
{
  KNConfig::PostNewsTechnical *pnt=knGlobals.cfgManager->postNewsTechnical();

  if(pnt->generateMessageID() && pnt->hostname().isEmpty()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Please set a hostname for the generation\nof the message-id or disable it."));
    return 0;
  }

  KNLocalArticle *art=new KNLocalArticle(0);
  KNConfig::Identity  *grpId=0,
                      *defId=0,
                      *id=0;
  QFont::CharSet cs=KNMimeBase::stringToCharset(pnt->charset());

  if(!g)
    grpId=0;
  else
    grpId=g->identity();

  defId=knGlobals.cfgManager->identity();

  //Message-id
  if(pnt->generateMessageID())
    art->messageID()->generate(pnt->hostname());

  //From
  KNHeaders::From *from=art->from();
  from->setRFC2047Charset(cs);

  //name
  if(grpId && grpId->hasName())
    id=grpId;
  else
    id=defId;
  if(id->hasName())
    from->setName(id->name().copy());

  //email
  if(grpId && grpId->hasEmail())
    id=grpId;
  else
    id=defId;
  if(id->hasEmail())
    from->setEmail(id->email().copy());

  //Reply-To
  if(grpId && grpId->hasReplyTo())
    id=grpId;
  else
    id=defId;
  if(id->hasReplyTo())
    art->replyTo()->fromUnicodeString(id->replyTo().copy(), cs);

  //Organization
  if(grpId && grpId->hasOrga())
    id=grpId;
  else
    id=defId;
  if(id->hasOrga())
    art->organization()->fromUnicodeString(id->orga().copy(), cs);

  //Date
  art->date()->setUnixTime(); //set current date+time

  //User-Agent
  if( !pnt->noUserAgent() ) {
    art->userAgent()->from7BitString("KNode/" KNODE_VERSION);
  }

  //Mime
  KNHeaders::ContentType *type=art->contentType();
  type->setMimeType("text/plain");
  if(g && g->useCharset())
    type->setCharset(g->defaultCharset());
  else
    type->setCharset(pnt->charset());
  art->contentTransferEncoding()->setCte((KNHeaders::contentEncoding)(pnt->encoding()));

  //X-Headers
  if(withXHeaders) {
    KNConfig::XHeaders::Iterator it;
    QFont::CharSet cs=KNMimeBase::stringToCharset(pnt->charset());
    for(it=pnt->xHeaders().begin(); it!=pnt->xHeaders().end(); ++it)
      art->setHeader( new KNHeaders::Generic( (QCString("X-")+(*it).name()), (*it).value(), cs ) );
  }

  //Signature
  if(grpId && grpId->hasSignature())
    id=grpId;
  else
    id=defId;
  if(id->hasSignature())
    sig=id->getSignature();
  else
    sig=QString::null;


  return art;
}


bool KNArticleFactory::cancelAllowed(KNArticle *a)
{
  if(!a)
    return false;

  if(a->type()==KNMimeBase::ATlocal) {
    KNLocalArticle *localArt=static_cast<KNLocalArticle*>(a);

    if(localArt->doMail() && !localArt->doPost()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("Emails cannot be canceled or superseded!"));
      return false;
    }

    KNHeaders::Control *ctrl=localArt->control(false);
    if(ctrl && ctrl->isCancel()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("Cancel messages cannot be canceled or superseded!"));
      return false;
    }

    if(!localArt->posted()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("Only sent articles can be canceled or superseded!"));
      return false;
    }

    if(localArt->canceled()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("This article has already been canceled or superseded!"));
      return false;
    }

    KNHeaders::MessageID *mid=localArt->messageID(false);
    if(!mid || mid->isEmpty()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("This article cannot be canceled or superseded,\nbecause it's message-id has not been created by KNode!\nBut you can look for your article in the newsgroup\nand cancel (or supersede) it there."));
      return false;
    }

    return true;
  }
  else if(a->type()==KNMimeBase::ATremote) {

    KNRemoteArticle *remArt=static_cast<KNRemoteArticle*>(a);
    KNGroup *g=static_cast<KNGroup*>(a->collection());
    KNConfig::Identity  *defId=knGlobals.cfgManager->identity(),
                        *gid=g->identity();
    bool ownArticle=true;

    if(gid && gid->hasName())
      ownArticle=( gid->name()==remArt->from()->name() );
    else
      ownArticle=( defId->name()==remArt->from()->name() );

    if(ownArticle) {
      if(gid && gid->hasEmail())
        ownArticle=( gid->email()==remArt->from()->email() );
      else
        ownArticle=( defId->email()==remArt->from()->email() );
    }

    if(!ownArticle) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("This article does not appear to be from you.\nYou can only cancel or supersede you own articles."));
      return false;
    }

    if(!remArt->hasContent())  {
      KMessageBox::sorry(knGlobals.topWidget, i18n("You have to download the article body\nbefore you can cancel or supersede the article."));
      return false;
    }

    return true;
  }

  return false;
}


int KNArticleFactory::findBreakPos(const QString &text, int start)
{
  int i;
  for(i=start;i>=0;i--)
    if(text[i].isSpace())
      break;
  if(i>0)
    return i;
  for(i=start;i<(int)text.length();i++)   // ok, the line is to long
    if(text[i].isSpace())
      break;
  return i;
}


void KNArticleFactory::appendTextWPrefix(QString &result, const QString &text, const QString &prefix)
{
  QString txt=text;
  int wrapAt=knGlobals.cfgManager->postNewsComposer()->maxLineLength(), breakPos;

  while(!txt.isEmpty()) {

    if((int)(prefix.length()+txt.length()) > wrapAt) {
      breakPos=findBreakPos(txt,wrapAt-prefix.length());
      result+=(prefix+txt.left(breakPos)+"\n");
      txt.remove(0,breakPos+1);
    }
    else {
      result+=(prefix+txt+"\n");
      txt=QString::null;
    }
  }
}


void KNArticleFactory::showSendErrorDialog()
{
  if(!s_endErrDlg) {
    s_endErrDlg=new KNSendErrorDialog();
    connect(s_endErrDlg, SIGNAL(dialogDone()), this, SLOT(slotSendErrorDialogDone()));
  }
  s_endErrDlg->show();
}


void KNArticleFactory::slotComposerDone(KNComposer *com)
{
  bool delCom=true;
  KNLocalArticle::List lst;
  lst.append(com->article());

  switch(com->result()) {

    case KNComposer::CRsendNow:
      delCom=com->hasValidData();
      if(delCom) {
        com->applyChanges();
        sendArticles(&lst, true);
      }
      else
        com->setDoneSuccess(false);
    break;

    case KNComposer::CRsendLater:
      delCom=com->hasValidData();
      if(delCom) {
        com->applyChanges();
        sendArticles(&lst, false);
      }
      else
        com->setDoneSuccess(false);
    break;

    case KNComposer::CRsave :
      delCom=com->hasValidData();
      if(delCom) {
        com->applyChanges();
        saveArticles(&lst, f_olManager->drafts());
      }
      else
        com->setDoneSuccess(false);
    break;

    case KNComposer::CRdelAsk:
      delCom=deleteArticles(&lst, true);
    break;

    case KNComposer::CRdel:
      delCom=deleteArticles(&lst, false);
    break;

    case KNComposer::CRcancel:
      // just close...
    break;

    default: break;

  };

  if(delCom)
    c_ompList.removeRef(com); //auto delete
  else
    KWin::setActiveWindow(com->winId());
}


void KNArticleFactory::slotSendErrorDialogDone()
{
  delete s_endErrDlg;
  s_endErrDlg=0;
}


//======================================================================================================


KNSendErrorDialog::KNSendErrorDialog() : QSemiModal(knGlobals.topWidget, 0, true)
{
  p_ixmap=knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::sendErr);

  QVBoxLayout *topL=new QVBoxLayout(this, 5,5);

  QLabel *l=new QLabel(QString("<b>%1</b>").arg(i18n("Failed tasks:")), this);
  topL->addWidget(l);

  j_obs=new QListBox(this);
  topL->addWidget(j_obs, 1);

  e_rror=new QLabel(this);
  topL->addSpacing(5);
  topL->addWidget(e_rror);

  KSeparator *sep=new KSeparator(this);
  topL->addSpacing(10);
  topL->addWidget(sep);

  c_loseBtn=new QPushButton(i18n("&Close"), this);
  c_loseBtn->setDefault(true);
  topL->addWidget(c_loseBtn, 0, Qt::AlignRight);

  connect(j_obs, SIGNAL(highlighted(int)), this, SLOT(slotHighlighted(int)));
  connect(c_loseBtn, SIGNAL(clicked()), this, SLOT(slotCloseBtnClicked()));

  setCaption(kapp->makeStdCaption(i18n("Errors while sending")));
  restoreWindowSize("sendDlg", this, sizeHint());
}


KNSendErrorDialog::~KNSendErrorDialog()
{
  saveWindowSize("sendDlg", size());
}


void KNSendErrorDialog::append(const QString &subject, const QString &error)
{

  LBoxItem *it=new LBoxItem(error, subject, &p_ixmap);
  j_obs->insertItem(it);
  j_obs->setCurrentItem(it);
}


void KNSendErrorDialog::slotHighlighted(int idx)
{
  LBoxItem *it=static_cast<LBoxItem*>(j_obs->item(idx));
  if(it) {
    QString tmp=i18n("<b>Error message:</b></br>")+it->error;
    e_rror->setText(tmp);
  }
}


void KNSendErrorDialog::slotCloseBtnClicked()
{
  emit dialogDone();
}


//-------------------------------
#include "knarticlefactory.moc"
