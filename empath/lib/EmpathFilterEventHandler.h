/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

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


#ifndef EMPATHFILTEREVENTHANDER_H
#define EMPATHFILTEREVENTHANDER_H

// Qt includes
#include <qobject.h>

// Local includes
#include "EmpathURL.h"

/**
 * @short Used to do the actual message processing for a filter
 * 
 * When an EmpathFilterEventHandler is passed an URL, it gets the message
 * pointed to and then performs the defined action.
 * 
 * This may be 'move to folder', 'delete', 'ignore', etc.
 * @author Rikkus
 */
class EmpathFilterEventHandler : public QObject
{
    Q_OBJECT

    public:

        enum ActionType { MoveFolder, CopyFolder, Delete, Ignore, Forward };

        EmpathFilterEventHandler();

        virtual ~EmpathFilterEventHandler();

        /**
         * Set this handler to move messages to the given folder.
         */
        void setMoveFolder(const EmpathURL &);
        /**
         * Set this handler to copy messages to the given folder.
         */
        void setCopyFolder(const EmpathURL &);
        /**
         * Set this handler to delete all messages.
         */
        void setDelete();
        /**
         * Set this handler to ignore all messages.
         */
        void setIgnore();
        /**
         * Set this handler to forward all messages to the given address.
         */
        void setForward(const QString &);

        /**
         * Handle the given message now !
         */
        void handleMessage    (const EmpathURL &);

        /**
         * Load settings. Called by the containing filter.
         */
        bool load            (const QString &);
        /**
         * Save settings. Called by the containing filter.
         */
        void save            (const QString &);

        QString        description()        const;
        ActionType     actionType()         const;
        EmpathURL      moveOrCopyFolder()   const;
        QString        forwardAddress()     const;

    private:

        QString        forwardAddress_;
        EmpathURL    moveCopyFolder_;
        ActionType    actionType_;
};

#endif // EMPATHFILTEREVENTHANDLER_H

// vim:ts=4:sw=4:tw=78
