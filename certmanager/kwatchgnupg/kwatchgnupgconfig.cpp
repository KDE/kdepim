#include "kwatchgnupgconfig.h"

#include <klocale.h>
#include <kurlrequester.h>
#include <kconfig.h>
#include <kapplication.h>

#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qdir.h>

KWatchGnuPGConfig::KWatchGnuPGConfig( QWidget* parent, const char* name )
  : KDialogBase( KDialogBase::Tabbed, i18n("Configure KWatchGnuPG"),
				 KDialogBase::Apply|KDialogBase::Ok|KDialogBase::Cancel,
				 KDialogBase::Ok,				 
				 parent, name )
{
  /******************* WatchGnuPG page *******************/
  QFrame* page = addPage( i18n("WatchGnuPg") );
  QVBoxLayout* topLayout = new QVBoxLayout( page, 0, KDialog::spacingHint() );

  QHBoxLayout* hbl = new QHBoxLayout( topLayout );
  QLabel *exeLA = new QLabel( i18n("KWatchGnuPG &executable"), page );
  hbl->addWidget( exeLA );
  mExeED = new  KURLRequester( page );
  hbl->addWidget( mExeED );
  connect( mExeED, SIGNAL(textChanged( const QString& )), this, SLOT(slotChanged()) );
  exeLA->setBuddy( mExeED );

  hbl = new QHBoxLayout( topLayout );
  QLabel *socketLA = new QLabel( i18n("KWatchGnuPG &socket"), page );
  hbl->addWidget( socketLA );
  mSocketED = new  KURLRequester( page );
  hbl->addWidget( mSocketED );
  connect( mSocketED, SIGNAL(textChanged( const QString& )), this, SLOT(slotChanged()) );
  socketLA->setBuddy( mSocketED );

  hbl = new QHBoxLayout( topLayout );
  QLabel* logLevelLA = new QLabel( i18n("Log Level"), page );
  hbl->addWidget( logLevelLA );

  /******************* Log Window page *******************/
  page = addPage( i18n("Log Window") );
  topLayout = new QVBoxLayout( page, 0, KDialog::spacingHint() );
  hbl = new QHBoxLayout( topLayout );
  
  QLabel* loglenLA = new QLabel(i18n("&Maximum number of lines in log (zero is infinite)"),
								page );
  hbl->addWidget( loglenLA );
  mLoglenSB = new QSpinBox( 0, 100000, 1, page );
  hbl->addWidget( mLoglenSB );
  loglenLA->setBuddy( mLoglenSB );
  connect( mLoglenSB, SIGNAL( valueChanged(int) ),
		   this, SLOT( slotChanged() ) );

  mWordWrapCB = new QCheckBox( i18n("&Enabled word wrapping"), page );
  connect( mWordWrapCB, SIGNAL( clicked() ),
		   this, SLOT( slotChanged() ) );
  topLayout->addWidget( mWordWrapCB );
  
  connect( this, SIGNAL( applyClicked() ),
		   this, SLOT( slotSave() ) );
  connect( this, SIGNAL( okClicked() ),
		   this, SLOT( slotSave() ) );
}

void KWatchGnuPGConfig::loadConfig()
{
  KConfig* config = kapp->config();
  config->setGroup("WatchGnuPG");
  mExeED->setURL( config->readEntry( "Executable", "watchgnupg" ) );
  mSocketED->setURL( config->readEntry( "Socket", QDir::home().canonicalPath() 
										+ "/.gnupg/log-socket") );

  config->setGroup("LogWindow");
  mLoglenSB->setValue( config->readNumEntry( "MaxLogLen", 10000 ) );
  mWordWrapCB->setChecked( config->readBoolEntry("WordWrap", false ) );

  config->setGroup( QString::null );
  enableButtonOK( false );
  enableButtonApply( false );
}

void KWatchGnuPGConfig::saveConfig()
{
  KConfig* config = kapp->config();
  config->setGroup("WatchGnuPG");
  config->writeEntry( "Executable", mExeED->url() );
  config->writeEntry( "Socket", mSocketED->url() );

  config->setGroup("LogWindow");
  config->writeEntry( "MaxLogLen", mLoglenSB->value() );
  config->writeEntry( "WordWrap", mWordWrapCB->isChecked() );

  config->setGroup( QString::null );
  config->sync();
  enableButtonOK( false );
  enableButtonApply( false );
}

void KWatchGnuPGConfig::slotChanged()
{
  enableButtonOK( true );
  enableButtonApply( true );
}

void KWatchGnuPGConfig::slotSave()
{
  saveConfig();
  emit reconfigure();
}

#include "kwatchgnupgconfig.moc"
