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


#ifndef PROTOCOL_H
#define PROTOCOL_H

/**
 * @file
 * This file has one class. That class is used to determine the properties
 * of a protocol. It gives back the way the configuration dialog should look like,
 * and the way the settings are merge together to be usefull for the maildrop.
 *
 * The function is to make it possible to use a small number of maildrop's for a 
 * (relative) big amount of protocols.
 */

class AccountInput;
class KConfigGroup;
class KIO_Protocol;
class KMailDrop;

class QObject;
class QString;
class QStringList;
class QWidget;

template< class T> class QList;
template< class T> class QVector;
template< class T, class S> class QMap;

/**
 * This is the base class for every protocol. This class manage the properties of a protocol.
 * For example, it defines the input boxes in the configuration dialog, but it determines also the way a configuration file is readed.
 *
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */
class Protocol
{
public:
	/**
	 * Empty contructor
	 */
	Protocol() {}
	/**
	 * Empty destructor
	 */
	virtual ~Protocol() {}

	/**
	 * This function return another protocol which has to be used given the configuration file.
	 * This way, it is possible to specify the protocol based on the configuration.
	 * 
	 * KMail_Proto makes use of this function. It reads the KMail configuration file, and gives the protocol of the
	 * specified KMail account.
	 *
	 * The protocol is not created with new, and thus it is not neccesairy to delete the returned pointer.
	 *
	 * @param config the configuration group of an account with protocol 'implementation'.
	 * @return a (new) protocol which states the protocol to use after this function call.
	 */
	virtual const Protocol* getProtocol( KConfigGroup* config ) const = 0;
	/**
	 * This function returns the mail drop belonging to this protocol.
	 * The return value of this function also depends on the configuration, just like getProtocol().
	 *
	 * @param config the configuration of the account
	 * @return the KMailDrop which should be used for this protocol.
	 */
	virtual KMailDrop* createMaildrop( KConfigGroup* config ) const = 0;
	/**
	 * This function makes a configuration mapping given a the contents of a configuration file.
	 * The password is shipped apart, because it can be stored in a wallet.
	 *
	 * This function makes it possible to get the settings from another place then the KOrn configuration file.
	 *
	 * The returning type is a mapping from a config key to a value.
	 * The caller is responsable for deleting this object.
	 *
	 * @param config the configuration file to determine the mapping
	 * @param password the password which could be stored elsewhere
	 * @return a mapping from a configuration key to a value
	 */
	virtual QMap< QString, QString > * createConfig( KConfigGroup* config, const QString& password ) const = 0;

	/**
	 * This function return the name of the protocol as it is specified in the configuration file.
	 * In the configuration file, only a name of a protocol is given.
	 * This function makes it possible to compare the protocol in a configuration file with the Protocol class.
	 *
	 * This function should be constant and only depend on the Protocol class.
	 *
	 * @return the type of the account and the name in the configuration file
	 */
	virtual QString configName() const = 0;

	/**
	 * This function fills a string list with names of groupboxes.
	 * If the configuration dialog is called, this account screen can be split up in serveral group boxes.
	 * The number of them and the names of the groupboxes are coming from this function.
	 * The input field are specified in configFields().
	 *
	 * @param list a pointer to an empty list which must be filled with names of group boxes in this function.
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const = 0;
	/**
	 * This function puts the input fields into the groupboxes.
	 * The groupboxes are the widgets in the @p vector.
	 * The input fields whould be append to the list @p list.
	 * The QObject of the caller is specified in the parameter @p obj.
	 *
	 * @param vector a vector which pointers to groupboxes where the input fields must be placed in
	 * @param obj the QObject of the configuration dialog to connect signals to
	 * @param list A pointer to a list of AccountInput*. This list should be filled in this function.
	 */
	virtual void configFields( QVector< QWidget* >* vector, const QObject* obj, QList< AccountInput* >* list ) const = 0;
	/**
	 * This function called to change a configuration mapping right after reading it.
	 * After the data is read in, the configuration mapping can be changed to
	 * fits the needs of the configuration dialog.
	 *
	 * @param config the configuration mapping which could be changed
	 */
	virtual void readEntries( QMap< QString, QString >* config ) const = 0;
	/**
	 * This function is called to change a configuration mapping right before writing it.
	 * It is a bit like the inverse function of readEntries().
	 * It can be used to put things back in place before writing it to file.
	 *
	 * @param config a pointer to the configuration mapping which can be changed
	 */
	virtual void writeEntries( QMap< QString, QString >* config ) const = 0;

	/**
	 * The function return the default port of a protocol.
	 *
	 * @param ssl true gives the default ssl-port back; false the default normal port
	 * @return a port number depending on the protocol and the usage of ssl
	 */
	virtual unsigned short defaultPort( bool ) const { return 0; }

	/**
	 * This is a function to prevent a cast. If gives a pointer to a KIO_Protocol
	 * if the Protocol* is an KIO_Protocol, and 0 elsewise.
	 *
	 * @return a pointer to this as KIO_Protocol if this implementation is a KIO_Protocol; 0 elsewise
	 */
	virtual const KIO_Protocol* getKIOProtocol() const { return 0; }

	/**
	 * This function returns if the protocol can be rechecked after a certain interval.
	 *
	 * @return true if it is possible to recheck after a certain interval; false otherwise
	 */
	virtual bool isPollable() const { return true; }
};

#endif //PROTOCOL_H

