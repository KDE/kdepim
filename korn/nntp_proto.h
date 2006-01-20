/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
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

#ifndef MK_NNTP_PROTO_H
#define MK_NNTP_PROTO_H

/**
 * @file
 *
 * This file contains the class Nntp_Protocol.
 */

#include "kio_proto.h"

/**
 * This class contains the information needed to use Nntp as protocol.
 * If is derived from KIO_Protocol, because KIO can be used to fetch messages.
 */
class Nntp_Protocol : public KIO_Protocol
{
public:
	/**
	 * Empty constructor
	 */
	Nntp_Protocol() { }
	/**
	 * Empty destructor
	 */
	virtual ~Nntp_Protocol() { }

	/**
	 * This function returns a Protocol which can be used after reading the configuration.
	 * In this class, if should not change the protocol, so it returns itself.
	 *
	 * @param config the configuration (not used in this class)
	 * @return a pointer to a Protocol; in this class, always itself (this)
	 */
	virtual const Protocol* getProtocol( KConfigGroup* ) const { return this; }

	/**
	 * This function returns a new instance of the class.
	 *
	 * @return a new intance of this class
	 */
	virtual KIO_Protocol * clone() const { return new Nntp_Protocol; }

	/**
	 * This function returns always true, because Nntp is connection based.
	 *
	 * @return for this class, always "true"
	 */
	virtual bool connectionBased() const { return true; }

	/**
	 * This function returns the name of the kioslave to use.
	 * For NNTP, the kioslave is called "nntp".
	 *
	 * @return in this class, it always returns "nntp"
	 */
	virtual QString protocol() const { return "nntp"; }
	/**
	 * This function returns the name for the configuration file.
	 * This class always returns "nntp" for this function.
	 *
	 * @return for this class, always "nntp"
	 */
	virtual QString configName() const { return "nntp"; }

	/**
	 * Using the NNTP-protocol, it is possible to read the subjects.
	 * That is why this function returns "true" for this class.
	 *
	 * @return for this class, it returns "true"
	 */
	virtual bool canReadSubjects() const { return true; }
	/**
	 * It is not possible to delete emails using NNTP, so this function returns false.
	 *
	 * @return for this class, it returns "false"
	 */
	virtual bool canDeleteMail() const { return false; }
	/**
	 * If is possible to read emails using NNTP, so this function return true.
	 *
	 * @return for this class, it returns "true"
	 */
	virtual bool canReadMail() const { return true; }
	/**
	 * If a message is downloaded, if always is the full message.
	 * Thus this function return true.
	 *
	 * @return always "true" for this class
	 */
	virtual bool fullMessage() const { return true; }

	/**
	 * This function return 119 because 119 is the default port for NNTP.
	 *
	 * @return for this class, it always returns 119
	 */
	virtual unsigned short defaultPort( bool ) const { return 119; }

	/**
	 * This function adds two group box titles to this list.
	 * 
	 * @param list The list with the names of groupboxes. This list is changed by this function.
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * This function adds configuration fields to the groupboxes in vector.
	 *
	 * @param vector the vector containing the groupboxes (parents for new inputs)
	 * @param obj the object to connect signals to
	 * @param input a list with input fields; some entries are added to this item
	 *
	 */
        virtual void configFields( QVector< QWidget* >* vector, const QObject* obj, QList< AccountInput* >* input ) const;
	/**
	 * This function does nothing for this class.
	 * In can be used to change some configuration options.
	 *
	 * @param config a list of configuration entries
	 * @param metadata a list of metadata entries
	 */
        virtual void readEntries( QMap< QString, QString > *config, QMap< QString, QString >* metadata ) const;
	/**
	 * In this class, this function clears unused fields.
	 * If can be used to change some configuration entries before writing to file.
	 *
	 * @param config the configuration mapping
	 */
        virtual void writeEntries( QMap< QString, QString >* config ) const;
};

/*
 * TODO: QVector -> QList?
 *       references instead of pointers?
 *       QMap -> QHash?
 */

#endif //MK_NNTP_PROTO_H
