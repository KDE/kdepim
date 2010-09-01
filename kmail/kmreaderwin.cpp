// -*- mode: C++; c-file-style: "gnu" -*-
// kmreaderwin.cpp
// Author: Markus Wuebben <markus.wuebben@kde.org>

// define this to copy all html that is written to the readerwindow to
// filehtmlwriter.out in the current working directory
//#define KMAIL_READER_HTML_DEBUG 1

#include <config.h>

#include "kmreaderwin.h"

#include "globalsettings.h"
#include "kmversion.h"
#include "kmmainwidget.h"
#include "kmreadermainwin.h"
#include <libkdepim/kfileio.h>
#include "kmfolderindex.h"
#include "kmcommands.h"
#include "kmmsgpartdlg.h"
#include "mailsourceviewer.h"
using KMail::MailSourceViewer;
#include "partNode.h"
#include "kmmsgdict.h"
#include "messagesender.h"
#include "kcursorsaver.h"
#include "kmfolder.h"
#include "vcardviewer.h"
using KMail::VCardViewer;
#include "objecttreeparser.h"
using KMail::ObjectTreeParser;
#include "partmetadata.h"
using KMail::PartMetaData;
#include "attachmentstrategy.h"
using KMail::AttachmentStrategy;
#include "headerstrategy.h"
using KMail::HeaderStrategy;
#include "headerstyle.h"
using KMail::HeaderStyle;
#include "khtmlparthtmlwriter.h"
using KMail::HtmlWriter;
using KMail::KHtmlPartHtmlWriter;
#include "htmlstatusbar.h"
using KMail::HtmlStatusBar;
#include "folderjob.h"
using KMail::FolderJob;
#include "csshelper.h"
using KMail::CSSHelper;
#include "isubject.h"
using KMail::ISubject;
#include "urlhandlermanager.h"
using KMail::URLHandlerManager;
#include "interfaces/observable.h"
#include "util.h"
#include "kmheaders.h"

#include "broadcaststatus.h"

#include <kmime_mdn.h>
using namespace KMime;
#ifdef KMAIL_READER_HTML_DEBUG
#include "filehtmlwriter.h"
using KMail::FileHtmlWriter;
#include "teehtmlwriter.h"
using KMail::TeeHtmlWriter;
#endif

#include <kasciistringtools.h>
#include <kstringhandler.h>

#include <mimelib/mimepp.h>
#include <mimelib/body.h>
#include <mimelib/utility.h>

#include <kleo/specialjob.h>
#include <kleo/cryptobackend.h>
#include <kleo/cryptobackendfactory.h>

// KABC includes
#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>

// khtml headers
#include <khtml_part.h>
#include <khtmlview.h> // So that we can get rid of the frames
#include <dom/html_element.h>
#include <dom/html_block.h>
#include <dom/html_document.h>
#include <dom/dom_string.h>
#include <dom/dom_exception.h>

#include <kapplication.h>
// for the click on attachment stuff (dnaber):
#include <kuserprofile.h>
#include <kcharsets.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>  // Sven's : for access and getpid
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <krun.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <kdialog.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kmdcodec.h>
#include <kasciistricmp.h>
#include <kurldrag.h>

#include <tqclipboard.h>
#include <tqhbox.h>
#include <tqtextcodec.h>
#include <tqpaintdevicemetrics.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqsplitter.h>
#include <tqstyle.h>

// X headers...
#undef Never
#undef Always

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

class NewByteArray : public QByteArray
{
public:
    NewByteArray &appendNULL();
    NewByteArray &operator+=( const char * );
    NewByteArray &operator+=( const TQByteArray & );
    NewByteArray &operator+=( const TQCString & );
    TQByteArray& qByteArray();
};

NewByteArray& NewByteArray::appendNULL()
{
    TQByteArray::detach();
    uint len1 = size();
    if ( !TQByteArray::resize( len1 + 1 ) )
        return *this;
    *(data() + len1) = '\0';
    return *this;
}
NewByteArray& NewByteArray::operator+=( const char * newData )
{
    if ( !newData )
        return *this;
    TQByteArray::detach();
    uint len1 = size();
    uint len2 = qstrlen( newData );
    if ( !TQByteArray::resize( len1 + len2 ) )
        return *this;
    memcpy( data() + len1, newData, len2 );
    return *this;
}
NewByteArray& NewByteArray::operator+=( const TQByteArray & newData )
{
    if ( newData.isNull() )
        return *this;
    TQByteArray::detach();
    uint len1 = size();
    uint len2 = newData.size();
    if ( !TQByteArray::resize( len1 + len2 ) )
        return *this;
    memcpy( data() + len1, newData.data(), len2 );
    return *this;
}
NewByteArray& NewByteArray::operator+=( const TQCString & newData )
{
    if ( newData.isEmpty() )
        return *this;
    TQByteArray::detach();
    uint len1 = size();
    uint len2 = newData.length(); // forget about the trailing 0x00 !
    if ( !TQByteArray::resize( len1 + len2 ) )
        return *this;
    memcpy( data() + len1, newData.data(), len2 );
    return *this;
}
TQByteArray& NewByteArray::qByteArray()
{
    return *((TQByteArray*)this);
}

// This function returns the complete data that were in this
// message parts - *after* all encryption has been removed that
// could be removed.
// - This is used to store the message in decrypted form.
void KMReaderWin::objectTreeToDecryptedMsg( partNode* node,
                                            NewByteArray& resultingData,
                                            KMMessage& theMessage,
                                            bool weAreReplacingTheRootNode,
                                            int recCount )
{
  kdDebug(5006) << TQString("-------------------------------------------------" ) << endl;
  kdDebug(5006) << TQString("KMReaderWin::objectTreeToDecryptedMsg( %1 )  START").arg( recCount ) << endl;
  if( node ) {

    kdDebug(5006) << node->typeString() << '/' << node->subTypeString() << endl;

    partNode* curNode = node;
    partNode* dataNode = curNode;
    partNode * child = node->firstChild();
    const bool bIsMultipart = node->type() == DwMime::kTypeMultipart ;
    bool bKeepPartAsIs = false;

    switch( curNode->type() ){
      case DwMime::kTypeMultipart: {
          switch( curNode->subType() ){
          case DwMime::kSubtypeSigned: {
              bKeepPartAsIs = true;
            }
            break;
          case DwMime::kSubtypeEncrypted: {
              if ( child )
                  dataNode = child;
            }
            break;
          }
        }
        break;
      case DwMime::kTypeMessage: {
          switch( curNode->subType() ){
          case DwMime::kSubtypeRfc822: {
              if ( child )
                dataNode = child;
            }
            break;
          }
        }
        break;
      case DwMime::kTypeApplication: {
          switch( curNode->subType() ){
          case DwMime::kSubtypeOctetStream: {
              if ( child )
                dataNode = child;
            }
            break;
          case DwMime::kSubtypePkcs7Signature: {
              // note: subtype Pkcs7Signature specifies a signature part
              //       which we do NOT want to remove!
              bKeepPartAsIs = true;
            }
            break;
          case DwMime::kSubtypePkcs7Mime: {
              // note: subtype Pkcs7Mime can also be signed
              //       and we do NOT want to remove the signature!
              if ( child && curNode->encryptionState() != KMMsgNotEncrypted )
                dataNode = child;
            }
            break;
          }
        }
        break;
    }


    DwHeaders& rootHeaders( theMessage.headers() );
    DwBodyPart * part = dataNode->dwPart() ? dataNode->dwPart() : 0;
    DwHeaders * headers(
        (part && part->hasHeaders())
        ? &part->Headers()
        : (  (weAreReplacingTheRootNode || !dataNode->parentNode())
            ? &rootHeaders
            : 0 ) );
    if( dataNode == curNode ) {
kdDebug(5006) << "dataNode == curNode:  Save curNode without replacing it." << endl;

      // A) Store the headers of this part IF curNode is not the root node
      //    AND we are not replacing a node that already *has* replaced
      //    the root node in previous recursion steps of this function...
      if( headers ) {
        if( dataNode->parentNode() && !weAreReplacingTheRootNode ) {
kdDebug(5006) << "dataNode is NOT replacing the root node:  Store the headers." << endl;
          resultingData += headers->AsString().c_str();
        } else if( weAreReplacingTheRootNode && part && part->hasHeaders() ){
kdDebug(5006) << "dataNode replace the root node:  Do NOT store the headers but change" << endl;
kdDebug(5006) << "                                 the Message's headers accordingly." << endl;
kdDebug(5006) << "              old Content-Type = " << rootHeaders.ContentType().AsString().c_str() << endl;
kdDebug(5006) << "              new Content-Type = " << headers->ContentType(   ).AsString().c_str() << endl;
          rootHeaders.ContentType()             = headers->ContentType();
          theMessage.setContentTransferEncodingStr(
              headers->HasContentTransferEncoding()
            ? headers->ContentTransferEncoding().AsString().c_str()
            : "" );
          rootHeaders.ContentDescription() = headers->ContentDescription();
          rootHeaders.ContentDisposition() = headers->ContentDisposition();
          theMessage.setNeedsAssembly();
        }
      }

      if ( bKeepPartAsIs ) {
          resultingData += dataNode->encodedBody();
      } else {

      // B) Store the body of this part.
      if( headers && bIsMultipart && dataNode->firstChild() )  {
kdDebug(5006) << "is valid Multipart, processing children:" << endl;
        TQCString boundary = headers->ContentType().Boundary().c_str();
        curNode = dataNode->firstChild();
        // store children of multipart
        while( curNode ) {
kdDebug(5006) << "--boundary" << endl;
          if( resultingData.size() &&
              ( '\n' != resultingData.at( resultingData.size()-1 ) ) )
            resultingData += TQCString( "\n" );
          resultingData += TQCString( "\n" );
          resultingData += "--";
          resultingData += boundary;
          resultingData += "\n";
          // note: We are processing a harmless multipart that is *not*
          //       to be replaced by one of it's children, therefor
          //       we set their doStoreHeaders to true.
          objectTreeToDecryptedMsg( curNode,
                                    resultingData,
                                    theMessage,
                                    false,
                                    recCount + 1 );
          curNode = curNode->nextSibling();
        }
kdDebug(5006) << "--boundary--" << endl;
        resultingData += "\n--";
        resultingData += boundary;
        resultingData += "--\n\n";
kdDebug(5006) << "Multipart processing children - DONE" << endl;
      } else if( part ){
        // store simple part
kdDebug(5006) << "is Simple part or invalid Multipart, storing body data .. DONE" << endl;
        resultingData += part->Body().AsString().c_str();
      }
      }
    } else {
kdDebug(5006) << "dataNode != curNode:  Replace curNode by dataNode." << endl;
      bool rootNodeReplaceFlag = weAreReplacingTheRootNode || !curNode->parentNode();
      if( rootNodeReplaceFlag ) {
kdDebug(5006) << "                      Root node will be replaced." << endl;
      } else {
kdDebug(5006) << "                      Root node will NOT be replaced." << endl;
      }
      // store special data to replace the current part
      // (e.g. decrypted data or embedded RfC 822 data)
      objectTreeToDecryptedMsg( dataNode,
                                resultingData,
                                theMessage,
                                rootNodeReplaceFlag,
                                recCount + 1 );
    }
  }
  kdDebug(5006) << TQString("\nKMReaderWin::objectTreeToDecryptedMsg( %1 )  END").arg( recCount ) << endl;
}


/*
 ===========================================================================


        E N D    O F     T E M P O R A R Y     M I M E     C O D E


 ===========================================================================
*/











void KMReaderWin::createWidgets() {
  TQVBoxLayout * vlay = new TQVBoxLayout( this );
  mSplitter = new TQSplitter( Qt::Vertical, this, "mSplitter" );
  vlay->addWidget( mSplitter );
  mMimePartTree = new KMMimePartTree( this, mSplitter, "mMimePartTree" );
  mBox = new TQHBox( mSplitter, "mBox" );
  setStyleDependantFrameWidth();
  mBox->setFrameStyle( mMimePartTree->frameStyle() );
  mColorBar = new HtmlStatusBar( mBox, "mColorBar" );
  mViewer = new KHTMLPart( mBox, "mViewer" );
  mSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  mSplitter->setResizeMode( mMimePartTree, TQSplitter::KeepSize );
}

const int KMReaderWin::delay = 150;

//-----------------------------------------------------------------------------
KMReaderWin::KMReaderWin(TQWidget *aParent,
			 TQWidget *mainWindow,
			 KActionCollection* actionCollection,
                         const char *aName,
                         int aFlags )
  : TQWidget(aParent, aName, aFlags | Qt::WDestructiveClose),
    mSerNumOfOriginalMessage( 0 ),
    mNodeIdOffset( -1 ),
    mAttachmentStrategy( 0 ),
    mHeaderStrategy( 0 ),
    mHeaderStyle( 0 ),
    mUpdateReaderWinTimer( 0, "mUpdateReaderWinTimer" ),
    mResizeTimer( 0, "mResizeTimer" ),
    mDelayedMarkTimer( 0, "mDelayedMarkTimer" ),
    mOldGlobalOverrideEncoding( "---" ), // init with dummy value
    mCSSHelper( 0 ),
    mRootNode( 0 ),
    mMainWindow( mainWindow ),
    mActionCollection( actionCollection ),
    mMailToComposeAction( 0 ),
    mMailToReplyAction( 0 ),
    mMailToForwardAction( 0 ),
    mAddAddrBookAction( 0 ),
    mOpenAddrBookAction( 0 ),
    mCopyAction( 0 ),
    mCopyURLAction( 0 ),
    mUrlOpenAction( 0 ),
    mUrlSaveAsAction( 0 ),
    mAddBookmarksAction( 0 ),
    mStartIMChatAction( 0 ),
    mSelectAllAction( 0 ),
    mHeaderOnlyAttachmentsAction( 0 ),
    mSelectEncodingAction( 0 ),
    mToggleFixFontAction( 0 ),
    mCanStartDrag( false ),
    mHtmlWriter( 0 ),
    mSavedRelativePosition( 0 ),
    mDecrytMessageOverwrite( false ),
    mShowSignatureDetails( false ),
    mShowAttachmentQuicklist( true ),
    mShowRawToltecMail( false )
{
  mExternalWindow  = (aParent == mainWindow );
  mSplitterSizes << 180 << 100;
  mMimeTreeMode = 1;
  mMimeTreeAtBottom = true;
  mAutoDelete = false;
  mLastSerNum = 0;
  mWaitingForSerNum = 0;
  mMessage = 0;
  mMsgDisplay = true;
  mPrinting = false;
  mShowColorbar = false;
  mAtmUpdate = false;

  createWidgets();
  createActions( actionCollection );
  initHtmlWidget();
  readConfig();

  mHtmlOverride = false;
  mHtmlLoadExtOverride = false;

  mLevelQuote = GlobalSettings::self()->collapseQuoteLevelSpin() - 1;

  connect( &mUpdateReaderWinTimer, TQT_SIGNAL(timeout()),
  	   this, TQT_SLOT(updateReaderWin()) );
  connect( &mResizeTimer, TQT_SIGNAL(timeout()),
  	   this, TQT_SLOT(slotDelayedResize()) );
  connect( &mDelayedMarkTimer, TQT_SIGNAL(timeout()),
           this, TQT_SLOT(slotTouchMessage()) );

}

