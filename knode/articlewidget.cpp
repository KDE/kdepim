/*
    KNode, the KDE newsreader
    Copyright (c) 2005-2006 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "articlewidget.h"

#include "utils/locale.h"


#include <QBuffer>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QMenu>
#include <QStringList>
#include <QTextCodec>
#include <QTimer>
#include <QHBoxLayout>

#include <kaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kascii.h>
#include <kbookmarkmanager.h>
#include <kcharsets.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <kselectaction.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <ktemporaryfile.h>
#include <ktoggleaction.h>
#include <kurl.h>
#include <kxmlguifactory.h>
#include <kicon.h>
#include <kde_file.h>

#include <libkdepim/job/addemailaddressjob.h>
#include <libkdepim/job/openemailaddressjob.h>

#include <libkpgp/kpgp.h>
#include <libkpgp/kpgpblock.h>

#include <messageviewer/header/kxface.h>
#include <messagecomposer/utils/util.h>
#include <kpimutils/kfileio.h>
#include <kpimutils/linklocator.h>
#include <kpimutils/email.h>

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
#include "nntpjobs.h"
#include "settings.h"

using namespace KNode;

QList<ArticleWidget*> ArticleWidget::mInstances;

ArticleWidget::ArticleWidget( QWidget *parent,
                              KXMLGUIClient *guiClient,
                              KActionCollection *actionCollection, bool isMainViewer ) :
  QWidget( parent ),
  mViewer( 0 ),
  mCSSHelper( 0 ),
  mHeaderStyle( "fancy" ),
  mAttachmentStyle( "inline" ),
  mShowHtml( false ),
  mRot13( false ),
  mForceCharset( false ),
  mOverrideCharset( KMime::Headers::Latin1 ),
  mTimer( 0 ),
  mIsMainViewer( isMainViewer ),
  mGuiClient( guiClient ),
  mActionCollection( actionCollection )
{
  mInstances.append( this );

  QHBoxLayout *box = new QHBoxLayout( this );
  box->setMargin( 0 );
  box->setSpacing( 0 );
  mViewer = new KHTMLPart( this );
  box->addWidget( mViewer->widget() );
  mViewer->widget()->setFocusPolicy( Qt::WheelFocus );
  mViewer->setPluginsEnabled( false );
  mViewer->setJScriptEnabled( false );
  mViewer->setJavaEnabled( false );
  mViewer->setMetaRefreshEnabled( false );
  mViewer->setOnlyLocalReferences( true );
  mViewer->view()->setFocusPolicy( Qt::WheelFocus );
  connect( mViewer->browserExtension(), SIGNAL(openUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
           SLOT(slotURLClicked(KUrl)) );
  connect( mViewer, SIGNAL(popupMenu(QString,QPoint)),
           SLOT(slotURLPopup(QString,QPoint)) );

  mTimer = new QTimer( this );
  mTimer->setSingleShot( true );
  connect( mTimer, SIGNAL(timeout()), SLOT(slotTimeout()) );

  initActions();
  readConfig();
  clear();
}


ArticleWidget::~ArticleWidget()
{
  mInstances.removeAll( this );
  delete mTimer;
  delete mCSSHelper;
  if ( mArticle && mArticle->isOrphant() ) {
    // if the article manager is still loading the current article,
    // cancel the job.
    knGlobals.articleManager()->cancelJobs( mArticle );
  }
  removeTempFiles();
}


void ArticleWidget::initActions()
{
  mSaveAction = KStandardAction::save( this, SLOT(slotSave()), mActionCollection );
  mSaveAction->setText( KStandardGuiItem::saveAs().text() );
  mPrintAction = KStandardAction::print( this, SLOT(slotPrint()), mActionCollection );
  mCopySelectionAction = KStandardAction::copy( this, SLOT(slotCopySelection()), mActionCollection );
  mSelectAllAction = KStandardAction::selectAll( this, SLOT(slotSelectAll()), mActionCollection );
  mFindAction = KStandardAction::find( this, SLOT(slotFind()), mActionCollection);
  mActionCollection->addAction("find_in_article", mFindAction);
  mFindAction->setText( i18n("F&ind in Article...") );
  mViewSourceAction = mActionCollection->addAction("article_viewSource");
  mViewSourceAction->setText(i18n("&View Source"));
  connect(mViewSourceAction, SIGNAL(triggered(bool)), SLOT(slotViewSource()));
  mViewSourceAction->setShortcut(QKeySequence(Qt::Key_V));
  mReplyAction = mActionCollection->addAction("article_postReply");
  mReplyAction->setIcon(KIcon("mail-reply-all"));
  mReplyAction->setText(i18n("&Followup to Newsgroup..."));
  connect(mReplyAction, SIGNAL(triggered(bool)), SLOT(slotReply()));
  mReplyAction->setShortcut(QKeySequence(Qt::Key_R));
  mRemailAction = mActionCollection->addAction("article_mailReply" );
  mRemailAction->setIcon(KIcon("mail-reply-sender"));
  mRemailAction->setText(i18n("Reply by E&mail..."));
  connect(mRemailAction, SIGNAL(triggered(bool)), SLOT(slotRemail()));
  mRemailAction->setShortcut(QKeySequence(Qt::Key_A));
  mForwardAction = mActionCollection->addAction("article_forward");
  mForwardAction->setIcon(KIcon("mail-forward"));
  mForwardAction->setText(i18n("Forw&ard by Email..."));
  connect(mForwardAction, SIGNAL(triggered(bool)), SLOT(slotForward()));
  mForwardAction->setShortcut(QKeySequence(Qt::Key_F));
  mCancelAction = mActionCollection->addAction("article_cancel");
  mCancelAction->setText(i18nc("article", "&Cancel Article"));
  connect(mCancelAction, SIGNAL(triggered(bool)), SLOT(slotCancel()));
  mSupersedeAction = mActionCollection->addAction("article_supersede");
  mSupersedeAction->setText(i18n("S&upersede Article"));
  connect(mSupersedeAction, SIGNAL(triggered(bool)), SLOT(slotSupersede()));
  mFixedFontToggle = mActionCollection->add<KToggleAction>("view_useFixedFont");
  mFixedFontToggle->setText(i18n("U&se Fixed Font"));
  connect(mFixedFontToggle, SIGNAL(triggered(bool)), SLOT(slotToggleFixedFont()));
  mFixedFontToggle->setShortcut(QKeySequence(Qt::Key_X));
  mFancyToggle = mActionCollection->add<KToggleAction>("view_fancyFormating");
  mFancyToggle->setText(i18n("Fancy Formatting"));
  connect(mFancyToggle, SIGNAL(triggered(bool)), SLOT(slotToggleFancyFormating()));
  mFancyToggle->setShortcut(QKeySequence(Qt::Key_Y));
  mRot13Toggle = mActionCollection->add<KToggleAction>("view_rot13");
  mRot13Toggle->setIcon(KIcon("document-decrypt"));
  mRot13Toggle->setText(i18n("&Unscramble (Rot 13)"));
  connect(mRot13Toggle, SIGNAL(triggered(bool)), SLOT(slotToggleRot13()));
  mRot13Toggle->setChecked( false );

  QActionGroup *ag = new QActionGroup( this );
  KToggleAction *ra;
  mHeaderStyleMenu = mActionCollection->add<KActionMenu>("view_headers");
  mHeaderStyleMenu->setText(i18n("&Headers"));
  ra = mActionCollection->add<KToggleAction>("view_headers_fancy");
  ra->setText(i18n("&Fancy Headers"));
  connect(ra, SIGNAL(triggered(bool)), SLOT(slotFancyHeaders()));
  ag->addAction ( ra );
  mHeaderStyleMenu->addAction( ra );
  ra = mActionCollection->add<KToggleAction>("view_headers_standard");
  ra->setText(i18n("&Standard Headers"));
  connect(ra, SIGNAL(triggered(bool)), SLOT(slotStandardHeaders()));
  ag->addAction( ra );
  mHeaderStyleMenu->addAction( ra );
  ra = mActionCollection->add<KToggleAction>("view_headers_all");
  ra->setText(i18n("&All Headers"));
  connect(ra, SIGNAL(triggered(bool)), SLOT(slotAllHeaders()));
  ag->addAction( ra );
  mHeaderStyleMenu->addAction( ra );

  ag = new QActionGroup( this );
  mAttachmentStyleMenu = mActionCollection->add<KActionMenu>("view_attachments");
  mAttachmentStyleMenu->setText(i18n("&Attachments"));
  ra = mActionCollection->add<KToggleAction>("view_attachments_icon");
  ra->setText(i18n("&As Icon"));
  connect(ra, SIGNAL(triggered(bool)), SLOT(slotIconAttachments()));
  ag->addAction( ra );
  mAttachmentStyleMenu->addAction( ra );
  ra = mActionCollection->add<KToggleAction>("view_attachments_inline");
  ra->setText(i18n("&Inline"));
  connect(ra, SIGNAL(triggered(bool)), SLOT(slotInlineAttachments()));
  ag->addAction( ra );
  mAttachmentStyleMenu->addAction( ra );
  ra = mActionCollection->add<KToggleAction>("view_attachments_hide");
  ra->setText(i18n("&Hide"));
  connect(ra, SIGNAL(triggered(bool)), SLOT(slotHideAttachments()));
  ag->addAction( ra );
  mAttachmentStyleMenu->addAction( ra );

  mCharsetSelect = mActionCollection->add<KSelectAction>("set_charset");
  mCharsetSelect->setText( i18n( "Set chars&et" ) );
  mCharsetSelect->setShortcutConfigurable( false );
  QStringList cs = Utilities::Locale::encodings();
  cs.prepend( i18nc( "@item default character set", "Default") );
  mCharsetSelect->setItems( cs );
  mCharsetSelect->setCurrentItem( 0 );
  connect( mCharsetSelect, SIGNAL(triggered(QString)),SLOT(slotSetCharset(QString)) );
  mCharsetSelectKeyb = mActionCollection->addAction("set_charset_keyboard");
  mCharsetSelectKeyb->setText( i18n( "Set charset" ) );
  connect(mCharsetSelectKeyb, SIGNAL(triggered(bool)), SLOT(slotSetCharsetKeyboard()));
  mCharsetSelectKeyb->setShortcut(QKeySequence(Qt::Key_C));

  QAction *action = mActionCollection->addAction("open_url");
  action->setIcon(KIcon("document-open"));
  action->setText(i18n("&Open URL"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotOpenURL()));
  action = mActionCollection->addAction("copy_url");
  action->setIcon(KIcon("edit-copy"));
  action->setText(i18n("&Copy Link Address"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotCopyURL()));
  action = mActionCollection->addAction("add_bookmark");
  action->setIcon(KIcon("bookmark-new"));
  action->setText(i18n("&Bookmark This Link"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotAddBookmark()));
  action = mActionCollection->addAction("add_addr_book");
  action->setText(i18n("&Add to Address Book"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotAddToAddressBook()));
  action = mActionCollection->addAction("openin_addr_book");
  action->setText(i18n("&Open in Address Book"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotOpenInAddressBook()));
  action = mActionCollection->addAction("open_attachment");
  action->setIcon(KIcon("document-open"));
  action->setText(i18n("&Open Attachment"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotOpenAttachment()));
  action = mActionCollection->addAction("save_attachment");
  action->setIcon(KIcon("document-save-as"));
  action->setText(i18n("&Save Attachment As..."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotSaveAttachment()));
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
  bool enabled = ( mArticle->type() == KNArticle::ATremote );
  mReplyAction->setEnabled( enabled );
  mRemailAction->setEnabled( enabled );

  enabled = ( mArticle->type() == KNArticle::ATremote
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
  mFixedFontToggle->setChecked( knGlobals.settings()->useFixedFont() );
  mFancyToggle->setChecked( knGlobals.settings()->interpretFormatTags() );

  mShowHtml = knGlobals.settings()->alwaysShowHTML();
  mViewer->setOnlyLocalReferences( !knGlobals.settings()->allowExternalReferences() );

  KConfigGroup conf(knGlobals.config(), "READNEWS" );
  mAttachmentStyle = conf.readEntry( "attachmentStyle", "inline" );
  mHeaderStyle = conf.readEntry( "headerStyle", "fancy" );
  KToggleAction *ra = 0;
  ra = static_cast<KToggleAction*>( mActionCollection->action( QString("view_attachments_%1").arg(mAttachmentStyle) ) );
  ra->setChecked( true );
  ra = static_cast<KToggleAction*>( mActionCollection->action( QString("view_headers_%1").arg(mHeaderStyle) ) );
  ra->setChecked( true );

  delete mCSSHelper;
  mCSSHelper = new CSSHelper( mViewer->view() );

  if ( !knGlobals.settings()->autoMark() )
    mTimer->stop();
}


void ArticleWidget::writeConfig()
{
  // main viewer determines the settings
  if ( !mIsMainViewer ) {
    return;
  }

  KConfigGroup conf(knGlobals.config(), "READNEWS" );
  conf.writeEntry( "attachmentStyle", mAttachmentStyle );
  conf.writeEntry( "headerStyle", mHeaderStyle );

  knGlobals.settings()->setUseFixedFont( mFixedFontToggle->isChecked() );
  knGlobals.settings()->setInterpretFormatTags( mFancyToggle->isChecked() );
}



void ArticleWidget::setArticle( KNArticle::Ptr article )
{
  mShowHtml = knGlobals.settings()->alwaysShowHTML();
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
        if( mArticle->hasContent() && !( mArticle->type() == KNArticle::ATremote ) )
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
  mViewer->write( QString("</body></html>") );
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

  if ( mForceCharset != mArticle->forceDefaultCharset()
       || ( mForceCharset && mArticle->defaultCharset() != mOverrideCharset ) ) {
        mArticle->setDefaultCharset( mOverrideCharset );
        mArticle->setForceDefaultCharset( mForceCharset );
  }

  removeTempFiles();

  mViewer->begin();
  mViewer->setUserStyleSheet( mCSSHelper->cssDefinitions( mFixedFontToggle->isChecked() ) );
  mViewer->write( mCSSHelper->htmlHead( mFixedFontToggle->isChecked() ) );

  // headers
  displayHeader();

  // body
  QString html;
  KMime::Content *text = 0;
  if ( mShowHtml ) {
    foreach ( KMime::Content *c, mArticle->contents() ) {
      if ( c->contentType()->isHTMLText() && c->contentType()->category() == KMime::Headers::CCalternativePart ) {
        text = c;
        break;
      }
    }
  }
  if ( !text )
    text = mArticle->textContent();

  // check if codec is available
  if ( text && !canDecodeText( text->contentType()->charset() ) ) {
    html += QString("<table width=\"100%\" border=\"0\"><tr><td bgcolor=\"#FF0000\">%1</td></tr></table>")
      .arg( i18n("Unknown charset. Default charset is used instead.") );
    kDebug(5003) <<"unknown charset =" << text->contentType()->charset();
  }

  // if the article is pgp signed and the user asked for verifying the
  // signature, we show a nice header:
  QList<Kpgp::Block> pgpBlocks;
  QList<QByteArray> nonPgpBlocks;
  bool containsPGP = Kpgp::Module::prepareMessageForDecryption( mArticle->body(), pgpBlocks, nonPgpBlocks );

  mViewer->write ( html );
  html.clear();

  if ( containsPGP ) {
    QList<Kpgp::Block>::Iterator pbit = pgpBlocks.begin();
    QList<QByteArray>::Iterator npbit = nonPgpBlocks.begin();
    QTextCodec *codec;
    if ( text )
      codec = KGlobal::charsets()->codecForName( text->contentType()->charset() );
    else
      codec = KGlobal::locale()->codecForEncoding();

    for( ; pbit != pgpBlocks.end(); ++pbit, ++npbit ) {
      // handle non-pgp block
      QByteArray str = *npbit;
      if( !str.isEmpty() ) {
        QStringList lines = codec->toUnicode( str ).split( '\n' );
        displayBodyBlock( lines );
      }
      // handle pgp block
      Kpgp::Block block = *pbit;
      if ( block.type() == Kpgp::ClearsignedBlock )
        block.verify();
      QStringList lines = codec->toUnicode( block.text() ).split( '\n' );
      if ( block.isSigned() ) {
        QString signClass = displaySigHeader( block );
        displayBodyBlock( lines );
        displaySigFooter( signClass );
      } else {
        displayBodyBlock( lines );
      }
    }
    // deal with the last non-pgp block
    QByteArray str = *npbit;
    if( !str.isEmpty() ) {
      QStringList lines = codec->toUnicode( str ).split( 'n' );
      displayBodyBlock( lines );
    }
  }

  KMime::Headers::ContentType *ct = mArticle->contentType();

  // get attachments
  mAttachments.clear();
  mAttachementMap.clear();
  if( !text || ct->isMultipart() )
    mAttachments = mArticle->attachments( knGlobals.settings()->showAlternativeContents() );

  // partial message
  if(ct->isPartial()) {
    mViewer->write( i18n("<br /><b>This article has the MIME type &quot;message/partial&quot;, which KNode cannot handle yet.<br />Meanwhile you can save the article as a text file and reassemble it by hand.</b>") );
  }

  // display body text
  if ( text && text->hasContent() && !ct->isPartial() ) {
    // handle HTML messages
    if ( text->contentType()->isHTMLText() ) {
      QString htmlTxt = text->decodedText( true, knGlobals.settings()->removeTrailingNewlines() );
      if ( mShowHtml ) {
        // strip </html> & </body>
        int i = qMin( htmlTxt.lastIndexOf( "</html>", -1, Qt::CaseInsensitive ), htmlTxt.lastIndexOf( "</body>", -1, Qt::CaseInsensitive ) );
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
        QStringList lines = text->decodedText( true, knGlobals.settings()->removeTrailingNewlines() ).split( '\n' );
        displayBodyBlock( lines );
      }
    }

  }
  mViewer->write( html );

  // display attachments
  if( !mAttachments.isEmpty() && !ct->isPartial() ) {
    int attCnt = 0;
    foreach ( KMime::Content *var, mAttachments ) {
      displayAttachment( var, attCnt );
      attCnt++;
    }
  }

  mViewer->write( QString("</body></html>") );
  mViewer->end();

  enableActions();
  if( mArticle->type() == KNArticle::ATremote && knGlobals.settings()->autoMark() )
    mTimer->start( knGlobals.settings()->autoMarkSeconds() * 1000 );
}


void ArticleWidget::displayErrorMessage( const QString &msg )
{
  mViewer->begin();
  mViewer->setUserStyleSheet( mCSSHelper->cssDefinitions( mFixedFontToggle->isChecked() ) );
  mViewer->write( mCSSHelper->htmlHead( mFixedFontToggle->isChecked() ) );
  QString errMsg = msg;
  mViewer->write( QString("<b><font size=\"+1\" color=\"red\">") );
  mViewer->write( i18n("An error occurred.") );
  mViewer->write( QString("</font></b><hr/><br/>") );
  mViewer->write( errMsg.replace( '\n', "<br/>" ) );
  mViewer->write( QString("</body></html>") );
  mViewer->end();

  disableActions();
}



void ArticleWidget::displayHeader()
{
  QString headerHtml;

  // full header style
  if ( mHeaderStyle == "all" ) {
    QByteArray head = mArticle->head();
    KMime::Headers::Base *header = 0;

    headerHtml += "<div class=\"header\">"
                  "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\"> ";
    while ( !head.isEmpty() ) {
      header = KMime::HeaderParsing::extractFirstHeader( head );
      if ( header ) {
        headerHtml+=QString( "<tr><td align=\"right\" valign=\"top\"><b>%1</b></td><td width=\"100%\">%2</td></tr>" )
          .arg( toHtmlString( header->type(), None ) + ": " )
          .arg( toHtmlString( header->asUnicodeString().remove( QRegExp( "^[^:]+:\\s*" ) ) , ParseURL ) );
        delete header;
      }
    }
    headerHtml += "</table></div>";

    mViewer->write( headerHtml );
    return;
  }

  // standard & fancy header style
  KMime::Headers::Base *hb;
  KNDisplayedHeader::List dhs = knGlobals.configManager()->displayedHeaders()->headers();
  for ( KNDisplayedHeader::List::Iterator it = dhs.begin(); it != dhs.end(); ++it ) {
    KNDisplayedHeader *dh = (*it);
    hb = mArticle->headerByType(dh->header().toLatin1());
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
          .arg( KPIMUtils::extractEmailAddress( hb->asUnicodeString() ) )
          .arg( toHtmlString( hb->asUnicodeString(), None ) );
      KMime::Headers::Base *orgHdr = mArticle->headerByType( "Organization" );
      if ( orgHdr && !orgHdr->isEmpty() ) {
        headerHtml += "&nbsp;&nbsp;(";
        headerHtml += toHtmlString( orgHdr->asUnicodeString() );
        headerHtml += ')';
      }
    } else if ( hb->is("Date") ) {
      KMime::Headers::Date *date=static_cast<KMime::Headers::Date*>(hb);
      headerHtml += toHtmlString( KGlobal::locale()->formatDateTime(date->dateTime().toLocalZone().dateTime(), KLocale::LongDate, true), None );
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
    mViewer->write( QString("<b style=\"font-size:130%\">" + toHtmlString( mArticle->subject()->asUnicodeString() ) + "</b>") );
    mViewer->write( QString("<div class=\"header\">") );
    mViewer->write( QString("<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\">") + headerHtml );
    mViewer->write( QString("</table></div>") );
    return;
  }

  // X-Face support
  QString xfhead;
  KMime::Headers::Base *temp = mArticle->headerByType("X-Face");
  if (temp)
    xfhead = temp->asUnicodeString();
  QString xface = "";
  if ( !xfhead.isEmpty() ) {
    MessageViewer::KXFace xf;
    xface = QString::fromLatin1( "<div class=\"senderpic\"><img src=\"%1\" width=\"48\" height=\"48\"/></div>" )
      .arg( imgToDataUrl( xf.toImage( xfhead ), "PNG" ) );
  }

  // fancy header style
  QString html = "<div class=\"fancy header\">";
  html += "<div>";
  html += toHtmlString( mArticle->subject()->asUnicodeString(), ParseURL | FancyFormatting );
  html += "</div>";

  html += "<table class=\"outer\"><tr><td width=\"100%\"><table>";
  html += headerHtml;
  html += "</table></td>";
  html += "<td align=\"center\">" + xface + "</td>";
  html += "</tr></table>";

  // references
  KMime::Headers::References *refs = mArticle->references( false );
  if ( mArticle->type() == KNArticle::ATremote && refs
       && knGlobals.settings()->showRefBar() ) {
    html += "<div class=\"spamheader\">";
    html += QString( "<b>%1</b>" ).arg( i18n("References:") );

    QList<QByteArray> references = refs->identifiers();
    for ( int i = 0; i < references.count(); ++i ) {
      html += " <a href=\"news:" + references.at(references.count() - i - 1)
          + "\">" + QString::number( i + 1 ) + "</a>";
    }
    html += "</div>";
  }
  html += "</div>";
  mViewer->write( html );
}


void ArticleWidget::displayBodyBlock( const QStringList &lines )
{
  int oldLevel = -2, newLevel = -2;
  bool isSig = false;
  QString line, html;
  QString quoteChars = knGlobals.settings()->quoteCharacters().simplified();
  if (quoteChars.isEmpty())
    quoteChars = '>';

  for ( QStringList::const_iterator it = lines.constBegin(); it != lines.constEnd(); ++it) {
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
        if ( knGlobals.settings()->showSignature() ) {
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


QString ArticleWidget::displaySigHeader( const Kpgp::Block &block )
{
  QString signClass = "signErr";
  QString signer = block.signatureUserId();
  QString signerKey = block.signatureKeyId();
  QString message;
  if ( signer.isEmpty() ) {
    message = i18n( "Message was signed with unknown key 0x%1." ,
        signerKey );
    message += "<br/>";
    message += i18n( "The validity of the signature cannot be verified." );
    signClass = "signWarn";
  } else {
    // determine the validity of the key
    Kpgp::Module *pgp = Kpgp::Module::getKpgp();
    Kpgp::Validity keyTrust;
    if( !signerKey.isEmpty() )
      keyTrust = pgp->keyTrust( signerKey );
    else
      // This is needed for the PGP 6 support because PGP 6 doesn't
      // print the key id of the signing key if the key is known.
      keyTrust = pgp->keyTrust( signer );

    // HTMLize the signer's user id and create mailto: link
    signer = toHtmlString( signer, None );
    signer = "<a href=\"mailto:" + KPIMUtils::extractEmailAddress( signer ) + "\">" + signer + "</a>";

    if( !signerKey.isEmpty() )
      message += i18n( "Message was signed by %1 (Key ID: 0x%2)." ,
        signer ,
        signerKey );
    else
      message += i18n( "Message was signed by %1.", signer );
    message += "<br/>";

    if( block.goodSignature() ) {
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
  if ( label.count( ' ' ) == label.length() )
    label.replace( QRegExp( " ", Qt::CaseSensitive, QRegExp::Wildcard ), "_" );
  label = toHtmlString( label, None );

  // attachment comment
  QString comment = att->contentDescription()->asUnicodeString();
  comment = toHtmlString( comment, ParseURL | FancyFormatting );

  QString href;
  KUrl file = writeAttachmentToTempFile( att, partNum );
  if ( file.isEmpty() ) {
    href = "part://" + QString::number( partNum );
  } else {
    href = file.url();
    mAttachementMap[ file.path() ] = partNum;
  }

  if ( mAttachmentStyle == "inline" && inlinePossible( att ) ) {
    if ( ct->isImage() ) {
      html += "<div><a href=\"" + href + "\">"
              "<img src=\"" + href + "\" border=\"0\"></a>"
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
      QString tmp = att->decodedText();
      /*if( ct->isHTMLText() )
        // ### to dangerous, we should use the same stuff as for the main text here
        html += tmp;
      else*/
        html += toHtmlString( tmp, ParseURL );
      // finish frame
      html += "</td></tr></table>";
    }
  } else { // icon
    QByteArray mimetype = ct->mimeType();
    kAsciiToLower( mimetype.data() );
    KMimeType::Ptr mimetypePtr = KMimeType::mimeType( mimetype );
    if(mimetypePtr.isNull()) {
      mimetypePtr = KMimeType::mimeType( "text/plain" );
    }
    QString iconName = mimetypePtr->iconName( QString() );
    QString iconFile = KIconLoader::global()->iconPath( iconName, KIconLoader::Desktop );
    html += "<div><a href=\"" + href + "\"><img src=\"" +
            iconFile + "\" border=\"0\">" + label +
            "</a></div><div>" + comment + "</div><br>";
  }
  mViewer->write( html );
}


