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
#include "EmpathCustomEvents.h"
#include "EmpathDefines.h"
#include "Empath.h"
#include "EmpathJob.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathIndex.h"

int EmpathJob::ID = 0;

EmpathJob::EmpathJob(QObject * parent, int tag, ActionType t)
    :   parent_(parent),
        tag_(tag),
        type_(t),
        id_(ID++)
{
}

// ------------------------------------------------------------------------

EmpathReadIndexJob::~EmpathReadIndexJob()
{
    // Empty.
}

EmpathWriteJob::~EmpathWriteJob()
{
    // Empty.
}

EmpathCopyJob::~EmpathCopyJob()
{
    // Empty.
}

EmpathMoveJob::~EmpathMoveJob()
{
    // Empty.
}

EmpathRemoveJob::~EmpathRemoveJob()
{
    // Empty.
}

EmpathRetrieveJob::~EmpathRetrieveJob()
{
    // Empty.
}

EmpathMarkJob::~EmpathMarkJob()
{
    // Empty.
}

EmpathCreateFolderJob::~EmpathCreateFolderJob()
{
    // Empty.
}

EmpathRemoveFolderJob::~EmpathRemoveFolderJob()
{
    // Empty.
}

// ------------------------------------------------------------------------

EmpathReadIndexJob::EmpathReadIndexJob(
    QObject * parent,
    int i,
    const EmpathURL & folder
)
    :
    EmpathJob(parent, i, ReadIndex),
    folder_(folder)
{
    // Empty.
}

EmpathWriteJob::EmpathWriteJob(
    QObject * parent,
    int i,
    RMM::Message & message,
    const EmpathURL & folder
)
    :
    EmpathJob(parent, i, RetrieveMessage),
    message_(message),
    folder_(folder)
{
    // Empty.
}

EmpathCopyJob::EmpathCopyJob(
    QObject * parent,
    int i,
    const EmpathURL & source,
    const EmpathURL & destination
)
    :
    EmpathJob(parent, i, CopyMessage),
    source_(source),
    destination_(destination)
{
    // Empty.
}

EmpathMoveJob::EmpathMoveJob(
    QObject * parent,
    int i,
    const EmpathURL & source,
    const EmpathURL & destination
)
    :
    EmpathJob(parent, i, MoveMessage),
    source_(source),
    destination_(destination)
{
    // Empty.
}

EmpathRemoveJob::EmpathRemoveJob(
    QObject * parent,
    int i,
    const EmpathURL & folder,
    const QStringList & IDList
)
    :
    EmpathJob(parent, i, RemoveMessage),
    folder_(folder),
    IDList_(IDList)
{
    // Empty.
}

EmpathRemoveJob::EmpathRemoveJob(
    QObject * parent,
    int i,
    const EmpathURL & url
)
    :
    EmpathJob(parent, i, RemoveMessage),
    url_(url)
{
    // Empty.
}

EmpathRetrieveJob::EmpathRetrieveJob(
    QObject * parent,
    int i,
    const EmpathURL & url
)
    :
    EmpathJob(parent, i, RetrieveMessage),
    url_(url)
{
    // Empty.
}

EmpathMarkJob::EmpathMarkJob(
    QObject * parent,
    int i,
    const EmpathURL & folder,
    const QStringList & IDList,
    EmpathIndexRecord::Status flags
)
    :
    EmpathJob(parent, i, MarkMessage),
    folder_(folder),
    IDList_(IDList),
    flags_(flags)
{
    // Empty.
}

EmpathMarkJob::EmpathMarkJob(
    QObject * parent,
    int i,
    const EmpathURL & url,
    EmpathIndexRecord::Status flags
)
    :
    EmpathJob(parent, i, MarkMessage),
    url_(url),
    flags_(flags)
{
    // Empty.
}

EmpathCreateFolderJob::EmpathCreateFolderJob(
    QObject * parent,
    int i,
    const EmpathURL & folder
)
    :
    EmpathJob(parent, i, CreateFolder),
    folder_(folder)
{
    // Empty.
}