void KMReaderWin::createActions( KActionCollection * ac ) {
  if ( !ac )
      return;

  KRadioAction *raction = 0;

  // header style
  KActionMenu *headerMenu =
    new KActionMenu( i18n("View->", "&Headers"), ac, "view_headers" );
  headerMenu->setToolTip( i18n("Choose display style of message headers") );

  connect( headerMenu, TQT_SIGNAL(activated()),
           this, TQT_SLOT(slotCycleHeaderStyles()) );

  raction = new KRadioAction( i18n("View->headers->", "&Enterprise Headers"), 0,
                              this, TQT_SLOT(slotEnterpriseHeaders()),
                              ac, "view_headers_enterprise" );
  raction->setToolTip( i18n("Show the list of headers in Enterprise style") );
  raction->setExclusiveGroup( "view_headers_group" );
  headerMenu->insert(raction);

  raction = new KRadioAction( i18n("View->headers->", "&Fancy Headers"), 0,
                              this, TQT_SLOT(slotFancyHeaders()),
                              ac, "view_headers_fancy" );
  raction->setToolTip( i18n("Show the list of headers in a fancy format") );
  raction->setExclusiveGroup( "view_headers_group" );
  headerMenu->insert( raction );

  raction = new KRadioAction( i18n("View->headers->", "&Brief Headers"), 0,
                              this, TQT_SLOT(slotBriefHeaders()),
                              ac, "view_headers_brief" );
  raction->setToolTip( i18n("Show brief list of message headers") );
  raction->setExclusiveGroup( "view_headers_group" );
  headerMenu->insert( raction );

  raction = new KRadioAction( i18n("View->headers->", "&Standard Headers"), 0,
                              this, TQT_SLOT(slotStandardHeaders()),
                              ac, "view_headers_standard" );
  raction->setToolTip( i18n("Show standard list of message headers") );
  raction->setExclusiveGroup( "view_headers_group" );
  headerMenu->insert( raction );

  raction = new KRadioAction( i18n("View->headers->", "&Long Headers"), 0,
                              this, TQT_SLOT(slotLongHeaders()),
                              ac, "view_headers_long" );
  raction->setToolTip( i18n("Show long list of message headers") );
  raction->setExclusiveGroup( "view_headers_group" );
  headerMenu->insert( raction );

  raction = new KRadioAction( i18n("View->headers->", "&All Headers"), 0,
                              this, TQT_SLOT(slotAllHeaders()),
                              ac, "view_headers_all" );
  raction->setToolTip( i18n("Show all message headers") );
  raction->setExclusiveGroup( "view_headers_group" );
  headerMenu->insert( raction );

  // attachment style
  KActionMenu *attachmentMenu =
    new KActionMenu( i18n("View->", "&Attachments"), ac, "view_attachments" );
  attachmentMenu->setToolTip( i18n("Choose display style of attachments") );
  connect( attachmentMenu, TQT_SIGNAL(activated()),
           this, TQT_SLOT(slotCycleAttachmentStrategy()) );

  raction = new KRadioAction( i18n("View->attachments->", "&As Icons"), 0,
                              this, TQT_SLOT(slotIconicAttachments()),
                              ac, "view_attachments_as_icons" );
  raction->setToolTip( i18n("Show all attachments as icons. Click to see them.") );
  raction->setExclusiveGroup( "view_attachments_group" );
  attachmentMenu->insert( raction );

  raction = new KRadioAction( i18n("View->attachments->", "&Smart"), 0,
                              this, TQT_SLOT(slotSmartAttachments()),
                              ac, "view_attachments_smart" );
  raction->setToolTip( i18n("Show attachments as suggested by sender.") );
  raction->setExclusiveGroup( "view_attachments_group" );
  attachmentMenu->insert( raction );

  raction = new KRadioAction( i18n("View->attachments->", "&Inline"), 0,
                              this, TQT_SLOT(slotInlineAttachments()),
                              ac, "view_attachments_inline" );
  raction->setToolTip( i18n("Show all attachments inline (if possible)") );
  raction->setExclusiveGroup( "view_attachments_group" );
  attachmentMenu->insert( raction );

  raction = new KRadioAction( i18n("View->attachments->", "&Hide"), 0,
                              this, TQT_SLOT(slotHideAttachments()),
                              ac, "view_attachments_hide" );
  raction->setToolTip( i18n("Do not show attachments in the message viewer") );
  raction->setExclusiveGroup( "view_attachments_group" );
  attachmentMenu->insert( raction );

  mHeaderOnlyAttachmentsAction = new KRadioAction( i18n( "View->attachments->", "In Header &Only" ), 0,
                              this, TQT_SLOT( slotHeaderOnlyAttachments() ),
                              ac, "view_attachments_headeronly" );
  mHeaderOnlyAttachmentsAction->setToolTip( i18n( "Show Attachments only in the header of the mail" ) );
  mHeaderOnlyAttachmentsAction->setExclusiveGroup( "view_attachments_group" );
  attachmentMenu->insert( mHeaderOnlyAttachmentsAction );

  // Set Encoding submenu
  mSelectEncodingAction = new KSelectAction( i18n( "&Set Encoding" ), "charset", 0,
                                 this, TQT_SLOT( slotSetEncoding() ),
                                 ac, "encoding" );
  TQStringList encodings = KMMsgBase::supportedEncodings( false );
  encodings.prepend( i18n( "Auto" ) );
  mSelectEncodingAction->setItems( encodings );
  mSelectEncodingAction->setCurrentItem( 0 );

  mMailToComposeAction = new KAction( i18n("New Message To..."), "mail_new",
                                      0, this, TQT_SLOT(slotMailtoCompose()), ac,
                                      "mailto_compose" );
  mMailToReplyAction = new KAction( i18n("Reply To..."), "mail_reply",
                                    0, this, TQT_SLOT(slotMailtoReply()), ac,
				    "mailto_reply" );
  mMailToForwardAction = new KAction( i18n("Forward To..."), "mail_forward",
                                      0, this, TQT_SLOT(slotMailtoForward()), ac,
                                      "mailto_forward" );
  mAddAddrBookAction = new KAction( i18n("Add to Address Book"),
				    0, this, TQT_SLOT(slotMailtoAddAddrBook()),
				    ac, "add_addr_book" );
  mOpenAddrBookAction = new KAction( i18n("Open in Address Book"),
                                     0, this, TQT_SLOT(slotMailtoOpenAddrBook()),
                                     ac, "openin_addr_book" );
  mCopyAction = KStdAction::copy( this, TQT_SLOT(slotCopySelectedText()), ac, "kmail_copy");
  mSelectAllAction = new KAction( i18n("Select All Text"), CTRL+SHIFT+Key_A, this,
                                  TQT_SLOT(selectAll()), ac, "mark_all_text" );
  mCopyURLAction = new KAction( i18n("Copy Link Address"), 0, this,
				TQT_SLOT(slotUrlCopy()), ac, "copy_url" );
  mUrlOpenAction = new KAction( i18n("Open URL"), 0, this,
                                TQT_SLOT(slotUrlOpen()), ac, "open_url" );
  mAddBookmarksAction = new KAction( i18n("Bookmark This Link"),
                                     "bookmark_add",
                                     0, this, TQT_SLOT(slotAddBookmarks()),
                                     ac, "add_bookmarks" );
  mUrlSaveAsAction = new KAction( i18n("Save Link As..."), 0, this,
                                  TQT_SLOT(slotUrlSave()), ac, "saveas_url" );

  mToggleFixFontAction = new KToggleAction( i18n("Use Fi&xed Font"),
                                            Key_X, this, TQT_SLOT(slotToggleFixedFont()),
                                            ac, "toggle_fixedfont" );

  mStartIMChatAction = new KAction( i18n("Chat &With..."), 0, this,
				    TQT_SLOT(slotIMChat()), ac, "start_im_chat" );
}

// little helper function
KRadioAction *KMReaderWin::actionForHeaderStyle( const HeaderStyle * style, const HeaderStrategy * strategy ) {
  if ( !mActionCollection )
    return 0;
  const char * actionName = 0;
  if ( style == HeaderStyle::enterprise() )
    actionName = "view_headers_enterprise";
  if ( style == HeaderStyle::fancy() )
    actionName = "view_headers_fancy";
  else if ( style == HeaderStyle::brief() )
    actionName = "view_headers_brief";
  else if ( style == HeaderStyle::plain() ) {
    if ( strategy == HeaderStrategy::standard() )
      actionName = "view_headers_standard";
    else if ( strategy == HeaderStrategy::rich() )
      actionName = "view_headers_long";
    else if ( strategy == HeaderStrategy::all() )
      actionName = "view_headers_all";
  }
  if ( actionName )
    return static_cast<KRadioAction*>(mActionCollection->action(actionName));
  else
    return 0;
}

KRadioAction *KMReaderWin::actionForAttachmentStrategy( const AttachmentStrategy * as ) {
  if ( !mActionCollection )
    return 0;
  const char * actionName = 0;
  if ( as == AttachmentStrategy::iconic() )
    actionName = "view_attachments_as_icons";
  else if ( as == AttachmentStrategy::smart() )
    actionName = "view_attachments_smart";
  else if ( as == AttachmentStrategy::inlined() )
    actionName = "view_attachments_inline";
  else if ( as == AttachmentStrategy::hidden() )
    actionName = "view_attachments_hide";
  else if ( as == AttachmentStrategy::headerOnly() )
    actionName = "view_attachments_headeronly";

  if ( actionName )
    return static_cast<KRadioAction*>(mActionCollection->action(actionName));
  else
    return 0;
}

void KMReaderWin::slotEnterpriseHeaders() {
  setHeaderStyleAndStrategy( HeaderStyle::enterprise(),
                             HeaderStrategy::rich() );
  if( !mExternalWindow )
     writeConfig();
}

void KMReaderWin::slotFancyHeaders() {
  setHeaderStyleAndStrategy( HeaderStyle::fancy(),
                             HeaderStrategy::rich() );
  if( !mExternalWindow )
     writeConfig();
}

void KMReaderWin::slotBriefHeaders() {
  setHeaderStyleAndStrategy( HeaderStyle::brief(),
                             HeaderStrategy::brief() );
  if( !mExternalWindow )
     writeConfig();
}

void KMReaderWin::slotStandardHeaders() {
  setHeaderStyleAndStrategy( HeaderStyle::plain(),
                             HeaderStrategy::standard());
  writeConfig();
}

void KMReaderWin::slotLongHeaders() {
  setHeaderStyleAndStrategy( HeaderStyle::plain(),
                             HeaderStrategy::rich() );
  if( !mExternalWindow )
     writeConfig();
}

void KMReaderWin::slotAllHeaders() {
  setHeaderStyleAndStrategy( HeaderStyle::plain(),
                             HeaderStrategy::all() );
  if( !mExternalWindow )
     writeConfig();
}

void KMReaderWin::slotLevelQuote( int l )
{
  mLevelQuote = l;
  saveRelativePosition();
  update(true);
}

void KMReaderWin::slotCycleHeaderStyles() {
  const HeaderStrategy * strategy = headerStrategy();
  const HeaderStyle * style = headerStyle();

  const char * actionName = 0;
  if ( style == HeaderStyle::enterprise() ) {
    slotFancyHeaders();
    actionName = "view_headers_fancy";
  }
  if ( style == HeaderStyle::fancy() ) {
    slotBriefHeaders();
    actionName = "view_headers_brief";
  } else if ( style == HeaderStyle::brief() ) {
    slotStandardHeaders();
    actionName = "view_headers_standard";
  } else if ( style == HeaderStyle::plain() ) {
    if ( strategy == HeaderStrategy::standard() ) {
      slotLongHeaders();
      actionName = "view_headers_long";
    } else if ( strategy == HeaderStrategy::rich() ) {
      slotAllHeaders();
      actionName = "view_headers_all";
    } else if ( strategy == HeaderStrategy::all() ) {
      slotEnterpriseHeaders();
      actionName = "view_headers_enterprise";
    }
  }

  if ( actionName )
    static_cast<KRadioAction*>( mActionCollection->action( actionName ) )->setChecked( true );
}


void KMReaderWin::slotIconicAttachments() {
  setAttachmentStrategy( AttachmentStrategy::iconic() );
}

void KMReaderWin::slotSmartAttachments() {
  setAttachmentStrategy( AttachmentStrategy::smart() );
}

