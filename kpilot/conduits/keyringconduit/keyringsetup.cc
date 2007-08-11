/* KPilot
**
** Copyright (C) 2007 Bertjan Broeksema
** Copyright (C) 2002-2003 Reinhold Kainhofer
**
** This file defines the setup dialog for the Keyring-conduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "keyringsetup.h"

#include <QApplication>

#include <kurlrequester.h>
#include <kaboutdata.h>
#include <kwallet.h>

#include "options.h"

#include "keyringsettings.h"

using namespace KWallet;

static KAboutData *createAbout()
{
	KAboutData *fAbout = new KAboutData( "keyringconduit", 0
		, ki18n( "Keyring Conduit for KPilot" )
		, KPILOT_VERSION
		, ki18n( "Configures the Keyring Conduit for KPilot" )
		, KAboutData::License_GPL
		, ki18n( "(C) 2007, Bertjan Broeksema" )
	);
	
	fAbout->addAuthor( ki18n( "Bertjan Broeksema" )
		, ki18n( "Primary Author" )
		, "b.broeksema@home.nl"
		, "http://bertjan.broeksemaatjes.nl"
	);
	
	return fAbout;
}

KeyringWidgetSetup::KeyringWidgetSetup( QWidget *w ) :
	ConduitConfigBase( w )
{
	FUNCTIONSETUP;

	fWidget = new QWidget();
	fUi.setupUi( fWidget );
	fUi.fLocalDatabaseUrl->setMode( KFile::ExistingOnly | KFile::LocalOnly );
	fUi.fLocalDatabaseUrl->setFilter( CSL1( "*.pdb") );
	
	QObject::connect( fUi.fLocalDatabaseUrl
		, SIGNAL( textChanged( const QString& ) ), this, SLOT( modified() ) );
	QObject::connect( fUi.fSavePassButton, SIGNAL( toggled( bool ) ), this
		, SLOT( modified() ) );
	QObject::connect( fUi.fAskPassButton, SIGNAL( toggled( bool ) ), this
		, SLOT( modified() ) );
	QObject::connect( fUi.fPassEdit
		, SIGNAL( textChanged( const QString& ) ), this, SLOT( modified() ) );

	fConduitName = i18n( "Keyring Conduit" );
	fAbout = createAbout();
	ConduitConfigBase::addAboutPage( fUi.tabWidget, fAbout );
}

KeyringWidgetSetup::~KeyringWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void KeyringWidgetSetup::commit()
{
	FUNCTIONSETUP;

	DEBUGKPILOT
		<< ": Database file="
		<< fUi.fLocalDatabaseUrl->url().url();

	KeyringConduitSettings::setDatabaseUrl( fUi.fLocalDatabaseUrl->url().url() );
	
	if( fUi.fSavePassButton->isChecked() )
	{
		KeyringConduitSettings::setPasswordSetting( KeyringConduitSettings::Wallet );
		savePassword();
	}
	else
	{
		KeyringConduitSettings::setPasswordSetting( KeyringConduitSettings::Ask );
	}
	
	KeyringConduitSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void KeyringWidgetSetup::load()
{
	FUNCTIONSETUP;
	KeyringConduitSettings::self()->readConfig();

	fUi.fLocalDatabaseUrl->setUrl( KeyringConduitSettings::databaseUrl() );
	
	if( KeyringConduitSettings::passwordSetting() == KeyringConduitSettings::Wallet )
	{
		fUi.fSavePassButton->setChecked( true );
		fUi.fPassEdit->setEnabled( true );
		fUi.fPassEdit->setText( loadPassword() );
	}
	else
	{
		fUi.fAskPassButton->setChecked( true );
		fUi.fPassEdit->setEnabled( false );
	}

	unmodified();
}

/* static */ ConduitConfigBase *KeyringWidgetSetup::create( QWidget *w )
{
	return new KeyringWidgetSetup( w );
}

void KeyringWidgetSetup::savePassword()
{
	// FIXME: Is this save? Or is there a good change that qApp->activeWindow()
	// return 0?
	WId window = qApp->activeWindow()->winId();
	
	Wallet *wallet = Wallet::openWallet( Wallet::LocalWallet(), window );
	
	QString passwordFolder = Wallet::PasswordFolder();
	if ( !wallet->hasFolder( passwordFolder ) )
	{
		wallet->createFolder( passwordFolder );
	}

	wallet->setFolder( passwordFolder );
	wallet->writePassword( CSL1( "kpilot-keyring" ), fUi.fPassEdit->text() );
	
	// Save the wallet to disk.
	wallet->sync();
	Wallet::disconnectApplication( Wallet::LocalWallet(), CSL1( "KPilot" ) );
}

QString KeyringWidgetSetup::loadPassword()
{
	WId window = qApp->activeWindow()->winId();
	
	Wallet *wallet = Wallet::openWallet( Wallet::LocalWallet(), window );

	if (! wallet)
	{
		return QString();
	}
	
	QString passwordFolder = Wallet::PasswordFolder();
	if ( !wallet->hasFolder( passwordFolder ) )
	{
		return QString();
	}

	wallet->setFolder( passwordFolder );
	
	QString pass;
	wallet->readPassword( CSL1( "kpilot-keyring" ), pass );
	Wallet::disconnectApplication( Wallet::LocalWallet(), CSL1( "KPilot" ) );
	
	return pass;
}
