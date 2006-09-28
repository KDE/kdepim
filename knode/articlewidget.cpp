/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <qbuffer.h>
#include <qclipboard.h>
#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpaintdevicemetrics.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qtimer.h>

#include <kaction.h>
#include <kaddrbook.h>
#include <kapplication.h>
#include <kbookmarkmanager.h>
#include <kcharsets.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>

#include <libemailfunctions/email.h>
#include <libemailfunctions/kasciistringtools.h>

#include <libkpgp/kpgp.h>
#include <libkpgp/kpgpblock.h>

#include <libkdepim/kfileio.h>
#include <libkdepim/kxface.h>
#include <libkdepim/linklocator.h>

#include "articlewidget.h"
#include "csshelper.h"
#include "knarticle.h"
#include "knarticlecollection.h"
#include "knarticlefactory.h"
#include "knarticlemanager.h"
#include "knconfig.h"
#include "knconfigmanager.h"
#include "kndisplayedheader.h"
#include "knfolder.h"
#include "knfoldermanager.h"
#include "knglobals.h"
#include "kngroup.h"
#include "knmainwidget.h"
#include "knnntpaccount.h"
#include "knsourceviewwindow.h"

using namespace KNode;

QValueList<ArticleWidget*> ArticleWidget::mInstances;

ArticleWidget::ArticleWidget( QWidget *parent,
                              KXMLGUIClient *guiClient,
                              KActionCollection *actionCollection,
                              const char *name ) :
  QWidget( parent, name ),
  mArticle( 0 ),
  mViewer( 0 ),
  mCSSHelper( 0 ),
  mHeaderStyle( "fancy" ),
  mAttachmentStyle( "inline" ),
  mShowHtml( false ),
  mRot13( false ),
  mForceCharset( false ),
  mOverrideCharset( KMime::Headers::Latin1 ),
  mTimer( 0 ),
  mGuiClient( guiClient ),
  mActionCollection( actionCollection )
{
  mInstances.append( this );

  QHBoxLayout *box = new QHBoxLayout( this );
  mViewer = new KHTMLPart( this, "mViewer" );
  box->addWidget( mViewer->widget() );
  mViewer->widget()->setFocusPolicy( WheelFocus );
  mViewer->setPluginsEnabled( false );
  mViewer->setJScriptEnabled( false );
  mViewer->setJavaEnabled( false );
  mViewer->setMetaRefreshEnabled( false );
  mViewer->setOnlyLocalReferences( true );
  mViewer->view()->setFocusPolicy( QWidget::WheelFocus );
  connect( mViewer->browserExtension(), SIGNAL(openURLRequestDelayed(const KURL&, const KParts::URLArgs&)),
           SLOT(slotURLClicked(const KURL&)) );
  connect( mViewer, SIGNAL(popupMenu(const QString&, const QPoint&)),
           SLOT(slotURLPopup(const QString&, const QPoint&)) );

  mTimer = new QTimer( this );
  connect( mTimer, SIGNAL(timeout()), SLOT(slotTimeout()) );

  initActions();
  readConfig();
  clear();

  installEventFilter( this );
}


ArticleWidget::~ArticleWidget()
{
  mInstances.remove( this );
  delete mTimer;
  delete mCSSHelper;
  if ( mArticle && mArticle->isOrphant() )
    delete mArticle;
  removeTempFiles();
}


void ArticleWidget::initActions()
{
  mSaveAction = KStdAction::save( this, SLOT(slotSave()), mActionCollection );
  mSaveAction->setText( KStdGuiItem::saveAs().text() );
  mPrintAction = KStdAction::print( this, SLOT(slotPrint()), mActionCollection );
  mCopySelectionAction = KStdAction::copy( this, SLOT(slotCopySelection()), mActionCollection );
  mSelectAllAction = KStdAction::selectAll( this, SLOT(slotSelectAll()), mActionCollection );
  mFindAction = KStdAction::find( this, SLOT(slotFind()), mActionCollection, "find_in_article" );
  mFindAction->setText( i18n("F&ind in Article...") );
  mViewSourceAction = new KAction( i18n("&View Source"),  Key_V , this,
    SLOT(slotViewSource()), mActionCollection, "article_viewSource" );
  mReplyAction = new KAction( i18n("&Followup to Newsgroup..."), "message_reply",
    Key_R, this, SLOT(slotReply()), mActionCollection, "article_postReply" );
  mRemailAction = new KAction( i18n("Reply by E&mail..."), "mail_reply",
    Key_A, this, SLOT(slotRemail()), mActionCollection, "article_mailReply" );
  mForwardAction = new KAction( i18n("Forw&ard by Email..."), "mail_forward",
    Key_F, this, SLOT(slotForward()), mActionCollection, "article_forward" );
  mCancelAction = new KAction( i18n("article","&Cancel Article"),
    0, this, SLOT(slotCancel()), mActionCollection, "article_cancel" );
  mSupersedeAction = new KAction(i18n("S&upersede Article"),
    0, this, SLOT(slotSupersede()), mActionCollection, "article_supersede" );
  mFixedFontToggle = new KToggleAction( i18n("U&se Fixed Font"),
    Key_X ,this, SLOT(slotToggleFixedFont()), mActionCollection, "view_useFixedFont" );
  mFancyToggle = new KToggleAction( i18n("Fancy Formating"),
    Key_Y, this, SLOT(slotToggleFancyFormating()), mActionCollection, "view_fancyFormating" );
  mRot13Toggle = new KToggleAction( i18n("&Unscramble (Rot 13)"), "decrypted", 0 , this,
    SLOT(slotToggleRot13()), mActionCollection, "view_rot13" );
  mRot13Toggle->setChecked( false );

  KRadioAction *ra;
  mHeaderStyleMenu = new KActionMenu( i18n("&Headers"), mActionCollection, "view_headers" );
  ra = new KRadioAction( i18n("&Fancy Headers"), 0, this, SLOT(slotFancyHeaders()),
                         mActionCollection, "view_headers_fancy" );
  ra->setExclusiveGroup( "view_headers" );
  mHeaderStyleMenu->insert( ra );
  ra = new KRadioAction( i18n("&Standard Headers"), 0, this, SLOT(slotStandardHeaders()),
                         mActionCollection, "view_headers_standard" );
  ra->setExclusiveGroup( "view_headers" );
  mHeaderStyleMenu->insert( ra );
  ra = new KRadioAction( i18n("&All Headers"), 0, this, SLOT(slotAllHeaders()),
                         mActionCollection, "view_headers_all" );
  ra->setExclusiveGroup( "view_headers" );
  mHeaderStyleMenu->insert( ra );

  mAttachmentStyleMenu = new KActionMenu( i18n("&Attachments"), mActionCollection, "view_attachments" );
  ra = new KRadioAction( i18n("&As Icon"), 0, this, SLOT(slotIconAttachments()),
                         mActionCollection, "view_attachments_icon" );
  ra->setExclusiveGroup( "view_attachments" );
  mAttachmentStyleMenu->insert( ra );
  ra = new KRadioAction( i18n("&Inline"), 0, this, SLOT(slotInlineAttachments()),
                         mActionCollection, "view_attachments_inline" );
  ra->setExclusiveGroup( "view_attachments" );
  mAttachmentStyleMenu->insert( ra );
  ra = new KRadioAction( i18n("&Hide"), 0, this, SLOT(slotHideAttachments()),
                         mActionCollection, "view_attachments_hide" );
  ra->setExclusiveGroup( "view_attachments" );
  mAttachmentStyleMenu->insert( ra );

  mCharsetSelect = new KSelectAction( i18n("Chars&et"), 0, mActionCollection, "set_charset" );
  mCharsetSelect->setShortcutConfigurable( false );
  QStringList cs = KGlobal::charsets()->descriptiveEncodingNames();
  cs.prepend( i18n("Automatic") );
  mCharsetSelect->setItems( cs );
  mCharsetSelect->setCurrentItem( 0 );
  connect( mCharsetSelect, SIGNAL(activated(const QString&)),SLOT(slotSetCharset(const QString&)) );
  mCharsetSelectKeyb = new KAction( i18n("Charset"), Key_C, this,
    SLOT(slotSetCharsetKeyboard()), mActionCollection, "set_charset_keyboard" );

  new KAction( i18n("&Open URL"), "fileopen", 0, this, SLOT(slotOpenURL()),
               mActionCollection, "open_url" );
  new KAction( i18n("&Copy Link Address"), "editcopy", 0, this, SLOT( slotCopyURL()),
               mActionCollection, "copy_url" );
  new KAction( i18n("&Bookmark This Link"), "bookmark_add", 0, this, SLOT(slotAddBookmark()),
               mActionCollection, "add_bookmark" );
  new KAction( i18n("&Add to Address Book"), 0, this, SLOT(slotAddToAddressBook()),
               mActionCollection, "add_addr_book" );
  new KAction( i18n("&Open in Address Book"), 0, this, SLOT(slotOpenInAddressBook()),
               mActionCollection, "openin_addr_book" );
  new KAction( i18n("&Open Attachment"), "fileopen", 0, this, SLOT(slotOpenAttachment()),
               mActionCollection, "open_attachment" );
  new KAction( i18n("&Save Attachment As..."), "filesaveas", 0, this, SLOT(slotSaveAttachment()),
               mActionCollection, "save_attachment" );
}



