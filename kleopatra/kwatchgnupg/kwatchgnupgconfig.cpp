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

#include <config-kleopatra.h>

#include "kwatchgnupgconfig.h"
#include "kwatchgnupg.h"

#include "libkleo/ui/filenamerequester.h"

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <knuminput.h>

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>

static const char* log_levels[] = { "none", "basic", "advanced", "expert", "guru" };

static int log_level_to_int( const QString& loglevel )
{
  if( loglevel == QLatin1String("none") ) {
      return 0;
  } else if( loglevel == QLatin1String("basic") ) {
      return 1;
  } else if( loglevel == QLatin1String("advanced") ) {
      return 2;
  } else if( loglevel == QLatin1String("expert") ) {
      return 3;
  } else if( loglevel == QLatin1String("guru") ) {
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

  QVBoxLayout * vlay = new QVBoxLayout( top );
  vlay->setSpacing( spacingHint() );
  vlay->setMargin( 0 );
  
  group = new QGroupBox( i18n("WatchGnuPG"), top );
  vlay->addWidget(group);

  glay = new QGridLayout( group );
  glay->setSpacing( spacingHint() );
  glay->setColumnStretch( 1, 1 );

  int row = -1;

  ++row;
  mExeED = new Kleo::FileNameRequester( group );
  QLabel *label = new QLabel( i18n("&Executable:"), group );
  label->setBuddy( mExeED );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mExeED, row, 1 );

  connect( mExeED, SIGNAL(fileNameChanged(QString)), SLOT(slotChanged()) );

  ++row;
  mSocketED = new Kleo::FileNameRequester( group );
  label = new QLabel( i18n("&Socket:"), group );
  label->setBuddy( mSocketED );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mSocketED, row, 1 );

  connect( mSocketED, SIGNAL(fileNameChanged(QString)), SLOT(slotChanged()) );

  ++row;
  mLogLevelCB = new QComboBox( group );
  mLogLevelCB->addItem( i18n("None") );
  mLogLevelCB->addItem( i18n("Basic") );
  mLogLevelCB->addItem( i18n("Advanced") );
  mLogLevelCB->addItem( i18n("Expert") );
  mLogLevelCB->addItem( i18n("Guru") );
  label = new QLabel( i18n("Default &log level:"), group );
  label->setBuddy( mLogLevelCB );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mLogLevelCB, row, 1 );

  connect( mLogLevelCB, SIGNAL(activated(int)), SLOT(slotChanged()) );

  /******************* Log Window group *******************/
  group = new QGroupBox( i18n("Log Window"), top );
  vlay->addWidget(group);

  glay = new QGridLayout( group );
  glay->setSpacing( spacingHint() );
  glay->setColumnStretch( 1, 1 );

  row = -1;

  ++row;
  mLoglenSB = new KIntSpinBox( group );
  mLoglenSB->setRange( 0, 1000000 );
  mLoglenSB->setSingleStep( 100 );
  mLoglenSB->setSuffix( ki18ncp("history size spinbox suffix", " line", " lines") );
  mLoglenSB->setSpecialValueText( i18n("unlimited") );
  label = new QLabel( i18n("&History size:"), group );
  label->setBuddy( mLoglenSB );
  glay->addWidget( label, row, 0 );
  glay->addWidget( mLoglenSB, row, 1 );
  QPushButton * button = new QPushButton( i18n("Set &Unlimited"), group );
  glay->addWidget( button, row, 2 );

  connect( mLoglenSB, SIGNAL(valueChanged(int)), SLOT(slotChanged()) );
  connect( button, SIGNAL(clicked()), SLOT(slotSetHistorySizeUnlimited()) );

  ++row;
  mWordWrapCB = new QCheckBox( i18n("Enable &word wrapping"), group );
  mWordWrapCB->hide(); // QTextEdit doesn't support word wrapping in LogText mode
  glay->addWidget( mWordWrapCB, row, 0, 1, 3 );

  connect( mWordWrapCB, SIGNAL(clicked()), SLOT(slotChanged()) );

  vlay->addStretch( 1 );

  connect( this, SIGNAL(okClicked()), SLOT(slotSave()) );
}

KWatchGnuPGConfig::~KWatchGnuPGConfig() {}

void KWatchGnuPGConfig::slotSetHistorySizeUnlimited() {
  mLoglenSB->setValue( 0 );
}

void KWatchGnuPGConfig::loadConfig()
{
  const KConfigGroup watchGnuPG( KGlobal::config(), "WatchGnuPG" );
  mExeED->setFileName( watchGnuPG.readEntry( "Executable", WATCHGNUPGBINARY ) );
  mSocketED->setFileName( watchGnuPG.readEntry( "Socket", WATCHGNUPGSOCKET ) );
  mLogLevelCB->setCurrentIndex( log_level_to_int( watchGnuPG.readEntry( "LogLevel", "basic" ) ) );

  const KConfigGroup logWindow( KGlobal::config(), "LogWindow" );
  mLoglenSB->setValue( logWindow.readEntry( "MaxLogLen", 10000 ) );
  mWordWrapCB->setChecked( logWindow.readEntry("WordWrap", false ) );

  enableButtonOk( false );
  enableButtonApply( false );
}

void KWatchGnuPGConfig::saveConfig()
{
  KConfigGroup watchGnuPG( KGlobal::config(), "WatchGnuPG" );
  watchGnuPG.writeEntry( "Executable", mExeED->fileName() );
  watchGnuPG.writeEntry( "Socket", mSocketED->fileName() );
  watchGnuPG.writeEntry( "LogLevel", log_levels[mLogLevelCB->currentIndex()] );

  KConfigGroup logWindow( KGlobal::config(), "LogWindow" );
  logWindow.writeEntry( "MaxLogLen", mLoglenSB->value() );
  logWindow.writeEntry( "WordWrap", mWordWrapCB->isChecked() );

  KGlobal::config()->sync();

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
