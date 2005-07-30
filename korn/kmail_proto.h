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


#ifndef MK_KMAILPROTOCOL
#define MK_KMAILPROTOCOL

#include "protocol.h"

class KConfig;
class KConfigBase;

/**
 * This class is the comminucation between KOrn and KMail.
 * If provides an configuration option to fill in the KMail account name,
 * and it provides a conversion from the KMail config to a configuration KOrn can read.
 * That conversion is done every time, so that changing the settings in KMail also affects
 * the settings in KOrn.
 */
class KMail_Protocol : public Protocol
{
public:
	/**
	 * Constructor
	 */
	KMail_Protocol();
	/**
	 * Destructor
	 */
	~KMail_Protocol();
	
	/**
	 * This function returns the protocol of the resulting configuration.
	 * This is not itself, because an real Protocol of type KMail doesn't exist.
	 * 
	 * @param config The configuration group to read the kmailname from.
	 * @return A pointer to a Protocol (not newly created) or 0 if an error orrured.
	 */
	virtual const Protocol* getProtocol( KConfigGroup* config ) const;
	/**
	 * This gives a new Maildrop back. The maildrop is responsible to execute the configuration.
	 * In most cases, it will return a new instance of the KKioDrop Maildrop.
	 * @param config The configuration group to read the kmailname from.
	 * @return A new KMailDrop, or a 0 pointer if an error occured.
	 */
        virtual KMailDrop* createMaildrop( KConfigGroup* config ) const;
	/**
	 * This does the real convertion: given a namen of a KMail account (hidden in config),
	 * it calculates how the configuration whould have looked in KOrn.
	 *
	 * @param config The configuration group to read the kmailname from.
	 * @param password The password fetched out of the configuration or KWallet.
	 * @return A mapping containing the configuration if success, and empty mapping otherwise.
	 */
        virtual QMap< QString, QString > * createConfig( KConfigGroup* config, const QString& password ) const;
	/**
	 * The name of thes protocol
	 *
	 * @return "kmail"
	 */
        virtual QString configName() const { return "kmail"; }

	/**
	 * This adds the names of the groupboxes neccesairy for configuration to list.
	 *
	 * @param list A empty list at calling; after this function, "KMail" is added, because KMail is the only group here.
	 */
        virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * This filles the config fields of the configuration.
	 * There is only one config field, containing the accounts of KMail.
	 *
	 * @param vector A vector containing the groupboxes.
	 * @param object An object to connect slots to.
	 * @param result The resulting object.
	 */
        virtual void configFields( QPtrVector< QWidget >* vector, const QObject* object, QPtrList< AccountInput >* result ) const;
	/**
	 * This can manipulate entries that are readed from the configuartion.
	 * This function does nothing here.
	 *
	 * @param config The configuration mapping as read from the configuration file
	 */
	virtual void readEntries( QMap< QString, QString >* config ) const;

	/**
	 * This can manipulate entries that are writed to the configuartion file.
	 * This function does nothing here.
	 *
	 * @param config The configuration mapping as to be written to the configuration file (can be modified)
	 */
	virtual void writeEntries( QMap< QString, QString >* config ) const;

private:
	QString readPassword( bool store, const KConfigBase &config, int id ) const;
	QString getTypeAndConfig( const QString& kmailname, KConfig &config, int &nummer ) const;
	
	static const char* kmailGroupName;
	static const char* kmailKeyName;
	static const char* kmailKeyType;
	static const char* kmailKeyId;
	static const char* kmailKeyMBox;
	static const char* kmailKeyQMail;
	static const int kmailFirstGroup;
};

#endif //MK_KMAILPROTOCOL
