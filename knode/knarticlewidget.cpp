/*
    knarticlewidget.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
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

#include <qaccel.h>
#include <qclipboard.h>
#include <qdir.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qurl.h>
#include <qbuffer.h>
#include <qbitmap.h>

#include <kprinter.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kprocess.h>
#include <krun.h>
#include <kcharsets.h>
#include <kaction.h>
#include <kapplication.h>
#include <kpgpblock.h>
#include <kstringhandler.h>

#include <libkdepim/kxface.h>

#include "resource.h"
#include "knarticlewidget.h"
#include "knglobals.h"
#include "knarticlemanager.h"
#include "knarticlewindow.h"
#include "knfoldermanager.h"
#include "knarticlefactory.h"
#include "knconfigmanager.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knnntpaccount.h"
#include "utilities.h"
#include <kpgp.h>
#include "knmainwidget.h"
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kbookmarkmanager.h>
#include <kaddrbook.h>
#include <ktempfile.h>

#define PUP_OPEN    1000
#define PUP_SAVE    2000
#define PUP_COPYURL 3000
#define PUP_SELALL  4000
#define PUP_COPY    5000
#define PUP_ADDBOOKMARKS 6000

#define PUP_ADDRESSBOOK 7000
#define PUP_COPYTOCLIPBOARD 8000
#define PUP_OPENADDRESSBOOK 9000

#define HDR_COL   0
#define QCOL_1    1
#define QCOL_2    2
#define QCOL_3    3


KNSourceViewWindow::KNSourceViewWindow(const QString &htmlCode)
  : KTextBrowser(0)
{
  setWFlags(WType_TopLevel | WDestructiveClose);
  QAccel *accel = new QAccel( this, "browser close-accel" );
  accel->connectItem( accel->insertItem( Qt::Key_Escape ), this , SLOT( close() ));
  KNConfig::Appearance *app=knGlobals.configManager()->appearance();

  setCaption(kapp->makeStdCaption(i18n("Article Source")));
  setPaper( QBrush(app->backgroundColor()) );
  setColor( app->textColor() );

  QStyleSheetItem *style;
  style=new QStyleSheetItem(styleSheet(), "txt");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNoWrap);
  style->setFontFamily(app->articleFixedFont().family());
  style->setFontSize(app->articleFixedFont().pointSize());
  style->setFontUnderline(app->articleFixedFont().underline());
  style->setFontWeight(app->articleFixedFont().weight());
  style->setFontItalic(app->articleFixedFont().italic());
  style->setColor( app->textColor() );

  setText(QString("<qt><txt>%1</txt></qt>").arg(htmlCode));
  KNHelper::restoreWindowSize("sourceWindow", this, QSize(500,300));
  show();
}


KNSourceViewWindow::~KNSourceViewWindow()
{
  KNHelper::saveWindowSize("sourceWindow",size());
}


//=============================================================================================================


KNMimeSource::KNMimeSource(QByteArray data, QCString mimeType)
 : d_ata(data), m_imeType(mimeType)
{}


KNMimeSource::~KNMimeSource()
{}


const char* KNMimeSource::format(int n) const
{
  return (n<1)? m_imeType.data() : 0;
}


QByteArray KNMimeSource::encodedData(const char *) const
{
  return d_ata;
}


//=============================================================================================================


KNArticleWidget::KNArticleWidget(KActionCollection* actColl, KXMLGUIClient* guiClient,
    QWidget *parent, const char *name )
    : KTextBrowser(parent, name), a_rticle(0), a_tt(0), h_tmlDone(false),
      f_inddialog(0), a_ctions(actColl), mGuiClient(guiClient)
{
  instances()->append(this);
  setNotifyClick( true );

  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  setFocusPolicy(QWidget::WheelFocus);

  installEventFilter(this);

  //popups
  u_rlPopup=new KPopupMenu();
  u_rlPopup->insertItem(SmallIcon("fileopen"),i18n("&Open Link"), PUP_OPEN);
  u_rlPopup->insertItem(SmallIcon("editcopy"),i18n("&Copy Link Address"), PUP_COPYURL);
  u_rlPopup->insertItem(SmallIcon("bookmark_add"),i18n("Bookmark This Link"), PUP_ADDBOOKMARKS);

  u_mailtoPopup=new KPopupMenu();
  u_mailtoPopup->insertItem(i18n("&Add to Address Book"), PUP_ADDRESSBOOK);
  u_mailtoPopup->insertItem(i18n("&Open in Address Book"), PUP_OPENADDRESSBOOK);
  u_mailtoPopup->insertItem(SmallIcon("editcopy"), i18n("&Copy to Clipboard"), PUP_COPYTOCLIPBOARD);

  a_ttPopup=new KPopupMenu();
  a_ttPopup->insertItem(SmallIcon("fileopen"),i18n("&Open Attachment"), PUP_OPEN);
  a_ttPopup->insertItem(SmallIcon("filesave"),i18n("&Save Attachment..."), PUP_SAVE);

  //actions
  a_ctSave              = KStdAction::save(this, SLOT(slotSave()), a_ctions);
  a_ctSave->setText(i18n("&Save..."));
  a_ctPrint             = KStdAction::print(this, SLOT(slotPrint()), a_ctions);
  a_ctSelAll            = KStdAction::selectAll(this, SLOT(slotSelectAll()), a_ctions);
  a_ctCopy              = KStdAction::copy(this, SLOT(copy()), a_ctions);
  connect( this, SIGNAL( copyAvailable( bool )), a_ctCopy, SLOT( setEnabled( bool )) );

  a_ctReply             = new KAction(i18n("&Followup to Newsgroup..."),"message_reply", Key_R , this,
                          SLOT(slotReply()), a_ctions, "article_postReply");
  a_ctRemail            = new KAction(i18n("Reply by E&mail..."),"mail_reply", Key_A , this,
                          SLOT(slotRemail()), a_ctions, "article_mailReply");
  a_ctForward           = new KAction(i18n("Forw&ard by Email..."),"mail_forward", Key_F , this,
                          SLOT(slotForward()), a_ctions, "article_forward");
  a_ctCancel            = new KAction(i18n("article","&Cancel Article"), 0 , this,
                          SLOT(slotCancel()), a_ctions, "article_cancel");
  a_ctSupersede         = new KAction(i18n("S&upersede Article"), 0 , this,
                          SLOT(slotSupersede()), a_ctions, "article_supersede");
  a_ctVerify            = new KAction(i18n("&Verify PGP Signature"), 0, this,
                          SLOT(slotVerify()), a_ctions, "article_verify");
  a_ctToggleFullHdrs    = new KToggleAction(i18n("Show &All Headers"), "text_block", 0 , this,
                          SLOT(slotToggleFullHdrs()), a_ctions, "view_showAllHdrs");
  a_ctToggleFullHdrs->setCheckedState(i18n("Hide &All Headers"));
  a_ctToggleFullHdrs->setChecked(knGlobals.configManager()->readNewsViewer()->showFullHdrs());
  a_ctToggleRot13       = new KToggleAction(i18n("&Unscramble (Rot 13)"), "decrypted", 0 , this,
                          SLOT(slotToggleRot13()), a_ctions, "view_rot13");
  a_ctToggleFixedFont   = new KToggleAction(i18n("U&se Fixed Font"),  Key_X , this,
                          SLOT(slotToggleFixedFont()), a_ctions, "view_useFixedFont");
  a_ctToggleFixedFont->setChecked(knGlobals.configManager()->readNewsViewer()->useFixedFont());
  a_ctViewSource        = new KAction(i18n("&View Source"),  Key_V , this,
                          SLOT(slotViewSource()), a_ctions, "article_viewSource");

  a_ctSetCharset = new KSelectAction(i18n("Chars&et"), 0, a_ctions, "set_charset");
  a_ctSetCharset->setShortcutConfigurable(false);
  QStringList cs=KGlobal::charsets()->availableEncodingNames();
  cs.prepend(i18n("Automatic"));
  a_ctSetCharset->setItems(cs);
  a_ctSetCharset->setCurrentItem(0);
  connect(a_ctSetCharset, SIGNAL(activated(const QString&)),
          this, SLOT(slotSetCharset(const QString&)));
  a_ctSetCharsetKeyb = new KAction(i18n("Charset"), Key_C, this,
                                   SLOT(slotSetCharsetKeyboard()), a_ctions, "set_charset_keyboard");


  o_verrideCS=KMime::Headers::Latin1;
  f_orceCS=false;

  //timer
  t_imer=new QTimer(this);
  connect(t_imer, SIGNAL(timeout()), this, SLOT(slotTimeout()));

  //config
  r_ot13=false;
  a_ctToggleRot13->setChecked(false);
  applyConfig();

  t_mpFile = new KTempFile(locateLocal("tmp", "knode"), ".png");
  t_mpFile->close();
  t_mpFile->setAutoDelete(true);
}


KNArticleWidget::~KNArticleWidget()
{
  if(a_rticle && a_rticle->isOrphant())
    delete a_rticle; //don't leak orphant articles

  instances()->removeRef(this);
  if(instances()->count() == 0) {
    delete i_nstances;
    i_nstances = 0;
  }
  delete a_tt;
  delete a_ttPopup;
  delete u_rlPopup;
  delete f_actory;
  delete t_imer;
  delete f_inddialog;
  delete u_mailtoPopup;
  delete t_mpFile;
}

void KNArticleWidget::setText( const QString& text, const QString& context )
{
  KTextBrowser::setText( text, context );
  setContentsPos( 0, 0 );
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


void KNArticleWidget::find()
{
  if( !f_inddialog ) {
    f_inddialog = new KEdFind( this, "knodefind", false);
    connect(f_inddialog,SIGNAL(search()),this,SLOT(slotFindStart()));
    connect(f_inddialog,SIGNAL(done()),this,SLOT(slotFindDone()));
  }

  QString string = f_inddialog->getText();
  f_inddialog->setText(string.isEmpty() ? f_ind_pattern : string);

  f_ind_first = true;
  f_ind_found = false;

  f_inddialog->show();
  f_inddialog->result();
}

void KNArticleWidget::slotFindStart()
{
  bool forward = !f_inddialog->get_direction();

  if (f_ind_first) {
    if (forward) {
      f_ind_para = 0;
      f_ind_index = 0;
   }
   else {
      f_ind_para = paragraphs()-1;
      f_ind_index = paragraphLength(f_ind_para);
   }
  }
  else
    f_ind_index++;

  f_ind_pattern = f_inddialog->getText();
  f_ind_first=!QTextEdit::find(f_ind_pattern,f_inddialog->case_sensitive(),false,forward,&f_ind_para,&f_ind_index);

  if (!f_ind_first)
    f_ind_found = true;
  else
    if (f_ind_found) {
      if (forward) {
        if ( KMessageBox::questionYesNo( this,
             i18n("End of article reached.\n" "Continue from the beginning?"),
  	     i18n("Find") ) == KMessageBox::Yes ) {
          f_ind_first = true;
    	  slotFindStart();
        }
      }
      else {
        if ( KMessageBox::questionYesNo( this,
             i18n("Beginning of article reached.\n" "Continue from the end?"),
  	     i18n("Find") ) == KMessageBox::Yes ) {
          f_ind_first = true;
   	  slotFindStart();
        }
      }
    }
  else
    KMessageBox::information( this,
    	i18n( "Search string '%1' not found." ).arg(KStringHandler::csqueeze(f_ind_pattern)),
	i18n( "Find" ) );
}

void KNArticleWidget::slotFindDone()
{
  if (!f_inddialog)
    return;

  removeSelection();
  f_inddialog->hide();
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


bool KNArticleWidget::eventFilter(QObject *o, QEvent *e)
{
  if ((e->type() == QEvent::KeyPress) && (static_cast<QKeyEvent*>(e)->key() == Key_Tab)) {
    emit(focusChangeRequest(this));
    if (!hasFocus())  // focusChangeRequest was successful
      return true;
  }
  return KTextBrowser::eventFilter(o, e);
}


void KNArticleWidget::viewportMousePressEvent(QMouseEvent *e)
{
  QString a= anchorAt(viewportToContents(e->pos()));

  if(!a.isEmpty() && (e->button()==RightButton || e->button()==MidButton))
    anchorClicked(a, e->button(), &e->globalPos());
  else
    if (e->button()==RightButton) {
      QPopupMenu *popup = static_cast<QPopupMenu *>(mGuiClient->factory()->container("body_popup", mGuiClient));
      if ( popup )
        popup->popup(e->globalPos());
    }

  QTextBrowser::viewportMousePressEvent(e);
}


bool KNArticleWidget::canDecode8BitText(const QCString &charset)
{
  if(charset.isEmpty())
    return false;
  bool ok=true;
  (void) KGlobal::charsets()->codecForName(charset,ok);
  return ok;
}


void KNArticleWidget::applyConfig()
{
  KNConfig::Appearance *app=knGlobals.configManager()->appearance();
  KNConfig::ReadNewsViewer *rnv=knGlobals.configManager()->readNewsViewer();

  QFont f=(a_ctToggleFixedFont->isChecked()? app->articleFixedFont():app->articleFont());

  //custom tags <articlefont>, <bodyblock>, <txt_attachment>
  QStyleSheetItem *style;
  style=new QStyleSheetItem(styleSheet(), "articlefont");
  style->setDisplayMode(QStyleSheetItem::DisplayInline);
  style->setFontFamily(f.family());
  style->setFontSize(f.pointSize());
  style->setFontUnderline(f.underline());
  style->setFontWeight(f.weight());
  style->setFontItalic(f.italic());

  style=new QStyleSheetItem(styleSheet(), "bodyblock");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  if (rnv->showHeaderDecoration()) {
    style->setMargin(QStyleSheetItem::MarginAll, 0);
    style->setMargin(QStyleSheetItem::MarginLeft, 5);
  } else
    style->setMargin(QStyleSheetItem::MarginAll, 0);
  if (rnv->rewrapBody())
    style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNormal);
  else
    style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNoWrap);
  style->setFontFamily(f.family());
  style->setFontSize(f.pointSize());
  style->setFontUnderline(f.underline());
  style->setFontWeight(f.weight());
  style->setFontItalic(f.italic());

  style=new QStyleSheetItem(styleSheet(), "txt_attachment");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNoWrap);
  style->setFontFamily(f.family());
  style->setFontSize(f.pointSize());
  style->setFontUnderline(f.underline());
  style->setFontWeight(f.weight());
  style->setFontItalic(f.italic());

  setPaper( QBrush( app->backgroundColor() ) );

  QPalette newPalette(palette());
  QColorGroup newColorGroup(newPalette.active());
  newColorGroup.setColor(QColorGroup::Text, app->textColor());
  newColorGroup.setColor(QColorGroup::Link, app->linkColor());
  newPalette.setActive(newColorGroup);
  newColorGroup = newPalette.inactive();
  newColorGroup.setColor(QColorGroup::Text, app->textColor());
  newColorGroup.setColor(QColorGroup::Link, app->linkColor());
  newPalette.setInactive(newColorGroup);
  setPalette(newPalette);

  if(!knGlobals.configManager()->readNewsGeneral()->autoMark())
    t_imer->stop();

  updateContents();
}

QString KNArticleWidget::toHtmlString(const QString &line, bool parseURLs, bool beautification, bool allowRot13, bool strictURLparsing)
{
  QString text,result,enc;
  QRegExp regExp;
  uint len=line.length();
  int matchLen;
  bool forceNBSP=false; //use "&nbsp;" for spaces => workaround for a bug in QTextBrowser

  if (allowRot13 && r_ot13)
    text = KNHelper::rot13(line);
  else
    text = line;

  if (!knGlobals.configManager()->readNewsViewer()->interpretFormatTags())
    beautification=false;

  int lastReplacement=-1;
  for(uint idx=0; idx<len; idx++){

    switch(text[idx].latin1()) {

      case '\r':  lastReplacement=idx; break;
      case '\n':  lastReplacement=idx; result+="<br>"; break;
      case '<' :  lastReplacement=idx; result+="&lt;"; break;
      case '>' :  lastReplacement=idx; result+="&gt;"; break;
      case '&' :  lastReplacement=idx; result+="&amp;"; break;
      case '"' :  lastReplacement=idx; result+="&quot;"; break;
      case '\t':  lastReplacement=idx; result+="&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"; break;  // tab == 8 spaces

      case 32 :
        if(text[idx+1].latin1()==32) {
          while(text[idx].latin1()==32) {
            result+="&nbsp;";
            idx++;
          }
          idx--;
          forceNBSP=true; // force &nbsp; for the rest of this line
        } else
          if(idx==0 || forceNBSP) {
            result+="&nbsp;";
            forceNBSP=true; // force &nbsp; for the rest of this line
          } else
            result+=' ';
        lastReplacement=idx;
        break;

      case '@' :            // email-addresses or message-ids
        if (parseURLs) {
          uint startIdx = idx;
          // move backwards to the begin of the address, stop when
          // the end of the last replacement is reached. (

          while ((startIdx>0) && (startIdx>(uint)(lastReplacement+1)) && (text[startIdx-1]!=' ') && (text[startIdx-1]!='\t')
                                                            && (text[startIdx-1]!=',') && (text[startIdx-1]!='<')
                                                            && (text[startIdx-1]!='>') && (text[startIdx-1]!='(')
                                                            && (text[startIdx-1]!=')') && (text[startIdx-1]!='[')
                                                            && (text[startIdx-1]!=']') && (text[startIdx-1]!='{')
                                                            && (text[startIdx-1]!='}') )
            startIdx--;

          regExp.setPattern("[^\\s<>\\(\\)\"\\[\\]\\{\\}]+");
          if (regExp.search(text,startIdx)!=-1) {
            matchLen = regExp.matchedLength();
            while (true) {
              if (text[startIdx+matchLen-1]=='.')   // remove trailing dot
                matchLen--;
              else if (text[startIdx+matchLen-1]==',')   // remove trailing comma
                matchLen--;
              else if (text[startIdx+matchLen-1]==':')   // remove trailing colon
                matchLen--;
              else
                break;
            }
            if (matchLen < 3)
              result+=text[idx];
            else {
              result.remove(result.length()-(idx-startIdx), idx-startIdx);
              result+=QString::fromLatin1("<a href=\"addrOrId://") + toHtmlString(text.mid(startIdx,matchLen),false,false,false) +
                    QString::fromLatin1("\">") + toHtmlString(text.mid(startIdx,matchLen),false,false,false) + QString::fromLatin1("</a>");
              idx=startIdx+matchLen-1;
              lastReplacement=idx;
            }
            break;
          }
        }
        result+=text[idx];
        break;

      case 'h' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'h'
          regExp.setPattern("https?://[^\\s<>\\(\\)\"\\[\\]\\{\\}]+");
          if (regExp.search(text,idx)==(int)idx) {
            matchLen = regExp.matchedLength();
            while (true) {
              if (text[idx+matchLen-1]=='.')   // remove trailing dot
                matchLen--;
              else if (text[idx+matchLen-1]==',')   // remove trailing comma
                matchLen--;
              else if (text[idx+matchLen-1]==':')   // remove trailing colon
                matchLen--;
              else
                break;
            }
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case 'w' :
        if((parseURLs)&&(!strictURLparsing)&&
           (text[idx+1].latin1()=='w')) {   // don't do all the stuff for every 'w'
          regExp.setPattern("www\\.[^\\s<>\\(\\)\"\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\[\\]\\{\\}]+");
          if (regExp.search(text,idx)==(int)idx) {
            matchLen = regExp.matchedLength();
            while (true) {
              if (text[idx+matchLen-1]=='.')   // remove trailing dot
                matchLen--;
              else if (text[idx+matchLen-1]==',')   // remove trailing comma
                matchLen--;
              else if (text[idx+matchLen-1]==':')   // remove trailing colon
                matchLen--;
              else
                break;
            }
            result+=QString::fromLatin1("<a href=\"http://") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case 'f' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'f'
          regExp.setPattern("ftp://[^\\s<>\\(\\)\"\\[\\]\\{\\}]+");
          if (regExp.search(text,idx)==(int)idx) {
            matchLen = regExp.matchedLength();
            while (true) {
              if (text[idx+matchLen-1]=='.')   // remove trailing dot
                matchLen--;
              else if (text[idx+matchLen-1]==',')   // remove trailing comma
                matchLen--;
              else if (text[idx+matchLen-1]==':')   // remove trailing colon
                matchLen--;
              else
                break;
            }
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
          if (!strictURLparsing) {
            regExp.setPattern("ftp\\.[^\\s<>\\(\\)\"\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\[\\]\\{\\}]+");
            if (regExp.search(text,idx)==(int)idx) {
              matchLen = regExp.matchedLength();

              while (true) {
                if (text[idx+matchLen-1]=='.')   // remove trailing dot
                  matchLen--;
                else if (text[idx+matchLen-1]==',')   // remove trailing comma
                  matchLen--;
                else if (text[idx+matchLen-1]==':')   // remove trailing colon
                  matchLen--;
                else
                  break;
              }

              result+=QString::fromLatin1("<a href=\"ftp://") + text.mid(idx,matchLen) +
                      QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
              idx+=matchLen-1;
              lastReplacement=idx;
              break;
            }
          }
        }
        result+=text[idx];
        break;

      case 'n' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='e')) {   // don't do all the stuff for every 'e'
          regExp.setPattern("news:[^\\s<>\\(\\)\"\\[\\]\\{\\}]+");
          if (regExp.search(text,idx)==(int)idx) {
            matchLen = regExp.matchedLength();

            while (true) {
              if (text[idx+matchLen-1]=='.')   // remove trailing dot
                matchLen--;
              else if (text[idx+matchLen-1]==',')   // remove trailing comma
                matchLen--;
              else if (text[idx+matchLen-1]==':')   // remove trailing colon
                matchLen--;
              else
                break;
            }

            // encode the given url, because a "news:foo" link has a different meaning than "news://foo",
            // and QTextBrowser merges both cases
            enc = text.mid(idx,matchLen);
            QUrl::encode(enc);
            enc.replace(QRegExp("/"), "%2F");

            result+=QString::fromLatin1("<a href=\"news://") + enc +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case 'm' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='a')) {   // don't do all the stuff for every 'm'
          regExp.setPattern("mailto:[^\\s<>\\(\\)\"\\[\\]\\{\\}]+");
          if (regExp.search(text,idx)==(int)idx) {
            matchLen = regExp.matchedLength();

            while (true) {
              if (text[idx+matchLen-1]=='.')   // remove trailing dot
                matchLen--;
              else if (text[idx+matchLen-1]==',')   // remove trailing comma
                matchLen--;
              else if (text[idx+matchLen-1]==':')   // remove trailing colon
                matchLen--;
              else
                break;
            }

            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case '_' :
      case '/' :
      case '*' :
        if(beautification) {
          regExp=QRegExp( QString("\\%1[^\\s%2]+\\%3").arg(text[idx]).arg(text[idx]).arg(text[idx]) );
          if (regExp.search(text,idx)==(int)idx) {
            matchLen = regExp.matchedLength();
            if (( matchLen >= 3 ) &&
                ((idx==0)||text[idx-1].isSpace()||(text[idx-1] == '(')) &&
                ((idx+matchLen==len)||text[idx+matchLen].isSpace()||(text[idx+matchLen]==',')||
                 (text[idx+matchLen]=='.')||(text[idx+matchLen]==')'))) {
              switch (text[idx].latin1()) {
                case '_' :
                  result+=QString("<u>%1</u>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification,strictURLparsing));
                  break;
                case '/' :
                  result+=QString("<i>%1</i>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification,strictURLparsing));
                  break;
                case '*' :
                  result+=QString("<b>%1</b>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification,strictURLparsing));
                  break;
              }
              idx+=matchLen-1;
              lastReplacement=idx;
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

bool KNArticleWidget::findExec( const QString & exec)
{
    QString path = QString::fromLocal8Bit(getenv("PATH")) + QString::fromLatin1(":/usr/sbin");
    QString exe = KStandardDirs::findExe( exec, path );
    if ( exe.isEmpty())
    {
        KMessageBox::error( this,  i18n("%1 not found").arg(exec ));
        return false;
    }
    return true;
}

void KNArticleWidget::addBookmarks(const QString &url)
{
    if(url.isEmpty()) return;
    QString filename = locateLocal( "data", QString::fromLatin1("konqueror/bookmarks.xml") );
    KBookmarkManager *bookManager = KBookmarkManager::managerForFile( filename,
                                                                      false );
    KBookmarkGroup group = bookManager->root();
    group.addBookmark( bookManager, url, KURL( url ) );
    bookManager->save();
}

void KNArticleWidget::openURL(const QString &url)
{
    if(url.isEmpty()) return;

    if (knGlobals.configManager()->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTdefault)
        (void) new KRun(KURL( url ));
    if (knGlobals.configManager()->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTkonq)
        kapp->invokeBrowser(url);
    else if (knGlobals.configManager()->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTnetscape){
        QString exec("netscape");
        if ( findExec( exec))
        {
            KProcess proc;
            proc << exec;

            struct stat info;      // QFileInfo is unable to handle netscape's broken symlink
            if (lstat((QDir::homeDirPath()+"/.netscape/lock").local8Bit(),&info)==0)
                proc << "-remote" << QString("openURL(%1)").arg(url);
            else
                proc << url;

            proc.start(KProcess::DontCare);
        }
    }
    else if (knGlobals.configManager()->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTmozilla){
        QString exec("mozilla");
        if ( findExec( exec))
        {
            KProcess proc;
            proc << exec;
            proc << url;
            proc.start(KProcess::DontCare);
        }
    }
    else if (knGlobals.configManager()->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTopera){
        QString exec("opera");
        if ( findExec( exec))
        {
            KProcess proc;
            proc << exec;
            proc << QString("-page=%1").arg(url);
            proc << url;
            proc.start(KProcess::DontCare);
        }
    } else {
        KProcess proc;

        QStringList command = QStringList::split(' ',knGlobals.configManager()->readNewsViewer()->browserCommand());
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
  KMime::Content *a=a_tt->at(id);

  if(a)
    knGlobals.articleManager()->saveContentToFile(a,this);
  else KMessageBox::error(this, i18n("Internal error: Malformed identifier."));
}


void KNArticleWidget::openAttachment(int id)
{
 KMime::Content *a=a_tt->at(id);

 if(a)
   knGlobals.articleManager()->openContent(a);
 else KMessageBox::error(this, i18n("Internal error: Malformed identifier."));
}


bool KNArticleWidget::inlinePossible(KMime::Content *c)
{
  KMime::Headers::ContentType *ct=c->contentType();
  return ( ct->isText() || ct->isImage() );
}


void KNArticleWidget::showBlankPage()
{
  kdDebug(5003) << "KNArticleWidget::showBlankPage()" << endl;

  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  // restore background color, might have been changed by html article
  setPaper(QBrush(knGlobals.configManager()->appearance()->backgroundColor()));

  setText(QString::null);

  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  a_rticle=0;
  delete a_tt;
  a_tt=0;
  h_tmlDone=false;
  a_ctSave->setEnabled(false);
  a_ctPrint->setEnabled(false);
  a_ctCopy->setEnabled(false); //probaly not needed, but who knows ;-)
  a_ctSelAll->setEnabled(false);
  a_ctReply->setEnabled(false);
  a_ctRemail->setEnabled(false);
  a_ctForward->setEnabled(false);
  a_ctCancel->setEnabled(false);
  a_ctSupersede->setEnabled(false);
  a_ctVerify->setEnabled(false);
  a_ctToggleFullHdrs->setEnabled(false);
  a_ctToggleRot13->setEnabled(false);
  a_ctSetCharset->setEnabled(false);
  a_ctSetCharsetKeyb->setEnabled(false);
  a_ctViewSource->setEnabled(false);
}


void KNArticleWidget::showErrorMessage(const QString &s)
{
  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  // restore background color, might have been changed by html article
  setPaper(QBrush(knGlobals.configManager()->appearance()->backgroundColor()));

  QString errMsg=s;
  errMsg.replace(QRegExp("\n"),QString("<br>"));  // error messages can contain html-links, but are plain text otherwise
  QString msg="<qt>"+i18n("<bodyblock><b><font size=+1 color=red>An error occurred.</font></b><hr><br>")+errMsg+"</bodyblock></qt>";
  setText(msg);

  // mark article as read, typically the article is expired on the server, so its
  // impossible to read it later anyway.
  if(knGlobals.configManager()->readNewsGeneral()->autoMark() &&
     a_rticle && a_rticle->type()==KMime::Base::ATremote && !a_rticle->isOrphant()) {
    KNRemoteArticle::List l;
    l.append((static_cast<KNRemoteArticle*>(a_rticle)));
    knGlobals.articleManager()->setRead(l, true);
  }

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
  a_ctVerify->setEnabled(false);
  a_ctToggleFullHdrs->setEnabled(false);
  a_ctToggleRot13->setEnabled(false);
  a_ctSetCharset->setEnabled(false);
  a_ctSetCharsetKeyb->setEnabled(false);
  a_ctViewSource->setEnabled(false);
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
  if(a_rticle && a_rticle->isOrphant())
    delete a_rticle; //don't leak orphant articles

  a_rticle=a;
  h_tmlDone=false;
  r_ot13=false;
  a_ctToggleRot13->setChecked(false);

  t_imer->stop();

  if(!a)
    showBlankPage();
  else {
    if(a->hasContent()) { //article is already loaded => just show it
      createHtmlPage();
    } else {
      if( !knGlobals.articleManager()->loadArticle(a_rticle) )
        articleLoadError( a, i18n("Unable to load the article.") );
      else if(a->hasContent() && !(a->type()==KMime::Base::ATremote)) // try again, but not for remote articles
        createHtmlPage();
    }
  }
}


void KNArticleWidget::slotKeyUp()
{
  scrollBy( 0, -10 );
}


void KNArticleWidget::slotKeyDown()
{
  scrollBy( 0, 10 );
}


void KNArticleWidget::slotKeyPrior()
{
  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;
  scrollBy( 0, -visibleHeight()+offs);
}


void KNArticleWidget::slotKeyNext()
{
  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;
  scrollBy( 0, visibleHeight()-offs);
}


void KNArticleWidget::processJob(KNJobData *j)
{
  if(j->type()==KNJobData::JTfetchSource) {
    KNRemoteArticle *a=static_cast<KNRemoteArticle*>(j->data());
    if(!j->canceled()) {
      QString html;
      if (!j->success())
        html= i18n("<b><font size=+1 color=red>An error occurred.</font></b><hr><br>")+j->errorString();
      else
        html= QString("%1<br>%2").arg(toHtmlString(a->head(),false,false)).arg(toHtmlString(a->body(),false,false));

      new KNSourceViewWindow(html);
    }
    delete j;
    delete a;
  }
  else
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

  if ((f_orceCS!=a_rticle->forceDefaultCS())||
      (f_orceCS && (a_rticle->defaultCharset()!=o_verrideCS))) {
    a_rticle->setDefaultCharset(o_verrideCS);
    a_rticle->setForceDefaultCS(f_orceCS);
  }

  KNConfig::Appearance *app=knGlobals.configManager()->appearance();
  KNConfig::ReadNewsViewer *rnv=knGlobals.configManager()->readNewsViewer();

  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  // restore background color, might have been changed by html article
  setPaper(QBrush(app->backgroundColor()));

  //----------------------------------- <Header> ---------------------------------------

  QString headerHtml;
  int headerLines=0;
  int headerCols=3;

  QString xfhead;
  KMime::Headers::Base *temp = a_rticle->getHeaderByType("X-Face");
  if (temp)
    xfhead = temp->asUnicodeString();
  QString xface = "";
  if ( !xfhead.isEmpty() )
  {
    KPIM::KXFace xf;
    xf.toImage(xfhead).save(t_mpFile->name(), "PNG");
    xface = QString::fromLatin1(
        "<td rowspan=\"4\" width=\"48\" height=\"48\"><img border=\"1\" src=\"%1\"></td>")
        .arg(t_mpFile->name());
    ++headerCols;
  }

  if(a_ctToggleFullHdrs->isChecked()) {
    QCString head = a_rticle->head();
    KMime::Headers::Generic *header=0;

    while(!head.isEmpty()) {
      header = a_rticle->getNextHeader(head);
      if (header) {
        if (headerLines > 0)
          headerHtml += "<tr>";
        headerHtml+=QString("<td align=right><articlefont><b>%1</b></articlefont></td><td width=\"100%\"><articlefont>%2</articlefont></td>%3</tr>")
                      .arg(toHtmlString(header->type())+":")
                      .arg(toHtmlString(header->asUnicodeString(),true, false, false, true))
                      .arg(xface);
        xface = "";
        delete header;
        headerLines++;
      }
    }
  }
  else {
    KMime::Headers::Base *hb;
    KNDisplayedHeader *dh;
    KNConfig::DisplayedHeaders::Iterator it=knGlobals.configManager()->displayedHeaders()->iterator();
    for(; it.current(); ++it) {
      dh=it.current();
      hb=a_rticle->getHeaderByType(dh->header().latin1());
      if(!hb) continue; //header not found

      if (headerLines > 0)
          headerHtml += "<tr>";

      if(dh->hasName()) {
        headerHtml += QString("<td align=\"right\"><articlefont>") + i18n("%1%2:%3")
        .arg(dh->nameOpenTag()).arg(toHtmlString(dh->translatedName()))
        .arg(dh->nameCloseTag()) + "</articlefont></td><td width=\"100%\"><articlefont>";
      }
      else
        headerHtml+="<td colspan=\"2\" width=\"100%\"><articlefont>";

      headerHtml+=dh->headerOpenTag();

      if(hb->is("From")) {
        headerHtml+=QString("<a href=\"internal://author\">%1</a>")
                .arg(toHtmlString(hb->asUnicodeString()));
      } else if(hb->is("Date")) {
        KMime::Headers::Date *date=static_cast<KMime::Headers::Date*>(hb);
        headerHtml+=toHtmlString(KGlobal::locale()->formatDateTime(date->qdt(), false, true));
      } else
        headerHtml+=toHtmlString(hb->asUnicodeString(),true, false, false, true);

      headerHtml += dh->headerCloseTag()+"</articlefont></td>"+xface+"</tr>";
      xface = "";
      headerLines++;
    }
  }

  QString html;

  if (rnv->showHeaderDecoration()) {
    html=QString("<qt><table width=\"100%\" cellpadding=\"0\" cellspacing=\"1\">");

    if (headerLines > 0) {
      html+=QString("<tr><td width=\"40\" bgcolor=\"%1\" rowspan=\"%2\"></td>"+headerHtml)
           .arg(app->headerDecoHexcode()).arg(headerLines);
      html+=QString("<tr><td colspan=\"%1\" bgcolor=\"%2\"><articlefont>")
          .arg(headerCols).arg(app->headerDecoHexcode());
    } else {
      html+=QString("<tr><td bgcolor=\"%1\"><articlefont>")
          .arg(app->headerDecoHexcode());
    }

    //References
    KMime::Headers::References *refs=a_rticle->references(false);
    if(a_rticle->type()==KMime::Base::ATremote && refs) {
      int refCnt=refs->count(), i=1;
      QCString id = refs->first();
      id = id.mid(1, id.length()-2);  // remove <>
      html+=QString("<b>%1</b>").arg(i18n("References:"));

      while (i <= refCnt) {
        html+=" <a href=\"news://news%3A"+QString("%1\">%2</a>").arg(id).arg(i);
        id = refs->next();
        id = id.mid(1, id.length()-2);  // remove <>
        i++;
      }
    }
    else html+=i18n("no references");

    html+="</articlefont></td></tr></table>";
  } else {   // !rnv->showHeaderDecoration()
    if (headerLines > 0)
      html="<qt><table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\"><tr>"+headerHtml+"</table>";
    else
      html="<qt>";
  }

  KMime::Content *text=a_rticle->textContent();
  if(text && !canDecode8BitText(text->contentType()->charset())) {
    html+=QString("<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\"><tr><td bgcolor=\"red\"><articlefont><font color=\"black\">%1</font></articlefont></td></tr></table>")
          .arg(i18n("Unknown charset. Default charset is used instead."));
    kdDebug(5003) << "KNArticleWidget::createHtmlPage() : unknown charset = " << text->contentType()->charset() << " not available!" << endl;
  }

  //----------------------------------- </Header> --------------------------------------


  //------------------------------------- <Body> ---------------------------------------

  // if the article is pgp signed and the user asked for verifying the
  // signature, we show a nice header:
  if ( a_rticle->type() == KMime::Base::ATremote ) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rticle);
    Kpgp::Module *pgp = knGlobals.pgp;

    if (knGlobals.configManager()->readNewsGeneral()->autoCheckPgpSigs() || ra->isPgpSigned()) {
      QPtrList<Kpgp::Block> pgpBlocks;
      QStrList nonPgpBlocks;
      Kpgp::Module::prepareMessageForDecryption( ra->body(), pgpBlocks, nonPgpBlocks );
      // Only the first OpenPGP block is verified (if there is one)
      Kpgp::Block* pgpBlock = pgpBlocks.first();
      if( pgpBlock && ( pgpBlock->type() == Kpgp::ClearsignedBlock ) )
        pgpBlock->verify();
      else
        pgpBlock = 0;
      html += "<p>";
      if( !pgpBlock || !pgpBlock->isSigned() ) {
        if (!knGlobals.configManager()->readNewsGeneral()->autoCheckPgpSigs())
          html += "<b>" + i18n("Cannot find a signature in this message.") + "</b>";
      }
      else {
        QString signer = pgpBlock->signatureUserId();
        QCString signerKey = pgpBlock->signatureKeyId();
        html += "<b>";
        if( signer.isEmpty() )
        {
          html += i18n( "Message was signed with unknown key 0x%1." )
                  .arg( signerKey );
          html += "<br />";
          html += i18n( "The validity of the signature cannot be verified." );
        }
        else {
          // determine the validity of the key
          Kpgp::Validity keyTrust;
          if( !signerKey.isEmpty() )
            keyTrust = pgp->keyTrust( signerKey );
          else
            // This is needed for the PGP 6 support because PGP 6 doesn't
            // print the key id of the signing key if the key is known.
            keyTrust = pgp->keyTrust( signer );

          // HTMLize the signer's user id and create mailto: link
          signer = toHtmlString( signer, false, false, false );
          signer = "<a href=\"mailto://" + signer + "\">" + signer + "</a>";

          if( !signerKey.isEmpty() )
            html += i18n( "Message was signed by %1 (Key ID: 0x%2)." )
                    .arg( signer )
                    .arg( signerKey );
          else
            html += i18n( "Message was signed by %1." ).arg( signer );
          html += "<br />";

          if( pgpBlock->goodSignature() )
          {
            switch( keyTrust )
            {
            case Kpgp::KPGP_VALIDITY_UNKNOWN:
              html += i18n( "The signature is valid, but the key's "
                            "validity is unknown." );
                break;
            case Kpgp::KPGP_VALIDITY_MARGINAL:
              html += i18n( "The signature is valid and the key is "
                            "marginally trusted." );
              break;
            case Kpgp::KPGP_VALIDITY_FULL:
              html += i18n( "The signature is valid and the key is "
                            "fully trusted." );
              break;
            case Kpgp::KPGP_VALIDITY_ULTIMATE:
              html += i18n( "The signature is valid and the key is "
                            "ultimately trusted." );
              break;
            default:
              html += i18n( "The signature is valid, but the key is "
                            "untrusted." );
            }
          }
          else // bad signature
          {
            html += i18n("Warning: The signature is bad.");
          }
        }
        html += "</b><br>";
      }
      html += "</p>";
    }
  }

  KMime::Headers::ContentType *ct=a_rticle->contentType();

  //Attachments
  if(!text || ct->isMultipart()) {
    if(a_tt) a_tt->clear();
    else {
      a_tt=new KMime::Content::List;
      a_tt->setAutoDelete(false);
    }

    a_rticle->attachments(a_tt, rnv->showAlternativeContents());
  } else {
    delete a_tt;
    a_tt=0;
  }

  //Partial message
  if(ct->isPartial()) {
    html+=i18n("<br><bodyblock><b>This article has the MIME type &quot;message/partial&quot;, which KNode cannot handle yet.<br>Meanwhile you can save the article as a text file and reassemble it by hand.</b></bodyblock></qt>");
    setText(html);
    h_tmlDone=true;

    //enable actions
    a_ctReply->setEnabled(a_rticle->type()==KMime::Base::ATremote);
    a_ctRemail->setEnabled(a_rticle->type()==KMime::Base::ATremote);
    a_ctForward->setEnabled(true);
    a_ctCancel->setEnabled( (a_rticle->type()==KMime::Base::ATremote) ||
                            (a_rticle->collection()==knGlobals.folderManager()->sent()));
    a_ctSupersede->setEnabled( (a_rticle->type()==KMime::Base::ATremote) ||
                               (a_rticle->collection()==knGlobals.folderManager()->sent()));
    a_ctSave->setEnabled(true);
    a_ctPrint->setEnabled(true);
    a_ctSelAll->setEnabled(true);
    a_ctToggleFullHdrs->setEnabled(true);
    a_ctSetCharset->setEnabled(true);
    a_ctSetCharsetKeyb->setEnabled(true);
    a_ctViewSource->setEnabled(true);

    //start automark-timer
    if(a_rticle->type()==KMime::Base::ATremote && knGlobals.configManager()->readNewsGeneral()->autoMark())
      t_imer->start( (knGlobals.configManager()->readNewsGeneral()->autoMarkSeconds()*1000), true);
    return;
  }

  //body text
  if(text && text->hasContent()) {
    html+="<bodyblock>";
    if(text->contentType()->isHTMLText()) {
      QString htmlTxt;
      text->decodedText(htmlTxt, true, knGlobals.configManager()->readNewsViewer()->removeTrailingNewlines());
      html+=htmlTxt+"</bodyblock>";
    }
    else {
      QChar firstChar;
      int oldLevel=0, newLevel=0;
      unsigned int idx=0;
      bool isSig=false;
      QStringList lines;
      QString line;
      text->decodedText(lines, true, knGlobals.configManager()->readNewsViewer()->removeTrailingNewlines());
      QString quoteChars = rnv->quoteCharacters().simplifyWhiteSpace();
      if (quoteChars.isEmpty()) quoteChars = ">";

      for(QStringList::Iterator it=lines.begin(); it!=lines.end(); ++it) {
        line=(*it);
        if(!line.isEmpty()) {
          if(!isSig && line=="-- ") {
            isSig=true;
            if(newLevel>0) {
              newLevel=0;
              html+="</font>";
            }
            if(rnv->showSignature()) {
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
              else if(quoteChars.find(firstChar)!=-1) { idx++; newLevel++; }
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
            html+=toHtmlString(line,true,true,true)+"<br>";
          }
          else
            html+=toHtmlString(line,true,false,true)+"<br>";
        }
        else
          html+="<br>";
      }
      if(newLevel>0) html+="</font>";
    }

    html+="</bodyblock>";
  }

  //------------------------------------- </Body> --------------------------------------


  //----------------------------------  <Attachments> ----------------------------------

  if(a_tt) {
    int attCnt=0;
    QString path;
    if(!a_tt->isEmpty()) {
      html+="<br><table border width=\"100%\">";
      html+=QString("<tr><td align=\"center\"><articlefont><b>%1</b></articlefont></td><td align=\"center\"><articlefont><b>%2</b></articlefont></td><td align=\"center\"><articlefont><b>%3</b></articlefont></td></tr>")
                    .arg(i18n("name")).arg(i18n("mime-type")).arg(i18n("description"));

      for(KMime::Content *var=a_tt->first(); var; var=a_tt->next()) {
        ct=var->contentType();
        QString att_name = ct->name();
        if ( att_name.isEmpty() ) att_name = i18n("unnamed" );
        // if att_name consists of only whitespace replace them by underscores
        if ( ( uint )att_name.contains( ' ' ) == att_name.length() )
          att_name.replace( QRegExp( " ", true, true ), "_" );
        html+=QString("<tr><td align=center><articlefont><a href=\"internal://att=%1\">%2</a></articlefont></td><td align=center><articlefont>%3</articlefont></td><td align=center><articlefont>%4</articlefont></td></tr>")
              .arg(attCnt)
              .arg( att_name )
              .arg(ct->mimeType())
              .arg(toHtmlString(var->contentDescription()->asUnicodeString()));

        if(rnv->showAttachmentsInline() && inlinePossible(var)) {
          html+="<tr><td colspan=3>";
          if(ct->isImage()) { //image
            path=QString::number(attCnt)+"_"+QString::number((unsigned long)(a_rticle));
            f_actory->setData(path,new KNMimeSource(var->decodedContent(),ct->mimeType()));
            html+=QString("<a href=\"internal://att=%1\"><img src=\"%2\"></a>").arg(attCnt).arg(path);
          }
          else { //text
            QString tmp;
            var->decodedText(tmp);
            if(ct->isHTMLText())
              html+=tmp;
            else
              html+="<txt_attachment>"+toHtmlString(tmp,true)+"</txt_attachment>";
          }
          html+="</td></tr>";
        }
        attCnt++;
      }
      html+="</table>";
    }
  }

  //----------------------------------  </Attachments> ---------------------------------

  //display html
  html+="</qt>";
  clear();
  setText(html);
  h_tmlDone=true;

  //enable actions
  a_ctSave->setEnabled(true);
  a_ctPrint->setEnabled(true);
  a_ctSelAll->setEnabled(true);

  a_ctReply->setEnabled(a_rticle->type()==KMime::Base::ATremote);
  a_ctRemail->setEnabled(a_rticle->type()==KMime::Base::ATremote);
  a_ctForward->setEnabled(true);
  a_ctCancel->setEnabled( (a_rticle->type()==KMime::Base::ATremote) ||
                          (a_rticle->collection()==knGlobals.folderManager()->sent()));
  a_ctSupersede->setEnabled( (a_rticle->type()==KMime::Base::ATremote) ||
                             (a_rticle->collection()==knGlobals.folderManager()->sent()));
  a_ctVerify->setEnabled(true);
  a_ctToggleFullHdrs->setEnabled(true);
  a_ctToggleRot13->setEnabled(true);
  a_ctSetCharset->setEnabled(true);
  a_ctSetCharsetKeyb->setEnabled(true);
  a_ctViewSource->setEnabled(true);

  //start automark-timer
  if(a_rticle->type()==KMime::Base::ATremote && knGlobals.configManager()->readNewsGeneral()->autoMark())
    t_imer->start( (knGlobals.configManager()->readNewsGeneral()->autoMarkSeconds()*1000), true);
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
  kdDebug(5003)<<" target :"<<a<<endl;
  if(a.left(17)=="internal://author") {
    type=ATauthor;
  }
  else if(a.left(15)=="internal://att=") {
    if(a.right(1) == "/")
      target=a.mid(15, a.length()-16);
    else
      target=a.mid(15, a.length()-15);
    type=ATattachment;
  }
  else if(a.left(7).lower()=="http://" || a.left(8).lower() == "https://" ||
          a.left(6).lower()=="ftp://") {
    target=a;
    type=ATurl;
  }
  else if(a.left(7).lower()=="news://") {
    // news urls are encoded, because a "news:foo" link has a different meaning than "news://foo",
    // and QTextBrowser merges both cases
    target=a.mid(7, a.length()-7);
    QUrl::decode(target);
    type=ATnews;
  }
  else if(a.left(7)=="mailto:") {
    target=a.mid(7, a.length()-7);
    type=ATmailto;
  }
  else if(a.left(11)=="addrOrId://") {

    target=a.mid(11, a.length()-11);

      if (a_rticle->collection()->type()!=KNArticleCollection::CTgroup)
        type=ATmailto;
      else {
        switch (KMessageBox::warningYesNoCancel(this, i18n("<qt>Do you want treat the selected text as an <b>email address</b> or a <b>message-id</b>?</qt>"),
                                                i18n("Address or ID"), i18n("&Email"), i18n("&Message-Id"))) {
          case KMessageBox::Yes:  type=ATmailto;
                                  break;
          case KMessageBox::No:   type=ATmsgid;
                                  break;
          default:                type=ATunknown;
        }
      }
  }

  if((button==LeftButton)||(button==MidButton)) {
    KNGroup *g;
    KNRemoteArticle *a;
    KNArticleWindow *awin;
    KMime::Headers::AddressField adr(a_rticle);
    switch(type) {
      case ATauthor:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : mailto author" << endl;
        knGlobals.artFactory->createMail(a_rticle->from());
      break;
      case ATmsgid:
        if (target.endsWith("/"))
		          target.truncate(target.length()-1);
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : message-id " << target << endl;

        if (a_rticle->collection()->type()!=KNArticleCollection::CTgroup)  // we need a group for doing network stuff
          break;

        g=static_cast<KNGroup*>(a_rticle->collection());
        if (target.find(QRegExp("<*>",false,true))==-1)   // add "<>" when necessary
          target = QString("<%1>").arg(target);

        if(!KNArticleWindow::raiseWindowForArticle(target.latin1())) { //article not yet opened
          a=g->byMessageId(target.latin1());
          if(a) {
            //article found in KNGroup
            awin=new KNArticleWindow(a);
            awin->show();
          }
          else {
            //article not in local group => create a new (orphant) article.
            //It will be deleted by the displaying widget/window
            a=new KNRemoteArticle(g); //we need "g" to access the nntp-account
            a->messageID()->from7BitString(target.latin1());
            awin=new KNArticleWindow(a);
            awin->show();
          }
        }
      break;
      case ATattachment:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : attachment " << target << endl;
        if(knGlobals.configManager()->readNewsViewer()->openAttachmentsOnClick())
          openAttachment(target.toInt());
        else
          saveAttachment(target.toInt());
      break;
      case ATurl:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : url " << target << endl;
        openURL(target);
      break;
      case ATnews:
        if (target.endsWith("/"))
          target.truncate(target.length()-1);
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : news-url " << target << endl;
        knGlobals.top->openURL(KURL( target ));
      break;
      case ATmailto:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : mailto author" << endl;
        adr.fromUnicodeString(target,"");
        knGlobals.artFactory->createMail(&adr);
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
    else if ( type==ATauthor || type == ATmailto ) {
        if ( type == ATauthor )
          target = a_rticle->from()->asUnicodeString();
        switch( u_mailtoPopup->exec( *p ) ) {
        case PUP_ADDRESSBOOK:
            addAddressbook(target);
            break;
        case PUP_OPENADDRESSBOOK:
            openAddressbook(target);
            break;
        case PUP_COPYTOCLIPBOARD:
            QApplication::clipboard()->setText(target, QClipboard::Clipboard);
            QApplication::clipboard()->setText(target, QClipboard::Selection);
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
          QApplication::clipboard()->setText(target, QClipboard::Clipboard);
          QApplication::clipboard()->setText(target, QClipboard::Selection);
        break;
      case PUP_ADDBOOKMARKS:
          addBookmarks(target );
          break;

      }
    }
  }
}

void KNArticleWidget::addAddressbook(const QString & target)
{
    KAddrBookExternal::addEmail( target, this );
}

void KNArticleWidget::openAddressbook(const QString & target)
{
    KAddrBookExternal::openEmail( target, target, this );
}

void KNArticleWidget::slotSave()
{
  kdDebug(5003) << "KNArticleWidget::slotSave()" << endl;
  if(a_rticle)
    knGlobals.articleManager()->saveArticleToFile(a_rticle,this);
}


void KNArticleWidget::slotPrint()
{
  kdDebug(5003) << "KNArticleWidget::slotPrint()" << endl;
  KPrinter *printer=new KPrinter();

  // FIXME: Add a better caption to the printingdialog
  if(printer->setup(this, i18n("Print Article"))) {

    QPaintDeviceMetrics metrics(printer);
    QPainter p;

    const int margin=20;
    int yPos=0;
    KMime::Headers::Base *hb=0;
    QString text;
    QString hdr;

    p.begin(printer);
    p.setFont( QFont(font().family(), 12, QFont::Bold) );
    QFontMetrics fm=p.fontMetrics();

    KNDisplayedHeader *dh;
    KNConfig::DisplayedHeaders::Iterator it=knGlobals.configManager()->displayedHeaders()->iterator();
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
        dh=++it;

    }

    yPos+=fm.lineSpacing()+10;


    QPen pen(QColor(0,0,0), 2);
    p.setPen(pen);

    p.drawLine(10, yPos+margin, metrics.width(), yPos+margin);
    yPos+=2*fm.lineSpacing();

    p.setFont( QFont(font().family(), 10, QFont::Normal) );
    fm=p.fontMetrics();

    QStringList lines;
    KMime::Content *txt=a_rticle->textContent();

    if(txt) {
      txt->decodedText(lines, true, knGlobals.configManager()->readNewsViewer()->removeTrailingNewlines());
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
}


void KNArticleWidget::slotReply()
{
  kdDebug(5003) << "KNArticleWidget::slotReply()" << endl;
  if(a_rticle && a_rticle->type()==KMime::Base::ATremote)
    knGlobals.artFactory->createReply(static_cast<KNRemoteArticle*>(a_rticle), selectedText(), true, false);
}


void KNArticleWidget::slotRemail()
{
  kdDebug(5003) << "KNArticleWidget::slotRemail()" << endl;
  if(a_rticle && a_rticle->type()==KMime::Base::ATremote)
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

  // ok, this is a hack
  if (knGlobals.artWidget == this)
    knGlobals.configManager()->readNewsViewer()->setShowFullHdrs(!knGlobals.configManager()->readNewsViewer()->showFullHdrs());
  updateContents();
}


void KNArticleWidget::slotToggleRot13()
{
  int x = contentsX();
  int y = contentsY();
  r_ot13=!r_ot13;
  updateContents();
  setContentsPos(x, y);
}


void KNArticleWidget::slotToggleFixedFont()
{
  int x = contentsX();
  int y = contentsY();
  // ok, this is a hack
  if (knGlobals.artWidget == this)
    knGlobals.configManager()->readNewsViewer()->setUseFixedFont(!knGlobals.configManager()->readNewsViewer()->useFixedFont());
  applyConfig();
  setContentsPos(x, y);
}


void KNArticleWidget::slotSetCharset(const QString &s)
{
  if(s.isEmpty())
    return;

  if (s == i18n("Automatic")) {
    f_orceCS=false;
    o_verrideCS=KMime::Headers::Latin1;
  } else {
    f_orceCS=true;
    o_verrideCS=s.latin1();
  }

  if (a_rticle && a_rticle->hasContent()) {
    a_rticle->setDefaultCharset(o_verrideCS);  // the article will choose the correct default,
    a_rticle->setForceDefaultCS(f_orceCS);     // when we disable the overdrive
    createHtmlPage();
  }
}


void KNArticleWidget::slotSetCharsetKeyboard()
{
  int newCS = KNHelper::selectDialog(this, i18n("Select Charset"), a_ctSetCharset->items(), a_ctSetCharset->currentItem());
  if (newCS != -1) {
    a_ctSetCharset->setCurrentItem(newCS);
    slotSetCharset(*(a_ctSetCharset->items().at(newCS)));
  }
}


void KNArticleWidget::slotViewSource()
{
  kdDebug(5003) << "KNArticleWidget::slotViewSource()" << endl;
  if (a_rticle && a_rticle->type()==KMime::Base::ATlocal && a_rticle->hasContent()) {
    new KNSourceViewWindow(toHtmlString(a_rticle->encodedContent(false),false,false));
  } else {
    if (a_rticle && a_rticle->type()==KMime::Base::ATremote) {
      KNGroup *g=static_cast<KNGroup*>(a_rticle->collection());
      KNRemoteArticle *a=new KNRemoteArticle(g); //we need "g" to access the nntp-account
      a->messageID(true)->from7BitString(a_rticle->messageID()->as7BitString(false));
      a->lines(true)->from7BitString(a_rticle->lines(true)->as7BitString(false));
      a->setArticleNumber(static_cast<KNRemoteArticle*>(a_rticle)->articleNumber());
      emitJob( new KNJobData(KNJobData::JTfetchSource, this, g->account(), a));
    }
  }
}


void KNArticleWidget::slotTimeout()
{
  if(a_rticle && a_rticle->type()==KMime::Base::ATremote && !a_rticle->isOrphant()) {
    KNRemoteArticle::List l;
    l.append((static_cast<KNRemoteArticle*>(a_rticle)));
    knGlobals.articleManager()->setRead(l, true);
  }
}


void KNArticleWidget::slotVerify()
{
  //  knGlobals.artManager->verifyPGPSignature(a_rticle);
  //create a PGP object and check if the posting is signed
  if (a_rticle->type() == KMime::Base::ATremote) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rticle);
    ra->setPgpSigned(true);
    updateContents();
  }
}


//--------------------------------------------------------------------------------------


QPtrList<KNArticleWidget>* KNArticleWidget::i_nstances = 0;

QPtrList<KNArticleWidget>* KNArticleWidget::instances()
{
  if(!i_nstances)
    i_nstances = new QPtrList<KNArticleWidget>();
  return i_nstances;
}


void KNArticleWidget::configChanged()
{
  for(KNArticleWidget *i=instances()->first(); i; i=instances()->next())
    i->applyConfig();
}


bool KNArticleWidget::articleVisible(KNArticle *a)
{
  for(KNArticleWidget *i=instances()->first(); i; i=instances()->next())
    if(a==i->article())
      return true;
  return false;
}


void KNArticleWidget::articleRemoved(KNArticle *a)
{
  for(KNArticleWidget *i=instances()->first(); i; i=instances()->next())
    if(a==i->article())
      i->showBlankPage();
}


void KNArticleWidget::articleChanged(KNArticle *a)
{
  for(KNArticleWidget *i=instances()->first(); i; i=instances()->next())
    if(a==i->article())
      i->updateContents();
}


void KNArticleWidget::articleLoadError(KNArticle *a, const QString &error)
{
  for(KNArticleWidget *i=instances()->first(); i; i=instances()->next())
    if(a==i->article())
      i->showErrorMessage(error);
}


void KNArticleWidget::collectionRemoved(KNArticleCollection *c)
{
  for(KNArticleWidget *i=instances()->first(); i; i=instances()->next())
    if(i->article() && i->article()->collection()==c)
      i->showBlankPage();
}


void KNArticleWidget::cleanup()
{
  for(KNArticleWidget *i=instances()->first(); i; i=instances()->next())
    i->setArticle(0); //delete orphant articles => avoid crash in destructor
}

//=============================================================================================================


// some standard headers
static const char *predef[] = { "Approved","Content-Transfer-Encoding","Content-Type","Control","Date","Distribution",
                                "Expires","Followup-To","From","Lines","Mail-Copies-To","Message-ID","Mime-Version","NNTP-Posting-Host",
                                "Newsgroups","Organization","Path","References","Reply-To", "Sender","Subject",
                                "Supersedes","To", "User-Agent","X-Mailer","X-Newsreader","X-No-Archive","XRef",0 };

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
  i18n("collection of article headers","Mail-Copies-To");
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


// *tries* to translate the name
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


// *tries* to retranslate the name to english
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
  const char *tokens[] = {  "<big>","</big>","<b>","</b>",
                            "<i>","</i>","<u>","</u>" };

  for(int i=0; i<4; i++) t_ags[i]=QString::null;

  if(f_lags.at(0)) {    // <big>
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
