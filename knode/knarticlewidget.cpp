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

#include <qstring.h>
#include <qclipboard.h>

#include <kcursor.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kcharsets.h>
#include <kmessagebox.h>

#include "resource.h"
#include "knarticlecollection.h"
#include "utilities.h"
#include "knglobals.h"
#include "knarticlewidget.h"
#include "knviewheader.h"
#include "knsavedarticlemanager.h"

#define PUP_OPEN  1000
#define PUP_SAVE  2000
#define PUP_COPY  3000
#define TXT_COL 	0
#define LNK_COL 1
#define BK_COL	  2
#define FG_COL 	3
#define QCOL_1	  4
#define QCOL_2		5
#define QCOL_3		6

//==================================================================================
bool KNArticleWidget::showSig;
bool KNArticleWidget::fullHdrs;
bool KNArticleWidget::openAtt;
bool KNArticleWidget::inlineAtt;
bool KNArticleWidget::altAsAtt;
QString KNArticleWidget::hexColors[7];
QString KNArticleWidget::htmlFont;
int KNArticleWidget::htmlFontSize;
KNArticleWidget::browserType KNArticleWidget::browser;
QList<KNArticleWidget> KNArticleWidget::instances;


void KNArticleWidget::readConfig()
{
	KConfig *c=CONF(); c->setGroup("READNEWS");
	showSig=c->readBoolEntry("showSig", true);
	fullHdrs=c->readBoolEntry("fullHdrs", false);
	inlineAtt=c->readBoolEntry("inlineAtt", false);
	openAtt=c->readBoolEntry("openAtt", false);
	altAsAtt=c->readBoolEntry("showAlts", false);
	browser=(browserType)(c->readNumEntry("Browser", 0));
	
	c->setGroup("FONTS-COLORS");
	
	htmlFont= c->readEntry("family", "helvetica");
	htmlFontSize=c->readNumEntry("size", 12);
	
	QColor col;
	for(int i=0; i<7; i++) {
		switch (i) {
			case 0:
			case 1:	col=black; break;			
			case 2:	col=white; break;			
			case 3: col=gray; break;
			default: col=black;
		}
		col=c->readColorEntry(QString("color%1").arg(i+1), &col);
		hexColors[i]= QString("#%1%2%3").arg(col.red(),2,16).arg(col.green(),2,16).arg(col.blue(),2,16);
	}
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
    : QVBox(parent, name), a_rticle(0), c_oll(0), att(0), h_tmlDone(false)
{
  p_art=new KHTMLPart(this);
	part()->setURLCursor(KCursor::handCursor());
	applyConfig();
	
	connect(part()->browserExtension(),SIGNAL(openURLRequest(const KURL &,const KParts::URLArgs &)),
					this,SLOT(slotURLRequest(const KURL &,const KParts::URLArgs &)));
	connect(p_art, SIGNAL(popupMenu(const QString&, const QPoint&)),
	        this, SLOT(slotPopup(const QString&, const QPoint&)));
	instances.append(this);	
	

	p_art->view()->viewport()->setFocusPolicy(QWidget::NoFocus);
  instances.append(this);	
  view=p_art->view();
	view->viewport()->setFocusProxy(this);
  setFocusPolicy(QWidget::WheelFocus);
  urlPopup=new QPopupMenu();
  urlPopup->insertItem(i18n("Open URL"), PUP_OPEN);
  urlPopup->insertItem(i18n("Copy to clipboard"), PUP_COPY);
  attPopup=new QPopupMenu();
  attPopup->insertItem(i18n("Open"), PUP_OPEN);
  attPopup->insertItem(i18n("Save"), PUP_SAVE);
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
  return ((view->contentsY()+view->visibleHeight())<view->contentsHeight());
}



void KNArticleWidget::scrollDown()
{
  int offs = (view->visibleHeight() < 30) ? view->visibleHeight() : 30;
  view->scrollBy( 0, view->visibleHeight()-offs);
}


void KNArticleWidget::print()
{
  KAction *print = p_art->actionCollection()->action( "printFrame" );
  if (print)
    print->activate();
}


void KNArticleWidget::copySelection()
{
  if (p_art->hasSelection())
		QApplication::clipboard()->setText(p_art->selectedText());
}


void KNArticleWidget::findText()
{
  KAction *find = p_art->actionCollection()->action( "find" );
  if (find)
    find->activate();
}


void KNArticleWidget::focusInEvent(QFocusEvent *e)
{
	emit focusChanged(e);
	repaint(false);
}



void KNArticleWidget::focusOutEvent(QFocusEvent *e)
{
	emit focusChanged(e);
	repaint(false);
}



void KNArticleWidget::keyPressEvent(QKeyEvent *e)
{
	if ( !e )	return;

	int offs = (view->visibleHeight() < 30) ? view->visibleHeight() : 30;
		
  switch(e->key()) {
    case Key_Up:
    	view->scrollBy( 0, -10 );
    	break;
    case Key_Down:
    	view->scrollBy( 0, 10 );
    	break;    	
    case Key_Left:
    	view->scrollBy( -10, 0 );
    	break;
    case Key_Right:
    	view->scrollBy( 10, 0 );
    	break;    	    	
    case Key_Prior:
      view->scrollBy( 0, -view->visibleHeight()+offs);
      break;
    case Key_Next:
      view->scrollBy( 0, view->visibleHeight()-offs);
      break;
	 	case Key_Home :
      view->scrollBy(0,-view->contentsY());
    	break;
  	case Key_End:
      view->scrollBy(0,view->contentsHeight());
    	break;    	
   	default:
   	  QVBox::keyPressEvent(e);
  }
}


void KNArticleWidget::applyConfig()
{
	part()->setStandardFont(htmlFont);
	
 	QValueList<int> fontsizes;          // taken from kmail
  part()->resetFontSizes();
  int diff = htmlFontSize - part()->fontSizes()[3];
  if (part()->fontSizes()[0]+diff > 0) {
    for (int i=0;i<7; i++)
      fontsizes << part()->fontSizes()[i] + diff;
    part()->setFontSizes(fontsizes);
  }
	
	updateContents();
}



void KNArticleWidget::slotURLRequest (const KURL &url, const KParts::URLArgs &args)
{
	QString type = url.protocol();
	QString urlText = url.url();
	int pos;
	
	if (type.length()) {             // valid url
		if(type=="http" || type=="ftp") {			
		  openURL(urlText);	
 		}	else {
			if(type=="mailto")
				xTop->sArtManager()->mailToClicked(this);						
		  else
		  	if(type=="news") {		
		  		pos = urlText.find(':');		  		
				  QString target=urlText.mid(pos+1);
				  if((pos=target.find("Ref."))!=-1)	{
				  	pos+=4;
				  	target=target.mid(pos, target.length()-pos);
						xTop->fArtManager()->referenceClicked(target.toInt(), this,0);
					}	else
						if((pos=target.find("Att."))!=-1) {
							if(openAtt) openAttachement(target);
							else saveAttachement(target);
						}						
				}
		}	
	}
}




QString KNArticleWidget::toHtmlString(const QString &line, bool parseURLs, bool beautification)
{
	QString result;
	QRegExp regExp;
	uint len=line.length();
	int matchLen;
					
	for(uint idx=0; idx<len; idx++){
		
		switch(line[idx].latin1()) {
			
			case '\r':	break;
			case '\n':	result+="<br>"; break;	
			case '<' : 	result+="&lt;"; break;
			case '>' : 	result+="&gt;"; break;
			case '&' : 	result+="&amp;"; break;
			case '"' : 	result+="&quot;"; break;
			case '\t':  result+="&nbsp;&nbsp;"; break;
			
			case 32 :	
				if(line[idx+1].latin1()==32)  {
					while(line[idx]==' ') {
						result+="&nbsp;";
						idx++;
					}
					idx--;
				}	else
					if(idx==0)
						result+="&nbsp;";
					else result+=' ';
				break;
			
			case 'h' :	
				if((parseURLs)&&
					 (line[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'h'					
					regExp="^http://[^\\s<>()\"|]+";
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
    // patch by Ian Nendesl - BEGIN
		FILE *fp;
		char lockfile[5];
		int netscape;
		QCString cmdline;
		fp=popen("ls ~/.netscape/lock | sed -e 's,.*cape/,,'", "r");
 		fgets( lockfile, sizeof lockfile, fp);
 		pclose(fp);
    netscape=strcmp(lockfile, "lock");
    if(netscape==0) {
      cmdline = "netscape -remote 'openURL(";
      cmdline += url.latin1();
      cmdline += ")'&";
    }
 		else {
 			cmdline = "netscape ";
      cmdline += url.latin1();
      cmdline +="&";
    }

    system(cmdline.data());
 		// Patch by Ian Nendesl - END
 	}

}



void KNArticleWidget::saveAttachement(const QString &id)
{
  int pos = id.findRev('.');
  if(pos!=-1)
    KNArticleManager::saveContentToFile(att->at(id.mid(++pos, id.length()-pos).toInt()));
  else KMessageBox::error(0, i18n("Internal error: Malformed identifier !!"));
}




void KNArticleWidget::openAttachement(const QString &id)
{
 int pos = id.findRev('.');
 if(pos!=-1)
  KNArticleManager::openContent(att->at(id.mid(++pos, id.length()-pos).toInt()));
 else KMessageBox::error(0, i18n("Internal error: Malformed identifier !!"));
}



bool KNArticleWidget::inlinePossible(KNMimeContent *c)
{
  bool ret;
  ret= (  c->mimeInfo()->ctMediaType()==KNArticleBase::MTtext &&
         ( c->mimeInfo()->ctSubType()==KNArticleBase::STplain ||
           c->mimeInfo()->ctSubType()==KNArticleBase::SThtml ) ) ||
          c->mimeInfo()->ctMediaType()==KNArticleBase::MTimage;
  return ret;
}



void KNArticleWidget::showBlankPage()
{
	part()->begin(KURL("file:/"));
	part()->write(QString("<html><body bgcolor=\"%1\" text=\"%2\" link=\"%3\"></body></html>")
								.arg(hexColors[BK_COL]).arg(hexColors[TXT_COL]).arg(hexColors[LNK_COL]));
	part()->end();
	
	a_rticle=0;
	c_oll=0;
	delete att;
	att=0;	
	h_tmlDone=false;
}



void KNArticleWidget::showErrorMessage(const QString &s)
{
	part()->begin();//(KURL("file:/"));
	part()->write(QString("<html><body bgcolor=\"%1\" text=\"%2\" link=\"%3\">")
								.arg(hexColors[BK_COL]).arg(hexColors[TXT_COL]).arg(hexColors[LNK_COL]));
	part()->write(i18n("<b><font size=+1 color=red>An error occured!</font></b><hr><br>"));
	part()->write(toHtmlString(s,true,false));
  part()->write("</font></body></html>");
	part()->end();
	
	a_rticle=0;
	c_oll=0;
	delete att;
	att=0;
  h_tmlDone=false;
}



void KNArticleWidget::updateContents()
{
	if(a_rticle) createHtmlPage();
	else showBlankPage();
}



void KNArticleWidget::setData(KNArticle *a, KNArticleCollection *c)
{
	a_rticle=a;
	c_oll=c;
	h_tmlDone=false;
	//if(!a || !a->hasContent()) showBlankPage();
}



void KNArticleWidget::createHtmlPage()
{
  if(!a_rticle || !a_rticle->hasContent()) {
  	showBlankPage();
  	return;
  }

 	KHTMLPart *p = part();
	p->begin(KURL("file:/"));
	p->write(QString("<html><body bgcolor=\"%1\" text=\"%2\" link=\"%3\"><table width=\"100%\" cols=3 cellpadding=0 style=\"padding-left: 3px\">\n")
									.arg(hexColors[BK_COL]).arg(hexColors[TXT_COL]).arg(hexColors[LNK_COL]));
									
 	QString buffer,hLine;									
	int rowCount=0, pos, refCnt=0;
																			
 	if(fullHdrs) { 	
		for(char *h=a_rticle->firstHeaderLine(); h; h=a_rticle->nextHeaderLine()) {
		  hLine = toHtmlString(h,false,false);				
      if ((pos=hLine.find(':'))!=-1)
        buffer += QString("<tr><td align=right valign=top width=\"1%\"><b>%1</b></td><td valign=top width=\"99%\">%2</td></tr>\n")
                          .arg(hLine.left(pos+1)).arg(hLine.mid(pos+1));
      else
        buffer += QString("<tr><td colspan =\"2\" width=\"100%\">%1</td></tr>\n").arg(hLine);
  		++rowCount;
  	}
	}	else {
		for(KNViewHeader *vh=KNViewHeader::first(); vh; vh=KNViewHeader::next()) {
			hLine=a_rticle->headerLine(vh->header().local8Bit().data());
			
			if(hLine.isEmpty()) continue;
			
			if(vh->hasName()) {
  			buffer += QString("<tr><td align=right valign=top width=\"1%\">%1%2:%3</td><td valign=top width=\"99%\">")
                          .arg(vh->nameOpenTag()).arg(toHtmlString(vh->name(),false,false)).arg(vh->nameCloseTag());
			} else {
			  buffer += "<tr><td colspan =\"2\" width=\"100%\">";
			}
			
			buffer += vh->headerOpenTag();
			
			if(vh->header().lower()=="subject")
				buffer+=toHtmlString(a_rticle->subject(), false);
			else
				if(vh->header().lower()=="from")
					buffer+=QString("<a href=\"mailto:AUTHOR\">%1 &lt;%2&gt;</a>")
									.arg(toHtmlString(a_rticle->fromName(),false))
									.arg(toHtmlString(a_rticle->fromEmail(), false));
		  else
			  if(vh->header().lower()=="date")
					buffer+=a_rticle->timeString();
			else
					buffer+=toHtmlString(hLine, false);		
				
			buffer += vh->headerCloseTag()+"</td></tr>\n";
			++rowCount;
		}	
	}	

	p->begin(KURL("file:/"));
	p->write(QString("<html><body bgcolor=\"%1\" text=\"%2\" link=\"%3\"><table width=\"100%%\" cellpadding=2><tr><td width=50 bgcolor=\"%4\">&nbsp;</td><td>")
									.arg(hexColors[BK_COL]).arg(hexColors[TXT_COL]).arg(hexColors[LNK_COL]).arg(hexColors[FG_COL]));
									
  if (!rowCount)
	  buffer += QString("<tr><td width=40 bgcolor=\"%1\">&nbsp;</td><td colspan=\"2\"></td></tr>")
	                    .arg(hexColors[FG_COL]);
	else
	  buffer.insert(4,QString("<td width=40 bgcolor=\"%1\" rowspan=\"%2\">&nbsp;</td>")
	                         .arg(hexColors[FG_COL]).arg(rowCount));
					
	p->write(buffer);
										
	buffer=QString("<tr><td colspan=3 bgcolor=\"%1\" width=\"100%\">").arg(hexColors[FG_COL]);
	
	if(a_rticle->type()==KNArticleBase::ATfetch && a_rticle->hasReferences()) {
		refCnt=a_rticle->references().count();
		buffer += QString("<b>%1&nbsp;</b>").arg(i18n("References:"));
		for(int refNr=0; refNr < refCnt; refNr++)
		  buffer += QString("<a href=\"news:Ref.%1\">%2</a>&nbsp;").arg(refNr).arg(refNr+1);
	}	else
		buffer+=i18n("no references");
	buffer+="</td></tr>\n";
	
 	KNMimeContent *body=a_rticle->mainContent();

 	if(body) {
 		body->prepareForDisplay();
 		
 		if (!p->setCharset(body->ctCharset())) {
 			buffer+=QString("<tr><td colspan=3 bgcolor=red width=\"100%\"><font color=black>%1</font></td></tr>\n")
 								.arg(i18n("Unknown charset! Default charset is used instead."));
			KCharsets *c = KGlobal::charsets();  	
 			p->setCharset(c->name(c->charsetForLocale()));
 		}
 	}
 	 	
 	buffer+="</table><br>\n<div style=\"padding-left: 4px\">\n";
 	p->write(buffer); 	
 	buffer=QString::null;
 	
	if(!body || a_rticle->isMultipart()) {
 		if(att) att->clear();
 		else {
 			att=new QList<KNMimeContent>;
 			att->setAutoDelete(false);
 		}
 		a_rticle->attachements(att, altAsAtt);
 	}	else {
 		delete att;
 		att=0;
 	}	
	
	if(a_rticle->mimeInfo()->ctSubType()==KNArticleBase::STpartial) {
		p->write("<b>This article has the Mime-Type &quot;message/partial&quot;, \
					   which KNode cannot handle yet.<br>Meanwhile you can save the \
						 article as a text-file and reassemble it by hand.<b></div></body></html>");
		p->end();
		h_tmlDone=true;
		return;
	}	
	
	if(body) {
  	if(body->mimeInfo()->ctSubType()==KNArticleBase::SThtml) {
  		body->prepareHtml();
  		for(char* l=body->firstBodyLine(); l; l=body->nextBodyLine()) {
  		  qDebug("KNArticleWidget::createHtmlPage() : HTML-Line = %s", l);
  			buffer+=l;
  		}
  	}
  	else {
			char firstChar;
			int oldLevel=0, newLevel=0;
			unsigned int idx=0;
			bool isSig=false;

  		for(const char* var=body->firstBodyLine(); var; var=body->nextBodyLine()) {
    		if(strcmp(var,"-- ")==0) {
    			isSig=true;
    			if(newLevel>0) {
    				newLevel=0;
    				buffer+="</font>";
    			}
    			if(showSig) {
    				buffer+="<br><hr size=2><br>";
    				continue;
    			}
    			else break;
    		}
    		if(!isSig) {
     			idx=0;
     			oldLevel=newLevel;
      		newLevel=0;
      		firstChar=var[idx];
      		while(idx < strlen(var)) {
      			firstChar=var[idx];
      			if(firstChar==' ') idx++;
      			else if(firstChar=='>') { idx++; newLevel++; }
      			else break;
      		}
     		
      		if(newLevel!=oldLevel) {
      			if(newLevel==0) buffer+="</font>";
      			else {
      				if(newLevel>=3) newLevel=3;
      					buffer+=QString("</font><font color=\"%1\">").arg(hexColors[newLevel+3]);
      			}
      		}
      	}   			
	   		buffer+=toHtmlString(var)+"<br>";
    	}
    	if(newLevel>0) buffer+="</font>";
    }	
  }

  p->write(buffer);
  buffer=QString::null;

	if(att) {
		int attCnt=0;
		QString path;
  	if(!att->isEmpty()) {
  		buffer+="<br><center><table width=\"90%\" border cellpadding=2>";
	 		buffer+=QString("<tr><th width=\"20%%\">%1</th><th width=\"20%%\">%2</th><th>%3</th></tr>")
  		              .arg(i18n("name")).arg(i18n("mime-type")).arg(i18n("description"));
  		
  		for(KNMimeContent *var=att->first(); var; var=att->next()) {
  			buffer+=QString("<tr><td align=center><a href=\"news:Att.%1\">%2</a></td><td align=center>%3</td><td align=center>%4</td></tr>")
  						.arg(attCnt).arg(var->ctName()).arg(var->ctMimeType()).arg(var->ctDescription());
  			if(inlineAtt && inlinePossible(var)) {
  			  buffer+="<tr><td colspan=3>";
  			  if(var->mimeInfo()->ctMediaType()==KNArticleBase::MTimage) {
  			    path=KNArticleManager::saveContentToTemp(var);
  			    if(!path.isEmpty()) {
  			      buffer+=QString("<img src=\"file:%1\">").arg(path);
  			    }
  			  }
  			  else if(var->mimeInfo()->ctMediaType()==KNArticleBase::MTtext) {
  			    if(var->mimeInfo()->ctSubType()==KNArticleBase::STplain) {
  			      buffer+="<pre>";
  			      for(char *line=var->firstBodyLine(); line; line=var->nextBodyLine()) {
  			        buffer+=line;
  			        buffer+"\n";
  			      }
  			      buffer+="</pre>";
  			    }
  			    else if(var->mimeInfo()->ctSubType()==KNArticleBase::SThtml) {
  			      var->prepareHtml();
  			      for(char *line=var->firstBodyLine(); line; line=var->nextBodyLine())
  			        buffer+=line;
  			    }
   			  }
          buffer+="</td></tr>";			
  			}
  			attCnt++;
  		}
  		buffer+="</table></center>";
  	}
  }	

  buffer += "</div></body></html>";
  p->write(buffer);
	p->end();
	h_tmlDone=true;
}	


void KNArticleWidget::slotPopup(const QString &url, const QPoint &p)
{
  if(url.isEmpty()) return;

  if(url.left(9)=="news:Att.") {
    switch(attPopup->exec(p)) {
      case PUP_OPEN:
        openAttachement(url);
      break;
      case PUP_SAVE:
        saveAttachement(url);
      break;
    }
  }
  else if(url.left(3).upper()=="FTP" || url.left(4).upper()=="HTTP") {
    switch(urlPopup->exec(p)) {
      case PUP_OPEN:
        openURL(url);
      break;
      case PUP_COPY:
        QApplication::clipboard()->setText(url);
      break;
    }
  }
}
