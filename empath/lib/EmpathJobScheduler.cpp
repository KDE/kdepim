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
    along with handler program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Local includes
#include "EmpathJobScheduler.h"
#include "EmpathDefines.h"

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
EmpathJobScheduler::newReadIndexJob(
    const EmpathURL & url,
    QObject * handler,
    int i
)
{
    empathDebug("Creating new EmpathReadIndexJob for url " + url.asString());
    EmpathReadIndexJob * j = new EmpathReadIndexJob(handler, i, url);
    int id = j->id();
    j->run();
    return id;
}


    EmpathJobID
EmpathJobScheduler::newWriteJob(
    RMM::Message & message,
    const EmpathURL & url,
    QObject * handler,
    int i
)
{
    EmpathWriteJob * j = new EmpathWriteJob(handler, i, message, url);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newRetrieveJob(
    const EmpathURL & url,
    QObject * handler,
    int i
)
{
    EmpathRetrieveJob * j = new EmpathRetrieveJob(handler, i, url);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newCopyJob(
    const EmpathURL & source,
    const EmpathURL & destination,
    QObject * handler,
    int i
)
{
    EmpathCopyJob * j = new EmpathCopyJob(handler, i, source, destination);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newMoveJob(
    const EmpathURL & source,
    const EmpathURL & destination,
    QObject * handler,
    int i
)
{
    EmpathMoveJob * j = new EmpathMoveJob(handler, i, source, destination);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newRemoveJob(
    const EmpathURL & url,
    QObject * handler,
    int i
)
{
    EmpathRemoveJob * j = new EmpathRemoveJob(handler, i, url);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newRemoveJob(
    const EmpathURL & folder,
    const QStringList & idList,
    QObject * handler,
    int i
)
{
    EmpathRemoveJob * j = new EmpathRemoveJob(handler, i, folder, idList);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newMarkJob(
    const EmpathURL & url,
    EmpathIndexRecord::Status status,
    QObject * handler,
    int i
)
{
    EmpathMarkJob * j = new EmpathMarkJob(handler, i, url, status);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newMarkJob(
    const EmpathURL & folder,
    const QStringList & idList,
    EmpathIndexRecord::Status status,
    QObject * handler,
    int i
)
{
    EmpathMarkJob * j = new EmpathMarkJob(handler, i, folder, idList, status);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newCreateFolderJob(
    const EmpathURL & url,
    QObject * handler,
    int i
)
{
    EmpathCreateFolderJob * j = new EmpathCreateFolderJob(handler, i, url);
    int id = j->id();
    j->run();
    return id;
}

    EmpathJobID
EmpathJobScheduler::newRemoveFolderJob(
    const EmpathURL & url,
    QObject * handler,
    int i
)
{
    EmpathRemoveFolderJob * j = new EmpathRemoveFolderJob(handler, i, url);
    int id = j->id();
    j->run();
    return id;
}

    bool
EmpathJobScheduler::event(QEvent * e)
{
    return QObject::event(e);
}

// vim:ts=4:sw=4:tw=78
