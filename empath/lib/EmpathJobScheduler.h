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

#ifndef EMPATH_JOB_SCHEDULER_H
#define EMPATH_JOB_SCHEDULER_H

// Qt includes
#include <qobject.h>
#include <qqueue.h>

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathDefines.h"
#include "EmpathJob.h"

class EmpathJobScheduler : public QObject
{
    Q_OBJECT

    public:

        EmpathJobScheduler();

        virtual ~EmpathJobScheduler();

        EmpathJobID newWriteJob(
            RMM::RMessage &,
            const EmpathURL &,
            QObject *,
            const char *
        );

        EmpathJobID newRetrieveJob(
            const EmpathURL &,
            QObject *,
            const char *
        );

        EmpathJobID newCopyJob(
            const EmpathURL &,
            const EmpathURL &,
            QObject *,
            const char *
        );

        EmpathJobID newMoveJob(
            const EmpathURL &,
            const EmpathURL &,
            QObject *,
            const char *
        );

        EmpathJobID newRemoveJob(
            const EmpathURL &,
            QObject *,
            const char *
        );

        EmpathJobID newRemoveJob(
            const EmpathURL &,
            const QStringList &,
            QObject *,
            const char *
        );

        EmpathJobID newMarkJob(
            const EmpathURL &,
            EmpathIndexRecord::Status,
            QObject *,
            const char *
        );

        EmpathJobID newMarkJob(
            const EmpathURL &,
            const QStringList &,
            EmpathIndexRecord::Status,
            QObject *,
            const char *
        );

        EmpathJobID newCreateFolderJob(
            const EmpathURL &,
            QObject *,
            const char *
        );

        EmpathJobID newRemoveFolderJob(
            const EmpathURL &,
            QObject *,
            const char *
        );

    protected:

        void timerEvent(QTimerEvent *);

    private:

        EmpathJobID _enqueue(EmpathJob *);

        void _runQueue();

        QQueue<EmpathJob> queue_;
};

#endif
// vim:ts=4:sw=4:tw=78
