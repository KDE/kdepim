
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kmmainwin.h"
#include "kmmainwidget.h"
#include "kstatusbar.h"
#include "messagesender.h"
#include "progressdialog.h"
#include "statusbarprogresswidget.h"
#include "accountwizard.h"
#include "broadcaststatus.h"
#include "accountmanager.h"
#include "kmtransport.h"

#include <kapplication.h>
#include <klocale.h>
#include <kedittoolbar.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstringhandler.h>
#include <kdebug.h>
#include <ktip.h>

#include "kmmainwin.moc"

KMMainWin::KMMainWin(TQWidget *)
    : KMainWindow( 0, "kmail-mainwindow#" ),
      mReallyClose( false )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  kapp->ref();

  (void) new KAction( i18n("New &Window"), "window_new", 0,
		      this, TQT_SLOT(slotNewMailReader()),
		      actionCollection(), "new_mail_client" );

  mKMMainWidget = new KMMainWidget( this, "KMMainWidget", this, actionCollection() );
  mKMMainWidget->resize( 450, 600 );
  setCentralWidget(mKMMainWidget);
  setupStatusBar();
  if (kmkernel->xmlGuiInstance())
    setInstance( kmkernel->xmlGuiInstance() );

  if ( kmkernel->firstInstance() )
    TQTimer::singleShot( 200, this, TQT_SLOT(slotShowTipOnStart()) );

  setStandardToolBarMenuEnabled(true);

  KStdAction::configureToolbars(this, TQT_SLOT(slotEditToolbars()),
				actionCollection());

  KStdAction::keyBindings(mKMMainWidget, TQT_SLOT(slotEditKeys()),
                          actionCollection());

  KStdAction::quit( this, TQT_SLOT(slotQuit()), actionCollection());
  createGUI( "kmmainwin.rc", false );
  // Don't use conserveMemory() because this renders dynamic plugging
  // of actions unusable!

  mKMMainWidget->setupForwardingActionsList();

  applyMainWindowSettings(KMKernel::config(), "Main Window");

  connect( KPIM::BroadcastStatus::instance(), TQT_SIGNAL( statusMsg( const TQString& ) ),
           this, TQT_SLOT( displayStatusMsg(const TQString&) ) );

  connect(kmkernel, TQT_SIGNAL(configChanged()),
    this, TQT_SLOT(slotConfigChanged()));

  connect(mKMMainWidget, TQT_SIGNAL(captionChangeRequest(const TQString&)),
	  TQT_SLOT(setCaption(const TQString&)) );

  // Enable mail checks again (see destructor)
  kmkernel->enableMailCheck();

  if ( kmkernel->firstStart() )
    AccountWizard::start( kmkernel, this );
}

KMMainWin::~KMMainWin()
{
  saveMainWindowSettings(KMKernel::config(), "Main Window");
  KMKernel::config()->sync();
  kapp->deref();

  if ( !kmkernel->haveSystemTrayApplet() ) {
    // Check if this was the last KMMainWin
    int not_withdrawn = 0;
    TQPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
    for (it.toFirst(); it.current(); ++it){
      if ( !it.current()->isHidden() &&
           it.current()->isTopLevel() &&
           it.current() != this &&
           ::qt_cast<KMMainWin *>( it.current() )
        )
        not_withdrawn++;
    }

    if ( not_withdrawn == 0 ) {
      kdDebug(5006) << "Closing last KMMainWin: stopping mail check" << endl;
      // Running KIO jobs prevent kapp from exiting, so we need to kill them
      // if they are only about checking mail (not important stuff like moving messages)
      kmkernel->abortMailCheck();
      kmkernel->acctMgr()->cancelMailCheck();
    }
  }
}

void KMMainWin::displayStatusMsg(const TQString& aText)
{
  if ( !statusBar() || !mLittleProgress) return;
  int statusWidth = statusBar()->width() - mLittleProgress->width()
                    - fontMetrics().maxWidth();
  TQString text = KStringHandler::rPixelSqueeze( " " + aText, fontMetrics(),
                                                statusWidth );

  // ### FIXME: We should disable richtext/HTML (to avoid possible denial of service attacks),
  // but this code would double the size of the satus bar if the user hovers
  // over an <foo@bar.com>-style email address :-(
//  text.replace("&", "&amp;");
//  text.replace("<", "&lt;");
//  text.replace(">", "&gt;");

  statusBar()->changeItem(text, mMessageStatusId);
}

//-----------------------------------------------------------------------------
void KMMainWin::slotNewMailReader()
{
  KMMainWin *d;

  d = new KMMainWin();
  d->show();
  d->resize(d->size());
}


void KMMainWin::slotEditToolbars()
{
  saveMainWindowSettings(KMKernel::config(), "Main Window");
  KEditToolbar dlg(actionCollection(), "kmmainwin.rc");

  connect( &dlg, TQT_SIGNAL(newToolbarConfig()),
	   TQT_SLOT(slotUpdateToolbars()) );

  dlg.exec();
}

void KMMainWin::slotUpdateToolbars()
{
  // remove dynamically created actions before editing
  mKMMainWidget->clearFilterActions();

  createGUI("kmmainwin.rc", false);
  applyMainWindowSettings(KMKernel::config(), "Main Window");

  // plug dynamically created actions again
  mKMMainWidget->initializeFilterActions();
}

void KMMainWin::setupStatusBar()
{
  mMessageStatusId = 1;

  /* Create a progress dialog and hide it. */
  mProgressDialog = new KPIM::ProgressDialog( statusBar(), this );
  mProgressDialog->hide();

  mLittleProgress = new StatusbarProgressWidget( mProgressDialog, statusBar() );
  mLittleProgress->show();

  statusBar()->addWidget( mLittleProgress, 0 , true );
  statusBar()->insertItem(i18n(" Initializing..."), 1, 4 );
  statusBar()->setItemAlignment( 1, AlignLeft | AlignVCenter );
  statusBar()->addWidget( mKMMainWidget->vacationScriptIndicator(), 1 );
  mLittleProgress->show();
}

/** Read configuration options after widgets are created. */
void KMMainWin::readConfig(void)
{
}

/** Write configuration options. */
void KMMainWin::writeConfig(void)
{
  mKMMainWidget->writeConfig();
}

void KMMainWin::slotQuit()
{
  mReallyClose = true;
  close();
}

void KMMainWin::slotConfigChanged()
{
  readConfig();
}

//-----------------------------------------------------------------------------
bool KMMainWin::queryClose()
{
  if ( kapp->sessionSaving() )
    writeConfig();

  if ( kmkernel->shuttingDown() || kapp->sessionSaving() || mReallyClose )
    return true;
  return kmkernel->canQueryClose();
}

void KMMainWin::slotShowTipOnStart()
{
  KTipDialog::showTip( this );
}