void ArticleWidget::enableActions()
{
  if ( !mArticle ) {
    disableActions();
    return;
  }

  mSaveAction->setEnabled( true );
  mPrintAction->setEnabled( true );
  mCopySelectionAction->setEnabled( true );
  mSelectAllAction->setEnabled( true );
  mFindAction->setEnabled( true );
  mForwardAction->setEnabled( true );
  mHeaderStyleMenu->setEnabled( true );
  mAttachmentStyleMenu->setEnabled( true );
  mRot13Toggle->setEnabled( true );
  mViewSourceAction->setEnabled( true );
  mCharsetSelect->setEnabled( true );
  mCharsetSelectKeyb->setEnabled( true );
  mFixedFontToggle->setEnabled( true );
  mFancyToggle->setEnabled( true );

  // only valid for remote articles
  bool enabled = ( mArticle->type() == KMime::Base::ATremote );
  mReplyAction->setEnabled( enabled );
  mRemailAction->setEnabled( enabled );

  enabled = ( mArticle->type() == KMime::Base::ATremote
    || mArticle->collection() == knGlobals.folderManager()->sent() );
  mCancelAction->setEnabled( enabled );
  mSupersedeAction->setEnabled( enabled );
}


void ArticleWidget::disableActions()
{
  mSaveAction->setEnabled( false );
  mPrintAction->setEnabled( false );
  mCopySelectionAction->setEnabled( false );
  mSelectAllAction->setEnabled( false );
  mFindAction->setEnabled( false );
  mReplyAction->setEnabled( false );
  mRemailAction->setEnabled( false );
  mForwardAction->setEnabled( false );
  mCancelAction->setEnabled( false );
  mSupersedeAction->setEnabled( false );
  mHeaderStyleMenu->setEnabled( false );
  mAttachmentStyleMenu->setEnabled( false );
  mRot13Toggle->setEnabled( false );
  mViewSourceAction->setEnabled( false );
  mCharsetSelect->setEnabled( false );
  mCharsetSelectKeyb->setEnabled( false );
  mFixedFontToggle->setEnabled( false );
  mFancyToggle->setEnabled( false );
}



void ArticleWidget::readConfig()
{
  mFixedFontToggle->setChecked( knGlobals.configManager()->readNewsViewer()->useFixedFont() );
  mFancyToggle->setChecked( knGlobals.configManager()->readNewsViewer()->interpretFormatTags() );

  mShowHtml = knGlobals.configManager()->readNewsViewer()->alwaysShowHTML();

  KConfig *conf = knGlobals.config();
  conf->setGroup( "READNEWS" );
  mAttachmentStyle = conf->readEntry( "attachmentStyle", "inline" );
  mHeaderStyle = conf->readEntry( "headerStyle", "fancy" );
  KRadioAction *ra = 0;
  ra = static_cast<KRadioAction*>( mActionCollection->action( QString("view_attachments_%1").arg(mAttachmentStyle).latin1() ) );
  ra->setChecked( true );
  ra = static_cast<KRadioAction*>( mActionCollection->action( QString("view_headers_%1").arg(mHeaderStyle).latin1() ) );
  ra->setChecked( true );

  delete mCSSHelper;
  mCSSHelper = new CSSHelper( QPaintDeviceMetrics( mViewer->view() ) );

  if ( !knGlobals.configManager()->readNewsGeneral()->autoMark() )
    mTimer->stop();
}


