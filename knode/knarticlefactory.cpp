/*
    knarticlefactory.cpp

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

#include <klocale.h>
#include <kmessagebox.h>
#include <kwin.h>
#include <kseparator.h>
#include <kapplication.h>

#include "knarticlefactory.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "kngroupmanager.h"
#include "knaccountmanager.h"
#include "knfoldermanager.h"
#include "knarticlemanager.h"
#include "knfolder.h"
#include "kncomposer.h"
#include "knnntpaccount.h"
#include "utilities.h"
#include "resource.h"
#include <qlabel.h>
#include <qpushbutton.h>


KNArticleFactory::KNArticleFactory(QObject *p, const char *n)
  : QObject(p, n), s_endErrDlg(0)
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
  KNLocalArticle *art=newArticle(a, sig, knGlobals.cfgManager->postNewsTechnical()->charset());
  if(!art)
    return;

  art->setServerId(a->id());
  art->setDoPost(true);
  art->setDoMail(false);

  KNComposer *c=new KNComposer(art, QString::null, sig, QString::null, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createPosting(KNGroup *g)
{
  if(!g)
    return;

  QCString chset;
  if (g->useCharset())
    chset = g->defaultCharset();
  else
    chset = knGlobals.cfgManager->postNewsTechnical()->charset();

  QString sig;
  KNLocalArticle *art=newArticle(g, sig, chset);

  if(!art)
    return;

  art->setServerId(g->account()->id());
  art->setDoPost(true);
  art->setDoMail(false);
  art->newsgroups()->fromUnicodeString(g->groupname(), art->defaultCharset());

  KNComposer *c=new KNComposer(art, QString::null, sig, QString::null, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createReply(KNRemoteArticle *a, QString selectedText, bool post, bool mail)
{
  if(!a)
    return;

  KNGroup *g=static_cast<KNGroup*>(a->collection());

  QCString chset;
  if (knGlobals.cfgManager->postNewsTechnical()->useOwnCharset()) {
    if (g->useCharset())
      chset = g->defaultCharset();
    else
      chset = knGlobals.cfgManager->postNewsTechnical()->charset();
  } else
    chset = knGlobals.cfgManager->postNewsTechnical()->findComposerCharset(a->contentType()->charset());

  //create new article
  QString sig;
  KNLocalArticle *art=newArticle(g, sig, chset);
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
  KMime::Headers::FollowUpTo *fup2=a->followUpTo(false);
  if(fup2 && !fup2->isEmpty()) {
    if(fup2->as7BitString(false).upper()=="POSTER") { //Followup-To: poster
      if (post)         // warn the user
        KMessageBox::information(knGlobals.topWidget,i18n("The author has requested a reply by email instead\nof a followup to the newsgroup. (Followup-To: poster)"),
                                 QString::null,"followupToPosterWarning");
      art->setDoPost(false);
      art->setDoMail(true);
      art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));
    }
    else
      art->newsgroups()->from7BitString(fup2->as7BitString(false));
  }
  else
    art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

  //To
  KMime::Headers::ReplyTo *replyTo=a->replyTo(false);
  KMime::Headers::AddressField address;
  if(replyTo && !replyTo->isEmpty()) {
    if(replyTo->hasName())
      address.setName(replyTo->name());
    if(replyTo->hasEmail())
      address.setEmail(replyTo->email().copy());
  }
  else {
    KMime::Headers::From *from=a->from();
    if(from->hasName())
      address.setName(from->name());
    if(from->hasEmail())
      address.setEmail(from->email().copy());
  }
  art->to()->addAddress(address);

  //References
  KMime::Headers::References *references=a->references(false);
  QCString refs;
  if (references)
    refs=references->as7BitString(false);
  else
    refs = "";

  art->references()->from7BitString(refs);
  art->references()->append(a->messageID()->as7BitString(false));

  //Mail-Copies-To
  bool authorDislikesMailCopies=false;
  bool authorWantsMailCopies=false;
  KMime::Headers::MailCopiesTo *mailCopiesTo=a->mailCopiesTo(false);

  if(mailCopiesTo && !mailCopiesTo->isEmpty() && mailCopiesTo->isValid()) {
    authorDislikesMailCopies = mailCopiesTo->neverCopy();
    authorWantsMailCopies = mailCopiesTo->alwaysCopy();
    if (authorWantsMailCopies)         // warn the user
      KMessageBox::information(knGlobals.topWidget,i18n("The author requested a mail copy of your reply. (Mail-Copies-To header)"),
                               QString::null,"mailCopiesToWarning");
    if (authorWantsMailCopies && mailCopiesTo->hasEmail()) {
      address.setName(mailCopiesTo->name());
      address.setEmail(mailCopiesTo->email());
      art->to()->clear();
      art->to()->addAddress(address);
    }
  }

  //------------------------- </Headers> ---------------------------

  //--------------------------- <Body> -----------------------------

  // attribution line
  QString attribution=knGlobals.cfgManager->postNewsComposer()->intro();
  QString name(a->from()->name());
  if (name.isEmpty())
    name = QString::fromLatin1(a->from()->email());
  attribution.replace(QRegExp("%NAME"),name);
  attribution.replace(QRegExp("%EMAIL"),QString::fromLatin1(a->from()->email()));
  attribution.replace(QRegExp("%DATE"),KGlobal::locale()->formatDateTime(a->date()->qdt(),false));
  attribution.replace(QRegExp("%MSID"),a->messageID()->asUnicodeString());
  attribution.replace(QRegExp("%GROUP"),g->groupname());
  attribution+="\n\n";

  QString quoted=attribution;
  QString notRewraped=QString::null;
  QStringList text;
  QStringList::Iterator line;
  bool incSig=knGlobals.cfgManager->postNewsComposer()->includeSignature();

  if (selectedText.isEmpty()) {
    KMime::Content *tc = a->textContent();
    if(tc)
      tc->decodedText(text, true);
  }
  else
    text = QStringList::split('\n',selectedText,true);

  for(line=text.begin(); line!=text.end(); ++line) {
    if(!incSig && (*line)=="-- ")
      break;

    if ((*line)[0]=='>')
      quoted+=">"+(*line)+"\n";  // second quote level without space
    else
      quoted+="> "+(*line)+"\n";
  }

  if(knGlobals.cfgManager->postNewsComposer()->rewrap()) {  //rewrap original article

    notRewraped=quoted;     // store the original text in here, the user can request it in the composer
    quoted=attribution;

    quoted += KNHelper::rewrapStringList(text, knGlobals.cfgManager->postNewsComposer()->maxLineLength(), '>', !incSig, false);
  }

  //-------------------------- </Body> -----------------------------

  if (mail && knGlobals.cfgManager->postNewsTechnical()->useExternalMailer()) {
    sendMailExternal(address.asUnicodeString(), subject, quoted);
    mail = false;
    art->setDoMail(true);
    if (!post) {
      delete art;
      return;
    }
  }

  //open composer
  KNComposer *c=new KNComposer(art, quoted, sig, notRewraped, true, authorDislikesMailCopies, authorWantsMailCopies);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createForward(KNArticle *a)
{
  if(!a)
    return;

  KMime::Headers::ContentType *ct=a->contentType();
  QCString chset;
  bool incAtt = ( !knGlobals.cfgManager->postNewsTechnical()->useExternalMailer() &&
                  ct->isMultipart() && ct->isSubtype("mixed") &&
                  KMessageBox::Yes == KMessageBox::questionYesNo(knGlobals.topWidget,
                  i18n("This article contains attachments. Do you want them to be forwarded too?"))
                );

  if (knGlobals.cfgManager->postNewsTechnical()->useOwnCharset())
    chset = knGlobals.cfgManager->postNewsTechnical()->charset();
  else
    chset = knGlobals.cfgManager->postNewsTechnical()->findComposerCharset(a->contentType()->charset());

  //create new article
  QString sig;
  KNLocalArticle *art=newArticle(knGlobals.grpManager->currentGroup(), sig, chset);
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

  QString fwd=QString("\n,--------------- %1\n\n").arg(i18n("Forwarded message (begin)"));

  fwd+=( i18n(" Subject")+": "+a->subject()->asUnicodeString()+"\n");
  fwd+=( i18n(" From")+": "+a->from()->asUnicodeString()+"\n");
  fwd+=( i18n(" Date")+": "+a->date()->asUnicodeString()+"\n\n");

  KMime::Content *text=a->textContent();
  if(text) {
    QStringList decodedLines;
    text->decodedText(decodedLines, true);
    for(QStringList::Iterator it=decodedLines.begin(); it!=decodedLines.end(); ++it)
      fwd+=" "+(*it)+"\n";
  }

  fwd+=QString("\n`--------------- %1\n").arg(i18n("Forwarded message (end)"));

  //--------------------------- </Body> ----------------------------


  //------------------------ <Attachments> -------------------------

  if(incAtt) {
    KMime::Content::List al;

    a->attachments(&al, false);
    for(KMime::Content *c=al.first(); c; c=al.next()) {
      art->addContent( new KMime::Content(c->head(), c->body()) );
    }
  }

  //------------------------ </Attachments> ------------------------


  if (knGlobals.cfgManager->postNewsTechnical()->useExternalMailer()) {
    sendMailExternal(QString::null, subject, fwd);
    delete art;
    return;
  }

  //open composer
  KNComposer *c=new KNComposer(art, fwd, sig, QString::null, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createCancel(KNArticle *a)
{
  if(!cancelAllowed(a))
    return;

  if(KMessageBox::No==KMessageBox::questionYesNo(knGlobals.topWidget,
    i18n("Do you really want to cancel this article?")))
    return;

  bool sendNow;
  switch (KMessageBox::warningYesNoCancel(knGlobals.topWidget, i18n("Do you want to send the cancel\nmessage now or later?"), i18n("Question"),i18n("&Now"),i18n("&Later"))) {
    case KMessageBox::Yes : sendNow = true; break;
    case KMessageBox::No :  sendNow = false; break;
    default :               return;
  }

  KNGroup *grp;
  KNNntpAccount *nntp=0;

  if(a->type()==KMime::Base::ATremote)
    nntp=(static_cast<KNGroup*>(a->collection()))->account();
  else {
    if(!nntp)
      nntp=knGlobals.accManager->first();
    if(!nntp) {
      KMessageBox::error(knGlobals.topWidget, i18n("You have no valid news accounts configured!"));
      return;
    }
    KNLocalArticle *la=static_cast<KNLocalArticle*>(a);
    la->setCanceled(true);
    la->updateListItem();
    nntp=knGlobals.accManager->account(la->serverId());
  }

  grp=knGlobals.grpManager->group(a->newsgroups()->firstGroup(), nntp);

  QString sig;
  KNLocalArticle *art=newArticle(grp, sig, "us-ascii", false);
  if(!art)
    return;

  //init
  art->setDoPost(true);
  art->setDoMail(false);

  //server
  art->setServerId(nntp->id());

  //subject
  KMime::Headers::MessageID *msgId=a->messageID();
  QCString tmp;
  tmp="cancel of "+msgId->as7BitString(false);
  art->subject()->from7BitString(tmp);

  //newsgroups
  art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

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
  if (!a)
    return;

  if(!cancelAllowed(a))
    return;

  if(KMessageBox::No==KMessageBox::questionYesNo(knGlobals.topWidget,
    i18n("Do you really want to supersede this article?")))
    return;

  KNGroup *grp;
  KNNntpAccount *nntp;

  if(a->type()==KMime::Base::ATremote)
    nntp=(static_cast<KNGroup*>(a->collection()))->account();
  else {
    KNLocalArticle *la=static_cast<KNLocalArticle*>(a);
    la->setCanceled(true);
    la->updateListItem();
    nntp=knGlobals.accManager->account(la->serverId());
    if(!nntp)
      nntp=knGlobals.accManager->first();
    if(!nntp) {
      KMessageBox::error(knGlobals.topWidget, i18n("You have no valid news accounts configured!"));
      return;
    }
  }

  grp=knGlobals.grpManager->group(a->newsgroups()->firstGroup(), nntp);

  //new article
  QString sig;
  KNLocalArticle *art=newArticle(grp, sig, knGlobals.cfgManager->postNewsTechnical()->findComposerCharset(a->contentType()->charset()));
  if(!art)
    return;

  art->setDoPost(true);
  art->setDoMail(false);

  //server
  art->setServerId(nntp->id());

  //subject
  art->subject()->fromUnicodeString(a->subject()->asUnicodeString(), a->subject()->rfc2047Charset());

  //newsgroups
  art->newsgroups()->from7BitString(a->newsgroups()->as7BitString(false));

  //followup-to
  art->followUpTo()->from7BitString(a->followUpTo()->as7BitString(false));

  //References
  art->references()->from7BitString(a->references()->as7BitString(false));

  //Supersedes
  art->supersedes()->from7BitString(a->messageID()->as7BitString(false));

  //Body
  QString text;
  KMime::Content *textContent=a->textContent();
  if(textContent)
    textContent->decodedText(text);

  //open composer
  KNComposer *c=new KNComposer(art, text, sig);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createMail(KMime::Headers::AddressField *address)
{
  if (knGlobals.cfgManager->postNewsTechnical()->useExternalMailer()) {
    sendMailExternal(address->asUnicodeString());
    return;
  }

  //create new article
  QString sig;
  KNLocalArticle *art=newArticle(knGlobals.grpManager->currentGroup(), sig, knGlobals.cfgManager->postNewsTechnical()->charset());
  if(!art)
    return;

  art->setDoMail(true);
  art->setDoPost(false);
  art->to()->addAddress((*address));

  //open composer
  KNComposer *c=new KNComposer(art, QString::null, sig, QString::null, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::sendMailExternal(const QString &address, const QString &subject, const QString &body)
{
  KURL mailtoURL;
  QStringList queries;
  QString query=QString::null;
  mailtoURL.setProtocol("mailto");

  if (!address.isEmpty())
    mailtoURL.setPath(address);
  if (!subject.isEmpty())
    queries.append("subject="+KURL::encode_string(subject));
  if (!body.isEmpty())
    queries.append("body="+KURL::encode_string(body));

  if (queries.count() > 0) {
    query = "?";
    for ( QStringList::Iterator it = queries.begin(); it != queries.end(); ++it ) {
      if (it != queries.begin())
        query.append("&");
      query.append((*it));
    }
  }

  if (!query.isEmpty())
    mailtoURL.setQuery(query);

  kapp->invokeMailer(mailtoURL);
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
      KMime::Headers::Newsgroups *grps=a->newsgroups();
      KNGroup *grp=knGlobals.grpManager->group(grps->firstGroup(), acc);
      if (grp && grp->identity() && grp->identity()->hasSignature())
        id=grp->identity();
      else if (acc->identity() && acc->identity()->hasSignature())
        id=acc->identity();
    }
  }

  //load article body
  if(!a->hasContent())
    knGlobals.artManager->loadArticle(a);

  //open composer
  com=new KNComposer(a, QString::null, id->getSignature());
  c_ompList.append(com);
  connect(com, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  com->show();
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
      s_endErrDlg->append(a->subject()->asUnicodeString(), i18n("Article has already been sent."));
  }

  if(!now) {
    knGlobals.artManager->moveIntoFolder(unsent, knGlobals.folManager->outbox());
    return;
  }


  for(KNLocalArticle *a=unsent.first(); a; a=unsent.next()) {

    if(a->isLocked())
      continue;

    if(!a->hasContent()) {
      if(!knGlobals.artManager->loadArticle(a)) {
        showSendErrorDialog();
        s_endErrDlg->append(a->subject()->asUnicodeString(), i18n("Unable to load article!"));
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
  KNFolder *ob=0;

  if(!knGlobals.folManager->loadOutbox()) {
    KMessageBox::error(knGlobals.topWidget, i18n("Unable to load the outbox-folder!"));
    return;
  }

  ob=knGlobals.folManager->outbox();
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
  QPtrList<KNComposer> list=c_ompList;

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
    c->setConfig(false);
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
    showSendErrorDialog();
    s_endErrDlg->append(art->subject()->asUnicodeString(), j->errorString());
    delete j; //unlock article

    //sending of this article failed => move it to the "Outbox-Folder"
    if(art->collection()!=knGlobals.folManager->outbox())
      knGlobals.artManager->moveIntoFolder(lst, knGlobals.folManager->outbox());
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
    knGlobals.artManager->moveIntoFolder(lst, knGlobals.folManager->sent());
  }
}


KNLocalArticle* KNArticleFactory::newArticle(KNCollection *col, QString &sig, QCString defChset, bool withXHeaders)
{
  KNConfig::PostNewsTechnical *pnt=knGlobals.cfgManager->postNewsTechnical();

  if(pnt->generateMessageID() && pnt->hostname().isEmpty()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Please set a hostname for the generation\nof the message-id or disable it."));
    return 0;
  }

  KNLocalArticle *art=new KNLocalArticle(0);
  KNConfig::Identity  *grpId=0,
                      *defId=0,
                      *accId=0,
                      *id=0;

  if (col) {
    if (col->type() == KNCollection::CTgroup) {
      grpId = (static_cast<KNGroup *>(col))->identity();
      accId = (static_cast<KNGroup *>(col))->account()->identity();
    } else
      if (col->type() == KNCollection::CTnntpAccount) {
        accId = (static_cast<KNNntpAccount *>(col))->identity();
      }
  }

  defId=knGlobals.cfgManager->identity();

  //Message-id
  if(pnt->generateMessageID())
    art->messageID()->generate(pnt->hostname());

  //From
  KMime::Headers::From *from=art->from();
  from->setRFC2047Charset(pnt->charset());

  //name
  if(grpId && grpId->hasName())
    id=grpId;
  else
    id=((accId) && accId->hasName())? accId:defId;
  if(id->hasName())
    from->setName(id->name());

  //email
  if(grpId && grpId->hasEmail())
    id=grpId;
  else
    id=((accId) && accId->hasEmail())? accId:defId;
  if(id->hasEmail()&&id->emailIsValid())
    from->setEmail(id->email().latin1());
  else {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Please enter a valid email address."));
    delete art;
    return 0;
  }

  //Reply-To
  if(grpId && grpId->hasReplyTo())
    id=grpId;
  else
    id=((accId) && accId->hasReplyTo())? accId:defId;
  if(id->hasReplyTo()) {
    art->replyTo()->fromUnicodeString(id->replyTo(), pnt->charset());
    if (!art->replyTo()->hasEmail())   // the header is invalid => drop it
      art->removeHeader("Reply-To");
  }

  //Mail-Copies-To
  if(grpId && grpId->hasMailCopiesTo())
    id=grpId;
  else
    id=((accId) && accId->hasMailCopiesTo())? accId:defId;
  if(id->hasMailCopiesTo()) {
    art->mailCopiesTo()->fromUnicodeString(id->mailCopiesTo(), pnt->charset());
    if (!art->mailCopiesTo()->isValid())   // the header is invalid => drop it
      art->removeHeader("Mail-Copies-To");
  }

  //Organization
  if(grpId && grpId->hasOrga())
    id=grpId;
  else
    id=((accId) && accId->hasOrga())? accId:defId;
  if(id->hasOrga())
    art->organization()->fromUnicodeString(id->orga(), pnt->charset());

  //Date
  art->date()->setUnixTime(); //set current date+time

  //User-Agent
  if( !pnt->noUserAgent() ) {
    art->userAgent()->from7BitString("KNode/" KNODE_VERSION);
  }

  //Mime
  KMime::Headers::ContentType *type=art->contentType();
  type->setMimeType("text/plain");

  type->setCharset(defChset);

  if (defChset.lower()=="us-ascii")
    art->contentTransferEncoding()->setCte(KMime::Headers::CE7Bit);
  else
    art->contentTransferEncoding()->setCte(pnt->allow8BitBody()? KMime::Headers::CE8Bit : KMime::Headers::CEquPr);

  //X-Headers
  if(withXHeaders) {
    KNConfig::XHeaders::Iterator it;
    for(it=pnt->xHeaders().begin(); it!=pnt->xHeaders().end(); ++it)
      art->setHeader( new KMime::Headers::Generic( (QCString("X-")+(*it).name()), art, (*it).value(), pnt->charset() ) );
  }

  //Signature
  if(grpId && grpId->hasSignature())
    id=grpId;
  else
    id=((accId) && accId->hasSignature())? accId:defId;
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

  if(a->type()==KMime::Base::ATlocal) {
    KNLocalArticle *localArt=static_cast<KNLocalArticle*>(a);

    if(localArt->doMail() && !localArt->doPost()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n("Emails cannot be canceled or superseded!"));
      return false;
    }

    KMime::Headers::Control *ctrl=localArt->control(false);
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

    KMime::Headers::MessageID *mid=localArt->messageID(false);
    if(!mid || mid->isEmpty()) {
      KMessageBox::sorry(knGlobals.topWidget, i18n(
"This article cannot be canceled or superseded,\n\
because its message-id has not been created by KNode!\n\
But you can look for your article in the newsgroup\n\
and cancel (or supersede) it there."));
      return false;
    }

    return true;
  }
  else if(a->type()==KMime::Base::ATremote) {

    KNRemoteArticle *remArt=static_cast<KNRemoteArticle*>(a);
    KNGroup *g=static_cast<KNGroup*>(a->collection());
    KNConfig::Identity  *defId=knGlobals.cfgManager->identity(),
                        *gid=g->identity(),
                        *accId=g->account()->identity();
    bool ownArticle=true;

    if(gid && gid->hasName())
      ownArticle=( gid->name()==remArt->from()->name() );
    else
      if (accId && accId->hasName())
        ownArticle=( accId->name()==remArt->from()->name() );
      else
        ownArticle=( defId->name()==remArt->from()->name() );

    if(ownArticle) {
      if(gid && gid->hasEmail())
        ownArticle=( gid->email().latin1()==remArt->from()->email() );
      else
        if (accId && accId->hasEmail())
          ownArticle=( accId->email().latin1()==remArt->from()->email() );
        else
          ownArticle=( defId->email().latin1()==remArt->from()->email() );
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
        if ( com->applyChanges() )
          sendArticles(&lst, true);
      }
      else
        com->setDoneSuccess(false);
    break;

    case KNComposer::CRsendLater:
      delCom=com->hasValidData();
      if(delCom) {
        if ( com->applyChanges() )
          sendArticles(&lst, false);
      }
      else
        com->setDoneSuccess(false);
    break;

    case KNComposer::CRsave :
      if ( com->applyChanges() )
        knGlobals.artManager->moveIntoFolder(lst, knGlobals.folManager->drafts());
    break;

    case KNComposer::CRdelAsk:
      delCom=knGlobals.artManager->deleteArticles(lst, true);
    break;

    case KNComposer::CRdel:
      delCom=knGlobals.artManager->deleteArticles(lst, false);
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


KNSendErrorDialog::KNSendErrorDialog() : QDialog(knGlobals.topWidget, 0, true)
{
  p_ixmap=knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::sendErr);

  QVBoxLayout *topL=new QVBoxLayout(this, 5,5);

  QLabel *l=new QLabel(QString("<b>%1</b>").arg(i18n("Failed tasks:")), this);
  topL->addWidget(l);

  j_obs=new KNDialogListBox(true,this);
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

  setCaption(kapp->makeStdCaption(i18n("Errors While Sending")));
  KNHelper::restoreWindowSize("sendDlg", this, sizeHint());
}


KNSendErrorDialog::~KNSendErrorDialog()
{
  KNHelper::saveWindowSize("sendDlg", size());
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


void KNSendErrorDialog::keyPressEvent(QKeyEvent *e)
{
  if ((e->key()==Key_Enter)||(e->key()==Key_Return)||(e->key()==Key_Escape))
    emit dialogDone();
  else
    QDialog::keyPressEvent(e);
}


void KNSendErrorDialog::closeEvent(QCloseEvent *e)
{
  e->accept();
  emit dialogDone();
}

//-------------------------------
#include "knarticlefactory.moc"
