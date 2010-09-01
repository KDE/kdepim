/*  -*- mode: C++; c-file-style: "gnu" -*- */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "config.h"
#include "kmkernel.h"

#include <weaver.h>
#include <weaverlogger.h>

#include "globalsettings.h"
#include "broadcaststatus.h"
using KPIM::BroadcastStatus;
#include "kmstartup.h"
#include "index.h"
#include "kmmainwin.h"
#include "composer.h"
#include "kmmsgpart.h"
#include "kmreadermainwin.h"
#include "kmfoldermgr.h"
#include "kmfoldercachedimap.h"
#include "kmacctcachedimap.h"
#include "kmfiltermgr.h"
#include "kmfilteraction.h"
#include "kmheaders.h"
#define REALLY_WANT_KMSENDER
#include "kmsender.h"
#undef REALLY_WANT_KMSENDER
#include "undostack.h"
#include "accountmanager.h"
using KMail::AccountManager;
#include <libkdepim/kfileio.h>
#include "kmversion.h"
#include "kmreaderwin.h"
#include "kmmainwidget.h"
#include "kmfoldertree.h"
#include "recentaddresses.h"
using KRecentAddress::RecentAddresses;
#include "kmmsgdict.h"
#include <libkpimidentities/identity.h>
#include <libkpimidentities/identitymanager.h>
#include "configuredialog.h"
#include "kmcommands.h"
#include "kmsystemtray.h"
#include "transportmanager.h"
#include "importarchivedialog.h"

#include <kwin.h>
#include "kmailicalifaceimpl.h"
#include "mailserviceimpl.h"
using KMail::MailServiceImpl;
#include "mailcomposerIface.h"
#include "folderIface.h"
using KMail::FolderIface;
#include "jobscheduler.h"
#include "templateparser.h"

#include <kapplication.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kstaticdeleter.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kprogress.h>
#include <kpassivepopup.h>
#include <dcopclient.h>
#include <ksystemtray.h>
#include <kpgp.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kwallet.h>
using KWallet::Wallet;
#include "actionscheduler.h"

#include <qutf7codec.h>
#include <tqvbox.h>
#include <tqdir.h>
#include <tqwidgetlist.h>
#include <tqobjectlist.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <fixx11h.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>

KMKernel *KMKernel::mySelf = 0;
static bool s_askingToGoOnline = false;

/********************************************************************/
/*                     Constructor and destructor                   */
/********************************************************************/
KMKernel::KMKernel (TQObject *parent, const char *name) :
  DCOPObject("KMailIface"), TQObject(parent, name),
  mIdentityManager(0), mConfigureDialog(0),
  mContextMenuShown( false ), mWallet( 0 )
{
  kdDebug(5006) << "KMKernel::KMKernel" << endl;
  mySelf = this;
  the_startingUp = true;
  closed_by_user = true;
  the_firstInstance = true;
  the_msgIndex = 0;

  the_inboxFolder = 0;
  the_outboxFolder = 0;
  the_sentFolder = 0;
  the_trashFolder = 0;
  the_draftsFolder = 0;
  the_templatesFolder = 0;

  the_folderMgr = 0;
  the_imapFolderMgr = 0;
  the_dimapFolderMgr = 0;
  the_searchFolderMgr = 0;
  the_undoStack = 0;
  the_acctMgr = 0;
  the_filterMgr = 0;
  the_popFilterMgr = 0;
  the_filterActionDict = 0;
  the_msgSender = 0;
  mWin = 0;
  mMailCheckAborted = false;

  // make sure that we check for config updates before doing anything else
  KMKernel::config();
  // this shares the kmailrc parsing too (via KSharedConfig), and reads values from it
  // so better do it here, than in some code where changing the group of config()
  // would be unexpected
  GlobalSettings::self();

  // Set up DCOP interface
  mICalIface = new KMailICalIfaceImpl();

  mJobScheduler = new JobScheduler( this );

  mXmlGuiInstance = 0;

  new Kpgp::Module();

  // register our own (libkdenetwork) utf-7 codec as long as Qt
  // doesn't have it's own:
  if ( !TQTextCodec::codecForName("utf-7") ) {
    kdDebug(5006) << "No Qt-native utf-7 codec found; registering QUtf7Codec from libkdenetwork" << endl;
    (void) new QUtf7Codec();
  }

  // In the case of Japan. Japanese locale name is "eucjp" but
  // The Japanese mail systems normally used "iso-2022-jp" of locale name.
  // We want to change locale name from eucjp to iso-2022-jp at KMail only.
  if ( TQCString(TQTextCodec::codecForLocale()->name()).lower() == "eucjp" )
  {
    netCodec = TQTextCodec::codecForName("jis7");
    // TQTextCodec *cdc = TQTextCodec::codecForName("jis7");
    // TQTextCodec::setCodecForLocale(cdc);
    // KGlobal::locale()->setEncoding(cdc->mibEnum());
  } else {
    netCodec = TQTextCodec::codecForLocale();
  }
  mMailService =  new MailServiceImpl();

  connectDCOPSignal( 0, 0, "kmailSelectFolder(TQString)",
                     "selectFolder(TQString)", false );
}

KMKernel::~KMKernel ()
{
  TQMap<KIO::Job*, putData>::Iterator it = mPutJobs.begin();
  while ( it != mPutJobs.end() )
  {
    KIO::Job *job = it.key();
    mPutJobs.remove( it );
    job->kill();
    it = mPutJobs.begin();
  }

  delete mICalIface;
  mICalIface = 0;
  delete mMailService;
  mMailService = 0;

  GlobalSettings::self()->writeConfig();
  delete mWallet;
  mWallet = 0;
  mySelf = 0;
  kdDebug(5006) << "KMKernel::~KMKernel" << endl;
}

bool KMKernel::handleCommandLine( bool noArgsOpensReader )
{
  TQString to, cc, bcc, subj, body;
  QCStringList customHeaders;
  KURL messageFile;
  KURL::List attachURLs;
  bool mailto = false;
  bool checkMail = false;
  bool viewOnly = false;
  bool calledWithSession = false; // for ignoring '-session foo'

  // process args:
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->getOption("subject"))
  {
     subj = TQString::fromLocal8Bit(args->getOption("subject"));
     // if kmail is called with 'kmail -session abc' then this doesn't mean
     // that the user wants to send a message with subject "ession" but
     // (most likely) that the user clicked on KMail's system tray applet
     // which results in KMKernel::raise() calling "kmail kmail newInstance"
     // via dcop which apparently executes the application with the original
     // command line arguments and those include "-session ..." if
     // kmail/kontact was restored by session management
     if ( subj == "ession" ) {
       subj = TQString::null;
       calledWithSession = true;
     }
     else
       mailto = true;
  }

  if (args->getOption("cc"))
  {
     mailto = true;
     cc = TQString::fromLocal8Bit(args->getOption("cc"));
  }

  if (args->getOption("bcc"))
  {
     mailto = true;
     bcc = TQString::fromLocal8Bit(args->getOption("bcc"));
  }

  if (args->getOption("msg"))
  {
     mailto = true;
     messageFile.setPath( TQString::fromLocal8Bit(args->getOption("msg")) );
  }

  if (args->getOption("body"))
  {
     mailto = true;
     body = TQString::fromLocal8Bit(args->getOption("body"));
  }

  QCStringList attachList = args->getOptionList("attach");
  if (!attachList.isEmpty())
  {
     mailto = true;
     for ( QCStringList::Iterator it = attachList.begin() ; it != attachList.end() ; ++it )
       if ( !(*it).isEmpty() )
         attachURLs += KURL( TQString::fromLocal8Bit( *it ) );
  }

  customHeaders = args->getOptionList("header");

  if (args->isSet("composer"))
    mailto = true;

  if (args->isSet("check"))
    checkMail = true;

  if ( args->getOption( "view" ) ) {
    viewOnly = true;
    const TQString filename =
      TQString::fromLocal8Bit( args->getOption( "view" ) );
    messageFile = KURL::fromPathOrURL( filename );
    if ( !messageFile.isValid() ) {
      messageFile = KURL();
      messageFile.setPath( filename );
    }
  }

  if ( !calledWithSession ) {
    // only read additional command line arguments if kmail/kontact is
    // not called with "-session foo"
    for(int i= 0; i < args->count(); i++)
    {
      if (strncasecmp(args->arg(i),"mailto:",7)==0)
        to += args->url(i).path() + ", ";
      else {
        TQString tmpArg = TQString::fromLocal8Bit( args->arg(i) );
        KURL url( tmpArg );
        if ( url.isValid() )
          attachURLs += url;
        else
          to += tmpArg + ", ";
      }
      mailto = true;
    }
    if ( !to.isEmpty() ) {
      // cut off the superfluous trailing ", "
      to.truncate( to.length() - 2 );
    }
  }

  if ( !calledWithSession )
    args->clear();

  if ( !noArgsOpensReader && !mailto && !checkMail && !viewOnly )
    return false;

  if ( viewOnly )
    viewMessage( messageFile );
  else
    action( mailto, checkMail, to, cc, bcc, subj, body, messageFile,
            attachURLs, customHeaders );
  return true;
}

/********************************************************************/
/*             DCOP-callable, and command line actions              */
/********************************************************************/
void KMKernel::checkMail () //might create a new reader but won't show!!
{
  if ( !kmkernel->askToGoOnline() )
    return;
  kmkernel->acctMgr()->checkMail(false);
}

TQStringList KMKernel::accounts()
{
  if( kmkernel->acctMgr() )
     return kmkernel->acctMgr()->getAccounts();
  return TQStringList();
}

void KMKernel::checkAccount (const TQString &account) //might create a new reader but won't show!!
{
  kdDebug(5006) << "KMKernel::checkMail called" << endl;

  KMAccount* acct = kmkernel->acctMgr()->findByName(account);
  if (acct)
    kmkernel->acctMgr()->singleCheckMail(acct, false);
}

void KMKernel::loadProfile( const TQString& )
{
}

void KMKernel::saveToProfile( const TQString& ) const
{
}

void KMKernel::openReader( bool onlyCheck )
{
  mWin = 0;
  KMainWindow *ktmw = 0;
  kdDebug(5006) << "KMKernel::openReader called" << endl;

  if (KMainWindow::memberList)
    for (ktmw = KMainWindow::memberList->first(); ktmw;
         ktmw = KMainWindow::memberList->next())
      if (ktmw->isA("KMMainWin"))
        break;

  bool activate;
  if (ktmw) {
    mWin = (KMMainWin *) ktmw;
    activate = !onlyCheck; // existing window: only activate if not --check
    if ( activate )
       mWin->show();
  } else {
    mWin = new KMMainWin;
    mWin->show();
    activate = false; // new window: no explicit activation (#73591)
  }

  if ( activate ) {
    // Activate window - doing this instead of KWin::activateWindow(mWin->winId());
    // so that it also works when called from KMailApplication::newInstance()
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    KStartupInfo::setNewStartupId( mWin, kapp->startupId() );
#endif
  }
}

