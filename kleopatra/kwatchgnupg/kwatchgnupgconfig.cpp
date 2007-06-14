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

#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
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

KWatchGnuPGConfig::KWatchGnuPGConfig( QWidget* parent )
  : KDialog( parent )
{
  setCaption( i18n("Configure KWatchGnuPG") );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );

  // tmp vars:
  QGridLayout * glay;
  QGroupBox * group;

  QWidget * top = new QWidget;
  setMainWidget( top ); 

  QVBoxLayout * vlay = new QVBoxLayout;
  vlay->setSpacing( spacingHint() );
  vlay->setMargin( 0 );
  top->setLayout(vlay);
  
  group = new QGroupBox( i18n("WatchGnuPG"));
  vlay->addWidget(group);
  //group->layout()->setSpacing( spacingHint() );

  glay = new QGridLayout;
  glay->setSpacing( spacingHint() );
  glay->setMargin( 0 );
  glay->setColumnStretch( 1, 1 );
  group->setLayout(glay);
  int row = -1;

  ++row;
  mExeED = new KUrlRequester;
  QLabel *label = new QLabel( i18n("&Executable:"));
  label->setBuddy( mExeED );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mExeED, row, 1 );
  connect( mExeED, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()) );

  ++row;
  mSocketED = new KUrlRequester;
  label = new QLabel( i18n("&Socket:"));
  label->setBuddy( mSocketED );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mSocketED, row, 1 );
  connect( mSocketED, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()) );

  ++row;
  mLogLevelCB = new QComboBox;
  mLogLevelCB->setEditable( false );
  mLogLevelCB->addItem( i18n("None") );
  mLogLevelCB->addItem( i18n("Basic") );
  mLogLevelCB->addItem( i18n("Advanced") );
  mLogLevelCB->addItem( i18n("Expert") );
  mLogLevelCB->addItem( i18n("Guru") );
  label = new QLabel( i18n("Default &log level:"));
  label->setBuddy( mLogLevelCB );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mLogLevelCB, row, 1 );
  connect( mLogLevelCB, SIGNAL(activated(int)), SLOT(slotChanged()) );

  /******************* Log Window group *******************/
  group = new QGroupBox( i18n("Log Window"));
  //group->layout()->setSpacing( spacingHint() );
  vlay->addWidget(group);

  glay = new QGridLayout;
  glay->setSpacing( spacingHint() );
  glay->setMargin( 0 );
  glay->setColumnStretch( 1, 1 );
  group->setLayout(glay);
  row = -1;

  ++row;
  mLoglenSB = new QSpinBox;
  mLoglenSB->setRange( 0, 1000000 );
  mLoglenSB->setSingleStep( 100 );
  mLoglenSB->setSuffix( i18nc("history size spinbox suffix"," lines") );
  mLoglenSB->setSpecialValueText( i18n("unlimited") );
  label = new QLabel( i18n("&History size:"));
  label->setBuddy( mLoglenSB );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mLoglenSB, row, 1 );
  QPushButton * button = new QPushButton( i18n("Set &Unlimited"));
  glay->addWidget( button, row, 2 );

  connect( mLoglenSB, SIGNAL(valueChanged(int)), SLOT(slotChanged()) );
  connect( button, SIGNAL(clicked()), SLOT(slotSetHistorySizeUnlimited()) );

  ++row;
  mWordWrapCB = new QCheckBox( i18n("Enable &word wrapping"));
  mWordWrapCB->hide(); // QTextEdit doesn't support word wrapping in LogText mode
  glay->addWidget( mWordWrapCB, row, 0, 1, 3 );

  connect( mWordWrapCB, SIGNAL(clicked()), SLOT(slotChanged()) );

  vlay->addStretch( 1 );

  connect( this, SIGNAL(okClicked()), SLOT(slotSave()) );
}

void KWatchGnuPGConfig::slotSetHistorySizeUnlimited() {
  mLoglenSB->setValue( 0 );
}

void KWatchGnuPGConfig::loadConfig()
{
  KConfigGroup config(KGlobal::config(), "WatchGnuPG");
  mExeED->setUrl( config.readEntry( "Executable", "watchgnupg" ) );
  mSocketED->setUrl( config.readEntry( "Socket", QDir::home().canonicalPath()
										+ "/.gnupg/log-socket") );
  mLogLevelCB->setCurrentIndex( log_level_to_int( config.readEntry( "LogLevel", "basic" ) ) );

  config.changeGroup("LogWindow");
  mLoglenSB->setValue( config.readEntry( "MaxLogLen", 10000 ) );
  mWordWrapCB->setChecked( config.readEntry("WordWrap", false ) );

  config.changeGroup( QString() );
  enableButtonOk( false );
  enableButtonApply( false );
}

void KWatchGnuPGConfig::saveConfig()
{
  KConfigGroup config(KGlobal::config(), "WatchGnuPG");
  config.writeEntry( "Executable", mExeED->url().url() );
  config.writeEntry( "Socket", mSocketED->url().path() );
  config.writeEntry( "LogLevel", log_levels[mLogLevelCB->currentIndex()] );

  config.changeGroup("LogWindow");
  config.writeEntry( "MaxLogLen", mLoglenSB->value() );
  config.writeEntry( "WordWrap", mWordWrapCB->isChecked() );

  config.changeGroup( QString() );
  config.sync();
  enableButtonOk( false );
  enableButtonApply( false );
}

void KWatchGnuPGConfig::slotChanged()
{
  enableButtonOk( true );
  enableButtonApply( true );
}

void KWatchGnuPGConfig::slotSave()
{
  saveConfig();
  emit reconfigure();
}

#include "kwatchgnupgconfig.moc"
