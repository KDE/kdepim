/*
    kwatchgnupgmainwin.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "kwatchgnupgmainwin.h"
#include "kwatchgnupgconfig.h"
#include "tray.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptoconfig.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kprocio.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>

#include <qtextedit.h>
#include <qdir.h>
#include <qeventloop.h>
#include <qtimer.h>
#include <qtextcodec.h>

#define WATCHGNUPGBINARY "watchgnupg"
#define WATCHGNUPGSOCKET ( QDir::home().canonicalPath() + "/.gnupg/log-socket")

KWatchGnuPGMainWindow::KWatchGnuPGMainWindow( QWidget* parent, const char* name )
  : KMainWindow( parent, name, WType_TopLevel ), mConfig(0)
{
  createActions();
  createGUI();

  mCentralWidget = new QTextEdit( this, "central log view" );
  mCentralWidget->setTextFormat( QTextEdit::LogText );
  setCentralWidget( mCentralWidget );

  mWatcher = new KProcIO( QTextCodec::codecForMib( 106 /*utf-8*/ ) );
  connect( mWatcher, SIGNAL( processExited(KProcess*) ),
		   this, SLOT( slotWatcherExited() ) );
  connect( mWatcher, SIGNAL( readReady(KProcIO*) ),
		   this, SLOT( slotReadStdout() ) );

  slotReadConfig();
  mSysTray = new KWatchGnuPGTray( this );
  mSysTray->show();
  connect( mSysTray, SIGNAL( quitSelected() ),
		   this, SLOT( slotQuit() ) );
  setAutoSaveSettings();
}

KWatchGnuPGMainWindow::~KWatchGnuPGMainWindow()
{
  delete mWatcher;
}

void KWatchGnuPGMainWindow::slotClear()
{
  mCentralWidget->clear();
  mCentralWidget->append( tr("[%1] Log cleared").arg( QDateTime::currentDateTime().toString(Qt::ISODate) ) );
}

void KWatchGnuPGMainWindow::createActions()
{
  (void)new KAction( i18n("C&lear History"), "history_clear", CTRL+Key_L,
		     this, SLOT( slotClear() ),
		     actionCollection(), "clear_log" );
  (void)KStdAction::saveAs( this, SLOT(slotSaveAs()), actionCollection() );
  (void)KStdAction::close( this, SLOT(close()), actionCollection() );
  (void)KStdAction::quit( this, SLOT(slotQuit()), actionCollection() );
  (void)KStdAction::preferences( this, SLOT(slotConfigure()), actionCollection() );
  ( void )KStdAction::keyBindings(this, SLOT(configureShortcuts()), actionCollection());
  ( void )KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());

#if 0
  (void)new KAction( i18n("Configure KWatchGnuPG..."), QString::fromLatin1("configure"),
					 0, this, SLOT( slotConfigure() ),
					 actionCollection(), "configure" );
#endif

}

void KWatchGnuPGMainWindow::configureShortcuts()
{
  KKeyDialog::configure( actionCollection(), this );
}

void KWatchGnuPGMainWindow::slotConfigureToolbars()
{
    KEditToolbar dlg( factory() );

    dlg.exec();
}

void KWatchGnuPGMainWindow::startWatcher()
{
  disconnect( mWatcher, SIGNAL( processExited(KProcess*) ),
			  this, SLOT( slotWatcherExited() ) );
  if( mWatcher->isRunning() ) {
	mWatcher->kill();
	while( mWatcher->isRunning() ) {
	  kapp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
	}
	mCentralWidget->append(tr("[%1] Log stopped")
						   .arg( QDateTime::currentDateTime().toString(Qt::ISODate)));
  }
  mWatcher->clearArguments();
  KConfig* config = kapp->config();
  config->setGroup("WatchGnuPG");
  *mWatcher << config->readEntry("Executable", WATCHGNUPGBINARY);
  *mWatcher << "--force";
  *mWatcher << config->readEntry("Socket", WATCHGNUPGSOCKET);
  config->setGroup(QString::null);
  if( !mWatcher->start() ) {
	KMessageBox::sorry( this, i18n("The watchgnupg logging process could not be started.\nPlease install watchgnupg somewhere in your $PATH.\nThis log window is now completely useless." ) );
  } else {
	mCentralWidget->append( tr("[%1] Log started")
							.arg( QDateTime::currentDateTime().toString(Qt::ISODate) ) );
  }
  connect( mWatcher, SIGNAL( processExited(KProcess*) ),
		   this, SLOT( slotWatcherExited() ) );
}