int KMKernel::openComposer (const TQString &to, const TQString &cc,
                            const TQString &bcc, const TQString &subject,
                            const TQString &body, int hidden,
                            const KURL &messageFile,
                            const KURL::List &attachURLs,
                            const QCStringList &customHeaders)
{
  kdDebug(5006) << "KMKernel::openComposer called" << endl;
  KMMessage *msg = new KMMessage;
  msg->initHeader();
  msg->setCharset("utf-8");
  // tentatively decode to, cc and bcc because invokeMailer calls us with
  // RFC 2047 encoded addresses in order to protect non-ASCII email addresses
  if (!to.isEmpty())
    msg->setTo( KMMsgBase::decodeRFC2047String( to.latin1() ) );
  if (!cc.isEmpty())
    msg->setCc( KMMsgBase::decodeRFC2047String( cc.latin1() ) );
  if (!bcc.isEmpty())
    msg->setBcc( KMMsgBase::decodeRFC2047String( bcc.latin1() ) );
  if (!subject.isEmpty()) msg->setSubject(subject);
  if (!messageFile.isEmpty() && messageFile.isLocalFile()) {
    TQCString str = KPIM::kFileToString( messageFile.path(), true, false );
    if( !str.isEmpty() ) {
      msg->setBody( TQString::fromLocal8Bit( str ).utf8() );
    } else {
      TemplateParser parser( msg, TemplateParser::NewMessage );
      parser.process( NULL, NULL );
    }
  }
  else if (!body.isEmpty())
  {
    msg->setBody(body.utf8());
  }
  else
  {
    TemplateParser parser( msg, TemplateParser::NewMessage );
    parser.process( NULL, NULL );
  }

  if (!customHeaders.isEmpty())
  {
    for ( QCStringList::ConstIterator it = customHeaders.begin() ; it != customHeaders.end() ; ++it )
      if ( !(*it).isEmpty() )
      {
        const int pos = (*it).find( ':' );
        if ( pos > 0 )
        {
          TQCString header, value;
          header = (*it).left( pos ).stripWhiteSpace();
          value = (*it).mid( pos+1 ).stripWhiteSpace();
          if ( !header.isEmpty() && !value.isEmpty() )
            msg->setHeaderField( header, value );
        }
      }
  }

  KMail::Composer * cWin = KMail::makeComposer( msg );
  cWin->setCharset("", true);
  for ( KURL::List::ConstIterator it = attachURLs.begin() ; it != attachURLs.end() ; ++it )
    cWin->addAttach((*it));
  if (hidden == 0) {
    cWin->show();
    // Activate window - doing this instead of KWin::activateWindow(cWin->winId());
    // so that it also works when called from KMailApplication::newInstance()
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    KStartupInfo::setNewStartupId( cWin, kapp->startupId() );
#endif
  }
  return 1;
}


int KMKernel::openComposer (const TQString &to, const TQString &cc,
                            const TQString &bcc, const TQString &subject,
                            const TQString &body, int hidden,
                            const TQString &attachName,
                            const TQCString &attachCte,
                            const TQCString &attachData,
                            const TQCString &attachType,
                            const TQCString &attachSubType,
                            const TQCString &attachParamAttr,
                            const TQString &attachParamValue,
                            const TQCString &attachContDisp )
{
  kdDebug(5006) << "KMKernel::openComposer called (deprecated version)" << endl;

  return openComposer ( to, cc, bcc, subject, body, hidden,
                        attachName, attachCte, attachData,
                        attachType, attachSubType, attachParamAttr,
                        attachParamValue, attachContDisp, TQCString() );
}

int KMKernel::openComposer (const TQString &to, const TQString &cc,
                            const TQString &bcc, const TQString &subject,
                            const TQString &body, int hidden,
                            const TQString &attachName,
                            const TQCString &attachCte,
                            const TQCString &attachData,
                            const TQCString &attachType,
                            const TQCString &attachSubType,
                            const TQCString &attachParamAttr,
                            const TQString &attachParamValue,
                            const TQCString &attachContDisp,
                            const TQCString &attachCharset )
{
  kdDebug(5006) << "KMKernel::openComposer called (deprecated version)" << endl;
  return openComposer ( to, cc, bcc, subject, body, hidden,
                        attachName, attachCte, attachData,
                        attachType, attachSubType, attachParamAttr,
                        attachParamValue, attachContDisp, attachCharset, 0 );
}

int KMKernel::openComposer (const TQString &to, const TQString &cc,
                            const TQString &bcc, const TQString &subject,
                            const TQString &body, int hidden,
                            const TQString &attachName,
                            const TQCString &attachCte,
                            const TQCString &attachData,
                            const TQCString &attachType,
                            const TQCString &attachSubType,
                            const TQCString &attachParamAttr,
                            const TQString &attachParamValue,
                            const TQCString &attachContDisp,
                            const TQCString &attachCharset,
                            unsigned int identity )
{
  kdDebug(5006) << "KMKernel::openComposer()" << endl;

  KMMessage *msg = new KMMessage;
  KMMessagePart *msgPart = 0;
  msg->initHeader();
  msg->setCharset( "utf-8" );
  if ( !cc.isEmpty() ) msg->setCc(cc);
  if ( !bcc.isEmpty() ) msg->setBcc(bcc);
  if ( !subject.isEmpty() ) msg->setSubject(subject);
  if ( !to.isEmpty() ) msg->setTo(to);
  if ( identity > 0 ) msg->setHeaderField( "X-KMail-Identity", TQString::number( identity ) );
  if ( !body.isEmpty() ) {
    msg->setBody(body.utf8());
  } else {
    TemplateParser parser( msg, TemplateParser::NewMessage );
    parser.process( NULL, NULL );
  }

  bool iCalAutoSend = false;
  bool noWordWrap = false;
  bool isICalInvitation = false;
  KConfigGroup options( config(), "Groupware" );
  if ( !attachData.isEmpty() ) {
    isICalInvitation = attachName == "cal.ics" &&
      attachType == "text" &&
      attachSubType == "calendar" &&
      attachParamAttr == "method";
    // Remove BCC from identity on ical invitations (https://intevation.de/roundup/kolab/issue474)
    if ( isICalInvitation && bcc.isEmpty() )
      msg->setBcc( "" );
    if ( isICalInvitation &&
        GlobalSettings::self()->legacyBodyInvites() ) {
      // KOrganizer invitation caught and to be sent as body instead
      msg->setBody( attachData );
      msg->setHeaderField( "Content-Type",
                           TQString( "text/calendar; method=%1; "
                                    "charset=\"utf-8\"" ).
                           arg( attachParamValue ) );

      iCalAutoSend = true; // no point in editing raw ICAL
      noWordWrap = true; // we shant word wrap inline invitations
    } else {
      // Just do what we're told to do
      msgPart = new KMMessagePart;
      msgPart->setName( attachName );
      msgPart->setCteStr( attachCte );
      msgPart->setBodyEncoded( attachData );
      msgPart->setTypeStr( attachType );
      msgPart->setSubtypeStr( attachSubType );
      msgPart->setParameter( attachParamAttr, attachParamValue );
       if( ! GlobalSettings::self()->exchangeCompatibleInvitations() ) {
        msgPart->setContentDisposition( attachContDisp );
      }
      if( !attachCharset.isEmpty() ) {
        // kdDebug(5006) << "KMKernel::openComposer set attachCharset to "
        // << attachCharset << endl;
        msgPart->setCharset( attachCharset );
      }
      // Don't show the composer window, if the automatic sending is checked
      KConfigGroup options( config(), "Groupware" );
      iCalAutoSend = options.readBoolEntry( "AutomaticSending", true );
    }
  }

  KMail::Composer * cWin = KMail::makeComposer();
  cWin->setMsg( msg, !isICalInvitation /* mayAutoSign */ );
  cWin->setSigningAndEncryptionDisabled( isICalInvitation
      && GlobalSettings::self()->legacyBodyInvites() );
  cWin->setAutoDelete( true );
  if( noWordWrap )
    cWin->disableWordWrap();
  else
    cWin->setCharset( "", true );
  if ( msgPart )
    cWin->addAttach(msgPart);

  if ( isICalInvitation ) {
    cWin->disableRecipientNumberCheck();
    cWin->disableForgottenAttachmentsCheck();
  }

  if ( hidden == 0 && !iCalAutoSend ) {
    cWin->show();
    // Activate window - doing this instead of KWin::activateWindow(cWin->winId());
    // so that it also works when called from KMailApplication::newInstance()
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    KStartupInfo::setNewStartupId( cWin, kapp->startupId() );
#endif
  } else {
    cWin->setAutoDeleteWindow( true );
    cWin->slotSendNow();
  }

  return 1;
}

void KMKernel::setDefaultTransport( const TQString & transport )
{
  TQStringList availTransports = KMail::TransportManager::transportNames();
  TQStringList::const_iterator it = availTransports.find( transport );
  if ( it == availTransports.end() ) {
    kdWarning() << "The transport you entered is not available" << endl;
    return;
  }
  GlobalSettings::self()->setDefaultTransport( transport );
}

DCOPRef KMKernel::openComposer(const TQString &to, const TQString &cc,
                               const TQString &bcc, const TQString &subject,
                               const TQString &body,bool hidden)
{
  KMMessage *msg = new KMMessage;
  msg->initHeader();
  msg->setCharset("utf-8");
  if (!cc.isEmpty()) msg->setCc(cc);
  if (!bcc.isEmpty()) msg->setBcc(bcc);
  if (!subject.isEmpty()) msg->setSubject(subject);
  if (!to.isEmpty()) msg->setTo(to);
  if (!body.isEmpty()) {
    msg->setBody(body.utf8());
  } else {
    TemplateParser parser( msg, TemplateParser::NewMessage );
    parser.process( NULL, NULL );
  }

  KMail::Composer * cWin = KMail::makeComposer( msg );
  cWin->setCharset("", true);
  if (!hidden) {
    cWin->show();
    // Activate window - doing this instead of KWin::activateWindow(cWin->winId());
    // so that it also works when called from KMailApplication::newInstance()
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    KStartupInfo::setNewStartupId( cWin, kapp->startupId() );
#endif
  }

  return DCOPRef( cWin->asMailComposerIFace() );
}

