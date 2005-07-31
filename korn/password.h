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


#ifndef KORNPASSWORD_H
#define KORNPASSWORD_H

class KConfigBase;
namespace KWallet { class Wallet; }

class QString;

/**
 * This class can be used to store and retrieve passwords.
 * It uses KWallet if possible, and the configuartion files otherwise.
 */
class KOrnPassword
{
public:
	/**
	 * Constructor: empty
	 */
	KOrnPassword();
	/**
	 * Destructor: empty
	 */
	~KOrnPassword();

	/**
	 * This function gets a password for KOrn.
	 *
	 * @param box The boxnumber of the account
	 * @param account The accountnumber of the account
	 * @param fallbackConfig The configuration file if KWallet cannot be used.
	 * @return The password, or QString::null if it failes.
	 */
	static QString readKOrnPassword( int box, int account, const KConfigBase& fallbackConfig );
	/**
	 * This function gets a password with is puts in KWallet by KMail
	 *
	 * @param accountnr The id of the KMail account
	 * @param fallbackConfig The configuration used if KWallet isn't available.
	 * @return The password, QStirng::null if it failes.
	 */
	static QString readKMailPassword( int accountnr, const KConfigBase& fallbackConfig );
	
	/**
	 * This function saves a password for usage in KOrn.
	 *
	 * @param box The boxnumber of the account.
	 * @param account The accountnumber of the account.
	 * @param fallbackConfig The configuration file if KWallet isn't available.
	 * @param password The password to be stored.
	 */
	static void writeKOrnPassword( int box, int account, KConfigBase& fallbackConfig, const QString& password );

	/**
	 * This function deletes a password from both KWallet and the configuration file
	 *
	 * @param box The boxnumber of the account.
	 * @param account The accountnumber of the account.
	 * @param fallbackConfig The configuration file if KWallet isn't available.
	 */
	static void deleteKOrnPassword( int box, int account, KConfigBase& fallbackConfig );

	/**
	 * This function deletes a password from KWallet 
	 *
	 * @param box The boxnumber of the account.
	 * @param account The accountnumber of the account.
	 */
	static bool deleteKOrnPassword( int box, int account );
	
	/**
	 * This function moves a password
	 *
	 * @param boxSrc The source box number.
	 * @param accountSrc The source account number.
	 * @param configSrc The source configuration group.
	 * @param boxDest The destination box number.
	 * @param accountDest The destination account number
	 * @param configDest The destination configuration group.
	 */
	static void moveKOrnPassword( int boxSrc, int accountSrc, KConfigBase &configSrc,
	                              int boxDest, int accountDest, KConfigBase &configDest );

	/**
	 * This swaps the password from one box/account combination to another.
	 *
	 * @param box1 The box number of the first password.
	 * @param account1 The account number of the first password.
	 * @param config1 The configurationgroup of the first password.
	 * @param box2 The box number of the second password.
	 * @param account2 The account number of the second password.
	 * @param config2 The configurationgroup of the second password.
	 */
	static void swapKOrnPassword( int box1, int account1, KConfigBase &config1, int box2, int account2, KConfigBase &config2 );

	/**
	 * This swaps the Wallet password from one box/account combination to another.
	 *
	 * @param box1 The box number of the first password.
	 * @param account1 The number of accounts to be investigated
	 * @param box2 The box number of the first password.
	 * @param account2 The number of accounts to be investigated
	 */
	static void swapKOrnWalletPassword( int box1, int account1 ,int box2, int account2 );
	
	/**
	 * This swaps the password from one box to another.
	 *
	 * @param box1 The box number of the first password.
	 * @param accountnumber1 The number of accounts to be investigated
	 * @param box2 The box number of the first password.
	 * @param accountnumber2 The number of accounts to be investigated
	 */
	static void swapKOrnWalletPasswords( int box1, int accountnumber1 ,int box2, int accountnumber2 );
	
	/**
	 * This function read the password from the configuration (wallet or configuration) with UseWallet set to !newUseWalletValue,
	 * afterwards, it writes the password to the configuration with UseWallet set to newUseWalletValue
	 *
	 * @param box The box number
	 * @param account The account number
	 * @param config The configuration group
	 * @param newUseWalletValue true to transport a password from a configuration to a wallet, false the other wat around
	 */
	static void rewritePassword( int box, int account, KConfigBase &config, bool newUseWalletValue );
	
	/**
	 * If set to true, this class will try to use KWallet,
	 * if false, it will not.
	 *
	 * The default value is true.
	 *
	 * @param value The value to be set to this property.
	 */
	static void setUseWallet( const bool value );
private:
	static void open();

	static bool readKOrnPassword( int box, int account, QString& password );
	static bool writeKOrnPassword( int box, int account, const QString& password );
	
	static QString KMailDecrypt( const QString& enc );
	
	static KWallet::Wallet *m_wallet;
	static bool m_openFailed;
	static bool m_useWallet;
};

#endif //KORNPASSWORD_H

