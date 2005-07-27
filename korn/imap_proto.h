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

#ifndef MK_IMAP_PROTO_H
#define MK_IMAP_PROTO_H

#include "kio_proto.h"
#include <kurl.h>

/*
 * With deleting and IMAP4 is a small problem: messages don't looks as deleted, as they
 * apear with their full body. By deletion, kio_imap marks the message with FLAGS.SILENT as \DELETED.
 * If there is a commit-function, it should be installed in this file.
 */

/**
 * This class define the way a imap-protocol works.
 * It implements function of KIO_Protocol to make the kio-modules work with it,
 * as well as function of Protocol, to configure it.
 */
class Imap_Protocol : public KIO_Protocol
{
public:
	/**
	 * Constructor
	 */
	Imap_Protocol()  {}
	/**
	 * Destructor
	 */
	virtual ~Imap_Protocol() {}

	/**
	 * This function should return true if the protocol is connection-based.
	 * imap is, so this return "true".
	 *
	 * @return true
	 */
	virtual bool connectionBased() const { return true; }
	
	/**
	 * This gives the two names for a kioslave: imaps if ssl is selected, imap if not.
	 *
	 * @param ssl true if ssl is selected.
	 * @return "imaps" if ssl is true, "imap" otherwise
	 */
	virtual QString protocol( bool ssl ) const { return ssl ? "imaps" : "imap"; }
	/**
	 * This name of this protocol: it goed in the configuration under this name.
	 *
	 * @return The name of this protocol: "imap"
	 */
	virtual QString configName() const { return "imap"; }
	/**
	 * true, because it is possible to read subjects with imap.
	 *
	 * @return true
	 */
	virtual bool canReadSubjects() const { return true; }
	/**
	 * false, because deleting imap-mails doesn't work that well. See the commen above this class:
	 * metadata expunge=auto doesn't work.
	 * 
	 * @return false
	 */
	virtual bool canDeleteMail() const { return false; } //See comment above class: metadata expunge=auto doesn't work.
	/**
	 * true, because it is possible to read the whole message.
	 *
	 * @return true
	 */
	virtual bool canReadMail() const { return true; }

	/**
	 * This function returns the default port. This depends whether ssl is used or not.
	 * If ssl is used, it return 993, elsewise 143.
	 *
	 * @param ssl Is ssl used?
	 * @return 993 if ssl is true, false otherwise.
	 */
	virtual unsigned short defaultPort( bool ssl ) const { return ssl ? 993 : 143; }

	virtual QStringList authList() const { return QStringList::split( '|', "*|LOGIN|ANONYMOUS|CRAM-MD5", false); }
	//Could not test did, my server don't support other authentication methods.

	/**
	 * These function change the kurl and the metadata.
	 * In this case, "unseen" is added to the query to only list unlees kurls.
	 * These function are called in kio_*.cpp
	 */
	virtual void recheckKURL    ( KURL &kurl, KIO::MetaData & ) const { kurl.setQuery( "unseen" ); }
	virtual void readSubjectKURL( KURL &kurl, KIO::MetaData & ) const { kurl.setPath( kurl.path() + ";section=ENVELOPE" ); }
	virtual void deleteMailConnectKURL( KURL &, KIO::MetaData & metadata ) const { metadata.insert( "expunge", "auto" ); }
	
	/**
	 * This functions gives a list of names of groupboxes which are to be set in the configuration.
	 * In this case, two elements are added: "Server" and "Identity".
	 *
	 * @param list A list to add the name of groupboxes in.
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * This function adds elements to the groupbox.
	 *
	 * @param vector The vector containing the groupBoxes
	 * @param object The object to connect signals to
	 * @param ptrlist A list with object which is filled in this function. The list must already be created.
	 */
	virtual void configFields( QPtrVector< QWidget >* vector, const QObject* object, QPtrList< AccountInput >* ptrlist ) const;
	/**
	 * This function is used to change the configuration.
	 * In the case, the metadata-key is splitted out, and put in the @p metadata parameter.
	 *
	 * @param map The mapping containing the configuration. This object can change in this function.
	 * @param metadata An empty mapping at the begin, a mapping containing metadata at the end.
	 */
        virtual void readEntries( QMap< QString, QString >* map, QMap< QString, QString >* metadata ) const;
	/**
	 * This function edits writeEntry. It merge things back to a metadata-key and adds this
	 * key to the configuration.
	 *
	 * @param map The mapping which contains the information to be written to a configuarion file.
	 *	the contents of this mapping can change in this function.
	 */
        virtual void writeEntries( QMap< QString, QString >* map ) const;
};

#endif