DCOPRef KMKernel::newMessage(const TQString &to,
                             const TQString &cc,
                             const TQString &bcc,
                             bool hidden,
                             bool useFolderId,
                             const KURL & /*messageFile*/,
                             const KURL &attachURL)
{
  KMail::Composer * win = 0;
  KMMessage *msg = new KMMessage;
  KMFolder *folder = NULL;
  uint id;

  if ( useFolderId ) {
    //create message with required folder identity
    folder = currentFolder();
    id = folder ? folder->identity() : 0;
    msg->initHeader( id );
  } else {
    msg->initHeader();
  }
  msg->setCharset("utf-8");
  //set basic headers
  if (!to.isEmpty()) msg->setTo(to);
  if (!cc.isEmpty()) msg->setCc(cc);
  if (!bcc.isEmpty()) msg->setBcc(bcc);

  if ( useFolderId ) {
    TemplateParser parser( msg, TemplateParser::NewMessage );
    parser.process( NULL, folder );
    win = makeComposer( msg, id );
  } else {
    TemplateParser parser( msg, TemplateParser::NewMessage );
    parser.process( NULL, folder );
    win = makeComposer( msg );
  }

  //Add the attachment if we have one
  if(!attachURL.isEmpty() && attachURL.isValid()) {
    win->addAttach(attachURL);
  }

  //only show window when required
  if(!hidden) {
    win->show();
  }
  return DCOPRef( win->asMailComposerIFace() );
}

int KMKernel::viewMessage( const KURL & messageFile )
{
  KMOpenMsgCommand *openCommand = new KMOpenMsgCommand( 0, messageFile );

  openCommand->start();

  return 1;
}

int KMKernel::sendCertificate( const TQString& to, const TQByteArray& certData )
{
  KMMessage *msg = new KMMessage;
  msg->initHeader();
  msg->setCharset("utf-8");
  msg->setSubject( i18n( "Certificate Signature Request" ) );
  if (!to.isEmpty()) msg->setTo(to);
  // ### Make this message customizable via KIOSK
  msg->setBody( i18n( "Please create a certificate from attachment and return to sender." ).utf8() );

  KMail::Composer * cWin = KMail::makeComposer( msg );
  cWin->setCharset("", true);
  cWin->slotSetAlwaysSend( true );
  if (!certData.isEmpty()) {
    KMMessagePart *msgPart = new KMMessagePart;
    msgPart->setName("smime.p10");
    msgPart->setCteStr("base64");
    msgPart->setBodyEncodedBinary(certData);
    msgPart->setTypeStr("application");
    msgPart->setSubtypeStr("pkcs10");
    msgPart->setContentDisposition("attachment; filename=smime.p10");
    cWin->addAttach(msgPart);
  }

  cWin->show();
  return 1;
}

KMMsgStatus KMKernel::strToStatus(const TQString &flags)
{
    KMMsgStatus status = 0;
    if (!flags.isEmpty()) {
        for (uint n = 0; n < flags.length() ; n++) {
            switch (flags[n]) {
                case 'N':
                    status |= KMMsgStatusNew;
                    break;
                case 'U':
                    status |= KMMsgStatusUnread;
                    break;
                case 'O':
                    status |= KMMsgStatusOld;
                    break;
                case 'R':
                    status |= KMMsgStatusRead;
                    break;
                case 'D':
                    status |= KMMsgStatusDeleted;
                    break;
                case 'A':
                    status |= KMMsgStatusReplied;
                    break;
                case 'F':
                    status |= KMMsgStatusForwarded;
                    break;
                case 'Q':
                    status |= KMMsgStatusQueued;
                    break;
                case 'K':
                    status |= KMMsgStatusTodo;
                    break;
                case 'S':
                    status |= KMMsgStatusSent;
                    break;
                case 'G':
                    status |= KMMsgStatusFlag;
                    break;
                case 'W':
                    status |= KMMsgStatusWatched;
                    break;
                case 'I':
                    status |= KMMsgStatusIgnored;
                    break;
                case 'P':
                    status |= KMMsgStatusSpam;
                    break;
                case 'H':
                    status |= KMMsgStatusHam;
                    break;
                case 'T':
                    status |= KMMsgStatusHasAttach;
                    break;
                case 'C':
                    status |= KMMsgStatusHasNoAttach;
                    break;
                default:
                    break;
            }
        }
    }
    return status;
}

int KMKernel::dcopAddMessage( const TQString & foldername, const TQString & msgUrlString,
                              const TQString & MsgStatusFlags)
{
  return dcopAddMessage(foldername, KURL(msgUrlString), MsgStatusFlags);
}

int KMKernel::dcopAddMessage( const TQString & foldername,const KURL & msgUrl,
                              const TQString & MsgStatusFlags)
{
  kdDebug(5006) << "KMKernel::dcopAddMessage called" << endl;

  if ( foldername.isEmpty() || foldername.startsWith("."))
    return -1;

  int retval;
  bool readFolderMsgIds = false;
  TQString _foldername = foldername.stripWhiteSpace();
  _foldername = _foldername.replace('\\',""); //try to prevent ESCAPE Sequences

  if ( foldername != mAddMessageLastFolder ) {
    mAddMessageMsgIds.clear();
    readFolderMsgIds = true;
    mAddMessageLastFolder = foldername;
  }

  if (!msgUrl.isEmpty() && msgUrl.isLocalFile()) {

    // This is a proposed change by Daniel Andor.
    // He proposed to change from the fopen(blah)
    // to a KPIM::kFileToString(blah).
    // Although it assigns a TQString to a TQString,
    // because of the implicit sharing this poses
    // no memory or performance penalty.

    const TQCString messageText =
      KPIM::kFileToString( msgUrl.path(), true, false );
    if ( messageText.isEmpty() )
      return -2;

    KMMessage *msg = new KMMessage();
    msg->fromString( messageText );

    if (readFolderMsgIds) {
      if ( foldername.contains("/")) {
        TQString tmp_fname = "";
        KMFolder *folder = NULL;
        KMFolderDir *subfolder;
        bool root = true;

        TQStringList subFList = TQStringList::split("/",_foldername,false);

        for ( TQStringList::Iterator it = subFList.begin(); it != subFList.end(); ++it ) {
          TQString _newFolder = *it;
          if(_newFolder.startsWith(".")) return -1;

          if(root) {
            folder = the_folderMgr->findOrCreate(*it, false);
            if (folder) {
              root = false;
              tmp_fname = "/" + *it;
            }
            else return -1;
          } else {
            subfolder = folder->createChildFolder();
            tmp_fname += "/" + *it;
            if(!the_folderMgr->getFolderByURL( tmp_fname )) {
             folder = the_folderMgr->createFolder(*it, false, folder->folderType(), subfolder);
            }

            if(!(folder = the_folderMgr->getFolderByURL( tmp_fname ))) return -1;
          }
        }

        mAddMsgCurrentFolder = the_folderMgr->getFolderByURL( tmp_fname );
        if(!folder) return -1;

      } else {
        mAddMsgCurrentFolder = the_folderMgr->findOrCreate(_foldername, false);
      }
    }

    if ( mAddMsgCurrentFolder ) {
      if (readFolderMsgIds) {

      	// OLD COMMENT:
        // Try to determine if a message already exists in
        // the folder. The message id that is searched for, is
        // the subject line + the date. This should be quite
        // unique. The change that a given date with a given
        // subject is in the folder twice is very small.
        // If the subject is empty, the fromStrip string
        // is taken.

	// NEW COMMENT from Danny Kukawka (danny.kukawka@web.de):
	// subject line + the date is only unique if the following
	// return a correct unique value:
	// 	time_t  DT = mb->date();
        // 	TQString dt = ctime(&DT);
	// But if the datestring in the Header isn't RFC conform
	// subject line + the date isn't unique.
	//
	// The only uique headerfield is the Message-ID. In some
	// cases this could be empty. I then I use the
	// subject line + dateStr .

        int i;

        mAddMsgCurrentFolder->open("dcopadd");
        for( i=0; i<mAddMsgCurrentFolder->count(); i++) {
          KMMsgBase *mb = mAddMsgCurrentFolder->getMsgBase(i);
	  TQString id = mb->msgIdMD5();
	  if ( id.isEmpty() ) {
            id = mb->subject();
            if ( id.isEmpty() )
              id = mb->fromStrip();
            if ( id.isEmpty() )
              id = mb->toStrip();

            id += mb->dateStr();
	  }

          //fprintf(stderr,"%s\n",(const char *) id);
          if ( !id.isEmpty() ) {
            mAddMessageMsgIds.append(id);
          }
        }
        mAddMsgCurrentFolder->close("dcopadd");
      }

      TQString msgId = msg->msgIdMD5();
      if ( msgId.isEmpty()) {
	msgId = msg->subject();
	if ( msgId.isEmpty() )
          msgId = msg->fromStrip();
        if ( msgId.isEmpty() )
          msgId = msg->toStrip();

	msgId += msg->dateStr();
      }

      int k = mAddMessageMsgIds.findIndex( msgId );
      //fprintf(stderr,"find %s = %d\n",(const char *) msgId,k);

      if ( k == -1 ) {
        if ( !msgId.isEmpty() ) {
          mAddMessageMsgIds.append( msgId );
        }

        if ( !MsgStatusFlags.isEmpty() ) {
          KMMsgStatus status = strToStatus(MsgStatusFlags);
          if (status) msg->setStatus(status);
        }

        int index;
        if ( mAddMsgCurrentFolder->addMsg( msg, &index ) == 0 ) {
          mAddMsgCurrentFolder->unGetMsg( index );
          retval = 1;
        } else {
          retval =- 2;
          delete msg;
          msg = 0;
        }
      } else {
        //qDebug( "duplicate: " + msgId + "; subj: " + msg->subject() + ", from: " + msgId = msg->fromStrip());
	retval = -4;
      }
    } else {
      retval = -1;
    }
  } else {
    retval = -2;
  }
  return retval;
}

void KMKernel::dcopResetAddMessage()
{
  mAddMessageMsgIds.clear();
  mAddMessageLastFolder = TQString();
}

int KMKernel::dcopAddMessage_fastImport( const TQString & foldername,
                                         const TQString & msgUrlString,
                                         const TQString & MsgStatusFlags)
{
  return dcopAddMessage_fastImport(foldername, KURL(msgUrlString), MsgStatusFlags);
}