void ArticleWidget::writeConfig()
{
  // main viewer determines the settings
  if ( knGlobals.artWidget != this )
    return;

  KConfig *conf = knGlobals.config();
  conf->setGroup( "READNEWS" );
  conf->writeEntry( "attachmentStyle", mAttachmentStyle );
  conf->writeEntry( "headerStyle", mHeaderStyle );

  knGlobals.configManager()->readNewsViewer()->setUseFixedFont( mFixedFontToggle->isChecked() );
  knGlobals.configManager()->readNewsViewer()->setInterpretFormatTags( mFancyToggle->isChecked() );
}



void ArticleWidget::setArticle( KNArticle *article )
{
  // don't leak orphant articles
  if ( mArticle && mArticle->isOrphant() )
    delete mArticle;

  mShowHtml = knGlobals.configManager()->readNewsViewer()->alwaysShowHTML();
  mRot13 = false;
  mRot13Toggle->setChecked( false );
  mTimer->stop();

  mArticle = article;

  if ( !mArticle )
    clear();
  else {
    if ( mArticle->hasContent() ) { // article is already loaded => just show it
      displayArticle();
    } else {
      if( !knGlobals.articleManager()->loadArticle( mArticle ) )
        articleLoadError( mArticle, i18n("Unable to load the article.") );
      else
         // try again for local articles
        if( mArticle->hasContent() && !( mArticle->type() == KMime::Base::ATremote ) )
          displayArticle();
    }
  }
}


void ArticleWidget::clear()
{
  disableActions();
  mViewer->begin();
  mViewer->setUserStyleSheet( mCSSHelper->cssDefinitions( mFixedFontToggle->isChecked() ) );
  mViewer->write( mCSSHelper->htmlHead( mFixedFontToggle->isChecked() ) );
  mViewer->write( "</body></html>" );
  mViewer->end();
}


void ArticleWidget::displayArticle()
{
  if ( !mArticle) {
    clear();
    return;
  }

  // scroll back to top
  mViewer->view()->ensureVisible( 0, 0 );

  if ( !mArticle->hasContent() ) {
    displayErrorMessage( i18n("The article contains no data.") );
    return;
  }

  if ( mForceCharset != mArticle->forceDefaultCS()
       || ( mForceCharset && mArticle->defaultCharset() != mOverrideCharset ) ) {
        mArticle->setDefaultCharset( mOverrideCharset );
        mArticle->setForceDefaultCS( mForceCharset );
  }

  KNConfig::ReadNewsViewer *rnv = knGlobals.configManager()->readNewsViewer();
  removeTempFiles();

  mViewer->begin();
  mViewer->setUserStyleSheet( mCSSHelper->cssDefinitions( mFixedFontToggle->isChecked() ) );
  mViewer->write( mCSSHelper->htmlHead( mFixedFontToggle->isChecked() ) );

  // headers
  displayHeader();

  // body
  QString html;
  KMime::Content *text = mArticle->textContent();

  // check if codec is available
  if ( text && !canDecodeText( text->contentType()->charset() ) ) {
    html += QString("<table width=\"100%\" border=\"0\"><tr><td bgcolor=\"#FF0000\">%1</td></tr></table>")
      .arg( i18n("Unknown charset. Default charset is used instead.") );
    kdDebug(5003) << k_funcinfo << "unknown charset = " << text->contentType()->charset() << endl;
  }

  // if the article is pgp signed and the user asked for verifying the
  // signature, we show a nice header:
  QPtrList<Kpgp::Block> pgpBlocks;
  QStrList nonPgpBlocks;
  bool containsPGP = Kpgp::Module::prepareMessageForDecryption( mArticle->body(), pgpBlocks, nonPgpBlocks );

  mViewer->write ( html );
  html = QString();

  if ( containsPGP ) {
    QPtrListIterator<Kpgp::Block> pbit( pgpBlocks );
    QStrListIterator npbit( nonPgpBlocks );
    QTextCodec *codec;
    if ( text )
      codec = KGlobal::charsets()->codecForName( text->contentType()->charset() );
    else
      codec = KGlobal::locale()->codecForEncoding();

    for( ; *pbit != 0; ++pbit, ++npbit ) {
      // handle non-pgp block
      QCString str( *npbit );
      if( !str.isEmpty() ) {
        QStringList lines = QStringList::split( '\n', codec->toUnicode( str ), true );
        displayBodyBlock( lines );
      }
      // handle pgp block
      Kpgp::Block* block = *pbit;
      if ( block->type() == Kpgp::ClearsignedBlock )
        block->verify();
      QStringList lines = QStringList::split( '\n', codec->toUnicode( block->text() ), true );
      if ( block->isSigned() ) {
        QString signClass = displaySigHeader( block );
        displayBodyBlock( lines );
        displaySigFooter( signClass );
      } else {
        displayBodyBlock( lines );
      }
    }
    // deal with the last non-pgp block
    QCString str( *npbit );
    if( !str.isEmpty() ) {
      QStringList lines = QStringList::split( '\n', codec->toUnicode( str ), true );
      displayBodyBlock( lines );
    }
  }

  KMime::Headers::ContentType *ct = mArticle->contentType();

  // get attachments
  mAttachments.clear();
  mAttachementMap.clear();
  if( !text || ct->isMultipart() )
    mArticle->attachments( &mAttachments, rnv->showAlternativeContents() );

  // partial message
  if(ct->isPartial()) {
    mViewer->write( i18n("<br/><b>This article has the MIME type &quot;message/partial&quot;, which KNode cannot handle yet.<br>Meanwhile you can save the article as a text file and reassemble it by hand.</b>") );
  }

  // display body text
  if ( text && text->hasContent() && !ct->isPartial() ) {
    // handle HTML messages
    if ( text->contentType()->isHTMLText() ) {
      QString htmlTxt;
      text->decodedText( htmlTxt, true, knGlobals.configManager()->readNewsViewer()->removeTrailingNewlines() );
      if ( mShowHtml ) {
        // strip </html> & </body>
        int i = kMin( htmlTxt.findRev( "</html>", -1, false ), htmlTxt.findRev( "</body>", -1, false ) );
        if ( i >= 0 )
          htmlTxt.truncate( i );
        html += htmlTxt;
      } else {
        html += "<div class=\"htmlWarn\">\n";
        html += i18n("<b>Note:</b> This is an HTML message. For "
                     "security reasons, only the raw HTML code "
                     "is shown. If you trust the sender of this "
                     "message then you can activate formatted "
                     "HTML display for this message "
                     "<a href=\"knode:showHTML\">by clicking here</a>.");
        html += "</div><br><br>";
        html += toHtmlString( htmlTxt );
      }
    }
    else {
      if ( !containsPGP ) {
        QStringList lines;
        text->decodedText( lines, true, knGlobals.configManager()->readNewsViewer()->removeTrailingNewlines() );
        displayBodyBlock( lines );
      }
    }

  }
  mViewer->write( html );

  // display attachments
  if( !mAttachments.isEmpty() && !ct->isPartial() ) {
    int attCnt = 0;
    for( KMime::Content *var = mAttachments.first(); var; var = mAttachments.next() ) {
      displayAttachment( var, attCnt );
      attCnt++;
    }
  }

  mViewer->write("</body></html>");
  mViewer->end();

  enableActions();
  if( mArticle->type() == KMime::Base::ATremote && knGlobals.configManager()->readNewsGeneral()->autoMark() )
    mTimer->start( knGlobals.configManager()->readNewsGeneral()->autoMarkSeconds() * 1000, true );
}