void KWatchGnuPGMainWindow::setGnuPGConfig()
{
  QStringList logclients;
  // Get config object
  Kleo::CryptoConfig* cconfig = Kleo::CryptoBackendFactory::instance()->config();
  if ( !cconfig )
    return;
  //Q_ASSERT( cconfig );
  KConfig* config = kapp->config();
  config->setGroup("WatchGnuPG");
  QStringList comps = cconfig->componentList();
  for( QStringList::const_iterator it = comps.begin(); it != comps.end(); ++it ) {
	Kleo::CryptoConfigComponent* comp = cconfig->component( *it );
	Q_ASSERT(comp);
	// Look for log-file entry in Debug group
	Kleo::CryptoConfigGroup* group = comp->group("Debug");
	if( group ) {
	  Kleo::CryptoConfigEntry* entry = group->entry("log-file");
	  if( entry ) {
		entry->setStringValue( QString("socket://")+
							   config->readEntry("Socket",
												 WATCHGNUPGSOCKET ));
		logclients << QString("%1 (%2)").arg(*it).arg(comp->description());
	  }
	  entry = group->entry("debug-level");
	  if( entry ) {
		entry->setStringValue( config->readEntry("LogLevel", "basic") );
	  }
	}
  }
  cconfig->sync(true);
  if( logclients.isEmpty() ) {
	KMessageBox::sorry( 0, i18n("There are no components available that support logging." ) );
  }
}

void KWatchGnuPGMainWindow::slotWatcherExited()
{
  if( KMessageBox::questionYesNo( this, i18n("The watchgnupg logging process died.\nDo you want to try to restart it?") ) == KMessageBox::Yes ) {
	mCentralWidget->append( i18n("====== Restarting logging process =====") );
	startWatcher();
  } else {
	KMessageBox::sorry( this, i18n("The watchgnupg logging process is not running.\nThis log window is now completely useless." ) );
  }
}

void KWatchGnuPGMainWindow::slotReadStdout()
{
  if ( !mWatcher )
    return;
  QString str;
  while( mWatcher->readln(str,false) > 0 ) {
	mCentralWidget->append( str );
	if( !isVisible() ) {
	  // Change tray icon to show something happened
	  // PENDING(steffen)
	  mSysTray->setAttention(true);
	}
  }
  QTimer::singleShot( 0, this, SLOT(slotAckRead()) );
}

void KWatchGnuPGMainWindow::slotAckRead() {
  if ( mWatcher )
    mWatcher->ackRead();
}

void KWatchGnuPGMainWindow::show()
{
  mSysTray->setAttention(false);
  KMainWindow::show();
}

void KWatchGnuPGMainWindow::slotSaveAs()
{
  QString filename = KFileDialog::getSaveFileName( QString::null, QString::null,
												   this, i18n("Save Log to File") );
  if( filename.isEmpty() ) return;
  QFile file(filename);
  if( file.exists() ) {
	if( KMessageBox::Yes !=
		KMessageBox::warningYesNo( this, i18n("The file named \"%1\" already "
											  "exists. Are you sure you want "
											  "to overwrite it?").arg(filename),
								   i18n("Overwrite File") ) ) {
	  return;
	}
  }
  if( file.open( IO_WriteOnly ) ) {
	QTextStream st(&file);
	st << mCentralWidget->text();
	file.close();
  }
}

void KWatchGnuPGMainWindow::slotQuit()
{
  disconnect( mWatcher, SIGNAL( processExited(KProcess*) ),
			  this, SLOT( slotWatcherExited() ) );
  mWatcher->kill();
  kapp->quit();
}

void KWatchGnuPGMainWindow::slotConfigure()
{
  if( !mConfig ) {
	mConfig = new KWatchGnuPGConfig( this, "config dialog" );
	connect( mConfig, SIGNAL( reconfigure() ),
			 this, SLOT( slotReadConfig() ) );
  }
  mConfig->loadConfig();
  mConfig->exec();
}

void KWatchGnuPGMainWindow::slotReadConfig()
{
  KConfig* config = kapp->config();
  config->setGroup("LogWindow");
  mCentralWidget->setWordWrap( config->readBoolEntry("WordWrap", false)
							   ?QTextEdit::WidgetWidth
							   :QTextEdit::NoWrap );
  mCentralWidget->setMaxLogLines( config->readNumEntry( "MaxLogLen", 10000 ) );
  setGnuPGConfig();
  startWatcher();
}

bool KWatchGnuPGMainWindow::queryClose()
{
  if ( !kapp->sessionSaving() ) {
    hide();
    return false;
  }
  return KMainWindow::queryClose();
}

#include "kwatchgnupgmainwin.moc"
