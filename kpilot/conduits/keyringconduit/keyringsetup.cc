/* KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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
#include "keyringsetup.moc"

#include <QApplication>
#include <QtCrypto>

#include <kurlrequester.h>
#include <kaboutdata.h>
#include <kiconloader.h>

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

KeyringWidgetSetup::KeyringWidgetSetup( QWidget *w, const QVariantList & ) :
	ConduitConfigBase( w ), 
	fWallet(0L)
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
	fWallet = 0L;
}

/* virtual */ void KeyringWidgetSetup::commit()
{
	FUNCTIONSETUP;

	DEBUGKPILOT
		<< ": Database file="
		<< fUi.fLocalDatabaseUrl->url().path();

	KeyringConduitSettings::setDatabaseUrl( fUi.fLocalDatabaseUrl->url().path() );
	
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

	WId window = fWidget->winId();
	
	fWallet = Wallet::openWallet( Wallet::LocalWallet(), window );

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

	// if we couldn't load the wallet system, then tell our user about it
	// and don't let him hurt himself
	if (fWallet)
	{
		fUi.fWalletErrorIcon->setVisible(false);
		fUi.fWalletErrorMessage->setVisible(false);
		fUi.fSavePassButton->setEnabled( true );
		fUi.fPassEdit->setEnabled( true );
	}
	else
	{
		fUi.fAskPassButton->setChecked( true );
		fUi.fWalletErrorIcon->setVisible(true);
		fUi.fWalletErrorMessage->setVisible(true);
		fUi.fSavePassButton->setEnabled( false );
		fUi.fPassEdit->setEnabled( false );
	}

	fUi.fQCAErrorIcon->setPixmap(
		KIcon(QLatin1String("dialog-error")).pixmap(32));
	fUi.fWalletErrorIcon->setPixmap(
		KIcon(QLatin1String("dialog-error")).pixmap(32));

	// the Initializer object sets things up, and
	// also does cleanup when it goes out of scope
	QCA::Initializer init;

	// TODO: make sure that we're able to load all required crypto modules.
	bool haveNecessaryQCAModules = QCA::isSupported("tripledes-cbc");

	if (!haveNecessaryQCAModules) {
		fUi.fQCAErrorIcon->setVisible(true);
		fUi.fQCAErrorMessage->setVisible(true);
	} else {
		fUi.fQCAErrorIcon->setVisible(false);
		fUi.fQCAErrorMessage->setVisible(false);
	}

	unmodified();
}

/* static */ ConduitConfigBase *KeyringWidgetSetup::create( QWidget *w )
{
	return new KeyringWidgetSetup( w );
}

void KeyringWidgetSetup::savePassword()
{

	if (! fWallet)
	{
		return;
	}
	
	QString passwordFolder = Wallet::PasswordFolder();
	if ( !fWallet->hasFolder( passwordFolder ) )
	{
		fWallet->createFolder( passwordFolder );
	}

	fWallet->setFolder( passwordFolder );
	fWallet->writePassword( CSL1( "kpilot-keyring" ), fUi.fPassEdit->text() );
	
	// Save the wallet to disk.
	fWallet->sync();
	Wallet::disconnectApplication( Wallet::LocalWallet(), CSL1( "KPilot" ) );
}

QString KeyringWidgetSetup::loadPassword()
{
	if (! fWallet)
	{
		return QString();
	}
	
	QString passwordFolder = Wallet::PasswordFolder();
	if ( !fWallet->hasFolder( passwordFolder ) )
	{
		return QString();
	}

	fWallet->setFolder( passwordFolder );
	
	QString pass;
	fWallet->readPassword( CSL1( "kpilot-keyring" ), pass );
	Wallet::disconnectApplication( Wallet::LocalWallet(), CSL1( "KPilot" ) );
	
	return pass;
}