int KMKernel::dcopAddMessage_fastImport( const TQString & foldername,
                                         const KURL & msgUrl,
                                         const TQString & MsgStatusFlags)
{
  // Use this function to import messages without
  // search for already existing emails.
  kdDebug(5006) << "KMKernel::dcopAddMessage_fastImport called" << endl;

  if ( foldername.isEmpty() || foldername.startsWith("."))
    return -1;

  int retval;
  bool createNewFolder = false;

  TQString _foldername = foldername.stripWhiteSpace();
  _foldername = _foldername.replace('\\',""); //try to prevent ESCAPE Sequences

  if ( foldername != mAddMessageLastFolder ) {
    createNewFolder = true;
    mAddMessageLastFolder = foldername;
  }


  if ( !msgUrl.isEmpty() && msgUrl.isLocalFile() ) {
    const TQCString messageText =
      KPIM::kFileToString( msgUrl.path(), true, false );
    if ( messageText.isEmpty() )
      return -2;

    KMMessage *msg = new KMMessage();
    msg->fromString( messageText );

    if (createNewFolder) {
      if ( foldername.contains("/")) {
        TQString tmp_fname = "";
        KMFolder *folder = NULL;
        KMFolderDir *subfolder;
        bool root = true;

        TQStringList subFList = TQStringList::split("/",_foldername,false);

        for ( TQStringList::Iterator it = subFList.begin(); it != subFList.end(); ++it ) {
          TQString _newFolder = *it;
          if(_newFolder.startsWith(".")) return -1;

          if(root) {
            folder = the_folderMgr->findOrCreate(*it, false);
            if (folder) {
              root = false;
              tmp_fname = "/" + *it;
            }
            else return -1;
          } else {
            subfolder = folder->createChildFolder();
            tmp_fname += "/" + *it;
            if(!the_folderMgr->getFolderByURL( tmp_fname )) {
              folder = the_folderMgr->createFolder(*it, false, folder->folderType(), subfolder);
            }
            if(!(folder = the_folderMgr->getFolderByURL( tmp_fname ))) return -1;
          }
        }

      mAddMsgCurrentFolder = the_folderMgr->getFolderByURL( tmp_fname );
      if(!folder) return -1;

      } else {
        mAddMsgCurrentFolder = the_folderMgr->findOrCreate(_foldername, false);
      }
    }

    if ( mAddMsgCurrentFolder ) {
      int index;

      if( !MsgStatusFlags.isEmpty() ) {
        KMMsgStatus status = strToStatus(MsgStatusFlags);
        if (status) msg->setStatus(status);
      }

      if ( mAddMsgCurrentFolder->addMsg( msg, &index ) == 0 ) {
        mAddMsgCurrentFolder->unGetMsg( index );
        retval = 1;
      } else {
        retval =- 2;
        delete msg;
        msg = 0;
      }
    } else {
      retval = -1;
    }
  } else {
    retval = -2;
  }

  return retval;
}

void KMKernel::showImportArchiveDialog()
{
  KMMainWidget *mainWidget = getKMMainWidget();
  KMail::ImportArchiveDialog *importDialog = new KMail::ImportArchiveDialog( mainWidget, WDestructiveClose );
  importDialog->setFolder( mainWidget->folderTree()->currentFolder() );
  importDialog->show();
}

TQStringList KMKernel::folderList() const
{
  TQStringList folders;
  const TQString localPrefix = "/Local";
  folders << localPrefix;
  the_folderMgr->getFolderURLS( folders, localPrefix );
  the_imapFolderMgr->getFolderURLS( folders );
  the_dimapFolderMgr->getFolderURLS( folders );
  return folders;
}

DCOPRef KMKernel::getFolder( const TQString& vpath )
{
  const TQString localPrefix = "/Local";
  if ( the_folderMgr->getFolderByURL( vpath ) )
    return DCOPRef( new FolderIface( vpath ) );
  else if ( vpath.startsWith( localPrefix ) &&
            the_folderMgr->getFolderByURL( vpath.mid( localPrefix.length() ) ) )
    return DCOPRef( new FolderIface( vpath.mid( localPrefix.length() ) ) );
  else if ( the_imapFolderMgr->getFolderByURL( vpath ) )
    return DCOPRef( new FolderIface( vpath ) );
  else if ( the_dimapFolderMgr->getFolderByURL( vpath ) )
    return DCOPRef( new FolderIface( vpath ) );
  return DCOPRef();
}

void KMKernel::raise()
{
  DCOPRef kmail( "kmail", "kmail" );
  kmail.call( "newInstance" );
}

bool KMKernel::showMail( Q_UINT32 serialNumber, TQString /* messageId */ )
{
  KMMainWidget *mainWidget = 0;
  if (KMainWindow::memberList) {
    KMainWindow *win = 0;
    TQObjectList *l;

    // First look for a KMainWindow.
    for (win = KMainWindow::memberList->first(); win;
         win = KMainWindow::memberList->next()) {
      // Then look for a KMMainWidget.
      l	= win->queryList("KMMainWidget");
      if (l && l->first()) {
	mainWidget = dynamic_cast<KMMainWidget *>(l->first());
	if (win->isActiveWindow())
	  break;
      }
    }
  }

  if (mainWidget) {
    int idx = -1;
    KMFolder *folder = 0;
    KMMsgDict::instance()->getLocation(serialNumber, &folder, &idx);
    if (!folder || (idx == -1))
      return false;
    KMFolderOpener openFolder(folder, "showmail");
    KMMsgBase *msgBase = folder->getMsgBase(idx);
    if (!msgBase)
      return false;
    bool unGet = !msgBase->isMessage();
    KMMessage *msg = folder->getMsg(idx);

    KMReaderMainWin *win = new KMReaderMainWin( false, false );
    KMMessage *newMessage = new KMMessage( *msg );
    newMessage->setParent( msg->parent() );
    newMessage->setMsgSerNum( msg->getMsgSerNum() );
    newMessage->setReadyToShow( true );
    win->showMsg( GlobalSettings::self()->overrideCharacterEncoding(), newMessage );
    win->show();

    if (unGet)
      folder->unGetMsg(idx);
    return true;
  }

  return false;
}

TQString KMKernel::getFrom( Q_UINT32 serialNumber )
{
  int idx = -1;
  KMFolder *folder = 0;
  KMMsgDict::instance()->getLocation(serialNumber, &folder, &idx);
  if (!folder || (idx == -1))
    return TQString::null;
  KMFolderOpener openFolder(folder, "getFrom");
  KMMsgBase *msgBase = folder->getMsgBase(idx);
  if (!msgBase)
    return TQString::null;
  bool unGet = !msgBase->isMessage();
  KMMessage *msg = folder->getMsg(idx);
  TQString result = msg->from();
  if (unGet)
    folder->unGetMsg(idx);
  return result;
}

TQString KMKernel::debugScheduler()
{
  TQString res = KMail::ActionScheduler::debug();
  return res;
}

TQString KMKernel::debugSernum( Q_UINT32 serialNumber )
{
  TQString res;
  if (serialNumber != 0) {
    int idx = -1;
    KMFolder *folder = 0;
    KMMsgBase *msg = 0;
    KMMsgDict::instance()->getLocation( serialNumber, &folder, &idx );
    // It's possible that the message has been deleted or moved into a
    // different folder
    if (folder && (idx != -1)) {
      // everything is ok
      KMFolderOpener openFolder(folder, "debugser");
      msg = folder->getMsgBase( idx );
      if (msg) {
        res.append( TQString( " subject %s,\n sender %s,\n date %s.\n" )
                             .arg( msg->subject() )
                             .arg( msg->fromStrip() )
                             .arg( msg->dateStr() ) );
      } else {
        res.append( TQString( "Invalid serial number." ) );
      }
    } else {
      res.append( TQString( "Invalid serial number." ) );
    }
  }
  return res;
}


void KMKernel::pauseBackgroundJobs()
{
  mBackgroundTasksTimer->stop();
  mJobScheduler->pause();
}

void KMKernel::resumeBackgroundJobs()
{
  mJobScheduler->resume();
  mBackgroundTasksTimer->start( 4 * 60 * 60 * 1000, true );
}

void KMKernel::stopNetworkJobs()
{
  if ( GlobalSettings::self()->networkState() == GlobalSettings::EnumNetworkState::Offline )
    return;

  GlobalSettings::setNetworkState( GlobalSettings::EnumNetworkState::Offline );
  BroadcastStatus::instance()->setStatusMsg( i18n("KMail is set to be offline; all network jobs are suspended"));
  emit onlineStatusChanged( (GlobalSettings::EnumNetworkState::type)GlobalSettings::networkState() );
}

void KMKernel::resumeNetworkJobs()
{
  if ( GlobalSettings::self()->networkState() == GlobalSettings::EnumNetworkState::Online )
    return;

  GlobalSettings::setNetworkState( GlobalSettings::EnumNetworkState::Online );
  BroadcastStatus::instance()->setStatusMsg( i18n("KMail is set to be online; all network jobs resumed"));
  emit onlineStatusChanged( (GlobalSettings::EnumNetworkState::type)GlobalSettings::networkState() );

  if ( kmkernel->msgSender()->sendImmediate() ) {
    kmkernel->msgSender()->sendQueued();
  }
}

bool KMKernel::isOffline()
{
  if ( GlobalSettings::self()->networkState() == GlobalSettings::EnumNetworkState::Offline )
    return true;
  else
    return false;
}

bool KMKernel::askToGoOnline()
{
  // already asking means we are offline and need to wait anyhow
  if ( s_askingToGoOnline ) {
    return false;
  }

  if ( kmkernel->isOffline() ) {
    s_askingToGoOnline = true;
    int rc =
    KMessageBox::questionYesNo( KMKernel::self()->mainWin(),
                                i18n("KMail is currently in offline mode. "
                                     "How do you want to proceed?"),
                                i18n("Online/Offline"),
                                i18n("Work Online"),
                                i18n("Work Offline"));

    s_askingToGoOnline = false;
    if( rc == KMessageBox::No ) {
      return false;
    } else {
      kmkernel->resumeNetworkJobs();
    }
  }
  return true;
}

/********************************************************************/
/*                        Kernel methods                            */
/********************************************************************/

