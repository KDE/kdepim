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


#ifndef EMPATHMAILBOXMAILDIR_H
#define EMPATHMAILBOXMAILDIR_H

// Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <qlist.h>

// Local includes
#include "rmm/Message.h"
#include "EmpathDefines.h"
#include "EmpathMaildir.h"
#include "EmpathMailbox.h"

/**
 * @short Maildir mailbox
 * @author Rikkus
 */
class EmpathMailboxMaildir : public EmpathMailbox
{
    Q_OBJECT

    public:

        EmpathMailboxMaildir(const QString & name);
        QString writeNewMail(RMM::Message & message);

        virtual ~EmpathMailboxMaildir();

        /**
         * Set the path to the Maildir.
         */
        void setPath(const QString & path);

        /**
         * @return The path to the Maildir.
         */
        const QString & path() const { return path_; }

#include "EmpathMailboxAbstract.h"

    private:

        bool                    _recursiveRemove(const QString &);
        void                    _recursiveReadFolders(const QString &);
        EmpathMaildir *         _box(const EmpathURL & id);
        void                    _setupDefaultFolders();

        QString                 path_;
        QString                 canonName_;
        EmpathMaildirList       boxList_;
        bool                    initialised_;
};

#endif

// vim:ts=4:sw=4:tw=78