void ArticleWidget::displayErrorMessage( const QString &msg )
{
  mViewer->begin();
  mViewer->setUserStyleSheet( mCSSHelper->cssDefinitions( mFixedFontToggle->isChecked() ) );
  mViewer->write( mCSSHelper->htmlHead( mFixedFontToggle->isChecked() ) );
  QString errMsg = msg;
  mViewer->write( "<b><font size=\"+1\" color=\"red\">" );
  mViewer->write( i18n("An error occurred.") );
  mViewer->write( "</font></b><hr/><br/>" );
  mViewer->write( errMsg.replace( "\n", "<br/>" ) );
  mViewer->write( "</body></html>");
  mViewer->end();

  // mark article as read if there is a negative reply from the server
  if ( knGlobals.configManager()->readNewsGeneral()->autoMark() &&
       mArticle && mArticle->type() == KMime::Base::ATremote && !mArticle->isOrphant() &&
       ( msg.find("430") != -1 || msg.find("423") != -1 ) ) {
    KNRemoteArticle::List l;
    l.append( static_cast<KNRemoteArticle*>( mArticle ) );
    knGlobals.articleManager()->setRead( l, true );
  }

  disableActions();
}



void ArticleWidget::displayHeader()
{
  QString headerHtml;

  // full header style
  if ( mHeaderStyle == "all" ) {
    QCString head = mArticle->head();
    KMime::Headers::Generic *header = 0;

    while ( !head.isEmpty() ) {
      header = mArticle->getNextHeader( head );
      if ( header ) {
        headerHtml += "<tr>";
        headerHtml+=QString( "<td align=\"right\" valign=\"top\"><b>%1</b></td><td width=\"100%\">%2</td></tr>" )
          .arg( toHtmlString( header->type(), None ) + ": " )
          .arg( toHtmlString( header->asUnicodeString() , ParseURL ) );
        delete header;
      }
    }

    mViewer->write( "<div class=\"header\">" );
    mViewer->write( "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\"> " );
    mViewer->write( headerHtml );
    mViewer->write( "</table></div>" );
    return;
  }

  // standard & fancy header style
  KMime::Headers::Base *hb;
  QValueList<KNDisplayedHeader*> dhs = knGlobals.configManager()->displayedHeaders()->headers();
  for ( QValueList<KNDisplayedHeader*>::Iterator it = dhs.begin(); it != dhs.end(); ++it ) {
    KNDisplayedHeader *dh = (*it);
    hb = mArticle->getHeaderByType(dh->header().latin1());
    if ( !hb || hb->is("Subject") || hb->is("Organization") )
      continue;

    if ( dh->hasName() ) {
      headerHtml += "<tr>";
      if ( mHeaderStyle == "fancy" )
        headerHtml += "<th>";
      else
        headerHtml += "<th align=\"right\">";
      headerHtml += toHtmlString( dh->translatedName(), None );
      headerHtml += ":</th><td width=\"100%\">";
    }
    else
      headerHtml+="<tr><td colspan=\"2\">";

    if ( hb->is("From") ) {
      headerHtml += QString( "<a href=\"mailto:%1\">%2</a>")
          .arg( KPIM::getEmailAddress( hb->asUnicodeString() ) )
          .arg( toHtmlString( hb->asUnicodeString(), None ) );
      KMime::Headers::Base *orgHdr = mArticle->getHeaderByType( "Organization" );
      if ( orgHdr && !orgHdr->isEmpty() ) {
        headerHtml += "&nbsp;&nbsp;(";
        headerHtml += toHtmlString( orgHdr->asUnicodeString() );
        headerHtml += ")";
      }
    } else if ( hb->is("Date") ) {
      KMime::Headers::Date *date=static_cast<KMime::Headers::Date*>(hb);
      headerHtml += toHtmlString( KGlobal::locale()->formatDateTime(date->qdt(), false, true), None );
    } else if ( hb->is("Newsgroups") ) {
      QString groups = hb->asUnicodeString();
      groups.replace( ',', ", " );
      headerHtml += toHtmlString( groups, ParseURL );
    } else
      headerHtml += toHtmlString( hb->asUnicodeString(), ParseURL );

    headerHtml += "</td></tr>";
  }

  // standard header style
  if ( mHeaderStyle == "standard" ) {
    mViewer->write( "<b style=\"font-size:130%\">" + toHtmlString( mArticle->subject()->asUnicodeString() ) + "</b>" );
    mViewer->write( "<div class=\"header\"" );
    mViewer->write( "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\"><tr>" + headerHtml );
    mViewer->write( "</tr></table></div>" );
    return;
  }

  // X-Face support
  QString xfhead;
  KMime::Headers::Base *temp = mArticle->getHeaderByType("X-Face");
  if (temp)
    xfhead = temp->asUnicodeString();
  QString xface = "";
  if ( !xfhead.isEmpty() ) {
    KPIM::KXFace xf;
    xface = QString::fromLatin1( "<div class=\"senderpic\"><img src=\"%1\" width=\"48\" height=\"48\"/></div>" )
      .arg( imgToDataUrl( xf.toImage( xfhead ), "PNG" ) );
  }

  // fancy header style
  mViewer->write( "<div class=\"fancy header\"" );
  mViewer->write( QString("<div>") );
  mViewer->write( toHtmlString( mArticle->subject()->asUnicodeString(), ParseURL | FancyFormatting ) );
  mViewer->write( QString("</div>") );

  QString html = QString("<table class=\"outer\"><tr><td width=\"100%\"><table>");

  html += headerHtml;
  html+="</td></tr></table></td>";
  html += "<td align=\"center\">" + xface + "</td>";
  html += "</tr></table>";

  // references
  KMime::Headers::References *refs = mArticle->references( false );
  if ( mArticle->type() == KMime::Base::ATremote && refs
       && knGlobals.configManager()->readNewsViewer()->showRefBar() ) {
    html += "<div class=\"spamheader\">";
    int refCnt = refs->count(), i = 1;
    QCString id = refs->first();
    id = id.mid( 1, id.length() - 2 );  // remove <>
    html += QString( "<b>%1</b>" ).arg( i18n("References:") );

    while ( i <= refCnt ) {
      html += " <a href=\"news:" + QString::fromLatin1( id ) + "\">" + QString::number( i ) + "</a>";
      id = refs->next();
      id = id.mid( 1, id.length() - 2 );  // remove <>
      i++;
    }
    html += "</div>";
  }

  mViewer->write( html );
  mViewer->write( "</div>" );
}