void KMKernel::quit()
{
  // Called when all windows are closed. Will take care of compacting,
  // sending... should handle session management too!!
}
  /* TODO later:
   Asuming that:
     - msgsender is nonblocking
       (our own, TQSocketNotifier based. Pops up errors and sends signal
        senderFinished when done)

   o If we are getting mail, stop it (but dont lose something!)
         [Done already, see mailCheckAborted]
   o If we are sending mail, go on UNLESS this was called by SM,
       in which case stop ASAP that too (can we warn? should we continue
       on next start?)
   o If we are compacting, or expunging, go on UNLESS this was SM call.
       In that case stop compacting ASAP and continue on next start, before
       touching any folders. [Not needed anymore with CompactionJob]

   KMKernel::quit ()
   {
     SM call?
       if compacting, stop;
       if sending, stop;
       if receiving, stop;
       Windows will take care of themselves (composer should dump
        its messages, if any but not in deadMail)
       declare us ready for the End of the Session

     No, normal quit call
       All windows are off. Anything to do, should compact or sender sends?
         Yes, maybe put an icon in panel as a sign of life
         if sender sending, connect us to his finished slot, declare us ready
                            for quit and wait for senderFinished
         if not, Folder manager, go compact sent-mail and outbox
}                (= call slotFinished())

void KMKernel::slotSenderFinished()
{
  good, Folder manager go compact sent-mail and outbox
  clean up stage1 (release folders and config, unregister from dcop)
    -- another kmail may start now ---
  kapp->quit();
}
*/


/********************************************************************/
/*            Init, Exit, and handler  methods                      */
/********************************************************************/
void KMKernel::testDir(const char *_name)
{
  TQString foldersPath = TQDir::homeDirPath() + TQString( _name );
  TQFileInfo info( foldersPath );
  if ( !info.exists() ) {
    if ( ::mkdir( TQFile::encodeName( foldersPath ) , S_IRWXU ) == -1 ) {
      KMessageBox::sorry(0, i18n("KMail could not create folder '%1';\n"
                                 "please make sure that you can view and "
                                 "modify the content of the folder '%2'.")
                            .arg( foldersPath ).arg( TQDir::homeDirPath() ) );
      ::exit(-1);
    }
  }
  if ( !info.isDir() || !info.isReadable() || !info.isWritable() ) {
    KMessageBox::sorry(0, i18n("The permissions of the folder '%1' are "
                               "incorrect;\n"
                               "please make sure that you can view and modify "
                               "the content of this folder.")
                          .arg( foldersPath ) );
    ::exit(-1);
  }
}


//-----------------------------------------------------------------------------
// Open a composer for each message found in the dead.letter folder
void KMKernel::recoverDeadLetters()
{
  TQDir dir( localDataPath() + "autosave/cur" );
  if ( !dir.exists() ) {
    kdWarning(5006) << "Autosave directory " << dir.path() << " not found!" << endl;
    return;
  }

  const TQStringList entryList = dir.entryList( TQDir::Files | TQDir::NoSymLinks, TQDir::Unsorted );
  for ( unsigned int i = 0; i < entryList.size(); i++ ) {
    const TQString fileName = entryList[i];
    TQFile file( dir.path() + '/' + fileName );
    if ( !file.open( IO_ReadOnly ) ) {
      kdWarning(5006) << "Unable to open autosave file " << fileName << endl;
      continue;
    }
    const TQByteArray msgData = file.readAll();
    file.close();

    if ( msgData.isEmpty() ) {
      kdWarning(5006) << "autosave file " << fileName << " is empty!" << endl;
      continue;
    }

    KMMessage *msg = new KMMessage(); // Composer will take ownership
    msg->fromByteArray( msgData );
    KMail::Composer * win = KMail::makeComposer();
    win->setMsg( msg, false, false, true );
    win->setAutoSaveFilename( fileName );
    win->show();
  }
}

//-----------------------------------------------------------------------------
void KMKernel::initFolders(KConfig* cfg)
{
  TQString name;

  name = cfg->readEntry("inboxFolder");

  // Currently the folder manager cannot manage folders which are not
  // in the base folder directory.
  //if (name.isEmpty()) name = getenv("MAIL");

  if (name.isEmpty()) name = I18N_NOOP("inbox");

  the_inboxFolder  = (KMFolder*)the_folderMgr->findOrCreate(name);

  if (the_inboxFolder->canAccess() != 0) {
    emergencyExit( i18n("You do not have read/write permission to your inbox folder.") );
  }

  the_inboxFolder->setSystemFolder(true);
  if ( the_inboxFolder->userWhoField().isEmpty() )
    the_inboxFolder->setUserWhoField( TQString::null );
  // inboxFolder->open();

  the_outboxFolder = the_folderMgr->findOrCreate(cfg->readEntry("outboxFolder", I18N_NOOP("outbox")));
  if (the_outboxFolder->canAccess() != 0) {
    emergencyExit( i18n("You do not have read/write permission to your outbox folder.") );
  }
  the_outboxFolder->setNoChildren(true);

  the_outboxFolder->setSystemFolder(true);
  if ( the_outboxFolder->userWhoField().isEmpty() )
    the_outboxFolder->setUserWhoField( TQString::null );
  /* Nuke the oubox's index file, to make sure that no ghost messages are in
   * it from a previous crash. Ghost messages happen in the outbox because it
   * the only folder where messages enter and leave within 5 seconds, which is
   * the leniency period for index invalidation. Since the number of mails in
   * this folder is expected to be very small, we can live with regenerating
   * the index on each start to be on the save side. */
  //if ( the_outboxFolder->folderType() == KMFolderTypeMaildir )
  //  unlink( TQFile::encodeName( the_outboxFolder->indexLocation() ) );
  the_outboxFolder->open("kmkernel");

  the_sentFolder = the_folderMgr->findOrCreate(cfg->readEntry("sentFolder", I18N_NOOP("sent-mail")));
  if (the_sentFolder->canAccess() != 0) {
    emergencyExit( i18n("You do not have read/write permission to your sent-mail folder.") );
  }
  the_sentFolder->setSystemFolder(true);
  if ( the_sentFolder->userWhoField().isEmpty() )
    the_sentFolder->setUserWhoField( TQString::null );
  // the_sentFolder->open();

  the_trashFolder  = the_folderMgr->findOrCreate(cfg->readEntry("trashFolder", I18N_NOOP("trash")));
  if (the_trashFolder->canAccess() != 0) {
    emergencyExit( i18n("You do not have read/write permission to your trash folder.") );
  }
  the_trashFolder->setSystemFolder( true );
  if ( the_trashFolder->userWhoField().isEmpty() )
    the_trashFolder->setUserWhoField( TQString::null );
  // the_trashFolder->open();

  the_draftsFolder = the_folderMgr->findOrCreate(cfg->readEntry("draftsFolder", I18N_NOOP("drafts")));
  if (the_draftsFolder->canAccess() != 0) {
    emergencyExit( i18n("You do not have read/write permission to your drafts folder.") );
  }
  the_draftsFolder->setSystemFolder( true );
  if ( the_draftsFolder->userWhoField().isEmpty() )
    the_draftsFolder->setUserWhoField( TQString::null );
  the_draftsFolder->open("kmkernel");

  the_templatesFolder =
    the_folderMgr->findOrCreate( cfg->readEntry( "templatesFolder",
                                                 I18N_NOOP("templates") ) );
  if ( the_templatesFolder->canAccess() != 0 ) {
    emergencyExit( i18n("You do not have read/write permission to your templates folder.") );
  }
  the_templatesFolder->setSystemFolder( true );
  if ( the_templatesFolder->userWhoField().isEmpty() )
    the_templatesFolder->setUserWhoField( TQString::null );
  the_templatesFolder->open("kmkernel");
}


void KMKernel::init()
{
  the_shuttingDown = false;
  the_server_is_ready = false;

  KConfig* cfg = KMKernel::config();

  TQDir dir;

  KConfigGroupSaver saver(cfg, "General");
  the_firstStart = cfg->readBoolEntry("first-start", true);
  cfg->writeEntry("first-start", false);
  the_previousVersion = cfg->readEntry("previous-version");
  cfg->writeEntry("previous-version", KMAIL_VERSION);
  TQString foldersPath = cfg->readPathEntry( "folders" );
  kdDebug(5006) << k_funcinfo << "foldersPath (from config): '" << foldersPath << "'" << endl;

  if ( foldersPath.isEmpty() ) {
    foldersPath = localDataPath() + "mail";
    if ( transferMail( foldersPath ) ) {
      cfg->writePathEntry( "folders", foldersPath );
    }
    kdDebug(5006) << k_funcinfo << "foldersPath (after transferMail): '" << foldersPath << "'" << endl;
  }

  // moved up here because KMMessage::stripOffPrefixes is used below
  KMMessage::readConfig();

  the_undoStack     = new UndoStack(20);
  the_folderMgr     = new KMFolderMgr(foldersPath);
  the_imapFolderMgr = new KMFolderMgr( KMFolderImap::cacheLocation(), KMImapDir);
  the_dimapFolderMgr = new KMFolderMgr( KMFolderCachedImap::cacheLocation(), KMDImapDir);
  recreateCorruptIndexFiles();

  the_searchFolderMgr = new KMFolderMgr(locateLocal("data","kmail/search"), KMSearchDir);
  KMFolder *lsf = the_searchFolderMgr->find( i18n("Last Search") );
  if (lsf)
    the_searchFolderMgr->remove( lsf );

  the_acctMgr       = new AccountManager();
  the_filterMgr     = new KMFilterMgr();
  the_popFilterMgr     = new KMFilterMgr(true);
  the_filterActionDict = new KMFilterActionDict;

  initFolders(cfg);
  the_acctMgr->readConfig();
  the_filterMgr->readConfig();
  the_popFilterMgr->readConfig();
  cleanupImapFolders();

  the_msgSender = new KMSender;
  the_server_is_ready = true;
  imProxy()->initialize();
  { // area for config group "Composer"
    KConfigGroupSaver saver(cfg, "Composer");
    if (cfg->readListEntry("pref-charsets").isEmpty())
    {
      cfg->writeEntry("pref-charsets", "us-ascii,iso-8859-1,locale,utf-8");
    }
  }
  readConfig();
  mICalIface->readConfig();
  // filterMgr->dump();
#ifdef HAVE_INDEXLIB
  the_msgIndex = new KMMsgIndex(this); //create the indexer
#else
  the_msgIndex = 0;
#endif

//#if 0
  the_weaver =  new KPIM::ThreadWeaver::Weaver( this );
  the_weaverLogger = new KPIM::ThreadWeaver::WeaverThreadLogger(this);
  the_weaverLogger->attach (the_weaver);
//#endif

  connect( the_folderMgr, TQT_SIGNAL( folderRemoved(KMFolder*) ),
           this, TQT_SIGNAL( folderRemoved(KMFolder*) ) );
  connect( the_dimapFolderMgr, TQT_SIGNAL( folderRemoved(KMFolder*) ),
           this, TQT_SIGNAL( folderRemoved(KMFolder*) ) );
  connect( the_imapFolderMgr, TQT_SIGNAL( folderRemoved(KMFolder*) ),
           this, TQT_SIGNAL( folderRemoved(KMFolder*) ) );
  connect( the_searchFolderMgr, TQT_SIGNAL( folderRemoved(KMFolder*) ),
           this, TQT_SIGNAL( folderRemoved(KMFolder*) ) );

  mBackgroundTasksTimer = new TQTimer( this, "mBackgroundTasksTimer" );
  connect( mBackgroundTasksTimer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( slotRunBackgroundTasks() ) );
#ifdef DEBUG_SCHEDULER // for debugging, see jobscheduler.h
  mBackgroundTasksTimer->start( 10000, true ); // 10s, singleshot
#else
  mBackgroundTasksTimer->start( 5 * 60000, true ); // 5 minutes, singleshot
#endif

  TQTextCodec *codec;
  for ( int i = 0; ( codec = TQTextCodec::codecForIndex ( i ) ); i++ ) {
    const TQString asciiString( "azAZ19,.-#+!?=()&" );
    const TQCString encodedString = codec->fromUnicode( asciiString );
    if ( TQString::fromAscii( encodedString ) != asciiString ) {
      mNonAsciiCompatibleCodecs.append( codec );
    }
  }
}

