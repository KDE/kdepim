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

// Local includes
#include "EmpathJobScheduler.h"

#include "config.h"

EmpathJobScheduler::EmpathJobScheduler()
{
    // Empty.
}

EmpathJobScheduler::~EmpathJobScheduler()
{
    // Empty.
}

    EmpathJobID
EmpathJobScheduler::newWriteJob(
    RMM::RMessage & message,
    const EmpathURL & url,
    QObject * o,
    const char * slot
)
{
    EmpathWriteJob * j = new EmpathWriteJob(message, url);

    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathWriteJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathWriteJob)),
                o,  SLOT(s_writeJobFinished(EmpathWriteJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newRetrieveJob(
    const EmpathURL & url,
    QObject * o,
    const char * slot
)
{
    empathDebug("url == " + url.asString());
    EmpathRetrieveJob * j = new EmpathRetrieveJob(url);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathRetrieveJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathRetrieveJob)),
                o,  SLOT(s_retrieveJobFinished(EmpathRetrieveJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newCopyJob(
    const EmpathURL & source,
    const EmpathURL & destination,
    QObject * o,
    const char * slot
)
{
    EmpathCopyJob * j = new EmpathCopyJob(source, destination);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathCopyJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathCopyJob)),
                o,  SLOT(s_copyJobFinished(EmpathCopyJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newMoveJob(
    const EmpathURL & source,
    const EmpathURL & destination,
    QObject * o,
    const char * slot
)
{
    EmpathMoveJob * j = new EmpathMoveJob(source, destination);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathMoveJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathMoveJob)),
                o,  SLOT(s_moveJobFinished(EmpathMoveJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newRemoveJob(
    const EmpathURL & url,
    QObject * o,
    const char * slot
)
{
    EmpathRemoveJob * j = new EmpathRemoveJob(url);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathRemoveJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathRemoveJob)),
                o,  SLOT(s_removeJobFinished(EmpathRemoveJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newRemoveJob(
    const EmpathURL & folder,
    const QStringList & idList,
    QObject * o,
    const char * slot
)
{
    EmpathRemoveJob * j = new EmpathRemoveJob(folder, idList);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathRemoveJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathRemoveJob)),
                o,  SLOT(s_removeJobFinished(EmpathRemoveJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newMarkJob(
    const EmpathURL & url,
    EmpathIndexRecord::Status status,
    QObject * o,
    const char * slot
)
{
    EmpathMarkJob * j = new EmpathMarkJob(url, status);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathMarkJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathMarkJob)),
                o,  SLOT(s_markJobFinished(EmpathMarkJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newMarkJob(
    const EmpathURL & folder,
    const QStringList & idList,
    EmpathIndexRecord::Status status,
    QObject * o,
    const char * slot
)
{
    EmpathMarkJob * j = new EmpathMarkJob(folder, idList, status);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathMarkJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathMarkJob)),
                o,  SLOT(s_markJobFinished(EmpathMarkJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newCreateFolderJob(
    const EmpathURL & url,
    QObject * o,
    const char * slot
)
{
    EmpathCreateFolderJob * j = new EmpathCreateFolderJob(url);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathCreateFolderJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathCreateFolderJob)),
                o,  SLOT(s_createFolderJobFinished(EmpathCreateFolderJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::newRemoveFolderJob(
    const EmpathURL & url,
    QObject * o,
    const char * slot
)
{
    EmpathRemoveFolderJob * j = new EmpathRemoveFolderJob(url);
    
    if (0 != o)
        if (0 != slot)
            QObject::connect(
                j,  SIGNAL(done(EmpathRemoveFolderJob)),
                o,  slot);
        else
            QObject::connect(
                j,  SIGNAL(done(EmpathRemoveFolderJob)),
                o,  SLOT(s_removeFolderJobFinished(EmpathRemoveFolderJob)));

    _enqueue(j);

    return j->id();
}

    EmpathJobID
EmpathJobScheduler::_enqueue(EmpathJob * j)
{
    queue_.enqueue(j);
    startTimer(0);
}

    void
EmpathJobScheduler::timerEvent(QTimerEvent *)
{
    killTimers();
    _runQueue();
}

    void
EmpathJobScheduler::_runQueue()
{
    if (!queue_.isEmpty()) {

        EmpathJob * j = queue_.dequeue();

        if (!j->finished())
            j->run();

        else {

            delete j;
            j = 0;
            _runQueue();
        }
    }
}

// vim:ts=4:sw=4:tw=78