void ArticleWidget::displayBodyBlock( const QStringList &lines )
{
  int oldLevel = -2, newLevel = -2;
  bool isSig = false;
  QString line, html;
  KNConfig::ReadNewsViewer *rnv = knGlobals.configManager()->readNewsViewer();
  QString quoteChars = rnv->quoteCharacters().simplifyWhiteSpace();
  if (quoteChars.isEmpty())
    quoteChars = ">";

  for ( QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
    line = (*it);
    if ( !line.isEmpty() ) {
      // signature found
      if ( !isSig && line == "-- " ) {
        isSig = true;
        // close previous body tag (if any) and open new one
        if ( newLevel != -2 )
          html += "</div>";
        html += mCSSHelper->nonQuotedFontTag();
        newLevel = -1;
        if ( rnv->showSignature() ) {
          html += "<hr size=\"1\"/>";
          continue;
        }
        else break;
      }
      // look for quoting characters
      if ( !isSig ) {
        oldLevel = newLevel;
        newLevel = quotingDepth( line, quoteChars );
        if ( newLevel >= 3 )
          newLevel = 2; // no more than three levels supported (0-2)

        // quoting level changed
        if ( newLevel != oldLevel ) {
          if ( oldLevel != -2 )
            html += "</div>"; // close previous level
          // open new level
          if ( newLevel == -1 )
            html += mCSSHelper->nonQuotedFontTag();
          else
            html += mCSSHelper->quoteFontTag( newLevel );
        }
        // output the actual line
        html += toHtmlString( line, ParseURL | FancyFormatting | AllowROT13 ) + "<br/>";
      } else {
        // signature
        html += toHtmlString( line, ParseURL | AllowROT13 ) + "<br/>";
      }
    } else {
      // empty line
      html += "<br/>";
    }
  }
      // close body quoting level tags
  if ( newLevel != -2 )
    html += "</div>";

  mViewer->write( html );
}


QString ArticleWidget::displaySigHeader( Kpgp::Block* block )
{
  QString signClass = "signErr";
  QString signer = block->signatureUserId();
  QCString signerKey = block->signatureKeyId();
  QString message;
  if ( signer.isEmpty() ) {
    message = i18n( "Message was signed with unknown key 0x%1." )
      .arg( signerKey );
    message += "<br/>";
    message += i18n( "The validity of the signature cannot be verified." );
    signClass = "signWarn";
  } else {
    // determine the validity of the key
    Kpgp::Module *pgp = knGlobals.pgp;
    Kpgp::Validity keyTrust;
    if( !signerKey.isEmpty() )
      keyTrust = pgp->keyTrust( signerKey );
    else
      // This is needed for the PGP 6 support because PGP 6 doesn't
      // print the key id of the signing key if the key is known.
      keyTrust = pgp->keyTrust( signer );

    // HTMLize the signer's user id and create mailto: link
    signer = toHtmlString( signer, None );
    signer = "<a href=\"mailto:" + KPIM::getEmailAddress( signer ) + "\">" + signer + "</a>";

    if( !signerKey.isEmpty() )
      message += i18n( "Message was signed by %1 (Key ID: 0x%2)." )
      .arg( signer )
      .arg( signerKey );
    else
      message += i18n( "Message was signed by %1." ).arg( signer );
    message += "<br/>";

    if( block->goodSignature() ) {
      if ( keyTrust < Kpgp::KPGP_VALIDITY_MARGINAL )
        signClass = "signOkKeyBad";
      else
        signClass = "signOkKeyOk";
      switch( keyTrust ) {
        case Kpgp::KPGP_VALIDITY_UNKNOWN:
          message += i18n( "The signature is valid, but the key's "
                          "validity is unknown." );
          break;
        case Kpgp::KPGP_VALIDITY_MARGINAL:
          message += i18n( "The signature is valid and the key is "
                          "marginally trusted." );
          break;
        case Kpgp::KPGP_VALIDITY_FULL:
          message += i18n( "The signature is valid and the key is "
                          "fully trusted." );
          break;
        case Kpgp::KPGP_VALIDITY_ULTIMATE:
          message += i18n( "The signature is valid and the key is "
                          "ultimately trusted." );
          break;
        default:
          message += i18n( "The signature is valid, but the key is "
                          "untrusted." );
      }
    } else {
      message += i18n("Warning: The signature is bad.");
      signClass = "signErr";
    }
  }

  QString html = "<table cellspacing=\"1\" cellpadding=\"1\" class=\"" + signClass + "\">";
  html += "<tr class=\"" + signClass + "H\"><td>";
  html += message;
  html += "</td></tr><tr class=\"" + signClass + "B\"><td>";
  mViewer->write( html );
  return signClass;
}


void ArticleWidget::displaySigFooter( const QString &signClass )
{
  QString html = "</td></tr><tr class=\"" + signClass + "H\">";
  html += "<td>" + i18n( "End of signed message" ) + "</td></tr></table>";
  mViewer->write( html );
}


