// -*- c-basic-offset: 2; tab-width: 2 -*-
/***************************************************************************
                          knarticlewidget.cpp  -  description
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

#include <kglobalsettings.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kprocess.h>

#include "knfetcharticlemanager.h"
#include "resource.h"
#include "knarticlecollection.h"
#include "knarticle.h"
#include "knarticlewidget.h"
#include "knviewheader.h"
#include "knsavedarticlemanager.h"
#include "knappmanager.h"
#include "knglobals.h"

#define PUP_OPEN    1000
#define PUP_SAVE    2000
#define PUP_COPYURL 3000
#define PUP_SELALL  4000
#define PUP_COPY    5000


#define HDR_COL   0
#define QCOL_1    1
#define QCOL_2    2
#define QCOL_3    3

//==================================================================================

//flags
bool KNArticleWidget::showSig;
bool KNArticleWidget::fullHdrs;
bool KNArticleWidget::openAtt;
bool KNArticleWidget::inlineAtt;
bool KNArticleWidget::altAsAtt;

//colors
QString KNArticleWidget::hexColors[4];
QColor KNArticleWidget::txtCol;
QColor KNArticleWidget::bgCol;
QColor KNArticleWidget::lnkCol;

//font
QFont KNArticleWidget::htmlFont;

KNArticleWidget::browserType KNArticleWidget::browser;
QList<KNArticleWidget> KNArticleWidget::instances;

void KNArticleWidget::readOptions()
{
  KConfig *c = KGlobal::config();     
  c->setGroup("READNEWS");
  
  showSig=c->readBoolEntry("showSig", true);
  fullHdrs=c->readBoolEntry("fullHdrs", false);
  inlineAtt=c->readBoolEntry("inlineAtt", true);
  openAtt=c->readBoolEntry("openAtt", false);
  altAsAtt=c->readBoolEntry("showAlts", false);
  browser=(browserType)(c->readNumEntry("Browser", 0));
  
  if (knGlobals.appManager->useFonts())
    htmlFont=knGlobals.appManager->font(KNAppManager::article);
  else
    htmlFont=KGlobalSettings::generalFont();

  QColor col;

  if (knGlobals.appManager->useColors()) {
    txtCol=knGlobals.appManager->color(KNAppManager::normalText);
    bgCol=knGlobals.appManager->color(KNAppManager::background);
    lnkCol=knGlobals.appManager->color(KNAppManager::url);
    col=knGlobals.appManager->color(KNAppManager::header);
  } else {
    txtCol = kapp->palette().active().text();
    bgCol = kapp->palette().active().base();
    lnkCol = KGlobalSettings::linkColor();
    col = kapp->palette().active().background();
  }

  hexColors[0]= QString("#%1%2%3").arg(col.red(),2,16).arg(col.green(),2,16).arg(col.blue(),2,16);

  for(int i=3; i<6; i++) {
    if (knGlobals.appManager->useColors())
      col=knGlobals.appManager->color(i);
    else
      col=kapp->palette().active().text();
    hexColors[i-2]= QString("#%1%2%3").arg(col.red(),2,16).arg(col.green(),2,16).arg(col.blue(),2,16);
  }
}

void KNArticleWidget::saveOptions()
{
  KConfig *c = KGlobal::config();     
  c->setGroup("READNEWS");

  c->writeEntry("fullHdrs", fullHdrs);
}


void KNArticleWidget::updateInstances()
{
  for(KNArticleWidget *i=instances.first(); i; i=instances.next())
    i->applyConfig();
}



KNArticleWidget* KNArticleWidget::find(KNArticle *a)
{
  KNArticleWidget *w=0;
  for(KNArticleWidget *i=instances.first(); i; i=instances.next())
    if(i->a_rticle==a) {
      w=i;
      break;
    }
  return w; 
}



KNArticleWidget* KNArticleWidget::mainWidget()
{
  return instances.first();
}



void KNArticleWidget::showArticle(KNArticle *a)
{
  for(KNArticleWidget *i=instances.first(); i; i=instances.next()) {
    if(i->a_rticle==a && !i->h_tmlDone) i->createHtmlPage();
  } 
}



void KNArticleWidget::setFullHeaders(bool b)
{
  if(fullHdrs!=b) {
    fullHdrs=b;
    for(KNArticleWidget *i=instances.first(); i; i=instances.next())
      i->updateContents();
  }
}


          
void KNArticleWidget::toggleFullHeaders()
{
  setFullHeaders(!fullHdrs);
}



bool KNArticleWidget::fullHeaders()
{
  return fullHdrs;
}
        


//==================================================================================



KNArticleWidget::KNArticleWidget(QWidget *parent, const char *name )
    : QTextBrowser(parent, name), a_rticle(0), c_oll(0), att(0), h_tmlDone(false)
{
  instances.append(this);

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
  urlPopup=new KPopupMenu();
  urlPopup->insertItem(i18n("Open URL"), PUP_OPEN);
  urlPopup->insertItem(i18n("Copy to clipboard"), PUP_COPYURL);
  attPopup=new KPopupMenu();
  attPopup->insertItem(i18n("Open"), PUP_OPEN);
  attPopup->insertItem(i18n("Save"), PUP_SAVE);

  //actions
  actSave = KStdAction::save(this, SLOT(slotSave()), &actionCollection);
  actSave->setEnabled(false);
  actPrint = KStdAction::print(this, SLOT(slotPrint()), &actionCollection);
  actPrint->setEnabled(false);
  actSelAll =  KStdAction::selectAll(this, SLOT(slotSelectAll()), &actionCollection);
  //actSelAll->setEnabled(false);
  actCopy = KStdAction::copy(this, SLOT(copy()), &actionCollection);
  actCopy->setEnabled(false);

  applyConfig();
}



KNArticleWidget::~KNArticleWidget()
{
  instances.remove(this);
  delete att;
  delete attPopup;
  delete urlPopup;
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
  if(!a.isEmpty() && e->button()==RightButton)
    anchorClicked(a, RightButton, &(e->globalPos()));

  QTextBrowser::viewportMousePressEvent(e);

}



void KNArticleWidget::viewportMouseReleaseEvent(QMouseEvent *e)
{
  QTextBrowser::viewportMouseReleaseEvent(e);

  if(hasSelectedText() && !selectedText().isEmpty()) {
    copy();
    actCopy->setEnabled(true);
  }
  else
    actCopy->setEnabled(false);
}



void KNArticleWidget::applyConfig()
{
  QColorGroup pcg(paperColorGroup());
  pcg.setColor(QColorGroup::Base, bgCol);
  pcg.setColor(QColorGroup::Text, txtCol);
  setPaperColorGroup(pcg);

  setLinkColor(lnkCol);
  
  updateContents();
}



QString KNArticleWidget::toHtmlString(const QString &line, bool parseURLs, bool beautification)
{
  QString result;
  QRegExp regExp;
  uint len=line.length();
  int matchLen;
  bool forceNBSP=false; //use "&nbsp;" for spaces => workaround for a bug in QTextBrowser
          
  for(uint idx=0; idx<len; idx++){
    
    switch(line[idx].latin1()) {
      
      case '\r':  break;
      case '\n':  result+="<br>"; break;  
      case '<' :  result+="&lt;"; break;
      case '>' :  result+="&gt;"; break;
      case '&' :  result+="&amp;"; break;
      case '"' :  result+="&quot;"; break;
      case '\t':  result+="&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"; break;   // tab == 8 spaces
      
      case 32 :
        if(line[idx+1].isSpace())  {
          while(line[idx].isSpace()) {
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
           (line[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'h'         
          regExp="^https?://[^\\s<>()\"|]+";
          if (regExp.match(line,idx,&matchLen)!=-1) {
            result+=QString("<a href=\"%1\">%2</a>").arg(line.mid(idx,matchLen)).arg(line.mid(idx,matchLen));
            idx+=matchLen-1;
            break;
          }
        }
        result+=line[idx];
        break;  
      
      case 'w' :
        if((parseURLs)&&
           (line[idx+1].latin1()=='w')) {   // don't do all the stuff for every 'w'         
          regExp="^www\\.[^\\s<>()\"|]+\\.[^\\s<>()\"|]+";
          if (regExp.match(line,idx,&matchLen)!=-1) {
            result+=QString("<a href=\"http://%1\">%2</a>").arg(line.mid(idx,matchLen)).arg(line.mid(idx,matchLen));
            idx+=matchLen-1;
            break;
          }
        }
        result+=line[idx];
        break;  
      
      case 'f' :
        if((parseURLs)&&
           (line[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'f'         
          regExp="^ftp://[^\\s<>()\"|]+";
          if (regExp.match(line,idx,&matchLen)!=-1) {
            result+=QString("<a href=\"%1\">%2</a>").arg(line.mid(idx,matchLen)).arg(line.mid(idx,matchLen));
            idx+=matchLen-1;
            break;
          }
          regExp="^ftp\\.[^\\s<>()\"|]+\\.[^\\s<>()\"|]+";
          if (regExp.match(line,idx,&matchLen)!=-1) {
            result+=QString("<a href=\"ftp://%1\">%2</a>").arg(line.mid(idx,matchLen)).arg(line.mid(idx,matchLen));
            idx+=matchLen-1;
            break;
          }
        }
        result+=line[idx];
        break;        

      case '_' :
      case '/' :
      case '*' :
        if(beautification) {
          regExp=QString("^\\%1[^\\s%2]+\\%3").arg(line[idx]).arg(line[idx]).arg(line[idx]);
          if (regExp.match(line,idx,&matchLen)!=-1) {
            if ((idx+matchLen==len)||(line[idx+matchLen].isSpace())) {
              switch (line[idx].latin1()) {
                case '_' :
                  result+=QString("<u>%1</u>").arg(toHtmlString(line.mid(idx+1,matchLen-2),parseURLs));
                  break;
                case '/' :
                  result+=QString("<i>%1</i>").arg(toHtmlString(line.mid(idx+1,matchLen-2),parseURLs));
                  break;
                case '*' :
                  result+=QString("<b>%1</b>").arg(toHtmlString(line.mid(idx+1,matchLen-2),parseURLs));
                  break;                  
              }
              idx+=matchLen-1;
              break;
            }
          }
        }
        result+=line[idx];
        break;    
            
      default  : result+=line[idx];
    }
  }           
  return result;
}



void KNArticleWidget::openURL(const QString &url)
{
  if(url.isEmpty()) return;

  if(browser==BTkonqueror)
    kapp->invokeBrowser(url);
  else {
    KProcess proc;
    proc << "netscape";
  
    struct stat info;      // QFileInfo is unable to handle netscape's broken symlink
    if (lstat((QDir::homeDirPath()+"/.netscape/lock").local8Bit(),&info)==0)
      proc << "-remote" << QString("openURL(%1)").arg(url);
    else
      proc << url;  

    proc.start(KProcess::DontCare);
  }
}



void KNArticleWidget::saveAttachment(int id)
{
  KNMimeContent *a=att->at(id);

  if(a)
    KNArticleManager::saveContentToFile(a);
  else KMessageBox::error(this, i18n("Internal error: Malformed identifier!"));
}




void KNArticleWidget::openAttachment(int id)
{
 KNMimeContent *a=att->at(id);

 if(a)
   KNArticleManager::openContent(a);
 else KMessageBox::error(this, i18n("Internal error: Malformed identifier!"));
}



bool KNArticleWidget::inlinePossible(KNMimeContent *c)
{
  bool ret;
  ret= (  ( c->mimeInfo()->ctMediaType()==KNArticleBase::MTtext &&
            c->mimeInfo()->ctSubType()!=KNArticleBase::STenriched ) ||
          c->mimeInfo()->ctMediaType()==KNArticleBase::MTimage );
  return ret;
}



void KNArticleWidget::showBlankPage()
{

  kdDebug(5003) << "KNArticleWidget::showBlankPage()" << endl;
  setText(QString::null);

  a_rticle=0;
  c_oll=0;
  delete att;
  att=0;
  h_tmlDone=false;
  actSave->setEnabled(false);
  actPrint->setEnabled(false);
  actCopy->setEnabled(false); //probaly not neede, but who knows ;-)
}



void KNArticleWidget::showErrorMessage(const QString &s)
{
  setFont(htmlFont);  // switch back from possible obscure charsets

  QString msg="<qt>"+i18n("<b><font size=+1 color=red>An error occured!</font></b><hr><br>");
  msg+=toHtmlString(s, false, false)+"</qt>";
  setText(msg);

  a_rticle=0;
  c_oll=0;
  delete att;
  att=0;
  h_tmlDone=false;
  actSave->setEnabled(false);
  actPrint->setEnabled(false);
}



void KNArticleWidget::updateContents()
{
  if (a_rticle) createHtmlPage();
  else showBlankPage();
}



void KNArticleWidget::setData(KNArticle *a, KNArticleCollection *c)
{
  a_rticle=a;
  c_oll=c;
  h_tmlDone=false;
  if(!a) showBlankPage();
}



void KNArticleWidget::createHtmlPage()
{
  kdDebug(5003) << "KNArticleWidget::createHtmlPage()" << endl;

  if(!a_rticle || !a_rticle->hasContent()) {
    kdDebug(5003) << "KNArticleWidget::createHtmlPage() : nothing to display - returning" << endl;
    showBlankPage();
    return;
  }
  actSave->setEnabled(true);
  actPrint->setEnabled(true);

  KNMimeContent *text=a_rticle->textContent();
  KNContentCodec codec(text);
  QString html, hLine;
  QFont fnt(htmlFont);

  codec.matchFont(fnt);
  setFont(fnt);


  html=QString("<qt><table width=\"100%\" cellpadding=0 cellspacing=1><tr><td width=40 bgcolor=\"%1\"></td><td width=\"1%\"><headerblock><table cellpadding=0 cellspacing=0>")
        .arg(hexColors[HDR_COL]);


  if(fullHdrs) {
    char name[128], *val;
    int nameLen=0;
    for( char *h=a_rticle->firstHeaderLine(); h; h=a_rticle->nextHeaderLine()){
      html+="<tr><td align=right>";
      val=strchr(h, ':');
      if(!val ||(val-h)>127) {
        html+=QString("</td><td width=\"100%\"> %1</td></tr>").arg(toHtmlString(h, false, false));
      }
      else {
        nameLen=val-h;
        memcpy(name, h, val-h);
        name[nameLen]='\0';
        if(*val==' ') val++;
        html+=QString("<b>%1</b></td><td width=\"100%\">%2</td></tr>").arg(toHtmlString(name, false, false))
        .arg(toHtmlString(val, false, false));
      }
    }
  }
  else {
    for(KNViewHeader *vh=KNViewHeader::first(); vh; vh=KNViewHeader::next()) {

      hLine=a_rticle->headerLine(vh->header().local8Bit().data(), true);

      if(hLine.isEmpty()) continue;


      if(vh->hasName()) {

        html += QString("<tr><td align=right>%1%2:%3</td><td width=\"100%\">")
        .arg(vh->nameOpenTag()).arg(toHtmlString(vh->translatedName(),false,false))
        .arg(vh->nameCloseTag());
      }
      else
        html+="<tr><td colspan=2>";

      html+=vh->headerOpenTag();

      if(vh->header().lower()=="subject")
        html+=toHtmlString(a_rticle->subject(), false);

      else if(vh->header().lower()=="from")
        html+=QString("<a href=\"internal:author\">%1 &lt;%2&gt;</a>")
                .arg(toHtmlString(a_rticle->fromName(),false))
                .arg(toHtmlString(a_rticle->fromEmail(), false));

      else if(vh->header().lower()=="date")
        html+=a_rticle->longTimeString();

      else
        html+=toHtmlString(hLine, false);

      html += vh->headerCloseTag()+"</td></tr>";
    }
  }

  html+=QString("</table></headerblock></td></tr><tr><td colspan=2 bgcolor=\"%1\"><headerblock>")
    .arg(hexColors[HDR_COL]);

  if(a_rticle->type()==KNArticleBase::ATfetch && a_rticle->references().count()>0) {
    int refCnt=a_rticle->references().count();
    html += QString("<b>%1</b>").arg(i18n("References:"));
    for(int refNr=0; refNr < refCnt; refNr++)
      html += QString(" <a href=\"internal:ref=%1\">%2</a>").arg(refNr).arg(refNr+1);
  }
  else html += i18n("no references");

  html+="</headerblock></td></tr>";


  if(text && !codec.charsetAvailable())
      html+=QString("<tr><td colspan=3 bgcolor=red><font color=black>%1</font></td></tr>")
              .arg(i18n("Unknown charset! Default charset is used instead."));

  html+="</table>";


  if(!text || a_rticle->isMultipart()) {
    if(att) att->clear();
    else {
      att=new QList<KNMimeContent>;
      att->setAutoDelete(false);
    }
    a_rticle->attachments(att, altAsAtt);
  } else {
    delete att;
    att=0;
  }

  if(a_rticle->mimeInfo()->ctSubType()==KNArticleBase::STpartial) {
    html+="<br><bodyblock><b>This article has the Mime-Type &quot;message/partial&quot;, \
             which KNode cannot handle yet.<br>Meanwhile you can save the \
             article as a text-file and reassemble it by hand.</b></bodyblock></qt>";
    setText(html);
    h_tmlDone=true;
    return;
  }

  if(text && text->hasContent()) {
    text->decodeText();
    html+="<bodyblock>";
    if(text->mimeInfo()->ctSubType()==KNArticleBase::SThtml) {

      html+=codec.asUnicodeString();

    }
    else {
      QChar firstChar;
      int oldLevel=0, newLevel=0;
      unsigned int idx=0;
      bool isSig=false;
      QString line;

      for(bool b=codec.setFirstLine(); b; b=codec.setNextLine()) {
        line=codec.currentUnicodeLine();
        if(!line.isNull()) {
          if(!isSig && line.left(3)=="-- ") {
            isSig=true;
            if(newLevel>0) {
              newLevel=0;
              html+="</font>";
            }
            if(showSig) {
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
                html+=QString("<font color=\"%1\">").arg(hexColors[newLevel]);
              }
            }
          }
          html+=toHtmlString(line)+"<br>";
	      }
	      else
          html+="<br>";
      }
      if(newLevel>0) html+="</font>";
    }

    html+="</bodyblock>";
  }


  if(att) {
    int attCnt=0;
    QString path;
    if(!att->isEmpty()) {
      html+="<table border width=\"100%\">";
      html+=QString("<tr><th>%1</th><th>%2</th><th>%3</th></tr>")
                    .arg(i18n("name")).arg(i18n("mime-type")).arg(i18n("description"));

      for(KNMimeContent *var=att->first(); var; var=att->next()) {
        html+=QString("<tr><td align=center><a href=\"internal:att=%1\">%2</a></td><td align=center>%3</td><td align=center>%4</td></tr>")
              .arg(attCnt).arg(var->ctName()).arg(var->ctMimeType()).arg(toHtmlString(var->ctDescription()));
        if(inlineAtt && inlinePossible(var)) {
          html+="<tr><td colspan=3>";
          if(var->mimeInfo()->ctMediaType()==KNArticleBase::MTimage) {
            path=KNArticleManager::saveContentToTemp(var);
            if(!path.isEmpty()) {
              html+=QString("<a href=\"internal:att=%1\"><img src=\"%2\"></a>").arg(attCnt).arg(path);
            }
          }
          else if(var->mimeInfo()->ctMediaType()==KNArticleBase::MTtext) {
            var->decodeText();
            codec.setSourceContent(var);
            if(var->mimeInfo()->ctSubType()==KNArticleBase::SThtml) {
              html+=codec.asUnicodeString();
            }
            else {
              html+="<pre>";
              html+=codec.asUnicodeString();
              html+="</pre>";
            }
          }
          html+="</td></tr>";
        }
        attCnt++;
      }
      html+="</table>";
    }
  }


  html+="</qt>";
  setText(html);


  h_tmlDone=true;
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

  if(button==LeftButton) {
    switch(type) {
      case ATauthor:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : mailto author" << endl;
        knGlobals.sArtManager->mailToClicked(this);
      break;
      case ATreference:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : reference " << target << endl;
        knGlobals.fArtManager->referenceClicked(target.toInt(), this, 0);
      break;
      case ATattachment:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : attachment " << target << endl;
        if(openAtt)
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
      switch(attPopup->exec(*p)) {
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
      switch(urlPopup->exec(*p)) {
        case PUP_OPEN:
          openURL(target);
        break;
        case PUP_COPY:
          QApplication::clipboard()->setText(target);
        break;
      }
    }
  }
}


void KNArticleWidget::slotSave()
{
  if(a_rticle)
    KNArticleManager::saveArticleToFile(a_rticle);
}



void KNArticleWidget::slotPrint()
{
  QPrinter *printer=new QPrinter();

  if(printer->setup(this)) {


    QPaintDeviceMetrics metrics(printer);

    QPainter p;

    const int margin=50;
    int yPos=0;
    QString text;
    QCString hdr;

    p.begin(printer);
    p.setFont( QFont(font().family(), 12, QFont::Bold) );
    QFontMetrics fm=p.fontMetrics();

    KNViewHeader *vh=KNViewHeader::first();
    while(vh!=0) {
      hdr=a_rticle->headerLine(vh->header().local8Bit().data(), true);

      if(!hdr.isEmpty()) {

        if(vh->hasName())
          text=QString("%1: %2").arg(vh->translatedName()).arg(hdr);
        else
          text=hdr;

        p.drawText( 10, yPos+margin,  metrics.width(),
                  fm.lineSpacing(), ExpandTabs | DontClip,
                  text );

        vh=KNViewHeader::next();

        if(vh)
          yPos+=fm.lineSpacing();
      }
      else
        vh=KNViewHeader::next();
    }

    yPos+=fm.lineSpacing()+10;


    QPen pen(QColor(0,0,0), 2);
    p.setPen(pen);

    p.drawLine(10, yPos+margin, metrics.width(), yPos+margin);
    yPos+=2*fm.lineSpacing();

    p.setFont( QFont(font().family(), 12, QFont::Normal) );
    fm=p.fontMetrics();

    for(const char *line=a_rticle->firstBodyLine(); line; line=a_rticle->nextBodyLine()) {
      if(yPos+margin > metrics.height()) {
        printer->newPage();
        yPos=0;
      }
      text=line;
      p.drawText( 10, yPos+margin,  metrics.width(),
                fm.lineSpacing(), ExpandTabs | DontClip,
                text );
      yPos+=fm.lineSpacing();
    }

    p.end();
  }

  delete printer;

}



void KNArticleWidget::slotSelectAll()
{
  selectAll();

  actCopy->setEnabled(true);
}


//--------------------------------

#include "knarticlewidget.moc"
