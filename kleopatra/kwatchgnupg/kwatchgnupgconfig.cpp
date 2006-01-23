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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include <q3frame.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <kglobal.h>

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
  : KDialogBase( Plain, i18n("Configure KWatchGnuPG"),
		 Ok|Cancel, Ok, parent, name )
{
  // tmp vars:
  QWidget * w;
  QGridLayout * glay;
  Q3GroupBox * group;

  QWidget * top = plainPage();

  QVBoxLayout * vlay = new QVBoxLayout( top, 0, spacingHint() );

  group = new Q3GroupBox( i18n("WatchGnuPG"), top );
  group->layout()->setSpacing( spacingHint() );

  w = new QWidget( group );

  glay = new QGridLayout( w, 3, 2, 0, spacingHint() );
  glay->setColStretch( 1, 1 );

  int row = -1;

  ++row;
  mExeED = new KUrlRequester( w );
  glay->addWidget( new QLabel( mExeED, i18n("&Executable:"), w ), row, 0 );
  glay->addWidget( mExeED, row, 1 );
  connect( mExeED, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()) );

  ++row;
  mSocketED = new KUrlRequester( w );
  glay->addWidget( new QLabel( mSocketED, i18n("&Socket:"), w ), row, 0 );
  glay->addWidget( mSocketED, row, 1 );
  connect( mSocketED, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()) );

  ++row;
  mLogLevelCB = new QComboBox( false, w );
  mLogLevelCB->insertItem( i18n("None") );
  mLogLevelCB->insertItem( i18n("Basic") );
  mLogLevelCB->insertItem( i18n("Advanced") );
  mLogLevelCB->insertItem( i18n("Expert") );
  mLogLevelCB->insertItem( i18n("Guru") );
  glay->addWidget( new QLabel( mLogLevelCB, i18n("Default &log level:"), w ), row, 0 );
  glay->addWidget( mLogLevelCB, row, 1 );
  connect( mLogLevelCB, SIGNAL(activated(int)), SLOT(slotChanged()) );

  vlay->addWidget( group );

  /******************* Log Window group *******************/
  group = new Q3GroupBox( i18n("Log Window"), top );
  group->layout()->setSpacing( spacingHint() );

  w = new QWidget( group );

  glay = new QGridLayout( w, 2, 3, 0, spacingHint() );
  glay->setColStretch( 1, 1 );

  row = -1;

  ++row;
  mLoglenSB = new QSpinBox( 0, 1000000, 100, w );
  mLoglenSB->setSuffix( i18n("history size spinbox suffix"," lines") );
  mLoglenSB->setSpecialValueText( i18n("unlimited") );
  glay->addWidget( new QLabel( mLoglenSB, i18n("&History size:"), w ), row, 0 );
  glay->addWidget( mLoglenSB, row, 1 );
  QPushButton * button = new QPushButton( i18n("Set &Unlimited"), w );
  glay->addWidget( button, row, 2 );

  connect( mLoglenSB, SIGNAL(valueChanged(int)), SLOT(slotChanged()) );
  connect( button, SIGNAL(clicked()), SLOT(slotSetHistorySizeUnlimited()) );

  ++row;
  mWordWrapCB = new QCheckBox( i18n("Enable &word wrapping"), w );
  mWordWrapCB->hide(); // QTextEdit doesn't support word wrapping in LogText mode
  glay->addMultiCellWidget( mWordWrapCB, row, row, 0, 2 );

  connect( mWordWrapCB, SIGNAL(clicked()), SLOT(slotChanged()) );

  vlay->addWidget( group );
  vlay->addStretch( 1 );

  connect( this, SIGNAL(applyClicked()), SLOT(slotSave()) );
  connect( this, SIGNAL(okClicked()), SLOT(slotSave()) );
}

void KWatchGnuPGConfig::slotSetHistorySizeUnlimited() {
  mLoglenSB->setValue( 0 );
}

void KWatchGnuPGConfig::loadConfig()
{
  KConfig* config = KGlobal::config();
  config->setGroup("WatchGnuPG");
  mExeED->setURL( config->readEntry( "Executable", "watchgnupg" ) );
  mSocketED->setURL( config->readEntry( "Socket", QDir::home().canonicalPath()
										+ "/.gnupg/log-socket") );
  mLogLevelCB->setCurrentItem( log_level_to_int( config->readEntry( "LogLevel", "basic" ) ) );

  config->setGroup("LogWindow");
  mLoglenSB->setValue( config->readEntry( "MaxLogLen", 10000 ) );
  mWordWrapCB->setChecked( config->readEntry("WordWrap", false ) );

  config->setGroup( QString() );
  enableButtonOK( false );
  enableButtonApply( false );
}

void KWatchGnuPGConfig::saveConfig()
{
  KConfig* config = KGlobal::config();
  config->setGroup("WatchGnuPG");
  config->writeEntry( "Executable", mExeED->url() );
  config->writeEntry( "Socket", mSocketED->url() );
  config->writeEntry( "LogLevel", log_levels[mLogLevelCB->currentItem()] );

  config->setGroup("LogWindow");
  config->writeEntry( "MaxLogLen", mLoglenSB->value() );
  config->writeEntry( "WordWrap", mWordWrapCB->isChecked() );

  config->setGroup( QString() );
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
