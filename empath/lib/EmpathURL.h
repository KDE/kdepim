/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef EMPATHURL_H
#define EMPATHURL_H

// Qt includes
#include <qstringlist.h>

// Local includes
#include "EmpathIndexRecord.h"

/**
 * This class will soon be renamed to something like KEmailURL.
 * It provides for referencing a mailbox, folder, subfolders, and a message id.
 * 
 * The form is like this:
 * empath://<mailbox>/[folder/][subfolder/][subsubfolder/][MessageID]
 * 
 * A single message may be referenced by giving the full path to it. In this way,
 * you get the location of the message.
 * 
 * If you're referring to a mailbox only, you may omit a trailing slash.
 * If you're referring to a folder, you must have a trailing slash, lest the last
 * folder is confused with a message ID.
 * 
 * There may NOT be a slash in the name of a mailbox, folder or message ID.
 */
class EmpathURL
{
	public:

		/**
		 * Default ctor.
		 */
		EmpathURL();
	
		/**
		 * Ctor with mailbox, folder and message ID.
		 */
		EmpathURL(
				const QString & mailboxName,
				const QString & folderPath,
				const QString & messageID);

		/**
		 * Ctor with a string representation that is immediately parsed and
		 * reassembled.
		 */
		EmpathURL(const QString & fullPath);

		/**
		 * Copy ctor.
		 */
		EmpathURL(const EmpathURL & url);
		
		EmpathURL & operator = (const EmpathURL & url);
		
		EmpathURL & operator = (const QString &);

		bool operator == (const EmpathURL & b);
		bool operator == (const QString & s);

		virtual ~EmpathURL();

		QString mailboxName() const { return mailboxName_; }
		QString folderPath() const { return folderPath_; }
		QString messageID() const { return messageID_; }
		
		EmpathURL withoutMessageID() const;
		
		/**
		 * This will always return true, theoretically.
		 */
		bool hasMailbox() const;
		
		/**
		 * Returns true if there is a folder part to this URL.
		 */
		bool hasFolder() const;
		
		/**
		 * Returns true if there's a message id at the end of this URL.
		 */
		bool hasMessageID() const;
		
		/**
		 * Returns the assembled representation of this URL.
		 */
		QString asString() const;
		
		void setMailboxName(const QString & mailboxName);
		void setFolderPath(const QString & folderPath);
		void setMessageID(const QString & messageID);

		const char * className() const { return "EmpathURL"; }

		/**
		 * Returns the folder path, by creating a string list using each folder
		 * and subfolder name.
		 */
		QStringList folderPathList();

	private:

		void _parse();
		void _assemble();
		void _stripSlashes(QString & s);
		void _simplifySlashes(QString & s);

		QString mailboxName_;
		QString folderPath_;
		QString messageID_;

		QString strRep_;

		bool isValid_;
};

#endif // EMPATHURL_H

