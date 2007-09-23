/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KMOBILETOOLSJOBMANAGER_H
#define KMOBILETOOLSJOBMANAGER_H

#include <QtCore/QObject>

#include "kmobiletools_export.h"
#include <libkmobiletools/jobxp.h>

namespace KMobileTools {

class JobManagerPrivate;
/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT JobManager : public QObject {
    Q_OBJECT
friend class JobManagerInstance;
public:
    /**
     * Returns a JobManager instance
     *
     * @return a job manager instance
     */
    static JobManager* instance();

    void enqueueJob( const QString& deviceName, JobXP* job );
    void dequeueJob( JobXP* job );

    ~JobManager();

private Q_SLOTS:
    void emitJobStarted( ThreadWeaver::Job* job );
    void emitJobDone( ThreadWeaver::Job* job );
    void emitJobFailed( ThreadWeaver::Job* job );

Q_SIGNALS:
    void jobEnqueued( const QString& deviceName, JobXP* job );
    void jobDequeued( const QString& deviceName, JobXP* job );
    void jobStarted( const QString& deviceName, JobXP* job );

    void jobDone( const QString& deviceName, JobXP* job );
    void jobFailed( const QString& deviceName, JobXP* job );

private:
    JobManager();
    JobManagerPrivate* const d;


};

}

#endif