EmpathRemoveFolderJob::EmpathRemoveFolderJob(
    QObject * parent,
    int i,
    const EmpathURL & folder
)
    :
    EmpathJob(parent, i, RemoveFolder),
    folder_(folder)
{
    // Empty.
}

// ------------------------------------------------------------------------

void EmpathReadIndexJob::run()
{
    empathDebug("");
    EmpathMailbox * m = empath->mailbox(folder_);

    if (0 != m) {
        index_ = m->index(folder_);
        if (0 != index_) {
            setSuccess(true);
            empathDebug("Got index from my mailbox.");
        } else {
            empathDebug("Couldn't get index from my mailbox.");
        }
    } else {
        empathDebug("Couldn't find my mailbox !");
    }

    empathDebug("posting event");

    if (parent())
        postEvent(
                parent(), new EmpathIndexReadEvent(index_, folder_, success()));

    empathDebug("finished");
}

void EmpathWriteJob::run()
{
    EmpathMailbox * m = empath->mailbox(folder_);

    if (0 != m) {
        messageID_ = m->writeMessage(message_, folder_);
        setSuccess(!messageID_.isNull());
    }

    if (parent())
        postEvent(parent(), new EmpathMessageWrittenEvent(messageID_, message_, folder_, success()));
}

void EmpathCopyJob::run()
{
    EmpathRetrieveJob retrieve(parent(), 0, source_);

    retrieve.run();

    while (retrieve.running())
        msleep(10);

    if (retrieve.success()) {

        RMM::Message msg = retrieve.message();

        EmpathWriteJob write(parent(), 0, msg, destination_);

        write.run();

        while (write.running())
            msleep(10);

        setSuccess(write.success());
    }

    if (parent())
        postEvent(parent(),
                new EmpathMessageCopiedEvent(source_, destination_, success()));
}

void EmpathMoveJob::run()
{
    EmpathRetrieveJob retrieve(parent(), 0, source_);

    retrieve.run();

    while (retrieve.running())
        msleep(10);

    if (retrieve.success()) {

        RMM::Message msg = retrieve.message();

        EmpathWriteJob write(parent(), 0, msg, destination_);

        write.run();

        if (write.success()) {

            EmpathRemoveJob remove(parent(), 0, source_);

            remove.run();

            while (remove.running())
                msleep(10);

            setSuccess(remove.success());
        }
    }

    if (parent())
        postEvent(parent(),
                new EmpathMessageMovedEvent(source_, destination_, success()));
}

void EmpathRemoveJob::run()
{
    setSuccess(false);
}

void EmpathRetrieveJob::run()
{
    empathDebug(url_.asString());
    RMM::Message cached = empath->message(url_);

    if (!cached.isNull()) {

        message_ = cached;
        setSuccess(true);

    } else {

        EmpathMailbox * m = empath->mailbox(url_);

        if (0 != m) {

            message_ = m->retrieveMessage(url_);

            if (message_.isNull()) {
                empathDebug("message is null");
            }

            setSuccess(!(message_.isNull()));

            if (success())
                empath->cacheMessage(url_, message_);
        } else {
            empathDebug("!m !");
        }
    }

    if (success()) {
        empathDebug("success");
    }
    else {
        empathDebug("fail");
    }

    if (parent())
        postEvent(parent(),
                new EmpathMessageRetrievedEvent(url_, message_, success()));
}

void EmpathMarkJob::run()
{
    setSuccess(false);
}

void EmpathCreateFolderJob::run()
{
    EmpathMailbox * m = empath->mailbox(folder_);

    if (0 != m)
        setSuccess(m->createFolder(folder_));

    if (parent())
        postEvent(parent(), new EmpathFolderCreatedEvent(folder_, success()));

}

void EmpathRemoveFolderJob::run()
{
    EmpathMailbox * m = empath->mailbox(folder_);

    if (0 != m)
        setSuccess(m->removeFolder(folder_));

    if (parent())
        postEvent(parent(), new EmpathFolderRemovedEvent(folder_, success()));
}

// vim:ts=4:sw=4:tw=78