bool KMKernel::isCodecAsciiCompatible( const TQTextCodec *codec )
{
  return !mNonAsciiCompatibleCodecs.contains( codec );
}

void KMKernel::readConfig()
{
  //Needed here, since this function is also called when the configuration
  //changes, and the static variables should be updated then - IOF
  KMMessage::readConfig();
}

void KMKernel::cleanupImapFolders()
{
  KMAccount *acct = 0;
  KMFolderNode *node = the_imapFolderMgr->dir().first();
  while (node)
  {
    if (node->isDir() || ((acct = the_acctMgr->find(node->id()))
			  && ( acct->type() == "imap" )) )
    {
      node = the_imapFolderMgr->dir().next();
    } else {
      KMFolder* folder = static_cast<KMFolder*>(node);
      // delete only local
      static_cast<KMFolderImap*>( folder->storage() )->setAlreadyRemoved( true );
      the_imapFolderMgr->remove(folder);
      node = the_imapFolderMgr->dir().first();
    }
  }

  node = the_dimapFolderMgr->dir().first();
  while (node)
  {
    if (node->isDir() || ((acct = the_acctMgr->find(node->id()))
			  && ( acct->type() == "cachedimap" )) )
    {
      node = the_dimapFolderMgr->dir().next();
    } else {
      the_dimapFolderMgr->remove(static_cast<KMFolder*>(node));
      node = the_dimapFolderMgr->dir().first();
    }
  }

  the_imapFolderMgr->quiet(true);
  for (acct = the_acctMgr->first(); acct; acct = the_acctMgr->next())
  {
    KMFolderImap *fld;
    KMAcctImap *imapAcct;

    if (acct->type() != "imap") continue;
    fld = static_cast<KMFolderImap*>(the_imapFolderMgr
      ->findOrCreate(TQString::number(acct->id()), false, acct->id())->storage());
    fld->setNoContent(true);
    fld->folder()->setLabel(acct->name());
    imapAcct = static_cast<KMAcctImap*>(acct);
    fld->setAccount(imapAcct);
    imapAcct->setImapFolder(fld);
    fld->close( "kernel", true );
  }
  the_imapFolderMgr->quiet(false);

  the_dimapFolderMgr->quiet( true );
  for (acct = the_acctMgr->first(); acct; acct = the_acctMgr->next())
  {
    KMFolderCachedImap *cfld = 0;
    KMAcctCachedImap *cachedImapAcct;

    if (acct->type() != "cachedimap" ) continue;

    KMFolder* fld = the_dimapFolderMgr->find(TQString::number(acct->id()));
    if( fld )
      cfld = static_cast<KMFolderCachedImap*>( fld->storage() );
    if (cfld == 0) {
      // Folder doesn't exist yet
      cfld = static_cast<KMFolderCachedImap*>(the_dimapFolderMgr->createFolder(TQString::number(acct->id()),
            false, KMFolderTypeCachedImap)->storage());
      if (!cfld) {
        KMessageBox::error(0,(i18n("Cannot create file `%1' in %2.\nKMail cannot start without it.").arg(acct->name()).arg(the_dimapFolderMgr->basePath())));
        exit(-1);
      }
      cfld->folder()->setId( acct->id() );
    }

    cfld->setNoContent(true);
    cfld->folder()->setLabel(acct->name());
    cachedImapAcct = static_cast<KMAcctCachedImap*>(acct);
    cfld->setAccount(cachedImapAcct);
    cachedImapAcct->setImapFolder(cfld);
    cfld->close("kmkernel");
  }
  the_dimapFolderMgr->quiet( false );
}

void KMKernel::recreateCorruptIndexFiles()
{
  TQValueList<TQGuardedPtr<KMFolder> > folders;
  TQValueList<KMFolderIndex*> foldersWithBrokenIndex;
  TQStringList strList;
  the_folderMgr->createFolderList( &strList, &folders );
  the_imapFolderMgr->createFolderList( &strList, &folders );
  the_dimapFolderMgr->createFolderList( &strList, &folders );
  for ( int i = 0; folders.at(i) != folders.end(); i++ ) {
    KMFolder * const folder = *folders.at(i);
    if ( !folder || folder->isDir() || folder->isOpened() )
      continue;
    KMFolderIndex * const index = dynamic_cast<KMFolderIndex*>( folder->storage() );
    if ( index && index->indexStatus() != KMFolderIndex::IndexOk ) {
      foldersWithBrokenIndex.append( index );
    }
  }

  if ( !foldersWithBrokenIndex.isEmpty() ) {
    TQStringList folderNames;
    for ( uint i = 0; i < foldersWithBrokenIndex.size(); i++ ) {
      folderNames << foldersWithBrokenIndex[i]->label();
    }

    KMessageBox::informationList( 0, i18n( "There is a problem with the mail index of the following "
                                           "folders, the indices will now be regenerated.\n"
                                           "This can happen because the index files are out of date, missing or corrupted.\n"
                                           "Contact your administrator if this happens frequently.\n"
                                           "Some information, like status flags, might get lost." ),
                                  folderNames, i18n( "Problem with mail indices" ) );

    for ( uint i = 0; i < foldersWithBrokenIndex.size(); i++ ) {
      foldersWithBrokenIndex[i]->silentlyRecreateIndex();
    }
  }
}

bool KMKernel::doSessionManagement()
{

  // Do session management
  if (kapp->isRestored()){
    int n = 1;
    while (KMMainWin::canBeRestored(n)){
      //only restore main windows! (Matthias);
      if (KMMainWin::classNameOfToplevel(n) == "KMMainWin")
        (new KMMainWin)->restore(n);
      n++;
    }
    return true; // we were restored by SM
  }
  return false;  // no, we were not restored
}

void KMKernel::closeAllKMailWindows()
{
  if (!KMainWindow::memberList) return;
  TQPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
  KMainWindow *window = 0;
  while ((window = it.current()) != 0) {
    ++it;
    if (window->isA("KMMainWindow") ||
	window->inherits("KMail::SecondaryWindow"))
      window->close( true ); // close and delete the window
  }
}

void KMKernel::cleanup(void)
{
  dumpDeadLetters();
  the_shuttingDown = true;
  closeAllKMailWindows();

  delete the_acctMgr;
  the_acctMgr = 0;
  delete the_filterMgr;
  the_filterMgr = 0;
  delete the_msgSender;
  the_msgSender = 0;
  delete the_filterActionDict;
  the_filterActionDict = 0;
  delete the_undoStack;
  the_undoStack = 0;
  delete the_popFilterMgr;
  the_popFilterMgr = 0;

#if 0
  delete the_weaver;
  the_weaver = 0;
#endif

  KConfig* config =  KMKernel::config();
  KConfigGroupSaver saver(config, "General");

  if (the_trashFolder) {

    the_trashFolder->close("kmkernel", true);

    if (config->readBoolEntry("empty-trash-on-exit", true))
    {
      if ( the_trashFolder->count( true ) > 0 )
        the_trashFolder->expunge();
    }
  }

  mICalIface->cleanup();

  TQValueList<TQGuardedPtr<KMFolder> > folders;
  TQStringList strList;
  KMFolder *folder;
  the_folderMgr->createFolderList(&strList, &folders);
  for (int i = 0; folders.at(i) != folders.end(); i++)
  {
    folder = *folders.at(i);
    if (!folder || folder->isDir()) continue;
    folder->close("kmkernel", true);
  }
  strList.clear();
  folders.clear();
  the_searchFolderMgr->createFolderList(&strList, &folders);
  for (int i = 0; folders.at(i) != folders.end(); i++)
  {
    folder = *folders.at(i);
    if (!folder || folder->isDir()) continue;
    folder->close("kmkernel", true);
  }

  delete the_msgIndex;
  the_msgIndex = 0;
  delete the_folderMgr;
  the_folderMgr = 0;
  delete the_imapFolderMgr;
  the_imapFolderMgr = 0;
  delete the_dimapFolderMgr;
  the_dimapFolderMgr = 0;
  delete the_searchFolderMgr;
  the_searchFolderMgr = 0;
  delete mConfigureDialog;
  mConfigureDialog = 0;
  // do not delete, because mWin may point to an existing window
  // delete mWin;
  mWin = 0;

  if ( RecentAddresses::exists() )
    RecentAddresses::self( config )->save( config );
  config->sync();
}

