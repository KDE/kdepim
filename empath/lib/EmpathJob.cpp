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
#include "EmpathDefines.h"
#include "Empath.h"
#include "EmpathJob.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"

EmpathJobID EmpathJob::ID_ = 0;

EmpathJob::EmpathJob(ActionType t)
    :
#ifdef USE_QPTHREAD
    QpThread(),
#endif
    id_(ID_++),
    type_(t),
    success_(false),
    finished_(false)
{
    // Empty.
}

EmpathJob::EmpathJob(const EmpathJob & other)
    :
#ifdef USE_QPTHREAD
    QpThread(other),
#endif
    id_(other.id_),
    type_(other.type_),
    success_(other.success_),
    successMap_(other.successMap_)
{
    // Empty.
}

// ------------------------------------------------------------------------

EmpathWriteJob::EmpathWriteJob(const EmpathWriteJob & j)
    :
    EmpathSingleJob(j),
    message_    (j.message_),
    folder_     (j.folder_),
    messageID_  (j.messageID_)
{
    // Empty.
}

EmpathCopyJob::EmpathCopyJob(const EmpathCopyJob & j)
    :
    EmpathMultiJob(j),
    source_         (j.source_),
    destination_    (j.destination_)
{
    // Empty.
}

EmpathMoveJob::EmpathMoveJob(const EmpathMoveJob & j)
    :
    EmpathMultiJob(j),
    source_         (j.source_),
    destination_    (j.destination_)
{
    // Empty.
}

EmpathRemoveJob::EmpathRemoveJob(const EmpathRemoveJob & j)
    :
    EmpathSingleJob(j),
    successMap_ (j.successMap_),
    url_        (j.url_),
    folder_     (j.folder_),
    IDList_     (j.IDList_)
{
    // Empty.
}

EmpathRetrieveJob::EmpathRetrieveJob(const EmpathRetrieveJob & j)
    :
    EmpathSingleJob(j),
    url_        (j.url_),
    message_    (j.message_)
{
    // Empty.
}

EmpathMarkJob::EmpathMarkJob(const EmpathMarkJob & j)
    :
    EmpathSingleJob(j),
    successMap_ (j.successMap_),
    url_        (j.url_),
    folder_     (j.folder_),
    IDList_     (j.IDList_),
    flags_      (j.flags_)
{
    // Empty.
}

EmpathCreateFolderJob::EmpathCreateFolderJob(const EmpathCreateFolderJob & j)
    :
    EmpathSingleJob(j),
    folder_(j.folder_)
{
    // Empty.
}

EmpathRemoveFolderJob::EmpathRemoveFolderJob(const EmpathRemoveFolderJob & j)
    :
    EmpathSingleJob(j),
    folder_(j.folder_)
{
    // Empty.
}

// ------------------------------------------------------------------------

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

EmpathWriteJob::EmpathWriteJob(
    RMM::RMessage & message,
    const EmpathURL & folder
)
    :
    EmpathSingleJob(WriteMessage),
    message_(message),
    folder_(folder)
{
    // Empty.
}

EmpathCopyJob::EmpathCopyJob(
    const EmpathURL & source,
    const EmpathURL & destination
)
    :
    EmpathMultiJob(CopyMessage),
    source_(source),
    destination_(destination)
{
    // Empty.
}

EmpathMoveJob::EmpathMoveJob(
    const EmpathURL & source,
    const EmpathURL & destination
)
    :
    EmpathMultiJob(MoveMessage),
    source_(source),
    destination_(destination)
{
    // Empty.
}

EmpathRemoveJob::EmpathRemoveJob(
    const EmpathURL & folder,
    const QStringList & IDList
)
    :
    EmpathSingleJob(RemoveMessage),
    folder_(folder),
    IDList_(IDList)
{
    // Empty.
}

EmpathRemoveJob::EmpathRemoveJob(
    const EmpathURL & url
)
    :
    EmpathSingleJob(RemoveMessage),
    url_(url)
{
    // Empty.
}

EmpathRetrieveJob::EmpathRetrieveJob(
    const EmpathURL & url
)
    :
    EmpathSingleJob(RetrieveMessage),
    url_(url)
{
    // Empty.
}

EmpathMarkJob::EmpathMarkJob(
    const EmpathURL & folder,
    const QStringList & IDList,
    EmpathIndexRecord::Status flags
)
    :
    EmpathSingleJob(MarkMessage),
    folder_(folder),
    IDList_(IDList),
    flags_(flags)
{
    // Empty.
}

EmpathMarkJob::EmpathMarkJob(
    const EmpathURL & url,
    EmpathIndexRecord::Status flags
)
    :
    EmpathSingleJob(MarkMessage),
    url_(url),
    flags_(flags)
{
    // Empty.
}

EmpathCreateFolderJob::EmpathCreateFolderJob(
    const EmpathURL & folder
)
    :
    EmpathSingleJob(CreateFolder),
    folder_(folder)
{
    // Empty.
}

