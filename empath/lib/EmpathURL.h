/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

#ifdef __GNUG__
# pragma interface "EmpathURL.h"
#endif

#ifndef EMPATHURL_H
#define EMPATHURL_H

// Qt includes
#include <qstrlist.h>

// Local includes
#include "EmpathIndexRecord.h"

/**
 * @short URL holding mailbox, folder and message id
 * This class will soon be renamed to something like KEmailURL.
 * It provides for referencing a mailbox, folder, subfolders, and a message id.
 * 
 * The form is like this:
 * empath://<mailbox>/[folder/][subfolder/][subsubfolder/][MessageID]
 * 
 * A single message may be referenced by giving the full path to it. In this
 * way, you get the location of the message.
 * 
 * If you're referring to a mailbox only, you may omit a trailing slash.
 * If you're referring to a folder, you must have a trailing slash, lest the
 * last folder is confused with a message ID.
 * 
 * There may NOT be a slash in the name of a mailbox, folder or message ID.
 * @author Rikkus
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

        bool operator == (const EmpathURL & b) const;
        bool operator == (const QString & s) const;

        virtual ~EmpathURL();

        QString mailboxName() const;
        QString folderPath() const;
        QString messageID() const;
        
        EmpathURL withoutMessageID() const;

        /**
         * Returns false if the URL is empty or if there was a 
         * parse error.
         */
        bool isValid() const { return isValid_; }
        
        /**
         * This will always return true, theoretically.
         */
        bool isMailbox() const
        {
            return !mailboxName_.isEmpty() &&
                    folderPath_.isEmpty()  &&
                    messageID_.isEmpty();
        }
        
        /**
         * Returns true if there is a folder part to this URL.
         */
        bool isFolder() const
        {
            return !folderPath_.isEmpty() && messageID_.isEmpty();
        }
        
        /**
         * Returns true if there's a message id at the end of this URL.
         */
        bool isMessage() const { return !messageID_.isEmpty(); }
        
        /**
         * Returns the assembled representation of this URL.
         */
        QString asString() const { return strRep_; }
        
        void setMailboxName(const QString & mailboxName);
        void setFolderPath(const QString & folderPath);
        void setMessageID(const QString & messageID);

        const char * className() const { return "EmpathURL"; }

        /**
         * Returns the folder path, by creating a string list using each folder
         * and subfolder name.
         */
        QStrList folderPathList();

    private:

        void _cleanUp(QString &);
        void _parse();
        void _assemble();
        QString _stripSlashes(const QString &) const;

        QString mailboxName_;
        QString folderPath_;
        QString messageID_;

        QString strRep_;

        bool isValid_;
};

#endif // EMPATHURL_H

// vim:ts=4:sw=4:tw=78
