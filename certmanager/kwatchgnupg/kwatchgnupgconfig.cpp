/*
    kwatchgnupgconfig.cpp

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
#include <qcombobox.h>
#include <qdir.h>

static const char* log_levels[] = { "none", "basic", "advanced", "expert", "guru" };

static int log_level_to_int( const QString& loglevel )
{
  if( loglevel == "none" ) {
	return 0;
  } else if( loglevel == "basic" ) {
	return 1;
  } else if( loglevel == "advanced" ) {
	return 2;
  } else if( loglevel == "expert" ) {
	return 3;
  } else if( loglevel == "guru" ) {
	return 4;
  } else {
	// default
	return 1;
  }
}

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
  QLabel *exeLA = new QLabel( i18n("KWatchGnuPG &executable:"), page );
  hbl->addWidget( exeLA );
  mExeED = new  KURLRequester( page );
  hbl->addWidget( mExeED );
  connect( mExeED, SIGNAL(textChanged( const QString& )), this, SLOT(slotChanged()) );
  exeLA->setBuddy( mExeED );

  hbl = new QHBoxLayout( topLayout );
  QLabel *socketLA = new QLabel( i18n("KWatchGnuPG &socket:"), page );
  hbl->addWidget( socketLA );
  mSocketED = new  KURLRequester( page );
  hbl->addWidget( mSocketED );
  connect( mSocketED, SIGNAL(textChanged( const QString& )), this, SLOT(slotChanged()) );
  socketLA->setBuddy( mSocketED );

  hbl = new QHBoxLayout( topLayout );
  QLabel* logLevelLA = new QLabel( i18n("Log level:"), page );
  hbl->addWidget( logLevelLA );
  mLogLevelCB = new QComboBox( page );
  hbl->addWidget( mLogLevelCB );
  mLogLevelCB->insertItem( i18n("None") );
  mLogLevelCB->insertItem( i18n("Basic") );
  mLogLevelCB->insertItem( i18n("Advanced") );
  mLogLevelCB->insertItem( i18n("Expert") );
  mLogLevelCB->insertItem( i18n("Guru") );
  connect( mLogLevelCB, SIGNAL( activated(int) ),
		   this, SLOT( slotChanged() ) );
  /******************* Log Window page *******************/
  page = addPage( i18n("Log Window") );
  topLayout = new QVBoxLayout( page, 0, KDialog::spacingHint() );
  hbl = new QHBoxLayout( topLayout );

  QLabel* loglenLA = new QLabel(i18n("&Maximum number of lines in log (zero is infinite):"),
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
  mLogLevelCB->setCurrentItem( log_level_to_int( config->readEntry( "LogLevel", "basic" ) ) );

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
  config->writeEntry( "LogLevel", log_levels[mLogLevelCB->currentItem()] );

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