void ArticleWidget::displayAttachment( KMime::Content *att, int partNum )
{
  if ( mAttachmentStyle == "hide" )
    return;

  QString html;
  KMime::Headers::ContentType *ct = att->contentType();

  // attachment label
  QString label = ct->name();
  if ( label.isEmpty() )
    label = i18n("unnamed" );
  // if label consists of only whitespace replace them by underscores
  if ( (uint)label.contains( ' ' ) == label.length() )
    label.replace( QRegExp( " ", true, true ), "_" );
  label = toHtmlString( label, None );

  // attachment comment
  QString comment = att->contentDescription()->asUnicodeString();
  comment = toHtmlString( comment, ParseURL | FancyFormatting );

  QString href;
  QString fileName = writeAttachmentToTempFile( att, partNum );
  if ( fileName.isEmpty() ) {
    href = "part://" + QString::number( partNum );
  } else {
    href = "file:" + KURL::encode_string( fileName );
    mAttachementMap[fileName] = partNum;
  }

  if ( mAttachmentStyle == "inline" && inlinePossible( att ) ) {
    if ( ct->isImage() ) {
      html += "<div><a href=\"" + href + "\">"
              "<img src=\"" + fileName + "\" border=\"0\"></a>"
              "</div><div><a href=\"" + href + "\">" + label + "</a>"
              "</div><div>" + comment + "</div><br>";
    } else { //text
      // frame
      html += "<table cellspacing=\"1\" class=\"textAtm\">"
              "<tr class=\"textAtmH\"><td>"
              "<a href=\"" + href + "\">" + label + "</a>";
      if ( !comment.isEmpty() )
        html += "<br>" + comment;
      html += "</td></tr><tr class=\"textAtmB\"><td>";
      // content
      QString tmp;
      att->decodedText( tmp );
      /*if( ct->isHTMLText() )
        // ### to dangerous, we should use the same stuff as for the main text here
        html += tmp;
      else*/
        html += toHtmlString( tmp, ParseURL );
      // finish frame
      html += "</td></tr></table>";
    }
  } else { // icon
    QCString mimetype = ct->mimeType();
    KPIM::kAsciiToLower( mimetype.data() );
    QString iconName = KMimeType::mimeType( mimetype )->icon( QString::null, false );
    QString iconFile = KGlobal::instance()->iconLoader()->iconPath( iconName, KIcon::Desktop );
    html += "<div><a href=\"" + href + "\"><img src=\"" +
            iconFile + "\" border=\"0\">" + label +
            "</a></div><div>" + comment + "</div><br>";
  }
  mViewer->write( html );
}


QString ArticleWidget::toHtmlString( const QString &line, int flags )
{
  int llflags = LinkLocator::PreserveSpaces;
  if ( !(flags & ArticleWidget::ParseURL) )
    llflags |= LinkLocator::IgnoreUrls;
  if ( mFancyToggle->isChecked() && (flags & ArticleWidget::FancyFormatting) )
    llflags |= LinkLocator::ReplaceSmileys | LinkLocator::HighlightText;
  QString text = line;
  if ( flags & ArticleWidget::AllowROT13 ) {
    if ( mRot13 )
      text = KNHelper::rot13( line );
  }
  return LinkLocator::convertToHtml( text, llflags );
}


// from KMail headerstyle.cpp
QString ArticleWidget::imgToDataUrl( const QImage &image, const char* fmt  )
{
  QByteArray ba;
  QBuffer buffer( ba );
  buffer.open( IO_WriteOnly );
  image.save( &buffer, fmt );
  return QString::fromLatin1("data:image/%1;base64,%2")
    .arg( fmt, KCodecs::base64Encode( ba ) );
}



int ArticleWidget::quotingDepth( const QString &line, const QString &quoteChars )
{
  int level = -1;
  for ( uint i = 0; i < line.length(); ++i ) {
    // skip spaces
    if ( line[i].isSpace() )
      continue;
    if ( quoteChars.find( line[i] ) != -1 )
      ++level;
    else
      break;
  }
  return level;
}


bool ArticleWidget::inlinePossible( KMime::Content *c )
{
  KMime::Headers::ContentType *ct = c->contentType();
  return ( ct->isText() || ct->isImage() );
}


bool ArticleWidget::canDecodeText( const QCString &charset ) const
{
  if ( charset.isEmpty() )
    return false;
  bool ok = true;
  KGlobal::charsets()->codecForName( charset,ok );
  return ok;
}



void ArticleWidget::updateContents()
{
  // save current scrollbar position
  float savedPosition = (float)mViewer->view()->contentsY() / (float)mViewer->view()->contentsHeight();
  if ( mArticle && mArticle->hasContent() )
    displayArticle();
  else
    clear();
  // restore scrollbar position
  mViewer->view()->setContentsPos( 0, qRound(  mViewer->view()->contentsHeight() * savedPosition ) );
}



QString ArticleWidget::writeAttachmentToTempFile( KMime::Content *att, int partNum )
{
  // more or less KMail code
  KTempFile *tempFile = new KTempFile( QString::null, "." + QString::number( partNum ) );
  tempFile->setAutoDelete( true );
  QString fname = tempFile->name();
  delete tempFile;

  if( ::access( QFile::encodeName( fname ), W_OK ) != 0 )
    // Not there or not writable
    if( ::mkdir( QFile::encodeName( fname ), 0 ) != 0
        || ::chmod( QFile::encodeName( fname ), S_IRWXU ) != 0 )
      return QString::null; //failed create

  Q_ASSERT( !fname.isNull() );

  mTempDirs.append( fname );
  // strip off a leading path
  KMime::Headers::ContentType* ct = att->contentType();
  QString attName = ct->name();
  int slashPos = attName.findRev( '/' );
  if( -1 != slashPos )
    attName = attName.mid( slashPos + 1 );
  if( attName.isEmpty() )
    attName = "unnamed";
  fname += "/" + attName;

  QByteArray data = att->decodedContent();
  size_t size = data.size();
  // ### KMail does crlf2lf conversion here before writing the file
  if( !KPIM::kBytesToFile( data.data(), size, fname, false, false, false ) )
    return QString::null;

  mTempFiles.append( fname );
  // make file read-only so that nobody gets the impression that he might
  // edit attached files
  ::chmod( QFile::encodeName( fname ), S_IRUSR );

  return fname;
}


void ArticleWidget::removeTempFiles( )
{
  for ( QStringList::Iterator it = mTempFiles.begin(); it != mTempFiles.end(); ++it )
    QFile::remove(*it);
  mTempFiles.clear();
  for ( QStringList::Iterator it = mTempDirs.begin(); it != mTempDirs.end(); ++it )
    QDir(*it).rmdir(*it);
  mTempDirs.clear();
}