void KMReaderWin::slotInlineAttachments() {
  setAttachmentStrategy( AttachmentStrategy::inlined() );
}

void KMReaderWin::slotHideAttachments() {
  setAttachmentStrategy( AttachmentStrategy::hidden() );
}

void KMReaderWin::slotHeaderOnlyAttachments() {
  setAttachmentStrategy( AttachmentStrategy::headerOnly() );
}

void KMReaderWin::slotCycleAttachmentStrategy() {
  setAttachmentStrategy( attachmentStrategy()->next() );
  KRadioAction * action = actionForAttachmentStrategy( attachmentStrategy() );
  assert( action );
  action->setChecked( true );
}


//-----------------------------------------------------------------------------
KMReaderWin::~KMReaderWin()
{
  clearBodyPartMementos();
  delete mHtmlWriter; mHtmlWriter = 0;
  delete mCSSHelper;
  if (mAutoDelete) delete message();
  delete mRootNode; mRootNode = 0;
  removeTempFiles();
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotMessageArrived( KMMessage *msg )
{
  if (msg && ((KMMsgBase*)msg)->isMessage()) {
    if ( msg->getMsgSerNum() == mWaitingForSerNum ) {
      setMsg( msg, true );
    } else {
      //kdDebug( 5006 ) <<  "KMReaderWin::slotMessageArrived - ignoring update" << endl;
    }
  }
}

//-----------------------------------------------------------------------------
void KMReaderWin::update( KMail::Interface::Observable * observable )
{
  if ( !mAtmUpdate ) {
    // reparse the msg
    //kdDebug(5006) << "KMReaderWin::update - message" << endl;
    updateReaderWin();
    return;
  }

  if ( !mRootNode )
    return;

  KMMessage* msg = static_cast<KMMessage*>( observable );
  assert( msg != 0 );

  // find our partNode and update it
  if ( !msg->lastUpdatedPart() ) {
    kdDebug(5006) << "KMReaderWin::update - no updated part" << endl;
    return;
  }
  partNode* node = mRootNode->findNodeForDwPart( msg->lastUpdatedPart() );
  if ( !node ) {
    kdDebug(5006) << "KMReaderWin::update - can't find node for part" << endl;
    return;
  }
  node->setDwPart( msg->lastUpdatedPart() );

  // update the tmp file
  // we have to set it writeable temporarily
  ::chmod( TQFile::encodeName( mAtmCurrentName ), S_IRWXU );
  TQByteArray data = node->msgPart().bodyDecodedBinary();
  size_t size = data.size();
  if ( node->msgPart().type() == DwMime::kTypeText && size) {
    size = KMail::Util::crlf2lf( data.data(), size );
  }
  KPIM::kBytesToFile( data.data(), size, mAtmCurrentName, false, false, false );
  ::chmod( TQFile::encodeName( mAtmCurrentName ), S_IRUSR );

  mAtmUpdate = false;
}

//-----------------------------------------------------------------------------
void KMReaderWin::removeTempFiles()
{
  for (TQStringList::Iterator it = mTempFiles.begin(); it != mTempFiles.end();
    it++)
  {
    TQFile::remove(*it);
  }
  mTempFiles.clear();
  for (TQStringList::Iterator it = mTempDirs.begin(); it != mTempDirs.end();
    it++)
  {
    TQDir(*it).rmdir(*it);
  }
  mTempDirs.clear();
}


//-----------------------------------------------------------------------------
bool KMReaderWin::event(TQEvent *e)
{
  if (e->type() == TQEvent::ApplicationPaletteChange)
  {
    delete mCSSHelper;
    mCSSHelper = new KMail::CSSHelper( 	TQPaintDeviceMetrics( mViewer->view() ) );
    if (message())
      message()->readConfig();
    update( true ); // Force update
    return true;
  }
  return TQWidget::event(e);
}


//-----------------------------------------------------------------------------
void KMReaderWin::readConfig(void)
{
  const KConfigGroup mdnGroup( KMKernel::config(), "MDN" );
  /*should be: const*/ KConfigGroup reader( KMKernel::config(), "Reader" );

  delete mCSSHelper;
  mCSSHelper = new KMail::CSSHelper( TQPaintDeviceMetrics( mViewer->view() ) );

  mNoMDNsWhenEncrypted = mdnGroup.readBoolEntry( "not-send-when-encrypted", true );

  mUseFixedFont = reader.readBoolEntry( "useFixedFont", false );
  if ( mToggleFixFontAction )
    mToggleFixFontAction->setChecked( mUseFixedFont );

  mHtmlMail = reader.readBoolEntry( "htmlMail", false );
  mHtmlLoadExternal = reader.readBoolEntry( "htmlLoadExternal", false );

  setHeaderStyleAndStrategy( HeaderStyle::create( reader.readEntry( "header-style", "fancy" ) ),
			     HeaderStrategy::create( reader.readEntry( "header-set-displayed", "rich" ) ) );
  KRadioAction *raction = actionForHeaderStyle( headerStyle(), headerStrategy() );
  if ( raction )
    raction->setChecked( true );

  setAttachmentStrategy( AttachmentStrategy::create( reader.readEntry( "attachment-strategy", "smart" ) ) );
  raction = actionForAttachmentStrategy( attachmentStrategy() );
  if ( raction )
    raction->setChecked( true );

  // if the user uses OpenPGP then the color bar defaults to enabled
  // else it defaults to disabled
  mShowColorbar = reader.readBoolEntry( "showColorbar", Kpgp::Module::getKpgp()->usePGP() );
  // if the value defaults to enabled and KMail (with color bar) is used for
  // the first time the config dialog doesn't know this if we don't save the
  // value now
  reader.writeEntry( "showColorbar", mShowColorbar );

  mMimeTreeAtBottom = reader.readEntry( "MimeTreeLocation", "bottom" ) != "top";
  const TQString s = reader.readEntry( "MimeTreeMode", "smart" );
  if ( s == "never" )
    mMimeTreeMode = 0;
  else if ( s == "always" )
    mMimeTreeMode = 2;
  else
    mMimeTreeMode = 1;

  const int mimeH = reader.readNumEntry( "MimePaneHeight", 100 );
  const int messageH = reader.readNumEntry( "MessagePaneHeight", 180 );
  mSplitterSizes.clear();
  if ( mMimeTreeAtBottom )
    mSplitterSizes << messageH << mimeH;
  else
    mSplitterSizes << mimeH << messageH;

  adjustLayout();

  readGlobalOverrideCodec();

  if (message())
    update();
  KMMessage::readConfig();
}


void KMReaderWin::adjustLayout() {
  if ( mMimeTreeAtBottom )
    mSplitter->moveToLast( mMimePartTree );
  else
    mSplitter->moveToFirst( mMimePartTree );
  mSplitter->setSizes( mSplitterSizes );

  if ( mMimeTreeMode == 2 && mMsgDisplay )
    mMimePartTree->show();
  else
    mMimePartTree->hide();

  if ( mShowColorbar && mMsgDisplay )
    mColorBar->show();
  else
    mColorBar->hide();
}


void KMReaderWin::saveSplitterSizes( KConfigBase & c ) const {
  if ( !mSplitter || !mMimePartTree )
    return;
  if ( mMimePartTree->isHidden() )
    return; // don't rely on TQSplitter maintaining sizes for hidden widgets.

  c.writeEntry( "MimePaneHeight", mSplitter->sizes()[ mMimeTreeAtBottom ? 1 : 0 ] );
  c.writeEntry( "MessagePaneHeight", mSplitter->sizes()[ mMimeTreeAtBottom ? 0 : 1 ] );
}

//-----------------------------------------------------------------------------
void KMReaderWin::writeConfig( bool sync ) const {
  KConfigGroup reader( KMKernel::config(), "Reader" );

  reader.writeEntry( "useFixedFont", mUseFixedFont );
  if ( headerStyle() )
    reader.writeEntry( "header-style", headerStyle()->name() );
  if ( headerStrategy() )
    reader.writeEntry( "header-set-displayed", headerStrategy()->name() );
  if ( attachmentStrategy() )
    reader.writeEntry( "attachment-strategy", attachmentStrategy()->name() );

  saveSplitterSizes( reader );

  if ( sync )
    kmkernel->slotRequestConfigSync();
}

//-----------------------------------------------------------------------------
void KMReaderWin::initHtmlWidget(void)
{
  mViewer->widget()->setFocusPolicy(WheelFocus);
  // Let's better be paranoid and disable plugins (it defaults to enabled):
  mViewer->setPluginsEnabled(false);
  mViewer->setJScriptEnabled(false); // just make this explicit
  mViewer->setJavaEnabled(false);    // just make this explicit
  mViewer->setMetaRefreshEnabled(false);
  mViewer->setURLCursor(KCursor::handCursor());
  // Espen 2000-05-14: Getting rid of thick ugly frames
  mViewer->view()->setLineWidth(0);
  // register our own event filter for shift-click
  mViewer->view()->viewport()->installEventFilter( this );

  if ( !htmlWriter() )
#ifdef KMAIL_READER_HTML_DEBUG
    mHtmlWriter = new TeeHtmlWriter( new FileHtmlWriter( TQString::null ),
				     new KHtmlPartHtmlWriter( mViewer, 0 ) );
#else
    mHtmlWriter = new KHtmlPartHtmlWriter( mViewer, 0 );
#endif

  connect(mViewer->browserExtension(),
          TQT_SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),this,
          TQT_SLOT(slotUrlOpen(const KURL &)));
  connect(mViewer->browserExtension(),
          TQT_SIGNAL(createNewWindow(const KURL &, const KParts::URLArgs &)),this,
          TQT_SLOT(slotUrlOpen(const KURL &)));
  connect(mViewer,TQT_SIGNAL(popupMenu(const TQString &, const TQPoint &)),
          TQT_SLOT(slotUrlPopup(const TQString &, const TQPoint &)));
  connect( kmkernel->imProxy(), TQT_SIGNAL( sigContactPresenceChanged( const TQString & ) ),
          this, TQT_SLOT( contactStatusChanged( const TQString & ) ) );
  connect( kmkernel->imProxy(), TQT_SIGNAL( sigPresenceInfoExpired() ),
          this, TQT_SLOT( updateReaderWin() ) );
}

void KMReaderWin::contactStatusChanged( const TQString &uid)
{
//  kdDebug( 5006 ) << k_funcinfo << " got a presence change for " << uid << endl;
  // get the list of nodes for this contact from the htmlView
  DOM::NodeList presenceNodes = mViewer->htmlDocument()
    .getElementsByName( DOM::DOMString( TQString::fromLatin1("presence-") + uid ) );
  for ( unsigned int i = 0; i < presenceNodes.length(); ++i ) {
    DOM::Node n =  presenceNodes.item( i );
    kdDebug( 5006 ) << "name is " << n.nodeName().string() << endl;
    kdDebug( 5006 ) << "value of content was " << n.firstChild().nodeValue().string() << endl;
    TQString newPresence = kmkernel->imProxy()->presenceString( uid );
    if ( newPresence.isNull() ) // KHTML crashes if you setNodeValue( TQString::null )
      newPresence = TQString::fromLatin1( "ENOIMRUNNING" );
    n.firstChild().setNodeValue( newPresence );
//    kdDebug( 5006 ) << "value of content is now " << n.firstChild().nodeValue().string() << endl;
  }
//  kdDebug( 5006 ) << "and we updated the above presence nodes" << uid << endl;
}

void KMReaderWin::setAttachmentStrategy( const AttachmentStrategy * strategy ) {
  mAttachmentStrategy = strategy ? strategy : AttachmentStrategy::smart();
  update( true );
}

void KMReaderWin::setHeaderStyleAndStrategy( const HeaderStyle * style,
					     const HeaderStrategy * strategy ) {
  mHeaderStyle = style ? style : HeaderStyle::fancy();
  mHeaderStrategy = strategy ? strategy : HeaderStrategy::rich();
  if ( mHeaderOnlyAttachmentsAction ) {
    const bool styleHasAttachmentQuickList = mHeaderStyle == HeaderStyle::fancy() ||
                                             mHeaderStyle == HeaderStyle::enterprise();
    mHeaderOnlyAttachmentsAction->setEnabled( styleHasAttachmentQuickList );
    if ( !styleHasAttachmentQuickList && mAttachmentStrategy == AttachmentStrategy::headerOnly() ) {
      // Style changed to something without an attachment quick list, need to change attachment
      // strategy
      setAttachmentStrategy( AttachmentStrategy::smart() );
    }
  }
  update( true );
}

//-----------------------------------------------------------------------------
void KMReaderWin::setOverrideEncoding( const TQString & encoding )
{
  if ( encoding == mOverrideEncoding )
    return;

  mOverrideEncoding = encoding;
  if ( mSelectEncodingAction ) {
    if ( encoding.isEmpty() ) {
      mSelectEncodingAction->setCurrentItem( 0 );
    }
    else {
      TQStringList encodings = mSelectEncodingAction->items();
      uint i = 0;
      for ( TQStringList::const_iterator it = encodings.begin(), end = encodings.end(); it != end; ++it, ++i ) {
        if ( KGlobal::charsets()->encodingForName( *it ) == encoding ) {
          mSelectEncodingAction->setCurrentItem( i );
          break;
        }
      }
      if ( i == encodings.size() ) {
        // the value of encoding is unknown => use Auto
        kdWarning(5006) << "Unknown override character encoding \"" << encoding
                        << "\". Using Auto instead." << endl;
        mSelectEncodingAction->setCurrentItem( 0 );
        mOverrideEncoding = TQString::null;
      }
    }
  }
  update( true );
}


void KMReaderWin::setPrintFont( const TQFont& font )
{

  mCSSHelper->setPrintFont( font );
}

