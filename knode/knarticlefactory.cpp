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

  KNLocalArticle *art=newArticle();
  if(!art)
    return;

  art->setServerId(a->id());
  art->setDoPost(true);
  art->setDoMail(false);

  KNComposer *c=new KNComposer(art, QString::null, knGlobals.cfgManager->identity()->getSignature(), true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createPosting(KNGroup *g)
{
  if(!g)
    return;

  KNLocalArticle *art=newArticle();

  if(!art)
    return;

  art->setServerId(g->account()->id());
  art->setDoPost(true);
  art->setDoMail(false);
  art->newsgroups()->from7BitString(g->groupname());

  if(g->useCharset())
    art->contentType()->setCharset(g->defaultCharset());

  KNConfig::Identity *i=g->identity();
  setIdentity(art, i);

  QString sig;
  if(i && i->hasSignature())
    sig=i->getSignature();
  else
    sig=knGlobals.cfgManager->identity()->getSignature();

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
  KNLocalArticle *art=newArticle();
  if(!art)
    return;

  KNNntpAccount *nntp=(static_cast<KNGroup*>(a->collection()))->account();
  art->setServerId(nntp->id());
  art->setDoPost(post);
  art->setDoMail(mail);

  //------------------------- <Headers> ----------------------------

  //subject
  QString subject=a->subject()->asUnicodeString();
  if(subject.left(3).upper()!="RE:")
    subject.prepend("Re: ");
  art->subject()->fromUnicodeString(subject, a->subject()->rfc2047Charset());

  //newsgroups
  KNHeaders::FollowUpTo *fup2=art->followUpTo(false);
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

  //Identity
  KNConfig::Identity *identity=0;
  QCString firstGrp=art->newsgroups()->firstGroup();
  KNGroup *g=knGlobals.grpManager->group(firstGrp, nntp);
  if(g) {
    identity=g->identity();
    if(g->useCharset())
      art->contentType()->setCharset(g->defaultCharset());
  }
  setIdentity(art, identity);


  //------------------------- </Headers> ---------------------------

  //--------------------------- <Body> -----------------------------

  QString quoted="";
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

      thisLine=(*line).stripWhiteSpace();

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
  QString sig;
  if(identity && identity->hasSignature())
    sig=identity->getSignature();
  else
    sig=knGlobals.cfgManager->identity()->getSignature();

  KNComposer *c=new KNComposer(art, quoted, sig, true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createForward(KNArticle *a)
{
  if(!a)
    return;

  //create new article
  KNLocalArticle *art=newArticle();
  if(!art)
    return;

  art->setDoPost(false);
  art->setDoMail(true);


  //------------------------- <Headers> ----------------------------

  //subject
  KNHeaders::Subject *subj=a->subject();
  QString subject=("Fwd: "+a->subject()->asUnicodeString());
  art->subject()->fromUnicodeString(subject, a->subject()->rfc2047Charset());

  //identity
  setIdentity(art, 0);

  //Mime
  KNHeaders::ContentType *type=art->contentType();
  type->setMimeType("text/plain");
  type->setCharset(knGlobals.cfgManager->postNewsTechnical()->charset());
  int e=knGlobals.cfgManager->postNewsTechnical()->encoding();
  art->contentTransferEncoding()->setCte((KNHeaders::contentEncoding)(e));

  //------------------------- </Headers> ---------------------------


  //--------------------------- <Body> -----------------------------

  QString fwd=QString("\n--------------- %1\n").arg(i18n("Forwarded message (begin)"));

  fwd+=( i18n("Subject")+": "+a->subject()->asUnicodeString()+"\n");
  fwd+=( i18n("From")+": "+a->from()->asUnicodeString()+"\n");
  fwd+=( i18n("Date")+": "+a->date()->asUnicodeString()+"\n\n");

  KNMimeContent *text=a->textContent();
  if(text) {
    QString decoded;
    text->decodedText(decoded);
    fwd+=decoded;
  }

  fwd+=QString("\n--------------- %1\n").arg(i18n("Forwarded message (end)"));

  //--------------------------- </Body> ----------------------------


  //open composer
  KNComposer *c=new KNComposer(art, fwd, knGlobals.cfgManager->identity()->getSignature(), true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();
}


void KNArticleFactory::createCancel(KNArticle *a)
{
}


void KNArticleFactory::createSupersede(KNArticle *a)
{
}


void KNArticleFactory::createMail(const QString &address)
{
  /*/create new article
  KNLocalArticle *art=newArticle();
  if(!art)
    return;

  art->setDoMail(true);
  art->setDoPost(false);
  art->to()->fromUnicodeString(address);

  //identity
  setIdentity(art, 0);

  //open composer
  KNComposer *c=new KNComposer(art, QString::null, knGlobals.cfgManager->identity()->getSignature(), true);
  c_ompList.append(c);
  connect(c, SIGNAL(composerDone(KNComposer*)), this, SLOT(slotComposerDone(KNComposer*)));
  c->show();*/
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
    if(!s_endErrDlg) {
      s_endErrDlg=new KNSendErrorDialog();
      connect(s_endErrDlg, SIGNAL(dialogDone()), this, SLOT(slotSendErrorDialogDone()));
      s_endErrDlg->show();
    }
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
    if(!s_endErrDlg) {
      s_endErrDlg=new KNSendErrorDialog();
      connect(s_endErrDlg, SIGNAL(dialogDone()), this, SLOT(slotSendErrorDialogDone()));
      s_endErrDlg->show();
    }
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


KNLocalArticle* KNArticleFactory::newArticle()
{
  KNConfig::PostNewsTechnical *pnt=knGlobals.cfgManager->postNewsTechnical();

  if(pnt->generateMessageID() && pnt->hostname().isEmpty()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Please set a hostname for the generation\nof the message-id or disable it."));
    return 0;
  }

  KNLocalArticle *art=new KNLocalArticle(0);

  //Message-id
  if(pnt->generateMessageID())
    art->messageID()->generate(pnt->hostname());

  //Date
  art->date()->setUnixTime(); //set current date+time

  //User-Agent
  if( !pnt->noUserAgent() ) {
    art->userAgent()->from7BitString("KNode/" KNODE_VERSION);
  }

  //Mime
  KNHeaders::ContentType *type=art->contentType();
  type->setMimeType("text/plain");
  type->setCharset(pnt->charset());
  art->contentTransferEncoding()->setCte((KNHeaders::contentEncoding)(pnt->encoding()));

  //X-Headers
  KNConfig::XHeaders::Iterator it;
  QFont::CharSet cs=KNMimeBase::stringToCharset(pnt->charset());
  for(it=pnt->xHeaders().begin(); it!=pnt->xHeaders().end(); ++it)
    art->setHeader( new KNHeaders::Generic( (QCString("X-")+(*it).name()), (*it).value(), cs ) );

  return art;
}


void KNArticleFactory::setIdentity(KNLocalArticle *a, KNConfig::Identity *i)
{
  if(!a)
    return;

  KNConfig::Identity *id;
  QFont::CharSet cs=KNMimeBase::stringToCharset(knGlobals.cfgManager->postNewsTechnical()->charset());

  //From
  KNHeaders::From *from=a->from();
  from->setRFC2047Charset(cs);

  //name
  if(i && i->hasName())
    id=i;
  else
    id=knGlobals.cfgManager->identity();
  if(id->hasName())
    from->setName(id->name().copy());

  //email
  if(i && i->hasEmail())
    id=i;
  else
    id=knGlobals.cfgManager->identity();
  if(id->hasEmail())
    from->setEmail(id->email().copy());


  //Reply-To
  if(i && i->hasReplyTo())
    id=i;
  else
    id=knGlobals.cfgManager->identity();
  if(id->hasReplyTo())
    a->replyTo()->fromUnicodeString(id->replyTo().copy(), cs);

  //Organization
  if(i && i->hasOrga())
    id=i;
  else
    id=knGlobals.cfgManager->identity();
  if(id->hasOrga())
    a->organization()->fromUnicodeString(id->orga().copy(), cs);

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


/*bool KNArticleFactory::cancelAllowed(KNLocalArticle *a)
{
  if (!a)
    return false;

  if(a->doMail() && !a->doPost()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Emails cannot be canceled or superseded!"));
    return false;
  }

  KNHeaders::Control *ctrl=a->control(false);
  if(ctrl && ctrl->isCancel()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Cancel messages cannot be canceled or superseded!"));
    return false;
  }

  if(!a->posted()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("Only sent articles can be canceled or superseded!"));
    return false;
  }

  if(a->canceled()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("This article has already been canceled or superseded!"));
    return false;
  }

  KNHeaders::MessageID *mid=a->messageID(false);

  if(!mid || mid->isEmpty()) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("This article cannot be canceled or superseded,\nbecause it's message-id has not been created by KNode!\nBut you can look for your article in the newsgroup\nand cancel (or supersede) it there."));
    return false;
  }

  return true;
}


bool KNArticleFactory::cancelAllowed(KNRemoteArticle *a, KNGroup *g)
{
  if (!a || !g)
    return false;

  KNConfig::Identity  *defId=knGlobals.cfgManager->identity(),
                      *gid=g->identity();
  bool ownArticle=true;

  if(gid->hasName())
    ownArticle=( gid->name()==a->from()->name() );
  else
    ownArticle=( defId->name()==a->from()->name() );

  if(ownArticle) {
    if(gid->hasEmail())
      ownArticle=( gid->email()==a->from()->email() );
    else
      ownArticle=( defId->email()==a->from()->email() );
  }

  if(!ownArticle) {
    KMessageBox::sorry(knGlobals.topWidget, i18n("This article does not appear to be from you.\nYou can only cancel or supersede you own articles."));
    return false;
  }

  if (!a->hasContent())  {
    KMessageBox::sorry(knGlobals.topWidget, i18n("You have to download the article body\nbefore you can cancel or supersede the article."));
    return false;
  }

  return true;
}*/


void KNArticleFactory::slotComposerDone(KNComposer *com)
{
  bool delCom=com->hasValidData();
  KNLocalArticle::List lst;
  lst.append(com->article());

  switch(com->result()) {

    case KNComposer::CRsendNow:
      if(delCom) {
        com->applyChanges();
        sendArticles(&lst, true);
      }
      else
        com->setDoneSuccess(false);
    break;

    case KNComposer::CRsendLater:
      if(delCom) {
        com->applyChanges();
        sendArticles(&lst, false);
      }
      else
        com->setDoneSuccess(false);
    break;

    case KNComposer::CRsave :
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
