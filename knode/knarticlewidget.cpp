/*
    knarticlewidget.cpp

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qstring.h>
#include <qclipboard.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qstylesheet.h>

#include <kpopupmenu.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kprocess.h>
#include <kcharsets.h>

#include "resource.h"
#include "knmime.h"
#include "knarticlewidget.h"
#include "knglobals.h"
#include "knarticlemanager.h"
#include "knfoldermanager.h"
#include "knarticlefactory.h"
#include "knconfigmanager.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knnntpaccount.h"
#include "knstringsplitter.h"
#include "utilities.h"

#define PUP_OPEN    1000
#define PUP_SAVE    2000
#define PUP_COPYURL 3000
#define PUP_SELALL  4000
#define PUP_COPY    5000

#define HDR_COL   0
#define QCOL_1    1
#define QCOL_2    2
#define QCOL_3    3


KNArticleWidget::KNArticleWidget(KActionCollection* actColl, QWidget *parent, const char *name )
    : QTextBrowser(parent, name), a_rticle(0), a_tt(0), h_tmlDone(false), a_ctions(actColl)
{
  i_nstances.append(this);

  //custom tags <bodyblock> , <headerblock>
  QStyleSheetItem *style;
  style=new QStyleSheetItem(styleSheet(), "bodyblock");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  style->setMargin(QStyleSheetItem::MarginAll, 5);
  style=new QStyleSheetItem(styleSheet(), "headerblock");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  style->setMargin(QStyleSheetItem::MarginLeft, 10);
  style->setMargin(QStyleSheetItem::MarginVertical, 2);

  setFocusPolicy(QWidget::WheelFocus);

  //popups
  u_rlPopup=new KPopupMenu();
  u_rlPopup->insertItem(i18n("Open URL"), PUP_OPEN);
  u_rlPopup->insertItem(i18n("Copy to clipboard"), PUP_COPYURL);
  a_ttPopup=new KPopupMenu();
  a_ttPopup->insertItem(i18n("Open"), PUP_OPEN);
  a_ttPopup->insertItem(i18n("Save"), PUP_SAVE);

  //actions
  a_ctSave              = KStdAction::save(this, SLOT(slotSave()), a_ctions);
  a_ctSave->setText("&Save...");
  a_ctPrint             = KStdAction::print(this, SLOT(slotPrint()), a_ctions);
  a_ctSelAll            = KStdAction::selectAll(this, SLOT(slotSelectAll()), a_ctions);
  a_ctCopy              = KStdAction::copy(this, SLOT(copy()), a_ctions);


  a_ctReply             = new KAction(i18n("&Followup to Newsgroup..."),"message_reply", Key_R , this,
                          SLOT(slotReply()), a_ctions, "article_postReply");
  a_ctRemail            = new KAction(i18n("Reply by E-&Mail..."),"mail_reply", Key_A , this,
                          SLOT(slotRemail()), a_ctions, "article_mailReply");
  a_ctForward           = new KAction(i18n("Forw&ard by E-Mail..."),"mail_forward", Key_F , this,
                          SLOT(slotForward()), a_ctions, "article_forward");
  a_ctCancel            = new KAction(i18n("article","&Cancel"), 0 , this,
                          SLOT(slotCancel()), a_ctions, "article_cancel");
  a_ctSupersede         = new KAction(i18n("S&upersede..."), 0 , this,
                          SLOT(slotSupersede()), a_ctions, "article_supersede");
  a_ctToggleFullHdrs    = new KToggleAction(i18n("Show &all headers"), "text_block", 0 , this,
                          SLOT(slotToggleFullHdrs()), a_ctions, "view_showAllHdrs");
  a_ctToggleRot13       = new KToggleAction(i18n("&Unscramble (Rot 13)"), "decrypted", 0 , this,
                          SLOT(slotToggleRot13()), a_ctions, "view_rot13");

  //timer
  t_imer=new QTimer(this);
  connect(t_imer, SIGNAL(timeout()), this, SLOT(slotTimeout()));

  //config
  f_ullHdrs=false;
  a_ctToggleFullHdrs->setChecked(f_ullHdrs);
  r_ot13=false;
  a_ctToggleRot13->setChecked(false);
  applyConfig();
}



KNArticleWidget::~KNArticleWidget()
{
  i_nstances.removeRef(this);
  delete a_tt;
  delete a_ttPopup;
  delete u_rlPopup;
}


bool KNArticleWidget::scrollingDownPossible()
{
  return ((contentsY()+visibleHeight())<contentsHeight());
}



void KNArticleWidget::scrollDown()
{
  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;
  scrollBy( 0, visibleHeight()-offs);
}


void KNArticleWidget::focusInEvent(QFocusEvent *e)
{
  emit focusChanged(e);
  QTextBrowser::focusInEvent(e);

}



void KNArticleWidget::focusOutEvent(QFocusEvent *e)
{
  emit focusChanged(e);
  QTextBrowser::focusOutEvent(e);
}



void KNArticleWidget::keyPressEvent(QKeyEvent *e)
{
  if ( !e ) return;

  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;
    
  switch(e->key()) {
    case Key_Prior:
      scrollBy( 0, -visibleHeight()+offs);
      break;
    case Key_Next:
      scrollBy( 0, visibleHeight()-offs);
      break;
    default:
      QTextBrowser::keyPressEvent(e);
  }
}



void KNArticleWidget::viewportMousePressEvent(QMouseEvent *e)
{
  QString a=QTextBrowser::anchorAt(e->pos());
  if(!a.isEmpty() && (e->button()==RightButton || e->button()==MidButton))
    anchorClicked(a, e->button(), &(e->globalPos()));
  else
    if (e->button()==RightButton)
      b_odyPopup->popup(e->globalPos());

  QTextBrowser::viewportMousePressEvent(e);
}



void KNArticleWidget::viewportMouseReleaseEvent(QMouseEvent *e)
{
  QTextBrowser::viewportMouseReleaseEvent(e);

  if (e->button()==LeftButton) {
    if(hasSelectedText() && !selectedText().isEmpty()) {
      copy();
      a_ctCopy->setEnabled(true);
    } else
      a_ctCopy->setEnabled(false);
  }
}



void KNArticleWidget::applyConfig()
{
  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();

  QColorGroup pcg(paperColorGroup());
  pcg.setColor(QColorGroup::Base, app->backgroundColor());
  pcg.setColor(QColorGroup::Text, app->textColor());
  setPaperColorGroup(pcg);
  setLinkColor(app->linkColor());

  if(!knGlobals.cfgManager->readNewsGeneral()->autoMark())
    t_imer->stop();

  updateContents();
}



QString KNArticleWidget::toHtmlString(const QString &line, bool parseURLs, bool beautification, bool allowRot13)
{
  QString text,result;
  QRegExp regExp;
  uint len=line.length();
  int matchLen;
  bool forceNBSP=false; //use "&nbsp;" for spaces => workaround for a bug in QTextBrowser

  if (allowRot13 && r_ot13)
    text = rot13(line);
  else
    text = line;

  if (!knGlobals.cfgManager->readNewsGeneral()->interpretFormatTags())
    beautification=false;

  for(uint idx=0; idx<len; idx++){
    
    switch(text[idx].latin1()) {
      
      case '\r':  break;
      case '\n':  result+="<br>"; break;  
      case '<' :  result+="&lt;"; break;
      case '>' :  result+="&gt;"; break;
      case '&' :  result+="&amp;"; break;
      case '"' :  result+="&quot;"; break;
      case '\t':  result+="&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"; break;   // tab == 8 spaces
      
      case 32 :
        if(text[idx+1].isSpace())  {
          while(text[idx].isSpace()) {
            result+="&nbsp;";
            idx++;

          }
          idx--;
          forceNBSP=true; // force &nbsp; for the rest of this line
        } else
          if(idx==0 || forceNBSP) {
            result+="&nbsp;";
            forceNBSP=true; // force &nbsp; for the rest of this line
          }
          else result+=' ';
        break;
      
      case 'h' :  
        if((parseURLs)&&
           (text[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'h'
          regExp="^https?://[^\\s<>()\"|]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            break;
          }
        }
        result+=text[idx];
        break;  
      
      case 'w' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='w')) {   // don't do all the stuff for every 'w'
          regExp="^www\\.[^\\s<>()\"|]+\\.[^\\s<>()\"|]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            result+=QString::fromLatin1("<a href=\"http://") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            break;
          }
        }
        result+=text[idx];
        break;  
      
      case 'f' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'f'
          regExp="^ftp://[^\\s<>()\"|]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            break;
          }
          regExp="^ftp\\.[^\\s<>()\"|]+\\.[^\\s<>()\"|]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            result+=QString::fromLatin1("<a href=\"ftp://") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            break;
          }
        }
        result+=text[idx];
        break;        

      case '_' :
      case '/' :
      case '*' :
        if(beautification) {
          regExp=QString("^\\%1[^\\s%2]+\\%3").arg(text[idx]).arg(text[idx]).arg(text[idx]);
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (( matchLen > 3 ) &&
                ((idx==0)||text[idx-1].isSpace()||(text[idx-1] == '(')) &&
                ((idx+matchLen==len)||text[idx+matchLen].isSpace()||(text[idx+matchLen]==',')||
                 (text[idx+matchLen]=='.')||(text[idx+matchLen]==')'))) {
              switch (text[idx].latin1()) {
                case '_' :
                  result+=QString("<u>%1</u>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification));
                  break;
                case '/' :
                  result+=QString("<i>%1</i>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification));
                  break;
                case '*' :
                  result+=QString("<b>%1</b>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification));
                  break;                  
              }
              idx+=matchLen-1;
              break;
            }
          }
        }
        result+=text[idx];
        break;    
            
      default  : result+=text[idx];
    }
  }           
  return result;
}



void KNArticleWidget::openURL(const QString &url)
{
  if(url.isEmpty()) return;

  if (knGlobals.cfgManager->readNewsGeneral()->browser()==KNConfig::ReadNewsGeneral::BTkonq)
    kapp->invokeBrowser(url);
  else if (knGlobals.cfgManager->readNewsGeneral()->browser()==KNConfig::ReadNewsGeneral::BTnetscape){
    KProcess proc;
    proc << "netscape";
  
    struct stat info;      // QFileInfo is unable to handle netscape's broken symlink
    if (lstat((QDir::homeDirPath()+"/.netscape/lock").local8Bit(),&info)==0)
      proc << "-remote" << QString("openURL(%1)").arg(url);
    else
      proc << url;  

    proc.start(KProcess::DontCare);
  }
  else if (knGlobals.cfgManager->readNewsGeneral()->browser()==KNConfig::ReadNewsGeneral::BTmozilla){
    KProcess proc;
    proc << "run-mozilla.sh";
    proc << "mozilla-bin";
    proc << url;
    proc.start(KProcess::DontCare);
  }
  else if (knGlobals.cfgManager->readNewsGeneral()->browser()==KNConfig::ReadNewsGeneral::BTopera){
    KProcess proc;
    proc << "opera";
    proc << QString("-page=%1").arg(url);
    proc << url;
    proc.start(KProcess::DontCare);
  } else {
    KProcess proc;

    QStringList command = QStringList::split(' ',knGlobals.cfgManager->readNewsGeneral()->browserCommand());
    bool urlAdded=false;
    for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it ) {
      if ((*it).contains("%u")) {
        (*it).replace(QRegExp("%u"),url);
        urlAdded=true;
      }
      proc << (*it);
    }
    if(!urlAdded)    // no %u in the browser command
      proc << url;

    proc.start(KProcess::DontCare);
  }
}



void KNArticleWidget::saveAttachment(int id)
{
  KNMimeContent *a=a_tt->at(id);

  if(a)
    knGlobals.artManager->saveContentToFile(a);
  else KMessageBox::error(this, i18n("Internal error: Malformed identifier!"));
}




void KNArticleWidget::openAttachment(int id)
{
 KNMimeContent *a=a_tt->at(id);

 if(a)
   knGlobals.artManager->openContent(a);
 else KMessageBox::error(this, i18n("Internal error: Malformed identifier!"));
}



bool KNArticleWidget::inlinePossible(KNMimeContent *c)
{
 	KNHeaders::ContentType *ct=c->contentType();
  return ( ct->isText() || ct->isImage() );
}



void KNArticleWidget::showBlankPage()
{
  kdDebug(5003) << "KNArticleWidget::showBlankPage()" << endl;
  setText(QString::null);

  a_rticle=0;
  delete a_tt;
  a_tt=0;
  h_tmlDone=false;
  a_ctSave->setEnabled(false);
  a_ctPrint->setEnabled(false);
  a_ctCopy->setEnabled(false); //probaly not neede, but who knows ;-)
  a_ctSelAll->setEnabled(false);
  a_ctReply->setEnabled(false);
  a_ctRemail->setEnabled(false);
  a_ctForward->setEnabled(false);
  a_ctCancel->setEnabled(false);
  a_ctSupersede->setEnabled(false);
  a_ctToggleFullHdrs->setEnabled(false);
  a_ctToggleRot13->setEnabled(false);
}



void KNArticleWidget::showErrorMessage(const QString &s)
{

  setFont(knGlobals.cfgManager->appearance()->articleFont());  // switch back from possible obscure charsets

  QString msg="<qt>"+i18n("<b><font size=+1 color=red>An error occured!</font></b><hr><br>");
  msg+=toHtmlString(s)+"</qt>";
  setText(msg);

  a_rticle=0;
  delete a_tt;
  a_tt=0;
  h_tmlDone=false;
  a_ctSave->setEnabled(false);
  a_ctPrint->setEnabled(false);
  a_ctSelAll->setEnabled(true);
  a_ctReply->setEnabled(false);
  a_ctRemail->setEnabled(false);
  a_ctForward->setEnabled(false);
  a_ctCancel->setEnabled(false);
  a_ctSupersede->setEnabled(false);
  a_ctToggleFullHdrs->setEnabled(false);
  a_ctToggleRot13->setEnabled(false);
}



void KNArticleWidget::updateContents()
{
  if(a_rticle && a_rticle->hasContent())
    createHtmlPage();
  else
    showBlankPage();
}



void KNArticleWidget::setArticle(KNArticle *a)
{
  a_rticle=a;
  h_tmlDone=false;
  r_ot13=false;
  a_ctToggleRot13->setChecked(false);

  t_imer->stop();

  if(!a)
  	showBlankPage();
  else {
  	//kdDebug(5003) << "KNArticleWidget::setArticle() : " << a->messageID()->as7BitString(false) << endl;
  	if(a->hasContent()) //article is already loaded => just show it
  		createHtmlPage();
  	else if(!a->isLocked()) {
  	  if(a->type()==KNMimeBase::ATremote) {//ok, this is a remote-article => fetch it from the server
    		KNGroup *g=static_cast<KNGroup*>(a->collection());
    		emitJob( new KNJobData(	KNJobData::JTfetchArticle, this, g->account(), a_rticle ) );
      }
  	  else { //local article
    	  KNLocalArticle *la=static_cast<KNLocalArticle*>(a_rticle);
    	  KNFolder *f=static_cast<KNFolder*>(a_rticle->collection());
    	  if(!f || !f->loadArticle(la))
    	    showErrorMessage(i18n("Cannot load the article from the mbox-file!"));
    	  else
    	    createHtmlPage();
    	}
    }
	}
}


void KNArticleWidget::processJob(KNJobData *j)
{
	KNRemoteArticle *a=static_cast<KNRemoteArticle*>(j->data());
	
	if(j->canceled()) {
	  articleChanged(a);	
	}
	else {
    if(j->success()) {
  	  a->updateListItem();
  	  articleChanged(a);
  	}
  	else if(a_rticle==a)
  	  showErrorMessage(j->errorString());
  }	
	
  delete j;
}


void KNArticleWidget::createHtmlPage()
{
  kdDebug(5003) << "KNArticleWidget::createHtmlPage()" << endl;

  if(!a_rticle) {
    showBlankPage();
    return;
  }

  if(!a_rticle->hasContent()) {
    showErrorMessage(i18n("the article contains no data"));
    return;
  }

  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();
  KNConfig::ReadNewsGeneral *rng=knGlobals.cfgManager->readNewsGeneral();

  //----------------------------------- <Header> ---------------------------------------


  QString html, hLine;
  html=QString("<qt><table width=\"100%\" cellpadding=0 cellspacing=1><tr><td width=40 bgcolor=\"%1\"></td><td width=\"1%\"><headerblock><table cellpadding=0 cellspacing=0>")
        .arg(app->headerDecoHexcode());

  if(f_ullHdrs) {
    KNStringSplitter split;
    split.init(a_rticle->head(), "\n");
		QString temp;
    int pos;
    bool splitOk=split.first();
    while(splitOk) {
    	html+="<tr><td align=right>";
      temp=QString::fromLatin1(split.string().data(), split.string().length());
      if( (pos=temp.find(':'))==-1 )
        html+=QString("</td><td width=\"100%\">%1</td></tr>").arg(toHtmlString(temp));
      else
        html+=QString("<b>%1</b></td><td width=\"100%\">%2</td></tr>")
                      .arg(toHtmlString(temp.left(pos+1)))
                      .arg(toHtmlString(temp.right(temp.length()-pos-2)));
			splitOk=split.next();
    }
  }
  else {
		KNHeaders::Base *hb;
		KNDisplayedHeader *dh;
    KNConfig::DisplayedHeaders::Iterator it=knGlobals.cfgManager->displayedHeaders()->iterator();
    for(; it.current(); ++it) {
			dh=it.current();
			hb=a_rticle->getHeaderByType(dh->header().latin1());
      if(!hb) continue; //header not found

      if(dh->hasName()) {
        html += QString("<tr><td align=right>%1%2:%3</td><td width=\"100%\">")
        .arg(dh->nameOpenTag()).arg(toHtmlString(dh->translatedName()))
        .arg(dh->nameCloseTag());
      }
      else
        html+="<tr><td colspan=2>";

      html+=dh->headerOpenTag();

      if(hb->is("From"))
				html+=QString("<a href=\"internal:author\">%1</a>")
                .arg(toHtmlString(hb->asUnicodeString()));
      else if(hb->is("Date")) {
      	KNHeaders::Date *date=static_cast<KNHeaders::Date*>(hb);
				html+=toHtmlString(KGlobal::locale()->formatDateTime(date->qdt(), false, true));
      }
      else
				html+=toHtmlString(hb->asUnicodeString());

      html += dh->headerCloseTag()+"</td></tr>";
    }
  }

  html+=QString("</table></headerblock></td></tr><tr><td colspan=2 bgcolor=\"%1\"><headerblock>")
    .arg(app->headerDecoHexcode());

  //References
  KNHeaders::References *refs=a_rticle->references(false);
  if(a_rticle->type()==KNMimeBase::ATremote && refs) {
    int refCnt=refs->count();
    html+=QString("<b>%1</b>").arg(i18n("References:"));
    for(int refNr=0; refNr<refCnt; refNr++)
      html+=QString(" <a href=\"internal:ref=%1\">%2</a>").arg(refNr).arg(refNr+1);
  }
  else html+=i18n("no references");

  html+="</headerblock></td></tr>";


  KNMimeContent *text=a_rticle->textContent();
	if(text) {
	  if(!text->canDecode8BitText()) {
  	  html+=QString("<tr><td colspan=3 bgcolor=red><font color=black><headerblock>%1</headerblock></font></td></tr>")
        .arg(i18n("Unknown charset! Default charset is used instead."));
		  kdDebug(5003) << "KNArticleWidget::createHtmlPage() : unknown charset = " << text->contentType()->charset() << " not available!" << endl;
	    setFont(app->articleFont());
	  }
	  else {
	    QFont f=app->articleFont();
      text->setFontForContent(f);
      setFont(f);
    }
  }
	else
	  setFont(app->articleFont());
	
	kdDebug(5003) << "KNArticleWidget::createHtmlPage() : font-family = " << font().family() << endl;
	
  html+="</table>";

  //----------------------------------- </Header> --------------------------------------


  //------------------------------------- <Body> ---------------------------------------

	KNHeaders::ContentType *ct=a_rticle->contentType();

  //Attachments
  if(!text || ct->isMultipart()) {
    if(a_tt) a_tt->clear();
    else {
      a_tt=new KNMimeContent::List;
      a_tt->setAutoDelete(false);
    }

    a_rticle->attachments(a_tt, rng->showAlternativeContents());
  } else {
    delete a_tt;
    a_tt=0;
  }

  //Partial message
  if(ct->isPartial()) {
    html+=i18n("<br><bodyblock><b>This article has the Mime-Type &quot;message/partial&quot;, which KNode cannot handle yet.<br>Meanwhile you can save the article as a text-file and reassemble it by hand.</b></bodyblock></qt>");
    setText(html);
    h_tmlDone=true;

    //enable actions
    a_ctReply->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
    a_ctRemail->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
    a_ctSave->setEnabled(true);
    a_ctPrint->setEnabled(true);
    a_ctSelAll->setEnabled(true);
    a_ctToggleFullHdrs->setEnabled(true);
    return;
  }

  //body text
  if(text && text->hasContent()) {
		html+="<bodyblock>";
    if(text->contentType()->isHTMLText()) {
    	QString htmlTxt;
    	text->decodedText(htmlTxt);
    	setText(htmlTxt);             // is this correct? what happens to the headers? (CG)
   	}
		else {
      QChar firstChar;
      int oldLevel=0, newLevel=0;
      unsigned int idx=0;
      bool isSig=false;
      QStringList lines;
      QString line;
      text->decodedText(lines);
			for(QStringList::Iterator it=lines.begin(); it!=lines.end(); ++it) {
      	line=(*it);
      	if(!line.isEmpty()) {
          if(!isSig && line=="-- ") {
            isSig=true;
            if(newLevel>0) {
              newLevel=0;
              html+="</font>";
            }
            if(rng->showSignature()) {
              html+="<hr size=2>";
              continue;
            }
            else break;
          }
          if(!isSig) {
            idx=0;
            oldLevel=newLevel;
            newLevel=0;
            firstChar=line[idx];
            while(idx < line.length()) {
              firstChar=line[idx];
              if(firstChar.isSpace()) idx++;
              else if(firstChar.latin1()=='>') { idx++; newLevel++; }
              else break;
            }

            if(newLevel!=oldLevel) {
              if(newLevel==0) html+="</font>";
              else {
                if(newLevel>=3) newLevel=3;
                if(oldLevel>0) html+="</font>";
                html+=QString("<font color=\"%1\">").arg(app->quotedTextHexcode(newLevel-1));
              }
            }
          }
          html+=toHtmlString(line,true,true,true)+"<br>";
	      }
	      else
          html+="<br>";
      }
      if(newLevel>0) html+="</font>";
    }

    html+="</bodyblock>";
  }

  //attachment table
  if(a_tt) {
    int attCnt=0;
    QString path;
    if(!a_tt->isEmpty()) {
      html+="<table border width=\"100%\">";
      html+=QString("<tr><th>%1</th><th>%2</th><th>%3</th></tr>")
                    .arg(i18n("name")).arg(i18n("mime-type")).arg(i18n("description"));

      for(KNMimeContent *var=a_tt->first(); var; var=a_tt->next()) {
				ct=var->contentType();
        html+=QString("<tr><td align=center><a href=\"internal:att=%1\">%2</a></td><td align=center>%3</td><td align=center>%4</td></tr>")
              .arg(attCnt)
              .arg(ct->name())
              .arg(ct->mimeType())
              .arg(toHtmlString(var->contentDescription()->asUnicodeString()));


        if(rng->showAttachmentsInline() && inlinePossible(var)) {
          html+="<tr><td colspan=3>";
          if(ct->isImage()) { //image
            path=knGlobals.artManager->saveContentToTemp(var);
            if(!path.isEmpty()) {
              html+=QString("<a href=\"internal:att=%1\"><img src=\"%2\"></a>").arg(attCnt).arg(path);
            }
          }
          else { //text
						QString tmp;
						var->decodedText(tmp);
						if(ct->isHTMLText())
							html+=tmp;
						else
              html+="<pre>"+toHtmlString(tmp,true,false,true)+"</pre>";
          }
          html+="</td></tr>";
        }
        attCnt++;
      }
      html+="</table>";
    }
  }

  //------------------------------------- </Body> --------------------------------------


  //display html
  html+="</qt>";
  setText(html);
  h_tmlDone=true;

  //enable actions
  a_ctSave->setEnabled(true);
  a_ctPrint->setEnabled(true);
  a_ctSelAll->setEnabled(true);

  a_ctReply->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
  a_ctRemail->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
  a_ctForward->setEnabled(true);
 	a_ctCancel->setEnabled( (knGlobals.folManager->currentFolder()!=knGlobals.folManager->outbox())
 	                         && (knGlobals.folManager->currentFolder()!=knGlobals.folManager->drafts()));
 	a_ctSupersede->setEnabled( (knGlobals.folManager->currentFolder()!=knGlobals.folManager->outbox())
 	                            && (knGlobals.folManager->currentFolder()!=knGlobals.folManager->drafts()));
 	                          	
  a_ctToggleFullHdrs->setEnabled(true);
  a_ctToggleRot13->setEnabled(true);

  //start automark-timer
  if(a_rticle->type()==KNMimeBase::ATremote && rng->autoMark())
    t_imer->start( (rng->autoMarkSeconds()*1000), true);
}



void KNArticleWidget::setSource(const QString &s)
{
  if(!s.isEmpty())
    anchorClicked(s);
}



void KNArticleWidget::anchorClicked(const QString &a, ButtonState button, const QPoint *p)
{
  anchorType type=ATunknown;
  QString target;

  if(a.left(15)=="internal:author") {
    type=ATauthor;
  }
  else if(a.left(13)=="internal:ref=") {
    target=a.mid(13, a.length()-13);
    type=ATreference;
  }
  else if(a.left(13)=="internal:att=") {
    target=a.mid(13, a.length()-13);
    type=ATattachment;
  }
  else if(a.left(7).lower()=="http://" ||a.left(6).lower()=="ftp://") {
    target=a;
    type=ATurl;
  }

  if((button==LeftButton)||(button==MidButton)) {
		KNGroup *g;
		KNRemoteArticle *a;
		
    switch(type) {
      case ATauthor:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : mailto author" << endl;
        knGlobals.artFactory->createMail(a_rticle->from());
      break;
      case ATreference:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : reference " << target << endl;
				g=static_cast<KNGroup*>(a_rticle->collection());
				a=g->byMessageId(a_rticle->references()->at(target.toInt()));
				if(a)
					setArticle(a);
			  else
					showErrorMessage(i18n("Article %1 not found in group %2")
																.arg(a_rticle->references()->at(target.toInt()))
																.arg(g->groupname()));
      break;
      case ATattachment:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : attachment " << target << endl;
        if(knGlobals.cfgManager->readNewsGeneral()->openAttachmentsOnClick())
          openAttachment(target.toInt());
        else
          saveAttachment(target.toInt());
      break;
      case ATurl:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : url " << target << endl;;
        openURL(target);
      break;
      default:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : unknown" << endl;
      break;
    }
  }
  else {

    if(type==ATattachment) {
      kdDebug(5003) << "KNArticleWidget::anchorClicked() : popup for attachment " << target << endl;
      switch(a_ttPopup->exec(*p)) {
        case PUP_OPEN:
          openAttachment(target.toInt());
        break;
        case PUP_SAVE:
          saveAttachment(target.toInt());
        break;
      }
    }

    else if(type==ATurl) {
      kdDebug(5003) << "KNArticleWidget::anchorClicked() : popup for url " << target << endl;
      switch(u_rlPopup->exec(*p)) {
        case PUP_OPEN:
          openURL(target);
        break;
        case PUP_COPYURL:
          QApplication::clipboard()->setText(target);
        break;
      }
    }
  }
}


void KNArticleWidget::slotSave()
{
  kdDebug(5003) << "KNArticleWidget::slotSave()" << endl;
  if(a_rticle)
    knGlobals.artManager->saveArticleToFile(a_rticle);
}


void KNArticleWidget::slotPrint()
{
  kdDebug(5003) << "KNArticleWidget::slotPrint()" << endl;
  QPrinter *printer=new QPrinter();

  if(printer->setup(this)) {

    QPaintDeviceMetrics metrics(printer);
    QPainter p;

    const int margin=20;
    int yPos=0;
    KNHeaders::Base *hb=0;
    QString text;
    QString hdr;

    p.begin(printer);
    p.setFont( QFont(font().family(), 12, QFont::Bold) );
    QFontMetrics fm=p.fontMetrics();

    KNDisplayedHeader *dh;
    KNConfig::DisplayedHeaders::Iterator it=knGlobals.cfgManager->displayedHeaders()->iterator();
    dh=it.current();
    while(dh) {
      hb=a_rticle->getHeaderByType(dh->header().latin1());

      if(hb && !hb->isEmpty()) {
      	if(dh->hasName())
          text=QString("%1: %2").arg(dh->translatedName()).arg(hb->asUnicodeString());
        else
          text=hb->asUnicodeString();

        p.drawText( 10, yPos+margin,  metrics.width(),
                  fm.lineSpacing(), ExpandTabs | DontClip,
                  text );

        if( (dh=++it)!=0 )
          yPos+=fm.lineSpacing();
      }
      else
        ++it;
    }

    yPos+=fm.lineSpacing()+10;


    QPen pen(QColor(0,0,0), 2);
    p.setPen(pen);

    p.drawLine(10, yPos+margin, metrics.width(), yPos+margin);
    yPos+=2*fm.lineSpacing();

    p.setFont( QFont(font().family(), 10, QFont::Normal) );
    fm=p.fontMetrics();

		QStringList lines;
		KNMimeContent *txt=a_rticle->textContent();
		
		if(txt) {
			txt->decodedText(lines);
			for(QStringList::Iterator it=lines.begin(); it!=lines.end(); ++it) {
				
				if(yPos+margin > metrics.height()) {
        	printer->newPage();
        	yPos=0;
      	}
      	
      	text=(*it);
      	p.drawText( 10, yPos+margin,  metrics.width(),
                		fm.lineSpacing(), ExpandTabs | DontClip,
                		text );
      	
      	yPos+=fm.lineSpacing();
    	}
    }	

    p.end();
  }

  delete printer;
}


void KNArticleWidget::slotSelectAll()
{
  kdDebug(5003) << "KNArticleWidget::slotSelectAll()" << endl;
  selectAll();
  a_ctCopy->setEnabled(true);
}


void KNArticleWidget::slotReply()
{
  kdDebug(5003) << "KNArticleWidget::slotReply()" << endl;
  if(a_rticle && a_rticle->type()==KNMimeBase::ATremote)
    knGlobals.artFactory->createReply(static_cast<KNRemoteArticle*>(a_rticle), selectedText(), true, false);
}


void KNArticleWidget::slotRemail()
{
  kdDebug(5003) << "KNArticleWidget::slotRemail()" << endl;
  if(a_rticle && a_rticle->type()==KNMimeBase::ATremote)
    knGlobals.artFactory->createReply(static_cast<KNRemoteArticle*>(a_rticle), selectedText(), false, true);
}


void KNArticleWidget::slotForward()
{
  kdDebug(5003) << "KNArticleWidget::slotForward()" << endl;
  knGlobals.artFactory->createForward(a_rticle);
}


void KNArticleWidget::slotCancel()
{
  kdDebug(5003) << "KNArticleWidget::slotCancel()" << endl;
  knGlobals.artFactory->createCancel(a_rticle);
}


void KNArticleWidget::slotSupersede()
{
  kdDebug(5003) << "KNArticleWidget::slotSupersede()" << endl;
  knGlobals.artFactory->createSupersede(a_rticle);
}


void KNArticleWidget::slotToggleFullHdrs()
{
  kdDebug(5003) << "KNArticleWidget::slotToggleFullHdrs()" << endl;
  f_ullHdrs=!f_ullHdrs;
  updateContents();
}


void KNArticleWidget::slotToggleRot13()
{
  r_ot13=!r_ot13;
  updateContents();
}


void KNArticleWidget::slotTimeout()
{
  KNRemoteArticle::List l;
  l.append((static_cast<KNRemoteArticle*>(a_rticle)));

  knGlobals.artManager->setRead(&l, true);
}


//--------------------------------------------------------------------------------------

QList<KNArticleWidget> KNArticleWidget::i_nstances;

void KNArticleWidget::configChanged()
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    i->applyConfig();
}


void KNArticleWidget::articleRemoved(KNArticle *a)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(a==i->article())
      i->showBlankPage();
}


void KNArticleWidget::articleChanged(KNArticle *a)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(a==i->article())
      i->updateContents();
}


void KNArticleWidget::collectionRemoved(KNArticleCollection *c)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(i->article() && i->article()->collection()==c)
      i->showBlankPage();
}


//=============================================================================================================


// some standard headers
static const char *predef[] = { "Approved","Content-Transfer-Encoding","Content-Type","Control","Date","Distribution",
                                "Expires","Followup-To","From","Lines","Message-ID","Mime-Version","NNTP-Posting-Host",
                                "Newsgroups","Organization","Path","References","Reply-To","Sender","Subject","Supersedes",
                                "To", "User-Agent","X-Mailer","X-Newsreader","X-No-Archive","XRef",0 };

// default display names KNode uses
static const char *disp[] = { "Groups", 0 };

void dummyHeader()
{
  i18n("collection of article headers","Approved");
  i18n("collection of article headers","Content-Transfer-Encoding");
  i18n("collection of article headers","Content-Type");
  i18n("collection of article headers","Control");
  i18n("collection of article headers","Date");
  i18n("collection of article headers","Distribution");
  i18n("collection of article headers","Expires");
  i18n("collection of article headers","Followup-To");
  i18n("collection of article headers","From");
  i18n("collection of article headers","Lines");
  i18n("collection of article headers","Message-ID");
  i18n("collection of article headers","Mime-Version");
  i18n("collection of article headers","NNTP-Posting-Host");
  i18n("collection of article headers","Newsgroups");
  i18n("collection of article headers","Organization");
  i18n("collection of article headers","Path");
  i18n("collection of article headers","References");
  i18n("collection of article headers","Reply-To");
  i18n("collection of article headers","Sender");
  i18n("collection of article headers","Subject");
  i18n("collection of article headers","Supersedes");
  i18n("collection of article headers","To");
  i18n("collection of article headers","User-Agent");
  i18n("collection of article headers","X-Mailer");
  i18n("collection of article headers","X-Newsreader");
  i18n("collection of article headers","X-No-Archive");
  i18n("collection of article headers","XRef");

  i18n("collection of article headers","Groups");
}


//=============================================================================================================


KNDisplayedHeader::KNDisplayedHeader()
 : t_ranslateName(true)
{
  f_lags.fill(false, 8);
  f_lags[1] = true;   // header name bold by default
}


KNDisplayedHeader::~KNDisplayedHeader()
{
}


// some common headers
const char** KNDisplayedHeader::predefs()
{
  return predef;
}


// *trys* to translate the name
QString KNDisplayedHeader::translatedName()
{
  if (t_ranslateName) {
    // major hack alert !!!
    if (!n_ame.isEmpty()) {
      if (i18n("collection of article headers",n_ame.local8Bit())!=n_ame.local8Bit().data())    // try to guess if this english or not
        return i18n("collection of article headers",n_ame.local8Bit());
      else
        return n_ame;
    } else
      return QString::null;
  } else
    return n_ame;
}


// *trys* to retranslate the name to english
void KNDisplayedHeader::setTranslatedName(const QString &s)
{
  bool retranslated = false;
  for (const char **c=predef;(*c)!=0;c++) {  // ok, first the standard header names
    if (s==i18n("collection of article headers",*c)) {
      n_ame = QString::fromLatin1(*c);
      retranslated = true;
      break;
    }
  }

  if (!retranslated) {
    for (const char **c=disp;(*c)!=0;c++)   // now our standard display names
      if (s==i18n("collection of article headers",*c)) {
        n_ame = QString::fromLatin1(*c);
        retranslated = true;
        break;
      }
  }

  if (!retranslated) {      // ok, we give up and store the maybe non-english string
    n_ame = s;
    t_ranslateName = false;  // and don't try to translate it, so a german user *can* use the original english name
  } else
    t_ranslateName = true;
}


void  KNDisplayedHeader::createTags()
{
  const char *tokens[] = {  "<large>","</large>","<b>","</b>",
                            "<i>","</i>","<u>","</u>" };

  for(int i=0; i<4; i++) t_ags[i]=QString::null;

  if(f_lags.at(0)) {    // <font>
    t_ags[0]=tokens[0];
    t_ags[1]=tokens[1];
  }
  if(f_lags.at(4)) {
    t_ags[2]=tokens[0];
    t_ags[3]=tokens[1];
  }

  if(f_lags.at(1)) {     // <b>
    t_ags[0]+=(tokens[2]);
    t_ags[1].prepend(tokens[3]);
  }
  if(f_lags.at(5)) {
    t_ags[2]+=tokens[2];
    t_ags[3].prepend(tokens[3]);
  }

  if(f_lags.at(2)) {     // <i>
    t_ags[0]+=tokens[4];
    t_ags[1].prepend(tokens[5]);
  }
  if(f_lags.at(6)) {
    t_ags[2]+=tokens[4];
    t_ags[3].prepend(tokens[5]);
  }

  if(f_lags.at(3)) {    // <u>
    t_ags[0]+=tokens[6];
    t_ags[1].prepend(tokens[7]);
  }
  if(f_lags.at(7)) {
    t_ags[2]+=tokens[6];
    t_ags[3].prepend(tokens[7]);
  }
}


//--------------------------------

#include "knarticlewidget.moc"