bool KMKernel::transferMail( TQString & destinationDir )
{
  TQString dir;

  // check whether the user has a ~/KMail folder
  TQFileInfo fi( TQDir::home(), "KMail" );
  if ( fi.exists() && fi.isDir() ) {
    dir = TQDir::homeDirPath() + "/KMail";
    // the following two lines can be removed once moving mail is reactivated
    destinationDir = dir;
    return true;
  }

  if ( dir.isEmpty() ) {
    // check whether the user has a ~/Mail folder
    fi.setFile( TQDir::home(), "Mail" );
    if ( fi.exists() && fi.isDir() &&
         TQFile::exists( TQDir::homeDirPath() + "/Mail/.inbox.index" ) ) {
      // there's a ~/Mail folder which seems to be used by KMail (because of the
      // index file)
      dir = TQDir::homeDirPath() + "/Mail";
      // the following two lines can be removed once moving mail is reactivated
      destinationDir = dir;
      return true;
    }
  }

  if ( dir.isEmpty() ) {
    return true; // there's no old mail folder
  }

#if 0
  // disabled for now since moving fails in certain cases (e.g. if symbolic links are involved)
  const TQString kmailName = kapp->aboutData()->programName();
  TQString msg;
  if ( KIO::NetAccess::exists( destinationDir, true, 0 ) ) {
    // if destinationDir exists, we need to warn about possible
    // overwriting of files. otherwise, we don't have to
    msg = i18n( "%1-%3 is the application name, %4-%7 are folder path",
                "<qt>The <i>%4</i> folder exists. "
                "%1 now uses the <i>%5</i> folder for "
                "its messages.<p>"
                "%2 can move the contents of <i>%6<i> into this folder for "
                "you, though this may replace any existing files with "
                "the same name in <i>%7</i>.<p>"
                "<strong>Would you like %3 to move the mail "
                "files now?</strong></qt>" )
          .arg( kmailName, kmailName, kmailName )
          .arg( dir, destinationDir, dir, destinationDir );
  } else {
    msg = i18n( "%1-%3 is the application name, %4-%6 are folder path",
                "<qt>The <i>%4</i> folder exists. "
                "%1 now uses the <i>%5</i> folder for "
                "its messages. %2 can move the contents of <i>%6</i> into "
                "this folder for you.<p>"
                "<strong>Would you like %3 to move the mail "
                "files now?</strong></qt>" )
          .arg( kmailName, kmailName, kmailName )
          .arg( dir, destinationDir, dir );
  }
  TQString title = i18n( "Migrate Mail Files?" );
  TQString buttonText = i18n( "Move" );

  if ( KMessageBox::questionYesNo( 0, msg, title, buttonText, i18n("Do Not Move") ) ==
       KMessageBox::No ) {
    destinationDir = dir;
    return true;
  }

  if ( !KIO::NetAccess::move( dir, destinationDir ) ) {
    kdDebug(5006) << k_funcinfo << "Moving " << dir << " to " << destinationDir << " failed: " << KIO::NetAccess::lastErrorString() << endl;
    kdDebug(5006) << k_funcinfo << "Deleting " << destinationDir << endl;
    KIO::NetAccess::del( destinationDir, 0 );
    destinationDir = dir;
    return false;
  }
#endif

  return true;
}


void KMKernel::ungrabPtrKb(void)
{
  if(!KMainWindow::memberList) return;
  TQWidget* widg = KMainWindow::memberList->first();
  Display* dpy;

  if (!widg) return;
  dpy = widg->x11Display();
  XUngrabKeyboard(dpy, CurrentTime);
  XUngrabPointer(dpy, CurrentTime);
}


// Message handler
void KMKernel::kmailMsgHandler(QtMsgType aType, const char* aMsg)
{
  static int recurse=-1;

  recurse++;

  switch (aType)
  {
  case QtDebugMsg:
  case QtWarningMsg:
    kdDebug(5006) << aMsg << endl;
    break;

  case QtFatalMsg: // Hm, what about using kdFatal() here?
    ungrabPtrKb();
    kdDebug(5006) << kapp->caption() << " fatal error "
		  << aMsg << endl;
    KMessageBox::error(0, aMsg);
    abort();
  }

  recurse--;
}


void KMKernel::dumpDeadLetters()
{
  if ( shuttingDown() )
    return; //All documents should be saved before shutting down is set!

  // make all composer windows autosave their contents
  if ( !KMainWindow::memberList )
    return;

  for ( TQPtrListIterator<KMainWindow> it(*KMainWindow::memberList) ; it.current() != 0; ++it ) {
    if ( KMail::Composer * win = ::qt_cast<KMail::Composer*>( it.current() ) ) {
      win->autoSaveMessage();
      // saving the message has to be finished right here, we are called from a dtor,
      // therefore we have no chance to finish this later
      // yes, this is ugly and potentially dangerous, but the alternative is losing
      // currently composed messages...
      while ( win->isComposing() )
        qApp->processEvents();
    }
  }
}



void KMKernel::action(bool mailto, bool check, const TQString &to,
                      const TQString &cc, const TQString &bcc,
                      const TQString &subj, const TQString &body,
                      const KURL &messageFile,
                      const KURL::List &attachURLs,
                      const QCStringList &customHeaders)
{
  if ( mailto )
    openComposer( to, cc, bcc, subj, body, 0, messageFile, attachURLs, customHeaders );
  else
    openReader( check );

  if ( check )
    checkMail();
  //Anything else?
}

void KMKernel::byteArrayToRemoteFile(const TQByteArray &aData, const KURL &aURL,
  bool overwrite)
{
  // ## when KDE 3.3 is out: use KIO::storedPut to remove slotDataReq altogether
  KIO::Job *job = KIO::put(aURL, -1, overwrite, false);
  putData pd; pd.url = aURL; pd.data = aData; pd.offset = 0;
  mPutJobs.insert(job, pd);
  connect(job, TQT_SIGNAL(dataReq(KIO::Job*,TQByteArray&)),
    TQT_SLOT(slotDataReq(KIO::Job*,TQByteArray&)));
  connect(job, TQT_SIGNAL(result(KIO::Job*)),
    TQT_SLOT(slotResult(KIO::Job*)));
}

void KMKernel::slotDataReq(KIO::Job *job, TQByteArray &data)
{
  // send the data in 64 KB chunks
  const int MAX_CHUNK_SIZE = 64*1024;
  TQMap<KIO::Job*, putData>::Iterator it = mPutJobs.find(job);
  assert(it != mPutJobs.end());
  int remainingBytes = (*it).data.size() - (*it).offset;
  if( remainingBytes > MAX_CHUNK_SIZE )
  {
    // send MAX_CHUNK_SIZE bytes to the receiver (deep copy)
    data.duplicate( (*it).data.data() + (*it).offset, MAX_CHUNK_SIZE );
    (*it).offset += MAX_CHUNK_SIZE;
    //kdDebug( 5006 ) << "Sending " << MAX_CHUNK_SIZE << " bytes ("
    //                << remainingBytes - MAX_CHUNK_SIZE << " bytes remain)\n";
  }
  else
  {
    // send the remaining bytes to the receiver (deep copy)
    data.duplicate( (*it).data.data() + (*it).offset, remainingBytes );
    (*it).data = TQByteArray();
    (*it).offset = 0;
    //kdDebug( 5006 ) << "Sending " << remainingBytes << " bytes\n";
  }
}

void KMKernel::slotResult(KIO::Job *job)
{
  TQMap<KIO::Job*, putData>::Iterator it = mPutJobs.find(job);
  assert(it != mPutJobs.end());
  if (job->error())
  {
    if (job->error() == KIO::ERR_FILE_ALREADY_EXIST)
    {
      if (KMessageBox::warningContinueCancel(0,
        i18n("File %1 exists.\nDo you want to replace it?")
        .arg((*it).url.prettyURL()), i18n("Save to File"), i18n("&Replace"))
        == KMessageBox::Continue)
        byteArrayToRemoteFile((*it).data, (*it).url, true);
    }
    else job->showErrorDialog();
  }
  mPutJobs.remove(it);
}

void KMKernel::slotRequestConfigSync() {
  // ### FIXME: delay as promised in the kdoc of this function ;-)
  KMKernel::config()->sync();
}

void KMKernel::slotShowConfigurationDialog()
{
  if( !mConfigureDialog ) {
    mConfigureDialog = new ConfigureDialog( 0, "configure", false );
    connect( mConfigureDialog, TQT_SIGNAL( configChanged() ),
             this, TQT_SLOT( slotConfigChanged() ) );
  }

  if( KMKernel::getKMMainWidget() == 0 )
  {
    // ensure that there is a main widget available
    // as parts of the configure dialog (identity) rely on this
    // and this slot can be called when there is only a KMComposeWin showing
    KMMainWin * win = new KMMainWin;
    win->show();
  }
  if( mConfigureDialog->isHidden() )
  {
    getKMMainWidget()->headers()->writeConfig();
    mConfigureDialog->show();
  }
  else
    mConfigureDialog->raise();
}

void KMKernel::slotConfigChanged()
{
  readConfig();
  emit configChanged();
}

//-------------------------------------------------------------------------------
//static
TQString KMKernel::localDataPath()
{
  return locateLocal( "data", "kmail/" );
}

//-------------------------------------------------------------------------------

bool KMKernel::haveSystemTrayApplet()
{
  return !systemTrayApplets.isEmpty();
}

bool KMKernel::registerSystemTrayApplet( const KSystemTray* applet )
{
  if ( systemTrayApplets.findIndex( applet ) == -1 ) {
    systemTrayApplets.append( applet );
    return true;
  }
  else
    return false;
}

bool KMKernel::unregisterSystemTrayApplet( const KSystemTray* applet )
{
  TQValueList<const KSystemTray*>::iterator it =
    systemTrayApplets.find( applet );
  if ( it != systemTrayApplets.end() ) {
    systemTrayApplets.remove( it );
    return true;
  }
  else
    return false;
}

void KMKernel::emergencyExit( const TQString& reason )
{
  TQString mesg;
  if ( reason.length() == 0 ) {
    mesg = i18n("KMail encountered a fatal error and will terminate now");
  } else {
    mesg = i18n("KMail encountered a fatal error and will "
                      "terminate now.\nThe error was:\n%1").arg( reason );
  }

  kdWarning() << mesg << endl;
  KNotifyClient::userEvent( 0, "<qt>"+mesg+"</qt>", KNotifyClient::Messagebox, KNotifyClient::Error );

  ::exit(1);
}

/**
 * Returns true if the folder is either the outbox or one of the drafts-folders
 */
bool KMKernel::folderIsDraftOrOutbox(const KMFolder * folder)
{
  assert( folder );
  if ( folder == the_outboxFolder )
    return true;
  return folderIsDrafts( folder );
}

bool KMKernel::folderIsDrafts(const KMFolder * folder)
{
  assert( folder );
  if ( folder == the_draftsFolder )
    return true;

  TQString idString = folder->idString();
  if ( idString.isEmpty() )
    return false;

  // search the identities if the folder matches the drafts-folder
  const KPIM::IdentityManager *im = identityManager();
  for ( KPIM::IdentityManager::ConstIterator it=im->begin(); it != im->end(); ++it )
    if ( (*it).drafts() == idString )
      return true;
  return false;
}

bool KMKernel::folderIsTemplates( const KMFolder *folder )
{
  assert( folder );
  if ( folder == the_templatesFolder )
    return true;

  TQString idString = folder->idString();
  if ( idString.isEmpty() )
    return false;

  // search the identities if the folder matches the templates-folder
  const KPIM::IdentityManager *im = identityManager();
  for ( KPIM::IdentityManager::ConstIterator it=im->begin(); it != im->end(); ++it )
    if ( (*it).templates() == idString )
      return true;
  return false;
}

