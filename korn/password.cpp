/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "password.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kwallet.h>

#include <qstring.h>
#include <qstringlist.h>

KWallet::Wallet* KOrnPassword::m_wallet = 0;
bool KOrnPassword::m_openFailed = false;
bool KOrnPassword::m_useWallet = false; //Not default true until moving works

QString KOrnPassword::readKOrnPassword( int box, int account, const KConfigBase &fallbackConfig )
{
	QString result;

	if( readKOrnPassword( box, account, result ) )
		return result;
	else
		return fallbackConfig.readEntry( "password" );
}

QString KOrnPassword::readKMailPassword( int accountnr, const KConfigBase& fallbackConfig )
{
	QString password;
	open();

	if( !m_wallet || !m_wallet->isOpen() || m_openFailed )
		return KMailDecrypt( fallbackConfig.readEntry( "pass" ) );

	if( !m_wallet->hasFolder( "kmail" ) )
		return KMailDecrypt( fallbackConfig.readEntry( "pass" ));
	m_wallet->setFolder( "kmail" );

	if( m_wallet->readPassword( QString( "account-%1" ).arg( accountnr ), password ) != 0 )
		return fallbackConfig.readEntry( "password" );

	return password;
}

void KOrnPassword::writeKOrnPassword( int box, int account, KConfigBase& fallbackConfig, const QString& password )
{
	if( writeKOrnPassword( box, account, password ) )
	{
		if( fallbackConfig.hasKey( "password" ) )
			fallbackConfig.deleteEntry( "password" );
	}
	else
		fallbackConfig.writeEntry( "password", password );
}

void KOrnPassword::deleteKOrnPassword( int box, int account, KConfigBase& fallbackConfig )
{
	deleteKOrnPassword( box, account );
	if( fallbackConfig.hasKey( "password" ) )
		fallbackConfig.deleteEntry( "password" );
}

bool KOrnPassword::deleteKOrnPassword( int box, int account )
{
	if( !m_useWallet )
		//Wallet should not be used => saving in config file
		return false;

	//Open wallet
	open();

	if( !m_wallet || !m_wallet->isOpen() || m_openFailed )
		//Opening failed => delete in config file
		return false;

	//Make folder for KOrn if needed
	if( !m_wallet->hasFolder( "korn" ) )
		return false; //It does not exist
	m_wallet->setFolder( "korn" );
	
	//Write to wallet
	if( m_wallet->removeEntry( QString( "account-%1-%2" ).arg( box ).arg( account ) ) != 0 )
		//Writing failed
		return false;
	
	//Password succesfully stored in the configuration.
	return true;
}

void KOrnPassword::moveKOrnPassword( int boxSrc, int accountSrc, KConfigBase& configSrc,
                                     int boxDest, int accountDest, KConfigBase &configDest )
{
	QString password;
	
	password = readKOrnPassword( boxSrc, accountSrc, configSrc );
	deleteKOrnPassword( boxSrc, accountSrc, configSrc );
	writeKOrnPassword( boxDest, accountDest, configDest, password );
}

void KOrnPassword::swapKOrnPassword( int box1, int account1, KConfigBase &config1, int box2, int account2, KConfigBase &config2 )
{
	QString password1, password2;
	password1 = readKOrnPassword( box1, account1, config1 );
	password2 = readKOrnPassword( box2, account2, config2 );
	deleteKOrnPassword( box1, account1, config1 );
	deleteKOrnPassword( box2, account2, config2 );
	writeKOrnPassword( box1, account1, config1, password2 );
	writeKOrnPassword( box2, account2, config2, password1 );
}

void KOrnPassword::swapKOrnWalletPassword( int box1, int account1, int box2, int account2 )
{
	QString password1, password2;
	bool passExist1, passExist2;
	passExist1 = readKOrnPassword( box1, account1, password1 );
	passExist2 = readKOrnPassword( box2, account2, password2 );

	if( passExist1 )
		deleteKOrnPassword( box1, account1 );
	if( passExist2 )
		deleteKOrnPassword( box2, account2 );

	if( passExist1 )
		writeKOrnPassword( box2, account2, password1 );
	if( passExist2 )
		writeKOrnPassword( box1, account1, password2 );
}

void KOrnPassword::swapKOrnWalletPasswords( int box1, int accountnumber1 ,int box2, int accountnumber2 )
{
	int max = accountnumber1 > accountnumber2 ? accountnumber1 : accountnumber2;
	for( int xx = 0; xx < max; ++xx )
		swapKOrnWalletPassword( box1, xx, box2, xx );
}

void KOrnPassword::rewritePassword( int box, int account, KConfigBase &config, bool newUseWalletValue )
{
	bool useWallet = m_useWallet;
	QString password;
	
	setUseWallet( !newUseWalletValue );
	password = readKOrnPassword( box, account, config );
	deleteKOrnPassword( box, account, config );
	
	setUseWallet( newUseWalletValue );
	writeKOrnPassword( box, account, config, password );

	setUseWallet( useWallet );
}

void KOrnPassword::setUseWallet( const bool value )
{
	m_useWallet = value;
}

void KOrnPassword::open()
{
	if( m_wallet && m_wallet->isOpen() )
		return; //Already open

	if( m_openFailed )
		return; //Not open, and can't be opened

	delete m_wallet; m_wallet = 0;
	m_wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), 0 );
	
	if( !m_wallet )
		m_openFailed = true;
}

bool KOrnPassword::readKOrnPassword( int box, int account, QString& password )
{
	if( !m_useWallet )
		return false;

	//Otherwise: try to open the wallet
	open();

	if( !m_wallet || !m_wallet->isOpen() || m_openFailed )
		//Opening failed: getting value out of config if it exists there
		return false;

	if( !m_wallet->hasFolder( "korn" ) )
		//No folder korn exist, so no password stored in the wallet.
		return false;
	m_wallet->setFolder( "korn" );

	if( m_wallet->readPassword( QString( "account-%1-%2" ).arg( box ).arg( account ), password ) != 0 )
		//Error during reading the password: use the one in the config file
		return false;

	//Reading completed: returning
	return true;
}

bool KOrnPassword::writeKOrnPassword( int box, int account, const QString& password )
{
	if( !m_useWallet )
		//Wallet should not be used => saving in the config file
		return false;
	
	//Open wallet
	open();

	if( !m_wallet || !m_wallet->isOpen() || m_openFailed )
		//Opening failed => write to configfile
		return false;

	//Make folder for KOrn if needed
	if( !m_wallet->hasFolder( "korn" ) )
		m_wallet->createFolder( "korn" );
	m_wallet->setFolder( "korn" );
	
	//Write to wallet
	if( m_wallet->writePassword( QString( "account-%1-%2" ).arg( box ).arg( account ), password ) != 0 )
		//Writing failed
		return false;

	//Password succesfully stored in the configuration.
	return true;
}

//This function is copyed from kmail/kmaccount.cpp
QString KOrnPassword::KMailDecrypt( const QString& enc )
{
	QString result;
	for (uint i = 0; i < enc.length(); i++)
		result += (enc[i].unicode() <= 0x21) ? enc[i] : QChar(0x1001F - enc[i].unicode());

	return result;
}
