/*
    main.cpp

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
#include "tray.h"

#include <cryptplugfactory.h>
#include <kleo/cryptoconfig.h>

#include <kprocess.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>

#include <qtextedit.h>
#include <qdir.h>

#define WATCHGNUPGBINARY "watchgnupg"
#define WATCHGNUPGSOCKET ( QDir::home().canonicalPath() + "/.gnupg/log-socket")

KWatchGnuPGMainWindow::KWatchGnuPGMainWindow( QWidget* parent, const char* name )
  : KMainWindow( parent, name, WType_TopLevel )
{
  createGUI();

  mCentralWidget = new QTextEdit( this, "central log view" );
  mCentralWidget->setTextFormat( QTextEdit::LogText );
  mCentralWidget->setWordWrap( QTextEdit::NoWrap );
  mCentralWidget->setMaxLogLines( 10000 ); // PENDING(steffen): make configurable
  setCentralWidget( mCentralWidget );
  
  mWatcher = new KProcess(this);
  connect( mWatcher, SIGNAL( processExited(KProcess*) ),
		   this, SLOT( slotWatcherExited(KProcess*) ) );
  connect( mWatcher, SIGNAL( receivedStdout(KProcess*,char*,int) ),
		   this, SLOT( slotReceivedStdout(KProcess*,char*,int) ) );
  
  startWatcher();
  setGnuPGConfig();

  mSysTray = new KWatchGnuPGTray( this );
  mSysTray->show();
  connect( mSysTray, SIGNAL( quitSelected() ),
		   this, SLOT( slotQuit() ) );
  setAutoSaveSettings();
}

void KWatchGnuPGMainWindow::startWatcher()
{
  mWatcher->clearArguments();
  *mWatcher << WATCHGNUPGBINARY << "--force" << WATCHGNUPGSOCKET;
  if( !mWatcher->start( KProcess::NotifyOnExit, KProcess::AllOutput ) ) {
	kdWarning() << "Cant start " << WATCHGNUPGBINARY << endl;
  }
}

void KWatchGnuPGMainWindow::setGnuPGConfig()
{
  QStringList logclients;
  // Get config object
  Kleo::CryptoConfig* config = Kleo::CryptPlugFactory::instance()->config();
  Q_ASSERT( config );
  QStringList comps = config->componentList();
  for( QStringList::const_iterator it = comps.begin(); it != comps.end(); ++it ) {
	Kleo::CryptoConfigComponent* comp = config->component( *it );
	Q_ASSERT(comp);
	// Look for log-file entry in Debug group
	Kleo::CryptoConfigGroup* group = comp->group("Debug");
	if( group ) {
	  Kleo::CryptoConfigEntry* entry = group->entry("log-file");
	  if( entry ) {
		entry->setStringValue( QString("socket://")+WATCHGNUPGSOCKET );
		logclients << QString("%1 (%2)").arg(*it).arg(comp->description());
	  }
	}
  }
  config->sync(true);
  if( logclients.isEmpty() ) {
	KMessageBox::sorry( this, i18n("There are no components available that support logging" ) );
  }
}

void KWatchGnuPGMainWindow::slotWatcherExited( KProcess* /*proc*/ )
{
  kdDebug() << "KWatchGnuPGMainWindow::slotWatcherExited()" << endl;
}

void KWatchGnuPGMainWindow::slotReceivedStdout( KProcess* /*proc*/, char* buf, int buflen )
{
  mCentralWidget->append( QString::fromUtf8( buf, buflen ) );
  if( !isVisible() ) {
	// Change tray icon to show something happened
	// PENDING(steffen) 
	mSysTray->setAttention(true);
  }
}

void KWatchGnuPGMainWindow::show()
{
  mSysTray->setAttention(false);
  KMainWindow::show();
}

void KWatchGnuPGMainWindow::slotQuit()
{
  mWatcher->kill();
  kapp->quit();
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