bool KMKernel::folderIsTrash(KMFolder * folder)
{
  assert(folder);
  if (folder == the_trashFolder) return true;
  TQStringList actList = acctMgr()->getAccounts();
  TQStringList::Iterator it( actList.begin() );
  for( ; it != actList.end() ; ++it ) {
    KMAccount* act = acctMgr()->findByName( *it );
    if ( act && ( act->trash() == folder->idString() ) )
      return true;
  }
  return false;
}

bool KMKernel::folderIsSentMailFolder( const KMFolder * folder )
{
  assert( folder );
  if ( folder == the_sentFolder )
    return true;

  TQString idString = folder->idString();
  if ( idString.isEmpty() ) return false;

  // search the identities if the folder matches the sent-folder
  const KPIM::IdentityManager * im = identityManager();
  for( KPIM::IdentityManager::ConstIterator it = im->begin(); it != im->end(); ++it )
    if ( (*it).fcc() == idString ) return true;
  return false;
}

KPIM::IdentityManager * KMKernel::identityManager() {
  if ( !mIdentityManager ) {
    kdDebug(5006) << "instantating KPIM::IdentityManager" << endl;
    mIdentityManager = new KPIM::IdentityManager( false, this, "mIdentityManager" );
  }
  return mIdentityManager;
}

KMMsgIndex *KMKernel::msgIndex()
{
    return the_msgIndex;
}

KMainWindow* KMKernel::mainWin()
{
  if (KMainWindow::memberList) {
    KMainWindow *kmWin = 0;

    // First look for a KMMainWin.
    for (kmWin = KMainWindow::memberList->first(); kmWin;
         kmWin = KMainWindow::memberList->next())
      if (kmWin->isA("KMMainWin"))
        return kmWin;

    // There is no KMMainWin. Use any other KMainWindow instead (e.g. in
    // case we are running inside Kontact) because we anyway only need
    // it for modal message boxes and for KNotify events.
    kmWin = KMainWindow::memberList->first();
    if ( kmWin )
      return kmWin;
  }

  // There's not a single KMainWindow. Create a KMMainWin.
  // This could happen if we want to pop up an error message
  // while we are still doing the startup wizard and no other
  // KMainWindow is running.
  mWin = new KMMainWin;
  return mWin;
}


/**
 * Empties all trash folders
 */
void KMKernel::slotEmptyTrash()
{
  TQString title = i18n("Empty Trash");
  TQString text = i18n("Are you sure you want to empty the trash folders of all accounts?");
  if (KMessageBox::warningContinueCancel(0, text, title,
                                         KStdGuiItem::cont(), "confirm_empty_trash")
      != KMessageBox::Continue)
  {
    return;
  }

  for (KMAccount* acct = acctMgr()->first(); acct; acct = acctMgr()->next())
  {
    KMFolder* trash = findFolderById(acct->trash());
    if (trash)
    {
      trash->expunge();
    }
  }
}

KConfig* KMKernel::config()
{
  assert(mySelf);
  if (!mySelf->mConfig)
  {
    mySelf->mConfig = KSharedConfig::openConfig( "kmailrc" );
    // Check that all updates have been run on the config file:
    KMail::checkConfigUpdates();
  }
  return mySelf->mConfig;
}

KMailICalIfaceImpl& KMKernel::iCalIface()
{
  assert( mICalIface );
  return *mICalIface;
}

void KMKernel::selectFolder( TQString folderPath )
{
  kdDebug(5006)<<"Selecting a folder "<<folderPath<<endl;
  const TQString localPrefix = "/Local";
  KMFolder *folder = kmkernel->folderMgr()->getFolderByURL( folderPath );
  if ( !folder && folderPath.startsWith( localPrefix ) )
    folder = the_folderMgr->getFolderByURL( folderPath.mid( localPrefix.length() ) );
  if ( !folder )
    folder = kmkernel->imapFolderMgr()->getFolderByURL( folderPath );
  if ( !folder )
    folder = kmkernel->dimapFolderMgr()->getFolderByURL( folderPath );
  Q_ASSERT( folder );

  KMMainWidget *widget = getKMMainWidget();
  Q_ASSERT( widget );
  if ( !widget )
    return;

  KMFolderTree *tree = widget->folderTree();
  tree->doFolderSelected( tree->indexOfFolder( folder ) );
  tree->ensureItemVisible( tree->indexOfFolder( folder ) );
}

KMMainWidget *KMKernel::getKMMainWidget()
{
  //This could definitely use a speadup
  TQWidgetList *l = kapp->topLevelWidgets();
  TQWidgetListIt it( *l );
  TQWidget *wid;

  while ( ( wid = it.current() ) != 0 ) {
    ++it;
    TQObjectList *l2 = wid->topLevelWidget()->queryList( "KMMainWidget" );
    if (l2 && l2->first()) {
      KMMainWidget* kmmw = dynamic_cast<KMMainWidget *>( l2->first() );
      Q_ASSERT( kmmw );
      delete l2;
      delete l;
      return kmmw;
    }
    delete l2;
  }
  delete l;
  return 0;
}

void KMKernel::slotRunBackgroundTasks() // called regularly by timer
{
  // Hidden KConfig keys. Not meant to be used, but a nice fallback in case
  // a stable kmail release goes out with a nasty bug in CompactionJob...
  KConfigGroup generalGroup( config(), "General" );

  if ( generalGroup.readBoolEntry( "auto-expiring", true ) ) {
    the_folderMgr->expireAllFolders( false /*scheduled, not immediate*/ );
    the_imapFolderMgr->expireAllFolders( false /*scheduled, not immediate*/ );
    the_dimapFolderMgr->expireAllFolders( false /*scheduled, not immediate*/ );
    // the_searchFolderMgr: no expiry there
  }

  if ( generalGroup.readBoolEntry( "auto-compaction", true ) ) {
    the_folderMgr->compactAllFolders( false /*scheduled, not immediate*/ );
    // the_imapFolderMgr: no compaction
    the_dimapFolderMgr->compactAllFolders( false /*scheduled, not immediate*/ );
    // the_searchFolderMgr: no compaction
  }

#ifdef DEBUG_SCHEDULER // for debugging, see jobscheduler.h
  mBackgroundTasksTimer->start( 60 * 1000, true ); // check again in 1 minute
#else
  mBackgroundTasksTimer->start( 4 * 60 * 60 * 1000, true ); // check again in 4 hours
#endif

}

void KMKernel::expireAllFoldersNow() // called by the GUI
{
  the_folderMgr->expireAllFolders( true /*immediate*/ );
  the_imapFolderMgr->expireAllFolders( true /*immediate*/ );
  the_dimapFolderMgr->expireAllFolders( true /*immediate*/ );
}

void KMKernel::compactAllFolders() // called by the GUI
{
  the_folderMgr->compactAllFolders( true /*immediate*/ );
  //the_imapFolderMgr->compactAllFolders( true /*immediate*/ );
  the_dimapFolderMgr->compactAllFolders( true /*immediate*/ );
}

KMFolder* KMKernel::findFolderById( const TQString& idString )
{
  KMFolder * folder = the_folderMgr->findIdString( idString );
  if ( !folder )
    folder = the_imapFolderMgr->findIdString( idString );
  if ( !folder )
    folder = the_dimapFolderMgr->findIdString( idString );
  if ( !folder )
    folder = the_searchFolderMgr->findIdString( idString );
  return folder;
}

::KIMProxy* KMKernel::imProxy()
{
  return KIMProxy::instance( kapp->dcopClient() );
}

void KMKernel::enableMailCheck()
{
  mMailCheckAborted = false;
}

bool KMKernel::mailCheckAborted() const
{
  return mMailCheckAborted;
}

void KMKernel::abortMailCheck()
{
  mMailCheckAborted = true;
}

bool KMKernel::canQueryClose()
{
  if ( KMMainWidget::mainWidgetList() &&
       KMMainWidget::mainWidgetList()->count() > 1 )
    return true;
  KMMainWidget *widget = getKMMainWidget();
  if ( !widget )
    return true;
  KMSystemTray* systray = widget->systray();
  if ( !systray || GlobalSettings::closeDespiteSystemTray() )
      return true;
  if ( systray->mode() == GlobalSettings::EnumSystemTrayPolicy::ShowAlways ) {
    systray->hideKMail();
    return false;
  } else if ( ( systray->mode() == GlobalSettings::EnumSystemTrayPolicy::ShowOnUnread ) && ( systray->hasUnreadMail() )) {
    systray->show();
    systray->hideKMail();
    return false;
  }
  return true;
}

void KMKernel::messageCountChanged()
{
  mTimeOfLastMessageCountChange = ::time( 0 );
}

int KMKernel::timeOfLastMessageCountChange() const
{
  return mTimeOfLastMessageCountChange;
}

Wallet *KMKernel::wallet() {
  static bool walletOpenFailed = false;
  if ( mWallet && mWallet->isOpen() )
    return mWallet;

  if ( !Wallet::isEnabled() || walletOpenFailed )
    return 0;

  // find an appropriate parent window for the wallet dialog
  WId window = 0;
  if ( qApp->activeWindow() )
    window = qApp->activeWindow()->winId();
  else if ( getKMMainWidget() )
    window = getKMMainWidget()->topLevelWidget()->winId();

  delete mWallet;
  mWallet = Wallet::openWallet( Wallet::NetworkWallet(), window );

  if ( !mWallet ) {
    walletOpenFailed = true;
    return 0;
  }

  if ( !mWallet->hasFolder( "kmail" ) )
    mWallet->createFolder( "kmail" );
  mWallet->setFolder( "kmail" );
  return mWallet;
}

TQValueList< TQGuardedPtr<KMFolder> > KMKernel::allFolders()
{
  TQStringList names;
  TQValueList<TQGuardedPtr<KMFolder> > folders;
  folderMgr()->createFolderList(&names, &folders);
  imapFolderMgr()->createFolderList(&names, &folders);
  dimapFolderMgr()->createFolderList(&names, &folders);
  searchFolderMgr()->createFolderList(&names, &folders);

  return folders;
}

KMFolder *KMKernel::currentFolder() {
  KMMainWidget *widget = getKMMainWidget();
  KMFolder *folder = 0;
  if ( widget && widget->folderTree() ) {
    folder = widget->folderTree()->currentFolder();
  }
  return folder;
}

// can't be inline, since KMSender isn't known to implement
// KMail::MessageSender outside this .cpp file
KMail::MessageSender * KMKernel::msgSender() { return the_msgSender; }

#include "kmkernel.moc"