QString ArticleWidget::toHtmlString( const QString &line, int flags )
{
  int llflags = KPIMUtils::LinkLocator::PreserveSpaces;
  if ( !(flags & ArticleWidget::ParseURL) )
    llflags |= KPIMUtils::LinkLocator::IgnoreUrls;
  if ( mFancyToggle->isChecked() && (flags & ArticleWidget::FancyFormatting) )
    llflags |= KPIMUtils::LinkLocator::ReplaceSmileys |
               KPIMUtils::LinkLocator::HighlightText;
  QString text = line;
  if ( flags & ArticleWidget::AllowROT13 ) {
    if ( mRot13 )
      text = MessageComposer::Util::rot13( line );
  }
  return KPIMUtils::LinkLocator::convertToHtml( text, llflags );
}


// from KMail headerstyle.cpp
QString ArticleWidget::imgToDataUrl( const QImage &image, const char* fmt  )
{
  QByteArray ba;
  QBuffer buffer( &ba );
  buffer.open( QIODevice::WriteOnly );
  image.save( &buffer, fmt );
  return QString::fromLatin1("data:image/%1;base64,%2")
      .arg( fmt ).arg( QString( KCodecs::base64Encode( ba ) ) );
}



int ArticleWidget::quotingDepth( const QString &line, const QString &quoteChars )
{
  int level = -1;
  for ( int i = 0; i < line.length(); ++i ) {
    // skip spaces
    if ( line[i].isSpace() )
      continue;
    if ( quoteChars.indexOf( line[i] ) != -1 )
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


bool ArticleWidget::canDecodeText( const QByteArray &charset ) const
{
  kDebug( 5003 ) << charset;
  if ( charset.isEmpty() )
    return false;
  bool ok = true;
  KGlobal::charsets()->codecForName( charset, ok );
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



KUrl ArticleWidget::writeAttachmentToTempFile( KMime::Content *att, int partNum )
{
  // more or less KMail code
  KTemporaryFile *tempFile = new KTemporaryFile();
  tempFile->setSuffix( '.' + QString::number( partNum ) );
  tempFile->open();
  KUrl file = KUrl( tempFile->fileName() );
  delete tempFile;

  if( ::access( QFile::encodeName( file.path() ), W_OK ) != 0 )
    // Not there or not writable
    if( KDE_mkdir( QFile::encodeName( file.path() ), 0 ) != 0 ||
        ::chmod( QFile::encodeName( file.path() ), S_IRWXU ) != 0 )
      return KUrl(); //failed create

  Q_ASSERT( !file.fileName().isNull() );

  mTempDirs.append( file.path() );
  // strip off a leading path
  KMime::Headers::ContentType* ct = att->contentType();
  QString attName = ct->name();
  int slashPos = attName.lastIndexOf( '/' );
  if( -1 != slashPos )
    attName = attName.mid( slashPos + 1 );
  if( attName.isEmpty() )
    attName = "unnamed";
  file.addPath( attName );

  QByteArray data = att->decodedContent();
  // ### KMail does crlf2lf conversion here before writing the file
  if( !KPIMUtils::kByteArrayToFile( data, file.toLocalFile(), false, false, false ) )
    return KUrl();

  mTempFiles.append( file.toLocalFile() );
  // make file read-only so that nobody gets the impression that he might
  // edit attached files
  ::chmod( QFile::encodeName( file.toLocalFile() ), S_IRUSR );

  return file;
}


void ArticleWidget::removeTempFiles( )
{
  for ( QStringList::Iterator it = mTempFiles.begin(); it != mTempFiles.end(); ++it )
    QFile::remove(*it);
  mTempFiles.clear();
  for ( QStringList::ConstIterator it = mTempDirs.constBegin(); it != mTempDirs.constEnd(); ++it )
    QDir(*it).rmdir(*it);
  mTempDirs.clear();
}



void ArticleWidget::processJob( KNJobData * job )
{
  if ( job->type() == KNJobData::JTfetchSource || job->type() == KNJobData::JTfetchArticle ) {
    if ( !job->canceled() ) {
      if ( !job->success() )
        KMessageBox::error( this, i18n("An error occurred while downloading the article source:\n%1",
            job->errorString() ) );
      else {
        KNRemoteArticle::Ptr a = boost::static_pointer_cast<KNRemoteArticle>( job->data() );
        new KNSourceViewWindow( a->head() + QLatin1Char('\n') + a->body() );
      }
    }
  }
  delete job;
}



typedef QList<ArticleWidget*>::ConstIterator InstanceIterator;

void ArticleWidget::configChanged()
{
  for( InstanceIterator it = mInstances.constBegin(); it != mInstances.constEnd(); ++it ) {
    (*it)->readConfig();
    (*it)->updateContents();
  }
}


bool ArticleWidget::articleVisible( KNArticle::Ptr article )
{
  for ( InstanceIterator it = mInstances.constBegin(); it != mInstances.constEnd(); ++it )
    if ( (*it)->article() == article )
      return true;
  return false;
}


void ArticleWidget::articleRemoved( KNArticle::Ptr article )
{
  for ( InstanceIterator it = mInstances.constBegin(); it != mInstances.constEnd(); ++it )
    if ( (*it)->article() == article )
      (*it)->setArticle( KNArticle::Ptr() );
}


void ArticleWidget::articleChanged( KNArticle::Ptr article )
{
  for ( InstanceIterator it = mInstances.constBegin(); it != mInstances.constEnd(); ++it )
    if ( (*it)->article() == article )
      (*it)->displayArticle();
}


void ArticleWidget::articleLoadError( KNArticle::Ptr article, const QString &error )
{
  for ( InstanceIterator it = mInstances.constBegin(); it != mInstances.constEnd(); ++it )
  if ( (*it)->article() == article )
    (*it)->displayErrorMessage( error );
}


void ArticleWidget::collectionRemoved( KNArticleCollection::Ptr coll )
{
  for ( InstanceIterator it = mInstances.constBegin(); it != mInstances.constEnd(); ++it )
    if ( (*it)->article() && (*it)->article()->collection() == coll )
      (*it)->setArticle( KNArticle::Ptr() );
}


void ArticleWidget::cleanup()
{
  for ( InstanceIterator it = mInstances.constBegin(); it != mInstances.constEnd(); ++it )
    (*it)->setArticle( KNArticle::Ptr() ); //delete orphant articles => avoid crash in destructor
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



void ArticleWidget::slotURLClicked( const KUrl &url, bool forceOpen)
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
    KMime::Types::Mailbox addr;
    addr.fromUnicodeString( url.path() );
    KNGlobals::self()->articleFactory()->createMail( &addr );
    return;
  }
  // handle news URL's
  if ( url.protocol() == "news" ) {
    kDebug( 5003 ) << url;
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
    if ( forceOpen || knGlobals.settings()->openAttachmentsOnClick() )
      knGlobals.articleManager()->openContent( c );
    else
      knGlobals.articleManager()->saveContentToFile( c, this );
    return;
  }
  // let KDE take care of the remaining protocols (http, ftp, etc.)
  new KRun( url, this );
}


void ArticleWidget::slotURLPopup( const QString &url, const QPoint &point )
{
  mCurrentURL = KUrl( url );
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
  QMenu *popup = static_cast<QMenu*>( mGuiClient->factory()->container( popupName, mGuiClient ) );
  if ( popup )
    popup->popup( point );
}



void ArticleWidget::slotTimeout()
{
  if ( mArticle && mArticle->type() == KNArticle::ATremote && !mArticle->isOrphant() ) {
    KNRemoteArticle::List l;
    l.append( boost::static_pointer_cast<KNRemoteArticle>( mArticle ) );
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
  QApplication::clipboard()->setText( mViewer->selectedText() );
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
  if ( mArticle && mArticle->type() == KNArticle::ATlocal && mArticle->hasContent() ) {
    new KNSourceViewWindow( mArticle->encodedContent( false ) );
  } else {
    // download remote article
    if ( mArticle && mArticle->type() == KNArticle::ATremote ) {
      KNGroup::Ptr g = boost::static_pointer_cast<KNGroup>( mArticle->collection() );
      KNRemoteArticle::Ptr a = KNRemoteArticle::Ptr( new KNRemoteArticle( g ) ); //we need "g" to access the nntp-account
      a->messageID( true )->from7BitString( mArticle->messageID()->as7BitString( false ) );
      a->lines( true )->from7BitString( mArticle->lines( true )->as7BitString( false ) );
      a->setArticleNumber( boost::static_pointer_cast<KNRemoteArticle>( mArticle )->articleNumber() );
      emitJob( new ArticleFetchJob( this, g->account(), a, false ) );
    }
  }
}


void ArticleWidget::slotReply()
{
  if ( mArticle && mArticle->type() == KNArticle::ATremote )
    KNGlobals::self()->articleFactory()->createReply( boost::static_pointer_cast<KNRemoteArticle>( mArticle ),
                                                      mViewer->selectedText(), true, false );
}


void ArticleWidget::slotRemail()
{
  if ( mArticle && mArticle->type()==KNArticle::ATremote )
    KNGlobals::self()->articleFactory()->createReply( boost::static_pointer_cast<KNRemoteArticle>( mArticle ),
                                                      mViewer->selectedText(), false, true );
}


void ArticleWidget::slotForward()
{
  KNGlobals::self()->articleFactory()->createForward( mArticle );
}


void ArticleWidget::slotCancel()
{
  KNGlobals::self()->articleFactory()->createCancel( mArticle );
}


void ArticleWidget::slotSupersede()
{
  KNGlobals::self()->articleFactory()->createSupersede( mArticle );
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

  if ( charset == i18nc( "@item default character set", "Default") ) {
    mForceCharset = false;
    mOverrideCharset = KMime::Headers::Latin1;
  } else {
    mForceCharset = true;
    mOverrideCharset = KGlobal::charsets()->encodingForName( charset ).toLatin1();
  }

  if ( mArticle && mArticle->hasContent() ) {
    mArticle->setDefaultCharset( mOverrideCharset );  // the article will choose the correct default,
    mArticle->setForceDefaultCharset( mForceCharset );     // when we disable the overdrive
    updateContents();
  }
}


void ArticleWidget::slotSetCharsetKeyboard( )
{
  int charset = KNHelper::selectDialog( this, i18n("Select Charset"),
    mCharsetSelect->items(), mCharsetSelect->currentItem() );
  if ( charset != -1 ) {
    mCharsetSelect->setCurrentItem( charset );
    slotSetCharset( mCharsetSelect->items()[charset] );
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
  QString filename = KStandardDirs::locateLocal( "data", QString::fromLatin1("konqueror/bookmarks.xml") );
  KBookmarkManager *bookManager = KBookmarkManager::managerForFile( filename, "konqueror" );
  KBookmarkGroup group = bookManager->root();
  group.addBookmark( mCurrentURL.url(), mCurrentURL );
  bookManager->save();
}

void ArticleWidget::slotAddToAddressBook()
{
  KPIM::AddEmailAddressJob *job = new KPIM::AddEmailAddressJob( mCurrentURL.path(), this, this );
  job->start();
}

void ArticleWidget::slotOpenInAddressBook()
{
  KPIM::OpenEmailAddressJob *job = new KPIM::OpenEmailAddressJob( mCurrentURL.path(), this, this );
  job->start();
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