void ArticleWidget::processJob( KNJobData * job )
{
  if ( job->type() == KNJobData::JTfetchSource ) {
    KNRemoteArticle *a = static_cast<KNRemoteArticle*>( job->data() );
    if ( !job->canceled() ) {
      if ( !job->success() )
        KMessageBox::error( this, i18n("An error occurred while downloading the article source:\n")
          .arg( job->errorString() ) );
      else
        new KNSourceViewWindow( a->head() + "\n" + a->body() );
    }
    delete job;
    delete a;
  }
  else
    delete job;
}



typedef QValueList<ArticleWidget*>::ConstIterator InstanceIterator;

void ArticleWidget::configChanged()
{
  for( InstanceIterator it = mInstances.begin(); it != mInstances.end(); ++it ) {
    (*it)->readConfig();
    (*it)->updateContents();
  }
}


bool ArticleWidget::articleVisible( KNArticle *article )
{
  for ( InstanceIterator it = mInstances.begin(); it != mInstances.end(); ++it )
    if ( (*it)->article() == article )
      return true;
  return false;
}


void ArticleWidget::articleRemoved( KNArticle *article )
{
  for ( InstanceIterator it = mInstances.begin(); it != mInstances.end(); ++it )
    if ( (*it)->article() == article )
      (*it)->setArticle( 0 );
}


void ArticleWidget::articleChanged( KNArticle *article )
{
  for ( InstanceIterator it = mInstances.begin(); it != mInstances.end(); ++it )
    if ( (*it)->article() == article )
      (*it)->displayArticle();
}


void ArticleWidget::articleLoadError( KNArticle *article, const QString &error )
{
  for ( InstanceIterator it = mInstances.begin(); it != mInstances.end(); ++it )
  if ( (*it)->article() == article )
    (*it)->displayErrorMessage( error );
}


void ArticleWidget::collectionRemoved( KNArticleCollection *coll )
{
  for ( InstanceIterator it = mInstances.begin(); it != mInstances.end(); ++it )
    if ( (*it)->article() && (*it)->article()->collection() == coll )
      (*it)->setArticle( 0 );
}


void ArticleWidget::cleanup()
{
  for ( InstanceIterator it = mInstances.begin(); it != mInstances.end(); ++it )
    (*it)->setArticle( 0 ); //delete orphant articles => avoid crash in destructor
}



bool ArticleWidget::atBottom() const
{
  const KHTMLView *view = mViewer->view();
  return view->contentsY() + view->visibleHeight() >= view->contentsHeight();
}

void ArticleWidget::scrollUp()
{
  mViewer->view()->scrollBy( 0, -10 );
}

void ArticleWidget::scrollDown()
{
  mViewer->view()->scrollBy( 0, 10 );
}

void ArticleWidget::scrollPrior()
{
  mViewer->view()->scrollBy( 0, -(int)(mViewer->view()->height() * 0.8) );
}

void ArticleWidget::scrollNext()
{
  mViewer->view()->scrollBy( 0, (int)(mViewer->view()->height() * 0.8) );
}



void ArticleWidget::slotURLClicked( const KURL &url, bool forceOpen)
{
  // internal URLs
  if ( url.protocol() == "knode" ) {
    if ( url.path() == "showHTML" ) {
      mShowHtml = true;
      updateContents();
    }
    return;
  }
  // handle mailto
  if ( url.protocol() == "mailto" ) {
    KMime::Headers::AddressField addr( mArticle );
    addr.fromUnicodeString( url.path(), "" );
    knGlobals.artFactory->createMail( &addr );
    return;
  }
  // handle news URL's
  if ( url.protocol() == "news" ) {
    kdDebug( 5003 ) << k_funcinfo << url << endl;
    knGlobals.top->openURL( url );
    return;
  }
  // handle attachments
  int partNum = 0;
  if ( url.protocol() == "file" || url.protocol() == "part" ) {
    if ( url.protocol() == "file" ) {
      if ( !mAttachementMap.contains( url.path() ) )
        return;
      partNum = mAttachementMap[url.path()];
    }
    if ( url.protocol() == "part" )
      partNum = url.path().toInt();
    KMime::Content *c = mAttachments.at( partNum );
    if ( !c )
      return;
    // TODO: replace with message box as done in KMail
    if ( forceOpen || knGlobals.configManager()->readNewsViewer()->openAttachmentsOnClick() )
      knGlobals.articleManager()->openContent( c );
    else
      knGlobals.articleManager()->saveContentToFile( c, this );
    return;
  }
  // let KDE take care of the remaing protocols (http, ftp, etc.)
  new KRun( url );
}


void ArticleWidget::slotURLPopup( const QString &url, const QPoint &point )
{
  mCurrentURL = KURL( url );
  QString popupName;
  if ( url.isEmpty() ) // plain text
    popupName = "body_popup";
  else if ( mCurrentURL.protocol() == "mailto" )
    popupName = "mailto_popup";
  else if ( mCurrentURL.protocol() == "file" || mCurrentURL.protocol() == "part" )
    popupName = "attachment_popup";
  // ### news URLS?
  else if ( mCurrentURL.protocol() == "knode" )
    return; // skip
  else
    popupName = "url_popup"; // all other URLs
  QPopupMenu *popup = static_cast<QPopupMenu*>( mGuiClient->factory()->container( popupName, mGuiClient ) );
  if ( popup )
    popup->popup( point );
}



void ArticleWidget::slotTimeout()
{
  if ( mArticle && mArticle->type() == KMime::Base::ATremote && !mArticle->isOrphant() ) {
    KNRemoteArticle::List l;
    l.append( static_cast<KNRemoteArticle*>( mArticle ) );
    knGlobals.articleManager()->setRead( l, true );
  }
}



void ArticleWidget::slotSave()
{
  if ( mArticle )
    knGlobals.articleManager()->saveArticleToFile( mArticle, this );
}

void ArticleWidget::slotPrint( )
{
  if ( mArticle )
    mViewer->view()->print();
}


void ArticleWidget::slotCopySelection( )
{
  kapp->clipboard()->setText( mViewer->selectedText() );
}


void ArticleWidget::slotSelectAll()
{
  mViewer->selectAll();
}


void ArticleWidget::slotFind()
{
  mViewer->findText();
}