EmpathRemoveFolderJob::EmpathRemoveFolderJob(
    const EmpathURL & folder
)
    :
    EmpathSingleJob(CreateFolder),
    folder_(folder)
{
    // Empty.
}

// ------------------------------------------------------------------------

void EmpathWriteJob::run()
{
    EmpathFolder * f = empath->folder(folder_);

    if (!f) {
        setSuccess(false);
        _done();
        return;
    }

    messageID_ = f->writeMessage(message_);
    setSuccess(!messageID_.isNull());
    _done();
}

void EmpathCopyJob::run()
{
    EmpathRetrieveJob retrieve(source_);
    retrieve.run();

    if (!retrieve.success()) {
        setSuccess(false);
        _done();
        return;
    }

    RMM::RMessage msg = retrieve.message();
    EmpathWriteJob write(msg, destination_);
    write.run();

    setSuccess(write.success());
    _done();
}

void EmpathMoveJob::run()
{
    EmpathRetrieveJob retrieve(source_);
    retrieve.run();

    if (!retrieve.success()) {
        setSuccess(false);
        _done();
        return;
    }

    RMM::RMessage msg = retrieve.message();

    EmpathWriteJob write(msg, destination_);
    write.run();

    if (!write.success()) {
        setSuccess(false);
        _done();
        return;
    }

    EmpathRemoveJob remove(source_);
    remove.run();

    setSuccess(remove.success());
    _done();
}

void EmpathRemoveJob::run()
{
    if (IDList_.isEmpty()) {

        EmpathFolder * f = empath->folder(url_);

        if (0 == f) {
            empathDebug("Folder `" + url_.asString() + "' does not exist ?");
            setSuccess(false);
            _done();
            return;
        }

        setSuccess(f->removeMessage(url_.messageID()));

    } else {

        EmpathFolder * f = empath->folder(folder_);

        if (0 == f) {
            empathDebug("Folder `" + url_.asString() + "' does not exist ?");
            setSuccess(false);
            _done();
            return;
        }

        setSuccessMap(f->removeMessage(IDList_));
    }

    _done();
}

void EmpathRetrieveJob::run()
{
    empathDebug("");
    RMM::RMessage cached = empath->message(url_);

    if (!cached.isNull()) {

        message_ = cached;
        setSuccess(true);
        _done();
        return;
    }

    EmpathFolder * f = empath->folder(url_);

    if (0 == f) {
        empathDebug("Folder `" + url_.asString() + "' does not exist ?");
        setSuccess(false);
        _done();
        return;
    }

    message_ = f->retrieveMessage(url_.messageID());

    if (!message_.isNull())
        empath->cacheMessage(url_, message_);

    setSuccess(!message_.isNull());
    _done();
    empathDebug("done");
}

void EmpathMarkJob::run()
{
    if (IDList_.isEmpty()) {

        EmpathFolder * f = empath->folder(url_);
    
        if (0 == f) {
            empathDebug("Folder `" + url_.asString() + "' does not exist ?");
            setSuccess(false);
            _done();
            return;
        }

       setSuccess(f->markMessage(url_.messageID(), flags_));

    } else {

        EmpathFolder * f = empath->folder(folder_);
     
        if (0 == f) {
            empathDebug("Folder `" + url_.asString() + "' does not exist ?");
            setSuccess(false);
            _done();
            return;
        }

       setSuccessMap(f->markMessage(IDList_, flags_));
    }
    
    _done();
}

void EmpathCreateFolderJob::run()
{
    EmpathMailbox * m = empath->mailbox(folder_);

    if (!m) {
      
        empathDebug("Mailbox `" + folder_.asString() + "' does not exist ?");
        setSuccess(false);
        _done();
        return;
    }

    setSuccess(m->createFolder(folder_));
    _done();
}

void EmpathRemoveFolderJob::run()
{
    EmpathMailbox * m = empath->mailbox(folder_);

    if (!m) {
        empathDebug("Mailbox `" + folder_.asString() + "' does not exist ?");
        setSuccess(false);
        _done();
        return;
    }

    setSuccess(m->removeFolder(folder_));
    _done();
}

// ------------------------------------------------------------------------

EmpathSingleJob::EmpathSingleJob(ActionType t) 
    : EmpathJob(t)
{
    // Empty.
}

EmpathSingleJob::EmpathSingleJob(const EmpathSingleJob & j)
    : EmpathJob(j)
{
    // Empty.
}


EmpathSingleJob::~EmpathSingleJob()
{
    // Empty.
}

// ------------------------------------------------------------------------

EmpathMultiJob::EmpathMultiJob(ActionType t) 
    : EmpathJob(t)
{
    // Empty.
}

EmpathMultiJob::EmpathMultiJob(const EmpathMultiJob & j)
    : EmpathJob(j)
{
    // Empty.
}


EmpathMultiJob::~EmpathMultiJob()
{
    // Empty.
}


// vim:ts=4:sw=4:tw=78
