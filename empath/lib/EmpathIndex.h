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


#ifndef EMPATHINDEX_H
#define EMPATHINDEX_H

// Qt includes
#include <qdict.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qthread.h>

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathURL.h"

/**
 * Dictionary of index records
 * @author Rikkus
 */
class EmpathIndex : public QObject
{
    Q_OBJECT

    public:

        EmpathIndex(const EmpathURL & folder);

        ~EmpathIndex();

        /**
         * Get the index record using the given key.
         */
        EmpathIndexRecord record(const QString & key);

        /**
         * Find out if the record exists
         */
        bool contains(const QString & key);

        /**
         * Clear out.
         */
        void clear();

        /**
         * Count the number of messages stored.
         */
        unsigned int count();

        /**
         * Count the number of unread messages stored.
         */
        unsigned int countUnread();

        /**
         * Insert entry.
         */
        bool insert(const QString &, EmpathIndexRecord &);

        /**
         * Insert entry.
         */
        bool replace(const QString &, EmpathIndexRecord &);
		
        /**
         * Remove entry.
         */
        bool remove(const QString &);

        /**
         * @return URL of the related folder.
         */
        const EmpathURL & folder() const { return folder_; }
		
        QString indexFileName() const { return filename_; }
		
        QStringList allKeys();
        QDict<EmpathIndexRecord> dict();

        bool setStatus(const QString & id, EmpathIndexRecord::Status);

        QDateTime lastSync() const;

    protected:

        void timerEvent(QTimerEvent *);

    signals:

        void itemGone(const QString &);
        void statusChange(const QString &, EmpathIndexRecord::Status);
        void countUpdated(unsigned int c, unsigned int uc);

    private:

        EmpathIndex();

        bool _read();
        bool _write();
        void _flush();
        void _resetIdleTimer();
        void _setDirty();

        QString filename_;
        QDict<EmpathIndexRecord> dict_;

        // Order dependency
        EmpathURL folder_;
        bool initialised_;
        bool dirty_;
        bool read_;
        unsigned int count_;
        unsigned int unreadCount_;
        // End order dependency

        QMutex mutex_;
};

#endif

// vim:ts=4:sw=4:tw=78