void ArticleWidget::slotViewSource()
{
  // local article can be shown directly
  if ( mArticle && mArticle->type() == KMime::Base::ATlocal && mArticle->hasContent() ) {
    new KNSourceViewWindow( mArticle->encodedContent( false ) );
  } else {
    // download remote article
    if ( mArticle && mArticle->type() == KMime::Base::ATremote ) {
      KNGroup *g = static_cast<KNGroup*>( mArticle->collection() );
      KNRemoteArticle *a = new KNRemoteArticle( g ); //we need "g" to access the nntp-account
      a->messageID( true )->from7BitString( mArticle->messageID()->as7BitString( false ) );
      a->lines( true )->from7BitString( mArticle->lines( true )->as7BitString( false ) );
      a->setArticleNumber( static_cast<KNRemoteArticle*>( mArticle)->articleNumber() );
      emitJob( new KNJobData( KNJobData::JTfetchSource, this, g->account(), a) );
    }
  }
}


void ArticleWidget::slotReply()
{
  if ( mArticle && mArticle->type() == KMime::Base::ATremote )
    knGlobals.artFactory->createReply( static_cast<KNRemoteArticle*>( mArticle ),
                                       mViewer->selectedText(), true, false );
}


void ArticleWidget::slotRemail()
{
  if ( mArticle && mArticle->type()==KMime::Base::ATremote )
    knGlobals.artFactory->createReply( static_cast<KNRemoteArticle*>( mArticle ),
                                       mViewer->selectedText(), false, true );
}


void ArticleWidget::slotForward()
{
  knGlobals.artFactory->createForward( mArticle );
}


void ArticleWidget::slotCancel()
{
  knGlobals.artFactory->createCancel( mArticle );
}


void ArticleWidget::slotSupersede()
{
  knGlobals.artFactory->createSupersede( mArticle );
}


void ArticleWidget::slotToggleFixedFont()
{
  writeConfig();
  updateContents();
}


void ArticleWidget::slotToggleFancyFormating( )
{
  writeConfig();
  updateContents();
}


void ArticleWidget::slotFancyHeaders()
{
  mHeaderStyle = "fancy";
  writeConfig();
  updateContents();
}

void ArticleWidget::slotStandardHeaders()
{
  mHeaderStyle = "standard";
  writeConfig();
  updateContents();
}

void ArticleWidget::slotAllHeaders()
{
  mHeaderStyle = "all";
  writeConfig();
  updateContents();
}


void ArticleWidget::slotIconAttachments()
{
  mAttachmentStyle = "icon";
  writeConfig();
  updateContents();
}

void ArticleWidget::slotInlineAttachments()
{
  mAttachmentStyle = "inline";
  writeConfig();
  updateContents();
}

void ArticleWidget::slotHideAttachments()
{
  mAttachmentStyle = "hide";
  writeConfig();
  updateContents();
}


void ArticleWidget::slotToggleRot13()
{
  mRot13 = !mRot13;
  updateContents();
}



void ArticleWidget::slotSetCharset( const QString &charset )
{
  if ( charset.isEmpty() )
    return;

  if ( charset == i18n("Automatic") ) {
    mForceCharset = false;
    mOverrideCharset = KMime::Headers::Latin1;
  } else {
    mForceCharset = true;
    mOverrideCharset = KGlobal::charsets()->encodingForName( charset ).latin1();
  }

  if ( mArticle && mArticle->hasContent() ) {
    mArticle->setDefaultCharset( mOverrideCharset );  // the article will choose the correct default,
    mArticle->setForceDefaultCS( mForceCharset );     // when we disable the overdrive
    updateContents();
  }
}


void ArticleWidget::slotSetCharsetKeyboard( )
{
  int charset = KNHelper::selectDialog( this, i18n("Select Charset"),
    mCharsetSelect->items(), mCharsetSelect->currentItem() );
  if ( charset != -1 ) {
    mCharsetSelect->setCurrentItem( charset );
    slotSetCharset( *(mCharsetSelect->items().at( charset )) );
  }
}



void ArticleWidget::slotOpenURL()
{
  slotURLClicked( mCurrentURL );
}

void ArticleWidget::slotCopyURL()
{
  QString address;
  if ( mCurrentURL.protocol() == "mailto" )
    address = mCurrentURL.path();
  else
    address = mCurrentURL.url();
  QApplication::clipboard()->setText( address, QClipboard::Clipboard );
  QApplication::clipboard()->setText( address, QClipboard::Selection );
}

void ArticleWidget::slotAddBookmark()
{
  if ( mCurrentURL.isEmpty() )
    return;
  QString filename = locateLocal( "data", QString::fromLatin1("konqueror/bookmarks.xml") );
  KBookmarkManager *bookManager = KBookmarkManager::managerForFile( filename, false );
  KBookmarkGroup group = bookManager->root();
  group.addBookmark( bookManager, mCurrentURL.url(), mCurrentURL );
  bookManager->save();
}

void ArticleWidget::slotAddToAddressBook()
{
  KAddrBookExternal::addEmail( mCurrentURL.path(), this );
}

void ArticleWidget::slotOpenInAddressBook()
{
  KAddrBookExternal::openEmail( mCurrentURL.path(), this );
}

void ArticleWidget::slotOpenAttachment()
{
  slotURLClicked( mCurrentURL, true );
}

void ArticleWidget::slotSaveAttachment()
{
  if ( mCurrentURL.protocol() != "file" && mCurrentURL.protocol() != "part" )
    return;
  int partNum = 0;
  if ( mCurrentURL.protocol() == "file" ) {
    if ( !mAttachementMap.contains( mCurrentURL.path() ) )
      return;
    partNum = mAttachementMap[mCurrentURL.path()];
  }
  if ( mCurrentURL.protocol() == "part" )
    partNum = mCurrentURL.path().toInt();
  KMime::Content *c = mAttachments.at( partNum );
  if ( !c )
    return;
  knGlobals.articleManager()->saveContentToFile( c, this );
}



void ArticleWidget::focusInEvent( QFocusEvent *e )
{
  emit focusChanged(e);
  QWidget::focusInEvent(e);
}

void ArticleWidget::focusOutEvent( QFocusEvent *e )
{
  emit focusChanged(e);
  QWidget::focusOutEvent(e);
}

bool ArticleWidget::eventFilter( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::KeyPress && (static_cast<QKeyEvent*>(e)->key() == Key_Tab) ) {
    emit focusChangeRequest( this );
    if ( !hasFocus() )  // focusChangeRequest was successful
      return true;
  }
  return QWidget::eventFilter(o, e);
}

#include "articlewidget.moc"