//-----------------------------------------------------------------------------
const TQTextCodec * KMReaderWin::overrideCodec() const
{
  if ( mOverrideEncoding.isEmpty() || mOverrideEncoding == "Auto" ) // Auto
    return 0;
  else
    return KMMsgBase::codecForName( mOverrideEncoding.latin1() );
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotSetEncoding()
{
  if ( mSelectEncodingAction->currentItem() == 0 ) // Auto
    mOverrideEncoding = TQString();
  else
    mOverrideEncoding = KGlobal::charsets()->encodingForName( mSelectEncodingAction->currentText() );
  update( true );
}

//-----------------------------------------------------------------------------
void KMReaderWin::readGlobalOverrideCodec()
{
  // if the global character encoding wasn't changed then there's nothing to do
  if ( GlobalSettings::self()->overrideCharacterEncoding() == mOldGlobalOverrideEncoding )
    return;

  setOverrideEncoding( GlobalSettings::self()->overrideCharacterEncoding() );
  mOldGlobalOverrideEncoding = GlobalSettings::self()->overrideCharacterEncoding();
}

//-----------------------------------------------------------------------------
void KMReaderWin::setOriginalMsg( unsigned long serNumOfOriginalMessage, int nodeIdOffset )
{
  mSerNumOfOriginalMessage = serNumOfOriginalMessage;
  mNodeIdOffset = nodeIdOffset;
}

//-----------------------------------------------------------------------------
void KMReaderWin::setMsg( KMMessage* aMsg, bool force, bool updateOnly )
{
  if ( aMsg ) {
    kdDebug(5006) << "(" << aMsg->getMsgSerNum() << ", last " << mLastSerNum << ") " << aMsg->subject() << " "
                  << aMsg->fromStrip() << ", readyToShow " << (aMsg->readyToShow()) << endl;
  }

  // Reset message-transient state
  if ( aMsg && aMsg->getMsgSerNum() != mLastSerNum && !updateOnly ){
    mLevelQuote = GlobalSettings::self()->collapseQuoteLevelSpin()-1;
    mShowRawToltecMail = !GlobalSettings::self()->showToltecReplacementText();
    clearBodyPartMementos();
  }
  if ( mPrinting )
    mLevelQuote = -1;

  bool complete = true;
  if ( aMsg &&
       !aMsg->readyToShow() &&
       (aMsg->getMsgSerNum() != mLastSerNum) &&
       !aMsg->isComplete() )
    complete = false;

  // If not forced and there is aMsg and aMsg is same as mMsg then return
  if (!force && aMsg && mLastSerNum != 0 && aMsg->getMsgSerNum() == mLastSerNum)
    return;

  // (de)register as observer
  if (aMsg && message())
    message()->detach( this );
  if (aMsg)
    aMsg->attach( this );
  mAtmUpdate = false;

  // connect to the updates if we have hancy headers

  mDelayedMarkTimer.stop();

  mMessage = 0;
  if ( !aMsg ) {
    mWaitingForSerNum = 0; // otherwise it has been set
    mLastSerNum = 0;
  } else {
    mLastSerNum = aMsg->getMsgSerNum();
    // Check if the serial number can be used to find the assoc KMMessage
    // If so, keep only the serial number (and not mMessage), to avoid a dangling mMessage
    // when going to another message in the mainwindow.
    // Otherwise, keep only mMessage, this is fine for standalone KMReaderMainWins since
    // we're working on a copy of the KMMessage, which we own.
    if (message() != aMsg) {
      mMessage = aMsg;
      mLastSerNum = 0;
    }
  }

  if (aMsg) {
    aMsg->setOverrideCodec( overrideCodec() );
    aMsg->setDecodeHTML( htmlMail() );
    // FIXME: workaround to disable DND for IMAP load-on-demand
    if ( !aMsg->isComplete() )
      mViewer->setDNDEnabled( false );
    else
      mViewer->setDNDEnabled( true );
  }

  // only display the msg if it is complete
  // otherwise we'll get flickering with progressively loaded messages
  if ( complete )
  {
    // Avoid flicker, somewhat of a cludge
    if (force) {
      // stop the timer to avoid calling updateReaderWin twice
      mUpdateReaderWinTimer.stop();
      updateReaderWin();
    }
    else if (mUpdateReaderWinTimer.isActive())
      mUpdateReaderWinTimer.changeInterval( delay );
    else
      mUpdateReaderWinTimer.start( 0, true );
  }

  if ( aMsg && (aMsg->isUnread() || aMsg->isNew()) && GlobalSettings::self()->delayedMarkAsRead() ) {
    if ( GlobalSettings::self()->delayedMarkTime() != 0 )
      mDelayedMarkTimer.start( GlobalSettings::self()->delayedMarkTime() * 1000, true );
    else
      slotTouchMessage();
  }
}

//-----------------------------------------------------------------------------
void KMReaderWin::clearCache()
{
  mUpdateReaderWinTimer.stop();
  clear();
  mDelayedMarkTimer.stop();
  mLastSerNum = 0;
  mWaitingForSerNum = 0;
  mMessage = 0;
}

// enter items for the "Important changes" list here:
static const char * const kmailChanges[] = {
  ""
};
static const int numKMailChanges =
  sizeof kmailChanges / sizeof *kmailChanges;

// enter items for the "new features" list here, so the main body of
// the welcome page can be left untouched (probably much easier for
// the translators). Note that the <li>...</li> tags are added
// automatically below:
static const char * const kmailNewFeatures[] = {
  I18N_NOOP("Full namespace support for IMAP"),
  I18N_NOOP("Offline mode"),
  I18N_NOOP("Sieve script management and editing"),
  I18N_NOOP("Account specific filtering"),
  I18N_NOOP("Filtering of incoming mail for online IMAP accounts"),
  I18N_NOOP("Online IMAP folders can be used when filtering into folders"),
  I18N_NOOP("Automatically delete older mails on POP servers")
};
static const int numKMailNewFeatures =
  sizeof kmailNewFeatures / sizeof *kmailNewFeatures;


//-----------------------------------------------------------------------------
//static
TQString KMReaderWin::newFeaturesMD5()
{
  TQCString str;
  for ( int i = 0 ; i < numKMailChanges ; ++i )
    str += kmailChanges[i];
  for ( int i = 0 ; i < numKMailNewFeatures ; ++i )
    str += kmailNewFeatures[i];
  KMD5 md5( str );
  return md5.base64Digest();
}

//-----------------------------------------------------------------------------
void KMReaderWin::displaySplashPage( const TQString &info )
{
  mMsgDisplay = false;
  adjustLayout();

  TQString location = locate("data", "kmail/about/main.html");
  TQString content = KPIM::kFileToString(location);
  content = content.arg( locate( "data", "libkdepim/about/kde_infopage.css" ) );
  if ( kapp->reverseLayout() )
    content = content.arg( "@import \"%1\";" ).arg( locate( "data", "libkdepim/about/kde_infopage_rtl.css" ) );
  else
    content = content.arg( "" );

  mViewer->begin(KURL( location ));

  TQString fontSize = TQString::number( pointsToPixel( mCSSHelper->bodyFont().pointSize() ) );
  TQString appTitle = i18n("KMail");
  TQString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
  TQString quickDescription = i18n("The email client for the K Desktop Environment.");
  mViewer->write(content.arg(fontSize).arg(appTitle).arg(catchPhrase).arg(quickDescription).arg(info));
  mViewer->end();
}

void KMReaderWin::displayBusyPage()
{
  TQString info =
    i18n( "<h2 style='margin-top: 0px;'>Retrieving Folder Contents</h2><p>Please wait . . .</p>&nbsp;" );

  displaySplashPage( info );
}

void KMReaderWin::displayOfflinePage()
{
  TQString info =
    i18n( "<h2 style='margin-top: 0px;'>Offline</h2><p>KMail is currently in offline mode. "
        "Click <a href=\"kmail:goOnline\">here</a> to go online . . .</p>&nbsp;" );

  displaySplashPage( info );
}


//-----------------------------------------------------------------------------
void KMReaderWin::displayAboutPage()
{
  TQString info =
    i18n("%1: KMail version; %2: help:// URL; %3: homepage URL; "
	 "%4: prior KMail version; %5: prior KDE version; "
	 "%6: generated list of new features; "
	 "%7: First-time user text (only shown on first start); "
         "%8: generated list of important changes; "
	 "--- end of comment ---",
	 "<h2 style='margin-top: 0px;'>Welcome to KMail %1</h2><p>KMail is the email client for the K "
	 "Desktop Environment. It is designed to be fully compatible with "
	 "Internet mailing standards including MIME, SMTP, POP3 and IMAP."
	 "</p>\n"
	 "<ul><li>KMail has many powerful features which are described in the "
	 "<a href=\"%2\">documentation</a></li>\n"
	 "<li>The <a href=\"%3\">KMail homepage</A> offers information about "
	 "new versions of KMail</li></ul>\n"
         "%8\n" // important changes
	 "<p>Some of the new features in this release of KMail include "
	 "(compared to KMail %4, which is part of KDE %5):</p>\n"
	 "<ul>\n%6</ul>\n"
	 "%7\n"
	 "<p>We hope that you will enjoy KMail.</p>\n"
	 "<p>Thank you,</p>\n"
	     "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The KMail Team</p>")
    .arg(KMAIL_VERSION) // KMail version
    .arg("help:/kmail/index.html") // KMail help:// URL
    .arg("http://kontact.kde.org/kmail/") // KMail homepage URL
    .arg("1.8").arg("3.4"); // prior KMail and KDE version

  TQString featureItems;
  for ( int i = 0 ; i < numKMailNewFeatures ; i++ )
    featureItems += i18n("<li>%1</li>\n").arg( i18n( kmailNewFeatures[i] ) );

  info = info.arg( featureItems );

  if( kmkernel->firstStart() ) {
    info = info.arg( i18n("<p>Please take a moment to fill in the KMail "
			  "configuration panel at Settings-&gt;Configure "
			  "KMail.\n"
			  "You need to create at least a default identity and "
			  "an incoming as well as outgoing mail account."
			  "</p>\n") );
  } else {
    info = info.arg( TQString::null );
  }

  if ( ( numKMailChanges > 1 ) || ( numKMailChanges == 1 && strlen(kmailChanges[0]) > 0 ) ) {
    TQString changesText =
      i18n("<p><span style='font-size:125%; font-weight:bold;'>"
           "Important changes</span> (compared to KMail %1):</p>\n")
      .arg("1.8");
    changesText += "<ul>\n";
    for ( int i = 0 ; i < numKMailChanges ; i++ )
      changesText += i18n("<li>%1</li>\n").arg( i18n( kmailChanges[i] ) );
    changesText += "</ul>\n";
    info = info.arg( changesText );
  }
  else
    info = info.arg(""); // remove the %8

  displaySplashPage( info );
}

void KMReaderWin::enableMsgDisplay() {
  mMsgDisplay = true;
  adjustLayout();
}


//-----------------------------------------------------------------------------

void KMReaderWin::updateReaderWin()
{
  if (!mMsgDisplay) return;

  mViewer->setOnlyLocalReferences(!htmlLoadExternal());

  htmlWriter()->reset();

  KMFolder* folder = 0;
  if (message(&folder))
  {
    if ( mShowColorbar )
      mColorBar->show();
    else
      mColorBar->hide();
    displayMessage();
  }
  else
  {
    mColorBar->hide();
    mMimePartTree->hide();
    mMimePartTree->clear();
    htmlWriter()->begin( mCSSHelper->cssDefinitions( isFixedFont() ) );
    htmlWriter()->write( mCSSHelper->htmlHead( isFixedFont() ) + "</body></html>" );
    htmlWriter()->end();
  }

  if (mSavedRelativePosition)
  {
    TQScrollView * scrollview = static_cast<TQScrollView *>(mViewer->widget());
    scrollview->setContentsPos( 0,
      qRound( scrollview->contentsHeight() * mSavedRelativePosition ) );
    mSavedRelativePosition = 0;
  }
}

//-----------------------------------------------------------------------------
int KMReaderWin::pointsToPixel(int pointSize) const
{
  const TQPaintDeviceMetrics pdm(mViewer->view());

  return (pointSize * pdm.logicalDpiY() + 36) / 72;
}

//-----------------------------------------------------------------------------
void KMReaderWin::showHideMimeTree( bool isPlainTextTopLevel ) {
  if ( mMimeTreeMode == 2 ||
       ( mMimeTreeMode == 1 && !isPlainTextTopLevel ) )
    mMimePartTree->show();
  else {
    // don't rely on TQSplitter maintaining sizes for hidden widgets:
    KConfigGroup reader( KMKernel::config(), "Reader" );
    saveSplitterSizes( reader );
    mMimePartTree->hide();
  }
}

void KMReaderWin::displayMessage() {
  KMMessage * msg = message();

  mMimePartTree->clear();
  showHideMimeTree( !msg || // treat no message as "text/plain"
		    ( msg->type() == DwMime::kTypeText
		      && msg->subtype() == DwMime::kSubtypePlain ) );

  if ( !msg )
    return;

  msg->setOverrideCodec( overrideCodec() );

  htmlWriter()->begin( mCSSHelper->cssDefinitions( isFixedFont() ) );
  htmlWriter()->queue( mCSSHelper->htmlHead( isFixedFont() ) );

  if (!parent())
    setCaption(msg->subject());

  removeTempFiles();

  mColorBar->setNeutralMode();

  parseMsg(msg);

  if( mColorBar->isNeutral() )
    mColorBar->setNormalMode();

  htmlWriter()->queue("</body></html>");
  htmlWriter()->flush();

  TQTimer::singleShot( 1, this, TQT_SLOT(injectAttachments()) );
}

static bool message_was_saved_decrypted_before( const KMMessage * msg ) {
  if ( !msg )
    return false;
  //kdDebug(5006) << "msgId = " << msg->msgId() << endl;
  return msg->msgId().stripWhiteSpace().startsWith( "<DecryptedMsg." );
}

//-----------------------------------------------------------------------------
void KMReaderWin::parseMsg(KMMessage* aMsg)
{
  KMMessagePart msgPart;
  TQCString subtype, contDisp;
  TQByteArray str;

  assert(aMsg!=0);

  aMsg->setIsBeingParsed( true );

  if ( mRootNode && !mRootNode->processed() )
  {
    kdWarning() << "The root node is not yet processed! Danger!\n";
    return;
  } else
    delete mRootNode;
  mRootNode = partNode::fromMessage( aMsg, this );
  const TQCString mainCntTypeStr = mRootNode->typeString() + '/' + mRootNode->subTypeString();

  TQString cntDesc = aMsg->subject();
  if( cntDesc.isEmpty() )
    cntDesc = i18n("( body part )");
  KIO::filesize_t cntSize = aMsg->msgSize();
  TQString cntEnc;
  if( aMsg->contentTransferEncodingStr().isEmpty() )
    cntEnc = "7bit";
  else
    cntEnc = aMsg->contentTransferEncodingStr();

  // fill the MIME part tree viewer
  mRootNode->fillMimePartTree( 0,
			       mMimePartTree,
			       cntDesc,
			       mainCntTypeStr,
			       cntEnc,
			       cntSize );

  partNode* vCardNode = mRootNode->findType( DwMime::kTypeText, DwMime::kSubtypeXVCard );
  bool hasVCard = false;
  if( vCardNode ) {
    // ### FIXME: We should only do this if the vCard belongs to the sender,
    // ### i.e. if the sender's email address is contained in the vCard.
    KABC::VCardConverter t;
#if defined(KABC_VCARD_ENCODING_FIX)
    const TQByteArray vcard = vCardNode->msgPart().bodyDecodedBinary();
    if ( !t.parseVCardsRaw( vcard.data() ).empty() ) {
#else
    const TQString vcard = vCardNode->msgPart().bodyToUnicode( overrideCodec() );
    if ( !t.parseVCards( vcard ).empty() ) {
#endif
      hasVCard = true;
      writeMessagePartToTempFile( &vCardNode->msgPart(), vCardNode->nodeId() );
    }
  }

  if ( !mRootNode || !mRootNode->isToltecMessage() || mShowRawToltecMail ) {
    htmlWriter()->queue( writeMsgHeader(aMsg, hasVCard ? vCardNode : 0, true ) );
  }

  // show message content
  ObjectTreeParser otp( this );
  otp.setAllowAsync( true );
  otp.setShowRawToltecMail( mShowRawToltecMail );
  otp.parseObjectTree( mRootNode );

  // store encrypted/signed status information in the KMMessage
  //  - this can only be done *after* calling parseObjectTree()
  KMMsgEncryptionState encryptionState = mRootNode->overallEncryptionState();
  KMMsgSignatureState  signatureState  = mRootNode->overallSignatureState();
  // Don't crash when switching message while GPG passphrase entry dialog is shown #53185
  if (aMsg != message()) {
    displayMessage();
    return;
  }
  aMsg->setEncryptionState( encryptionState );
  // Don't reset the signature state to "not signed" (e.g. if one canceled the
  // decryption of a signed messages which has already been decrypted before).
  if ( signatureState != KMMsgNotSigned ||
       aMsg->signatureState() == KMMsgSignatureStateUnknown ) {
    aMsg->setSignatureState( signatureState );
  }

  bool emitReplaceMsgByUnencryptedVersion = false;
  const KConfigGroup reader( KMKernel::config(), "Reader" );
  if ( reader.readBoolEntry( "store-displayed-messages-unencrypted", false ) ) {

  // Hack to make sure the S/MIME CryptPlugs follows the strict requirement
  // of german government:
  // --> All received encrypted messages *must* be stored in unencrypted form
  //     after they have been decrypted once the user has read them.
  //     ( "Aufhebung der Verschluesselung nach dem Lesen" )
  //
  // note: Since there is no configuration option for this, we do that for
  //       all kinds of encryption now - *not* just for S/MIME.
  //       This could be changed in the objectTreeToDecryptedMsg() function
  //       by deciding when (or when not, resp.) to set the 'dataNode' to
  //       something different than 'curNode'.


kdDebug(5006) << "\n\n\nKMReaderWin::parseMsg()  -  special post-encryption handling:\n1." << endl;
kdDebug(5006) << "(aMsg == msg) = "                               << (aMsg == message()) << endl;
kdDebug(5006) << "aMsg->parent() && aMsg->parent() != kmkernel->outboxFolder() = " << (aMsg->parent() && aMsg->parent() != kmkernel->outboxFolder()) << endl;
kdDebug(5006) << "message_was_saved_decrypted_before( aMsg ) = " << message_was_saved_decrypted_before( aMsg ) << endl;
kdDebug(5006) << "this->decryptMessage() = " << decryptMessage() << endl;
kdDebug(5006) << "otp.hasPendingAsyncJobs() = " << otp.hasPendingAsyncJobs() << endl;
kdDebug(5006) << "   (KMMsgFullyEncrypted == encryptionState) = "     << (KMMsgFullyEncrypted == encryptionState) << endl;
kdDebug(5006) << "|| (KMMsgPartiallyEncrypted == encryptionState) = " << (KMMsgPartiallyEncrypted == encryptionState) << endl;
         // only proceed if we were called the normal way - not by
         // double click on the message (==not running in a separate window)
  if(    (aMsg == message())
         // don't remove encryption in the outbox folder :)
      && ( aMsg->parent() && aMsg->parent() != kmkernel->outboxFolder() )
         // only proceed if this message was not saved encryptedly before
      && !message_was_saved_decrypted_before( aMsg )
         // only proceed if the message has actually been decrypted
      && decryptMessage()
         // only proceed if no pending async jobs are running:
      && !otp.hasPendingAsyncJobs()
         // only proceed if this message is (at least partially) encrypted
      && (    (KMMsgFullyEncrypted == encryptionState)
           || (KMMsgPartiallyEncrypted == encryptionState) ) ) {

kdDebug(5006) << "KMReaderWin  -  calling objectTreeToDecryptedMsg()" << endl;

    NewByteArray decryptedData;
    // note: The following call may change the message's headers.
    objectTreeToDecryptedMsg( mRootNode, decryptedData, *aMsg );
    // add a \0 to the data
    decryptedData.appendNULL();
    TQCString resultString( decryptedData.data() );
kdDebug(5006) << "KMReaderWin  -  resulting data:" << resultString << endl;

    if( !resultString.isEmpty() ) {
kdDebug(5006) << "KMReaderWin  -  composing unencrypted message" << endl;
      // try this:
      aMsg->setBody( resultString );
      KMMessage* unencryptedMessage = new KMMessage( *aMsg );
      unencryptedMessage->setParent( 0 );
      // because this did not work:
      /*
      DwMessage dwMsg( aMsg->asDwString() );
      dwMsg.Body() = DwBody( DwString( resultString.data() ) );
      dwMsg.Body().Parse();
      KMMessage* unencryptedMessage = new KMMessage( &dwMsg );
      */
      //kdDebug(5006) << "KMReaderWin  -  resulting message:" << unencryptedMessage->asString() << endl;
      kdDebug(5006) << "KMReaderWin  -  attach unencrypted message to aMsg" << endl;
      aMsg->setUnencryptedMsg( unencryptedMessage );
      emitReplaceMsgByUnencryptedVersion = true;
    }
  }
  }

  // save current main Content-Type before deleting mRootNode
  const int rootNodeCntType = mRootNode ? mRootNode->type() : DwMime::kTypeText;
  const int rootNodeCntSubtype = mRootNode ? mRootNode->subType() : DwMime::kSubtypePlain;

  // store message id to avoid endless recursions
  setIdOfLastViewedMessage( aMsg->msgId() );

  if( emitReplaceMsgByUnencryptedVersion ) {
    kdDebug(5006) << "KMReaderWin  -  invoce saving in decrypted form:" << endl;
    emit replaceMsgByUnencryptedVersion();
  } else {
    kdDebug(5006) << "KMReaderWin  -  finished parsing and displaying of message." << endl;
    showHideMimeTree( rootNodeCntType == DwMime::kTypeText &&
		      rootNodeCntSubtype == DwMime::kSubtypePlain );
  }

  aMsg->setIsBeingParsed( false );
}


//-----------------------------------------------------------------------------
TQString KMReaderWin::writeMsgHeader( KMMessage* aMsg, partNode *vCardNode, bool topLevel )
{
  kdFatal( !headerStyle(), 5006 )
    << "trying to writeMsgHeader() without a header style set!" << endl;
  kdFatal( !headerStrategy(), 5006 )
    << "trying to writeMsgHeader() without a header strategy set!" << endl;
  TQString href;
  if ( vCardNode )
    href = vCardNode->asHREF( "body" );

  return headerStyle()->format( aMsg, headerStrategy(), href, mPrinting, topLevel );
}



//-----------------------------------------------------------------------------
TQString KMReaderWin::writeMessagePartToTempFile( KMMessagePart* aMsgPart,
                                                 int aPartNum )
{
  TQString fileName = aMsgPart->fileName();
  if( fileName.isEmpty() )
    fileName = aMsgPart->name();

  //--- Sven's save attachments to /tmp start ---
  TQString fname = createTempDir( TQString::number( aPartNum ) );
  if ( fname.isEmpty() )
    return TQString();

  // strip off a leading path
  int slashPos = fileName.findRev( '/' );
  if( -1 != slashPos )
    fileName = fileName.mid( slashPos + 1 );
  if( fileName.isEmpty() )
    fileName = "unnamed";
  fname += "/" + fileName;

  TQByteArray data = aMsgPart->bodyDecodedBinary();
  size_t size = data.size();
  if ( aMsgPart->type() == DwMime::kTypeText && size) {
    // convert CRLF to LF before writing text attachments to disk
    size = KMail::Util::crlf2lf( data.data(), size );
  }
  if( !KPIM::kBytesToFile( data.data(), size, fname, false, false, false ) )
    return TQString::null;

  mTempFiles.append( fname );
  // make file read-only so that nobody gets the impression that he might
  // edit attached files (cf. bug #52813)
  ::chmod( TQFile::encodeName( fname ), S_IRUSR );

  return fname;
}

TQString KMReaderWin::createTempDir( const TQString &param )
{
  KTempFile *tempFile = new KTempFile( TQString::null, "." + param );
  tempFile->setAutoDelete( true );
  TQString fname = tempFile->name();
  delete tempFile;

  if( ::access( TQFile::encodeName( fname ), W_OK ) != 0 )
    // Not there or not writable
    if( ::mkdir( TQFile::encodeName( fname ), 0 ) != 0
        || ::chmod( TQFile::encodeName( fname ), S_IRWXU ) != 0 )
      return TQString::null; //failed create

  assert( !fname.isNull() );

  mTempDirs.append( fname );
  return fname;
}

//-----------------------------------------------------------------------------
void KMReaderWin::showVCard( KMMessagePart *msgPart )
{
#if defined(KABC_VCARD_ENCODING_FIX)
  const TQByteArray vCard = msgPart->bodyDecodedBinary();
#else
  const TQString vCard = msgPart->bodyToUnicode( overrideCodec() );
#endif
  VCardViewer *vcv = new VCardViewer( this, vCard, "vCardDialog" );
  vcv->show();
}

//-----------------------------------------------------------------------------
void KMReaderWin::printMsg()
{
  if (!message()) return;
  mViewer->view()->print();
}


//-----------------------------------------------------------------------------
int KMReaderWin::msgPartFromUrl(const KURL &aUrl)
{
  if (aUrl.isEmpty()) return -1;
  if (!aUrl.isLocalFile()) return -1;

  TQString path = aUrl.path();
  uint right = path.findRev('/');
  uint left = path.findRev('.', right);

  bool ok;
  int res = path.mid(left + 1, right - left - 1).toInt(&ok);
  return (ok) ? res : -1;
}


//-----------------------------------------------------------------------------
void KMReaderWin::resizeEvent(TQResizeEvent *)
{
  if( !mResizeTimer.isActive() )
  {
    //
    // Combine all resize operations that are requested as long a
    // the timer runs.
    //
    mResizeTimer.start( 100, true );
  }
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotDelayedResize()
{
  mSplitter->setGeometry(0, 0, width(), height());
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotTouchMessage()
{
  if ( !message() )
    return;

  if ( !message()->isNew() && !message()->isUnread() )
    return;

  SerNumList serNums;
  serNums.append( message()->getMsgSerNum() );
  KMCommand *command = new KMSetStatusCommand( KMMsgStatusRead, serNums );
  command->start();

  // should we send an MDN?
  if ( mNoMDNsWhenEncrypted &&
       message()->encryptionState() != KMMsgNotEncrypted &&
       message()->encryptionState() != KMMsgEncryptionStateUnknown )
    return;

  KMFolder *folder = message()->parent();
  if (folder &&
     (folder->isOutbox() || folder->isSent() || folder->isTrash() ||
      folder->isDrafts() || folder->isTemplates() ) )
    return;

  if ( KMMessage * receipt = message()->createMDN( MDN::ManualAction,
						   MDN::Displayed,
						   true /* allow GUI */ ) )
    if ( !kmkernel->msgSender()->send( receipt ) ) // send or queue
      KMessageBox::error( this, i18n("Could not send MDN.") );
}


//-----------------------------------------------------------------------------
void KMReaderWin::closeEvent(TQCloseEvent *e)
{
  TQWidget::closeEvent(e);
  writeConfig();
}


bool foundSMIMEData( const TQString aUrl,
                     TQString& displayName,
                     TQString& libName,
                     TQString& keyId )
{
  static TQString showCertMan("showCertificate#");
  displayName = "";
  libName = "";
  keyId = "";
  int i1 = aUrl.find( showCertMan );
  if( -1 < i1 ) {
    i1 += showCertMan.length();
    int i2 = aUrl.find(" ### ", i1);
    if( i1 < i2 )
    {
      displayName = aUrl.mid( i1, i2-i1 );
      i1 = i2+5;
      i2 = aUrl.find(" ### ", i1);
      if( i1 < i2 )
      {
        libName = aUrl.mid( i1, i2-i1 );
        i2 += 5;

        keyId = aUrl.mid( i2 );
        /*
        int len = aUrl.length();
        if( len > i2+1 ) {
          keyId = aUrl.mid( i2, 2 );
          i2 += 2;
          while( len > i2+1 ) {
            keyId += ':';
            keyId += aUrl.mid( i2, 2 );
            i2 += 2;
          }
        }
        */
      }
    }
  }
  return !keyId.isEmpty();
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotUrlOn(const TQString &aUrl)
{
  const KURL url(aUrl);

  if ( url.protocol() == "kmail" || url.protocol() == "x-kmail" || url.protocol() == "attachment"
       || (url.protocol().isEmpty() && url.path().isEmpty()) ) {
    mViewer->setDNDEnabled( false );
  } else {
    mViewer->setDNDEnabled( true );
  }

  if ( aUrl.stripWhiteSpace().isEmpty() ) {
    KPIM::BroadcastStatus::instance()->reset();
    mHoveredUrl = KURL();
    mLastClickImagePath = TQString();
    return;
  }

  mHoveredUrl = url;

  const TQString msg = URLHandlerManager::instance()->statusBarMessage( url, this );

  kdWarning( msg.isEmpty(), 5006 ) << "KMReaderWin::slotUrlOn(): Unhandled URL hover!" << endl;
  KPIM::BroadcastStatus::instance()->setTransientStatusMsg( msg );
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotUrlOpen(const KURL &aUrl, const KParts::URLArgs &)
{
  mClickedUrl = aUrl;

  if ( URLHandlerManager::instance()->handleClick( aUrl, this ) )
    return;

  kdWarning( 5006 ) << "KMReaderWin::slotOpenUrl(): Unhandled URL click!" << endl;
  emit urlClicked( aUrl, Qt::LeftButton );
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotUrlPopup(const TQString &aUrl, const TQPoint& aPos)
{
  const KURL url( aUrl );
  mClickedUrl = url;

  if ( url.protocol() == "mailto" ) {
    mCopyURLAction->setText( i18n( "Copy Email Address" ) );
  } else {
    mCopyURLAction->setText( i18n( "Copy Link Address" ) );
  }

  if ( URLHandlerManager::instance()->handleContextMenuRequest( url, aPos, this ) )
    return;

  if ( message() ) {
    kdWarning( 5006 ) << "KMReaderWin::slotUrlPopup(): Unhandled URL right-click!" << endl;
    emitPopupMenu( url, aPos );
  }
}

// Checks if the given node has a parent node that is a DIV which has an ID attribute
// with the value specified here
static bool hasParentDivWithId( const DOM::Node &start, const TQString &id )
{
  if ( start.isNull() )
    return false;

  if ( start.nodeName().string() == "div" ) {
    for ( unsigned int i = 0; i < start.attributes().length(); i++ ) {
      if ( start.attributes().item( i ).nodeName().string() == "id" &&
           start.attributes().item( i ).nodeValue().string() == id )
        return true;
    }
  }

  if ( !start.parentNode().isNull() )
    return hasParentDivWithId( start.parentNode(), id );
  else return false;
}

//-----------------------------------------------------------------------------
void KMReaderWin::showAttachmentPopup( int id, const TQString & name, const TQPoint & p )
{
  mAtmCurrent = id;
  mAtmCurrentName = name;
  KPopupMenu *menu = new KPopupMenu();
  menu->insertItem(SmallIcon("fileopen"),i18n("to open", "Open"), 1);
  menu->insertItem(i18n("Open With..."), 2);
  menu->insertItem(i18n("to view something", "View"), 3);
  menu->insertItem(SmallIcon("filesaveas"),i18n("Save As..."), 4);
  menu->insertItem(SmallIcon("editcopy"), i18n("Copy"), 9 );
  const bool canChange = message()->parent() ? !message()->parent()->isReadOnly() : false;
  if ( GlobalSettings::self()->allowAttachmentEditing() && canChange )
    menu->insertItem(SmallIcon("edit"), i18n("Edit Attachment"), 8 );
  if ( GlobalSettings::self()->allowAttachmentDeletion() && canChange )
    menu->insertItem(SmallIcon("editdelete"), i18n("Delete Attachment"), 7 );
  if ( name.endsWith( ".xia", false ) &&
       Kleo::CryptoBackendFactory::instance()->protocol( "Chiasmus" ) )
    menu->insertItem( i18n( "Decrypt With Chiasmus..." ), 6 );
  menu->insertItem(i18n("Properties"), 5);

  const bool attachmentInHeader = hasParentDivWithId( mViewer->nodeUnderMouse(), "attachmentInjectionPoint" );
  const bool hasScrollbar = mViewer->view()->verticalScrollBar()->isVisible();
  if ( attachmentInHeader && hasScrollbar ) {
    menu->insertItem( i18n("Scroll To"), 10 );
  }

  connect(menu, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotHandleAttachment(int)));
  menu->exec( p ,0 );
  delete menu;
}

//-----------------------------------------------------------------------------
void KMReaderWin::setStyleDependantFrameWidth()
{
  if ( !mBox )
    return;
  // set the width of the frame to a reasonable value for the current GUI style
  int frameWidth;
  if( style().isA("KeramikStyle") )
    frameWidth = style().pixelMetric( TQStyle::PM_DefaultFrameWidth ) - 1;
  else
    frameWidth = style().pixelMetric( TQStyle::PM_DefaultFrameWidth );
  if ( frameWidth < 0 )
    frameWidth = 0;
  if ( frameWidth != mBox->lineWidth() )
    mBox->setLineWidth( frameWidth );
}

//-----------------------------------------------------------------------------
void KMReaderWin::styleChange( TQStyle& oldStyle )
{
  setStyleDependantFrameWidth();
  TQWidget::styleChange( oldStyle );
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotHandleAttachment( int choice )
{
  mAtmUpdate = true;
  partNode* node = mRootNode ? mRootNode->findId( mAtmCurrent ) : 0;
  if ( mAtmCurrentName.isEmpty() && node )
    mAtmCurrentName = tempFileUrlFromPartNode( node ).path();
  if ( choice < 7 ) {
  KMHandleAttachmentCommand* command = new KMHandleAttachmentCommand(
      node, message(), mAtmCurrent, mAtmCurrentName,
      KMHandleAttachmentCommand::AttachmentAction( choice ), 0, this );
  connect( command, TQT_SIGNAL( showAttachment( int, const TQString& ) ),
      this, TQT_SLOT( slotAtmView( int, const TQString& ) ) );
  command->start();
  } else if ( choice == 7 ) {
    slotDeleteAttachment( node );
  } else if ( choice == 8 ) {
    slotEditAttachment( node );
  } else if ( choice == 9 ) {
    if ( !node ) return;
    KURL::List urls;
    KURL url = tempFileUrlFromPartNode( node );
    if (!url.isValid() ) return;
    urls.append( url );
    KURLDrag* drag = new KURLDrag( urls, this );
    TQApplication::clipboard()->setData( drag, QClipboard::Clipboard );
  } else if ( choice == 10 ) { // Scroll To
    scrollToAttachment( node );
  }
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotFind()
{
  mViewer->findText();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotFindNext()
{
  mViewer->findTextNext();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotToggleFixedFont()
{
  mUseFixedFont = !mUseFixedFont;
  saveRelativePosition();
  update(true);
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotCopySelectedText()
{
  kapp->clipboard()->setText( mViewer->selectedText() );
}


//-----------------------------------------------------------------------------
void KMReaderWin::atmViewMsg( KMMessagePart* aMsgPart, int nodeId )
{
  assert(aMsgPart!=0);
  KMMessage* msg = new KMMessage;
  msg->fromString(aMsgPart->bodyDecoded());
  assert(msg != 0);
  msg->setMsgSerNum( 0 ); // because lookups will fail
  // some information that is needed for imap messages with LOD
  msg->setParent( message()->parent() );
  msg->setUID(message()->UID());
  msg->setReadyToShow(true);
  KMReaderMainWin *win = new KMReaderMainWin();
  win->showMsg( overrideEncoding(), msg, message()->getMsgSerNum(), nodeId );
  win->show();
}


void KMReaderWin::setMsgPart( partNode * node ) {
  htmlWriter()->reset();
  mColorBar->hide();
  htmlWriter()->begin( mCSSHelper->cssDefinitions( isFixedFont() ) );
  htmlWriter()->write( mCSSHelper->htmlHead( isFixedFont() ) );
  // end ###
  if ( node ) {
    ObjectTreeParser otp( this, 0, true );
    otp.parseObjectTree( node );
  }
  // ### this, too
  htmlWriter()->queue( "</body></html>" );
  htmlWriter()->flush();
}

//-----------------------------------------------------------------------------
void KMReaderWin::setMsgPart( KMMessagePart* aMsgPart, bool aHTML,
			      const TQString& aFileName, const TQString& pname )
{
  KCursorSaver busy(KBusyPtr::busy());
  if (kasciistricmp(aMsgPart->typeStr(), "message")==0) {
      // if called from compose win
      KMMessage* msg = new KMMessage;
      assert(aMsgPart!=0);
      msg->fromString(aMsgPart->bodyDecoded());
      mMainWindow->setCaption(msg->subject());
      setMsg(msg, true);
      setAutoDelete(true);
  } else if (kasciistricmp(aMsgPart->typeStr(), "text")==0) {
      if (kasciistricmp(aMsgPart->subtypeStr(), "x-vcard") == 0) {
        showVCard( aMsgPart );
	return;
      }
      htmlWriter()->begin( mCSSHelper->cssDefinitions( isFixedFont() ) );
      htmlWriter()->queue( mCSSHelper->htmlHead( isFixedFont() ) );

      if (aHTML && (kasciistricmp(aMsgPart->subtypeStr(), "html")==0)) { // HTML
        // ### this is broken. It doesn't stip off the HTML header and footer!
        htmlWriter()->queue( aMsgPart->bodyToUnicode( overrideCodec() ) );
        mColorBar->setHtmlMode();
      } else { // plain text
        const TQCString str = aMsgPart->bodyDecoded();
        ObjectTreeParser otp( this );
        otp.writeBodyStr( str,
                          overrideCodec() ? overrideCodec() : aMsgPart->codec(),
                          message() ? message()->from() : TQString::null );
      }
      htmlWriter()->queue("</body></html>");
      htmlWriter()->flush();
      mMainWindow->setCaption(i18n("View Attachment: %1").arg(pname));
  } else if (kasciistricmp(aMsgPart->typeStr(), "image")==0 ||
             (kasciistricmp(aMsgPart->typeStr(), "application")==0 &&
              kasciistricmp(aMsgPart->subtypeStr(), "postscript")==0))
  {
      if (aFileName.isEmpty()) return;  // prevent crash
      // Open the window with a size so the image fits in (if possible):
      TQImageIO *iio = new TQImageIO();
      iio->setFileName(aFileName);
      if( iio->read() ) {
          TQImage img = iio->image();
          TQRect desk = KGlobalSettings::desktopGeometry(mMainWindow);
          // determine a reasonable window size
          int width, height;
          if( img.width() < 50 )
              width = 70;
          else if( img.width()+20 < desk.width() )
              width = img.width()+20;
          else
              width = desk.width();
          if( img.height() < 50 )
              height = 70;
          else if( img.height()+20 < desk.height() )
              height = img.height()+20;
          else
              height = desk.height();
          mMainWindow->resize( width, height );
      }
      // Just write the img tag to HTML:
      htmlWriter()->begin( mCSSHelper->cssDefinitions( isFixedFont() ) );
      htmlWriter()->write( mCSSHelper->htmlHead( isFixedFont() ) );
      htmlWriter()->write( "<img src=\"file:" +
                           KURL::encode_string( aFileName ) +
                           "\" border=\"0\">\n"
                           "</body></html>\n" );
      htmlWriter()->end();
      setCaption( i18n("View Attachment: %1").arg( pname ) );
      show();
      delete iio;
  } else {
    htmlWriter()->begin( mCSSHelper->cssDefinitions( isFixedFont() ) );
    htmlWriter()->queue( mCSSHelper->htmlHead( isFixedFont() ) );
    htmlWriter()->queue( "<pre>" );

    TQString str = aMsgPart->bodyDecoded();
    // A TQString cannot handle binary data. So if it's shorter than the
    // attachment, we assume the attachment is binary:
    if( str.length() < (unsigned) aMsgPart->decodedSize() ) {
      str.prepend( i18n("[KMail: Attachment contains binary data. Trying to show first character.]",
          "[KMail: Attachment contains binary data. Trying to show first %n characters.]",
          str.length()) + TQChar('\n') );
    }
    htmlWriter()->queue( TQStyleSheet::escape( str ) );
    htmlWriter()->queue( "</pre>" );
    htmlWriter()->queue("</body></html>");
    htmlWriter()->flush();
    mMainWindow->setCaption(i18n("View Attachment: %1").arg(pname));
  }
  // ---Sven's view text, html and image attachments in html widget end ---
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotAtmView( int id, const TQString& name )
{
  partNode* node = mRootNode ? mRootNode->findId( id ) : 0;
  if( node ) {
    mAtmCurrent = id;
    mAtmCurrentName = name;
    if ( mAtmCurrentName.isEmpty() )
      mAtmCurrentName = tempFileUrlFromPartNode( node ).path();

    KMMessagePart& msgPart = node->msgPart();
    TQString pname = msgPart.fileName();
    if (pname.isEmpty()) pname=msgPart.name();
    if (pname.isEmpty()) pname=msgPart.contentDescription();
    if (pname.isEmpty()) pname="unnamed";
    // image Attachment is saved already
    if (kasciistricmp(msgPart.typeStr(), "message")==0) {
      atmViewMsg( &msgPart,id );
    } else if ((kasciistricmp(msgPart.typeStr(), "text")==0) &&
	       (kasciistricmp(msgPart.subtypeStr(), "x-vcard")==0)) {
      setMsgPart( &msgPart, htmlMail(), name, pname );
    } else {
      KMReaderMainWin *win = new KMReaderMainWin(&msgPart, htmlMail(),
          name, pname, overrideEncoding() );
      win->show();
    }
  }
}

//-----------------------------------------------------------------------------
void KMReaderWin::openAttachment( int id, const TQString & name )
{
  mAtmCurrentName = name;
  mAtmCurrent = id;

  TQString str, pname, cmd, fileName;

  partNode* node = mRootNode ? mRootNode->findId( id ) : 0;
  if( !node ) {
    kdWarning(5006) << "KMReaderWin::openAttachment - could not find node " << id << endl;
    return;
  }
  if ( mAtmCurrentName.isEmpty() )
    mAtmCurrentName = tempFileUrlFromPartNode( node ).path();

  KMMessagePart& msgPart = node->msgPart();
  if (kasciistricmp(msgPart.typeStr(), "message")==0)
  {
    atmViewMsg( &msgPart, id );
    return;
  }

  TQCString contentTypeStr( msgPart.typeStr() + '/' + msgPart.subtypeStr() );
  KPIM::kAsciiToLower( contentTypeStr.data() );

  if ( qstrcmp( contentTypeStr, "text/x-vcard" ) == 0 ) {
    showVCard( &msgPart );
    return;
  }

  // determine the MIME type of the attachment
  KMimeType::Ptr mimetype;
  // prefer the value of the Content-Type header
  mimetype = KMimeType::mimeType( TQString::fromLatin1( contentTypeStr ) );
  if ( mimetype->name() == "application/octet-stream" ) {
    // consider the filename if Content-Type is application/octet-stream
    mimetype = KMimeType::findByPath( name, 0, true /* no disk access */ );
  }
  if ( ( mimetype->name() == "application/octet-stream" )
       && msgPart.isComplete() ) {
    // consider the attachment's contents if neither the Content-Type header
    // nor the filename give us a clue
    mimetype = KMimeType::findByFileContent( name );
  }

  KService::Ptr offer =
    KServiceTypeProfile::preferredService( mimetype->name(), "Application" );

  TQString open_text;
  TQString filenameText = msgPart.fileName();
  if ( filenameText.isEmpty() )
    filenameText = msgPart.name();
  if ( offer ) {
    open_text = i18n("&Open with '%1'").arg( offer->name() );
  } else {
    open_text = i18n("&Open With...");
  }
  const TQString text = i18n("Open attachment '%1'?\n"
                            "Note that opening an attachment may compromise "
                            "your system's security.")
                       .arg( filenameText );
  const int choice = KMessageBox::questionYesNoCancel( this, text,
      i18n("Open Attachment?"), KStdGuiItem::saveAs(), open_text,
      TQString::fromLatin1("askSave") + mimetype->name() ); // dontAskAgainName

  if( choice == KMessageBox::Yes ) {		// Save
    mAtmUpdate = true;
    KMHandleAttachmentCommand* command = new KMHandleAttachmentCommand( node,
        message(), mAtmCurrent, mAtmCurrentName, KMHandleAttachmentCommand::Save,
        offer, this );
    connect( command, TQT_SIGNAL( showAttachment( int, const TQString& ) ),
        this, TQT_SLOT( slotAtmView( int, const TQString& ) ) );
    command->start();
  }
  else if( choice == KMessageBox::No ) {	// Open
    KMHandleAttachmentCommand::AttachmentAction action = ( offer ?
        KMHandleAttachmentCommand::Open : KMHandleAttachmentCommand::OpenWith );
    mAtmUpdate = true;
    KMHandleAttachmentCommand* command = new KMHandleAttachmentCommand( node,
        message(), mAtmCurrent, mAtmCurrentName, action, offer, this );
    connect( command, TQT_SIGNAL( showAttachment( int, const TQString& ) ),
        this, TQT_SLOT( slotAtmView( int, const TQString& ) ) );
    command->start();
  } else {					// Cancel
    kdDebug(5006) << "Canceled opening attachment" << endl;
  }
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotScrollUp()
{
  static_cast<TQScrollView *>(mViewer->widget())->scrollBy(0, -10);
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotScrollDown()
{
  static_cast<TQScrollView *>(mViewer->widget())->scrollBy(0, 10);
}

bool KMReaderWin::atBottom() const
{
    const TQScrollView *view = static_cast<const TQScrollView *>(mViewer->widget());
    return view->contentsY() + view->visibleHeight() >= view->contentsHeight();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotJumpDown()
{
    TQScrollView *view = static_cast<TQScrollView *>(mViewer->widget());
    int offs = (view->clipper()->height() < 30) ? view->clipper()->height() : 30;
    view->scrollBy( 0, view->clipper()->height() - offs );
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotScrollPrior()
{
  static_cast<TQScrollView *>(mViewer->widget())->scrollBy(0, -(int)(height()*0.8));
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotScrollNext()
{
  static_cast<TQScrollView *>(mViewer->widget())->scrollBy(0, (int)(height()*0.8));
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotDocumentChanged()
{

}


//-----------------------------------------------------------------------------
void KMReaderWin::slotTextSelected(bool)
{
  TQString temp = mViewer->selectedText();
  kapp->clipboard()->setText(temp);
}

//-----------------------------------------------------------------------------
void KMReaderWin::selectAll()
{
  mViewer->selectAll();
}

//-----------------------------------------------------------------------------
TQString KMReaderWin::copyText()
{
  TQString temp = mViewer->selectedText();
  return temp;
}


//-----------------------------------------------------------------------------
void KMReaderWin::slotDocumentDone()
{
  // mSbVert->setValue(0);
}


//-----------------------------------------------------------------------------
void KMReaderWin::setHtmlOverride(bool override)
{
  mHtmlOverride = override;
  if (message())
      message()->setDecodeHTML(htmlMail());
}


//-----------------------------------------------------------------------------
void KMReaderWin::setHtmlLoadExtOverride(bool override)
{
  mHtmlLoadExtOverride = override;
  //if (message())
  //    message()->setDecodeHTML(htmlMail());
}


//-----------------------------------------------------------------------------
bool KMReaderWin::htmlMail()
{
  return ((mHtmlMail && !mHtmlOverride) || (!mHtmlMail && mHtmlOverride));
}


//-----------------------------------------------------------------------------
bool KMReaderWin::htmlLoadExternal()
{
  return ((mHtmlLoadExternal && !mHtmlLoadExtOverride) ||
          (!mHtmlLoadExternal && mHtmlLoadExtOverride));
}


//-----------------------------------------------------------------------------
void KMReaderWin::saveRelativePosition()
{
  const TQScrollView * scrollview = static_cast<TQScrollView *>( mViewer->widget() );
  mSavedRelativePosition =
    static_cast<float>( scrollview->contentsY() ) / scrollview->contentsHeight();
}


//-----------------------------------------------------------------------------
void KMReaderWin::update( bool force )
{
  KMMessage* msg = message();
  if ( msg )
    setMsg( msg, force, true /* updateOnly */ );
}


//-----------------------------------------------------------------------------
KMMessage* KMReaderWin::message( KMFolder** aFolder ) const
{
  KMFolder*  tmpFolder;
  KMFolder*& folder = aFolder ? *aFolder : tmpFolder;
  folder = 0;
  if (mMessage)
      return mMessage;
  if (mLastSerNum) {
    KMMessage *message = 0;
    int index;
    KMMsgDict::instance()->getLocation( mLastSerNum, &folder, &index );
    if (folder )
      message = folder->getMsg( index );
    if (!message)
      kdWarning(5006) << "Attempt to reference invalid serial number " << mLastSerNum << "\n" << endl;
    return message;
  }
  return 0;
}



//-----------------------------------------------------------------------------
void KMReaderWin::slotUrlClicked()
{
  KMMainWidget *mainWidget = dynamic_cast<KMMainWidget*>(mMainWindow);
  uint identity = 0;
  if ( message() && message()->parent() ) {
    identity = message()->parent()->identity();
  }

  KMCommand *command = new KMUrlClickedCommand( mClickedUrl, identity, this,
						false, mainWidget );
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotMailtoCompose()
{
  KMCommand *command = new KMMailtoComposeCommand( mClickedUrl, message() );
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotMailtoForward()
{
  KMCommand *command = new KMMailtoForwardCommand( mMainWindow, mClickedUrl,
						   message() );
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotMailtoAddAddrBook()
{
  KMCommand *command = new KMMailtoAddAddrBookCommand( mClickedUrl,
						       mMainWindow);
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotMailtoOpenAddrBook()
{
  KMCommand *command = new KMMailtoOpenAddrBookCommand( mClickedUrl,
							mMainWindow );
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotUrlCopy()
{
  // we don't necessarily need a mainWidget for KMUrlCopyCommand so
  // it doesn't matter if the dynamic_cast fails.
  KMCommand *command =
    new KMUrlCopyCommand( mClickedUrl,
                          dynamic_cast<KMMainWidget*>( mMainWindow ) );
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotUrlOpen( const KURL &url )
{
  if ( !url.isEmpty() )
    mClickedUrl = url;
  KMCommand *command = new KMUrlOpenCommand( mClickedUrl, this );
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotAddBookmarks()
{
    KMCommand *command = new KMAddBookmarksCommand( mClickedUrl, this );
    command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotUrlSave()
{
  KMCommand *command = new KMUrlSaveCommand( mClickedUrl, mMainWindow );
  command->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotMailtoReply()
{
  KMCommand *command = new KMMailtoReplyCommand( mMainWindow, mClickedUrl,
                                                 message(), copyText() );
  command->start();
}

//-----------------------------------------------------------------------------
partNode * KMReaderWin::partNodeFromUrl( const KURL & url ) {
  return mRootNode ? mRootNode->findId( msgPartFromUrl( url ) ) : 0 ;
}

partNode * KMReaderWin::partNodeForId( int id ) {
  return mRootNode ? mRootNode->findId( id ) : 0 ;
}


KURL KMReaderWin::tempFileUrlFromPartNode( const partNode * node )
{
  if (!node) return KURL();
  TQStringList::const_iterator it = mTempFiles.begin();
  TQStringList::const_iterator end = mTempFiles.end();

  while ( it != end ) {
      TQString path = *it;
      it++;
      uint right = path.findRev('/');
      uint left = path.findRev('.', right);

      bool ok;
      int res = path.mid(left + 1, right - left - 1).toInt(&ok);
      if ( res == node->nodeId() )
          return KURL( path );
  }
  return KURL();
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotSaveAttachments()
{
  mAtmUpdate = true;
  KMSaveAttachmentsCommand *saveCommand = new KMSaveAttachmentsCommand( mMainWindow,
                                                                        message() );
  saveCommand->start();
}

//-----------------------------------------------------------------------------
void KMReaderWin::saveAttachment( const KURL &tempFileName )
{
  mAtmCurrent = msgPartFromUrl( tempFileName );
  mAtmCurrentName = mClickedUrl.path();
  slotHandleAttachment( KMHandleAttachmentCommand::Save ); // save
}

//-----------------------------------------------------------------------------
void KMReaderWin::slotSaveMsg()
{
  KMSaveMsgCommand *saveCommand = new KMSaveMsgCommand( mMainWindow, message() );

  if (saveCommand->url().isEmpty())
    delete saveCommand;
  else
    saveCommand->start();
}
//-----------------------------------------------------------------------------
void KMReaderWin::slotIMChat()
{
  KMCommand *command = new KMIMChatCommand( mClickedUrl, message() );
  command->start();
}

//-----------------------------------------------------------------------------
static TQString linkForNode( const DOM::Node &node )
{
  try {
    if ( node.isNull() )
      return TQString();

    const DOM::NamedNodeMap attributes = node.attributes();
    if ( !attributes.isNull() ) {
      const DOM::Node href = attributes.getNamedItem( DOM::DOMString( "href" ) );
      if ( !href.isNull() ) {
        return href.nodeValue().string();
      }
    }
    if ( !node.parentNode().isNull() ) {
      return linkForNode( node.parentNode() );
    } else {
      return TQString();
    }
  } catch ( DOM::DOMException &e ) {
    kdWarning(5006) << "Got an exception when trying to determine link under cursor!" << endl;
    return TQString();
  }
}

//-----------------------------------------------------------------------------
bool KMReaderWin::eventFilter( TQObject *, TQEvent *e )
{
  if ( e->type() == TQEvent::MouseButtonPress ) {
    TQMouseEvent* me = static_cast<TQMouseEvent*>(e);
    if ( me->button() == LeftButton && ( me->state() & ShiftButton ) ) {
      // special processing for shift+click
      URLHandlerManager::instance()->handleShiftClick( mHoveredUrl, this );
      return true;
    }

    if ( me->button() == LeftButton ) {

      TQString imagePath;
      const DOM::Node nodeUnderMouse = mViewer->nodeUnderMouse();
      if ( !nodeUnderMouse.isNull() ) {
        const DOM::NamedNodeMap attributes = nodeUnderMouse.attributes();
        if ( !attributes.isNull() ) {
          const DOM::Node src = attributes.getNamedItem( DOM::DOMString( "src" ) );
          if ( !src.isNull() ) {
            imagePath = src.nodeValue().string();
          }
        }
      }

      mCanStartDrag = URLHandlerManager::instance()->willHandleDrag( mHoveredUrl, imagePath, this );
      mLastClickPosition = me->pos();
      mLastClickImagePath = imagePath;
    }
  }

  if ( e->type() ==  TQEvent::MouseButtonRelease ) {
    mCanStartDrag = false;
  }

  if ( e->type() == TQEvent::MouseMove ) {
    TQMouseEvent* me = static_cast<TQMouseEvent*>( e );

    // Handle this ourselves instead of connecting to mViewer::onURL(), since KHTML misses some
    // notifications in case we started a drag ourselves
    slotUrlOn( linkForNode( mViewer->nodeUnderMouse() ) );

    if ( ( mLastClickPosition - me->pos() ).manhattanLength() > KGlobalSettings::dndEventDelay() ) {
      if ( mCanStartDrag && ( !( mHoveredUrl.isEmpty() && mLastClickImagePath.isEmpty() ) ) ) {
        if ( URLHandlerManager::instance()->handleDrag( mHoveredUrl, mLastClickImagePath, this ) ) {
          mCanStartDrag = false;
          slotUrlOn( TQString() );

          // HACK: Send a mouse release event to the KHTMLView, as otherwise that will be missed in
          //       case we started a drag. If the event is missed, the HTML view gets into a wrong
          //       state, in which funny things like unsolicited drags start to happen.
          TQMouseEvent mouseEvent( TQEvent::MouseButtonRelease, me->pos(), TQt::NoButton, TQt::NoButton );
          static_cast<TQObject*>( mViewer->view() )->eventFilter( mViewer->view()->viewport(),
                                                                 &mouseEvent );
          return true;
        }
      }
    }
  }

  // standard event processing
  return false;
}

void KMReaderWin::fillCommandInfo( partNode *node, KMMessage **msg, int *nodeId )
{
  Q_ASSERT( msg && nodeId );

  if ( mSerNumOfOriginalMessage != 0 ) {
    KMFolder *folder = 0;
    int index = -1;
    KMMsgDict::instance()->getLocation( mSerNumOfOriginalMessage, &folder, &index );
    if ( folder && index != -1 )
      *msg = folder->getMsg( index );

    if ( !( *msg ) ) {
      kdWarning( 5006 ) << "Unable to find the original message, aborting attachment deletion!" << endl;
      return;
    }

    *nodeId = node->nodeId() + mNodeIdOffset;
  }
  else {
    *nodeId = node->nodeId();
    *msg = message();
  }
}

void KMReaderWin::slotDeleteAttachment(partNode * node)
{
  if ( KMessageBox::warningContinueCancel( this,
       i18n("Deleting an attachment might invalidate any digital signature on this message."),
       i18n("Delete Attachment"), KStdGuiItem::del(), "DeleteAttachmentSignatureWarning" )
     != KMessageBox::Continue ) {
    return;
  }

  int nodeId = -1;
  KMMessage *msg = 0;
  fillCommandInfo( node, &msg, &nodeId );
  if ( msg && nodeId != -1 ) {
    KMDeleteAttachmentCommand* command = new KMDeleteAttachmentCommand( nodeId, msg, this );
    command->start();
    connect( command, TQT_SIGNAL( completed( KMCommand * ) ),
             this, TQT_SLOT( updateReaderWin() ) );
    connect( command, TQT_SIGNAL( completed( KMCommand * ) ),
             this, TQT_SLOT( disconnectMsgAdded() ) );

    // ### HACK: Since the command will do delete + add, a new message will arrive. However, we don't
    // want the selection to change. Therefore, as soon as a new message arrives, select it, and then
    // disconnect.
    // Of course the are races, another message can arrive before ours, but we take the risk.
    // And it won't work properly with multiple main windows
    const KMHeaders * const headers = KMKernel::self()->getKMMainWidget()->headers();
    connect( headers, TQT_SIGNAL( msgAddedToListView( TQListViewItem* ) ),
             this, TQT_SLOT( msgAdded( TQListViewItem* ) ) );
  }

  // If we are operating on a copy of parts of the message, make sure to update the copy as well.
  if ( mSerNumOfOriginalMessage != 0 && message() ) {
    message()->deleteBodyPart( node->nodeId() );
    update( true );
  }
}

void KMReaderWin::msgAdded( TQListViewItem *item )
{
  // A new message was added to the message list view. Select it.
  // This is only connected right after we started a attachment delete command, so we expect a new
  // message. Disconnect right afterwards, we only want this particular message to be selected.
  disconnectMsgAdded();
  KMHeaders * const headers = KMKernel::self()->getKMMainWidget()->headers();
  headers->setCurrentItem( item );
  headers->clearSelection();
  headers->setSelected( item, true );
}

void KMReaderWin::disconnectMsgAdded()
{
  const KMHeaders *const headers = KMKernel::self()->getKMMainWidget()->headers();
  disconnect( headers, TQT_SIGNAL( msgAddedToListView( TQListViewItem* ) ),
              this, TQT_SLOT( msgAdded( TQListViewItem* ) ) );
}

void KMReaderWin::slotEditAttachment(partNode * node)
{
  if ( KMessageBox::warningContinueCancel( this,
        i18n("Modifying an attachment might invalidate any digital signature on this message."),
        i18n("Edit Attachment"), KGuiItem( i18n("Edit"), "edit" ), "EditAttachmentSignatureWarning" )
        != KMessageBox::Continue ) {
    return;
  }

  int nodeId = -1;
  KMMessage *msg = 0;
  fillCommandInfo( node, &msg, &nodeId );
  if ( msg && nodeId != -1 ) {
    KMEditAttachmentCommand* command = new KMEditAttachmentCommand( nodeId, msg, this );
    command->start();
  }

  // FIXME: If we are operating on a copy of parts of the message, make sure to update the copy as well.
}

KMail::CSSHelper* KMReaderWin::cssHelper()
{
  return mCSSHelper;
}

bool KMReaderWin::decryptMessage() const
{
  if ( !GlobalSettings::self()->alwaysDecrypt() )
    return mDecrytMessageOverwrite;
  return true;
}

void KMReaderWin::scrollToAttachment( const partNode *node )
{
  DOM::Document doc = mViewer->htmlDocument();

  // The anchors for this are created in ObjectTreeParser::parseObjectTree()
  mViewer->gotoAnchor( TQString::fromLatin1( "att%1" ).arg( node->nodeId() ) );

  // Remove any old color markings which might be there
  const partNode *root = node->topLevelParent();
  for ( int i = 0; i <= root->totalChildCount() + 1; i++ ) {
    DOM::Element attachmentDiv = doc.getElementById( TQString( "attachmentDiv%1" ).arg( i + 1 ) );
    if ( !attachmentDiv.isNull() )
      attachmentDiv.removeAttribute( "style" );
  }

  // Don't mark hidden nodes, that would just produce a strange yellow line
  if ( node->isDisplayedHidden() )
    return;

  // Now, color the div of the attachment in yellow, so that the user sees what happened.
  // We created a special marked div for this in writeAttachmentMarkHeader() in ObjectTreeParser,
  // find and modify that now.
  DOM::Element attachmentDiv = doc.getElementById( TQString( "attachmentDiv%1" ).arg( node->nodeId() ) );
  if ( attachmentDiv.isNull() ) {
    kdWarning( 5006 ) << "Could not find attachment div for attachment " << node->nodeId() << endl;
    return;
  }

  attachmentDiv.setAttribute( "style", TQString( "border:2px solid %1" )
      .arg( cssHelper()->pgpWarnColor().name() ) );

  // Update rendering, otherwise the rendering is not updated when the user clicks on an attachment
  // that causes scrolling and the open attachment dialog
  doc.updateRendering();
}

void KMReaderWin::injectAttachments()
{
  // inject attachments in header view
  // we have to do that after the otp has run so we also see encrypted parts
  DOM::Document doc = mViewer->htmlDocument();
  DOM::Element injectionPoint = doc.getElementById( "attachmentInjectionPoint" );
  if ( injectionPoint.isNull() )
    return;

  TQString imgpath( locate("data","kmail/pics/") );
  TQString visibility;
  TQString urlHandle;
  TQString imgSrc;
  if( !showAttachmentQuicklist() ) {
    urlHandle.append( "kmail:showAttachmentQuicklist" );
    imgSrc.append( "attachmentQuicklistClosed.png" );
  } else {
    urlHandle.append( "kmail:hideAttachmentQuicklist" );
    imgSrc.append( "attachmentQuicklistOpened.png" );
  }

  TQString html = renderAttachments( mRootNode, TQApplication::palette().active().background() );
  if ( html.isEmpty() )
    return;

  TQString link("");
  if ( headerStyle() == HeaderStyle::fancy() ) {
    link += "<div style=\"text-align: left;\"><a href=\"" + urlHandle + "\"><img src=\"" +
            imgpath + imgSrc + "\"/></a></div>";
    html.prepend( link );
    html.prepend( TQString::fromLatin1( "<div style=\"float:left;\">%1&nbsp;</div>" ).
                  arg( i18n( "Attachments:" ) ) );
  } else {
    link += "<div style=\"text-align: right;\"><a href=\"" + urlHandle + "\"><img src=\"" +
            imgpath + imgSrc + "\"/></a></div>";
    html.prepend( link );
  }

  assert( injectionPoint.tagName() == "div" );
  static_cast<DOM::HTMLElement>( injectionPoint ).setInnerHTML( html );
}

static TQColor nextColor( const TQColor & c )
{
  int h, s, v;
  c.hsv( &h, &s, &v );
  return TQColor( (h + 50) % 360, QMAX(s, 64), v, TQColor::Hsv );
}

TQString KMReaderWin::renderAttachments(partNode * node, const TQColor &bgColor )
{
  if ( !node )
    return TQString();

  TQString html;
  if ( node->firstChild() ) {
    TQString subHtml = renderAttachments( node->firstChild(), nextColor( bgColor ) );
    if ( !subHtml.isEmpty() ) {

      TQString visibility;
      if ( !showAttachmentQuicklist() ) {
        visibility.append( "display:none;" );
      }

      TQString margin;
      if ( node != mRootNode || headerStyle() != HeaderStyle::enterprise() )
        margin = "padding:2px; margin:2px; ";
      TQString align = "left";
      if ( headerStyle() == HeaderStyle::enterprise() )
        align = "right";
      if ( node->msgPart().typeStr().lower() == "message" || node == mRootNode )
        html += TQString::fromLatin1("<div style=\"background:%1; %2"
                "vertical-align:middle; float:%3; %4\">").arg( bgColor.name() ).arg( margin )
                                                         .arg( align ).arg( visibility );
      html += subHtml;
      if ( node->msgPart().typeStr().lower() == "message" || node == mRootNode )
        html += "</div>";
    }
  } else {
    partNode::AttachmentDisplayInfo info = node->attachmentDisplayInfo();
    if ( info.displayInHeader ) {
      html += "<div style=\"float:left;\">";
      html += TQString::fromLatin1( "<span style=\"white-space:nowrap; border-width: 0px; border-left-width: 5px; border-color: %1; 2px; border-left-style: solid;\">" ).arg( bgColor.name() );
      TQString fileName = writeMessagePartToTempFile( &node->msgPart(), node->nodeId() );
      TQString href = node->asHREF( "header" );
      html += TQString::fromLatin1( "<a href=\"" ) + href +
              TQString::fromLatin1( "\">" );
      html += "<img style=\"vertical-align:middle;\" src=\"" + info.icon + "\"/>&nbsp;";
      if ( headerStyle() == HeaderStyle::enterprise() ) {
        TQFont bodyFont = mCSSHelper->bodyFont( isFixedFont() );
        TQFontMetrics fm( bodyFont );
        html += KStringHandler::rPixelSqueeze( info.label, fm, 140 );
      } else if ( headerStyle() == HeaderStyle::fancy() ) {
        TQFont bodyFont = mCSSHelper->bodyFont( isFixedFont() );
        TQFontMetrics fm( bodyFont );
        html += KStringHandler::rPixelSqueeze( info.label, fm, 640 );
      } else {
        html += info.label;
      }
      html += "</a></span></div> ";
    }
  }

  html += renderAttachments( node->nextSibling(), nextColor ( bgColor ) );
  return html;
}

using namespace KMail::Interface;

void KMReaderWin::setBodyPartMemento( const partNode * node, const TQCString & which, BodyPartMemento * memento )
{
  const TQCString index = node->path() + ':' + which.lower();

  const std::map<TQCString,BodyPartMemento*>::iterator it = mBodyPartMementoMap.lower_bound( index );
  if ( it != mBodyPartMementoMap.end() && it->first == index ) {

    if ( memento && memento == it->second )
      return;

    delete it->second;

    if ( memento ) {
      it->second = memento;
    }
    else {
      mBodyPartMementoMap.erase( it );
    }

  } else {
    if ( memento ) {
      mBodyPartMementoMap.insert( it, std::make_pair( index, memento ) );
    }
  }

  if ( Observable * o = memento ? memento->asObservable() : 0 )
    o->attach( this );
}

BodyPartMemento * KMReaderWin::bodyPartMemento( const partNode * node, const TQCString & which ) const
{
  const TQCString index = node->path() + ':' + which.lower();
  const std::map<TQCString,BodyPartMemento*>::const_iterator it = mBodyPartMementoMap.find( index );
  if ( it == mBodyPartMementoMap.end() ) {
    return 0;
  }
  else {
    return it->second;
  }
}

static void detach_and_delete( BodyPartMemento * memento, KMReaderWin * obs ) {
  if ( Observable * const o = memento ? memento->asObservable() : 0 )
    o->detach( obs );
  delete memento;
}

void KMReaderWin::clearBodyPartMementos()
{
  for ( std::map<TQCString,BodyPartMemento*>::const_iterator it = mBodyPartMementoMap.begin(), end = mBodyPartMementoMap.end() ; it != end ; ++it )
    // Detach the memento from the reader. When cancelling it, it might trigger an update of the
    // reader, which we are not interested in, and which is dangerous, since half the mementos are
    // already deleted.
    // https://issues.kolab.org/issue4187
    detach_and_delete( it->second, this );

  mBodyPartMementoMap.clear();
}

#include "kmreaderwin.moc"


