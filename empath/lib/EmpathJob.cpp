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
#include "EmpathJob.h"
#include "EmpathDefines.h"
#include "Empath.h"

EmpathJob::EmpathJob()
{
}

    EmpathJob *
EmpathJob::copy(
    const EmpathURL & from,
    const EmpathURL & to,
    const QString & extra)
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(CopyMessage);
    j->setSourceFolder(from);
    j->setDestinationFolder(to);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::move(
    const EmpathURL & from,
    const EmpathURL & to,
    const QString & extra)
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(CopyMessage);
    j->setSourceFolder(from);
    j->setDestinationFolder(to);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::write(
    RMM::RMessage & msg,
    const EmpathURL & to,
    const QString & extra)
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(WriteMessage);
    j->setDestinationFolder(to);
    j->setMessage(msg);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::remove(
    const EmpathURL & folder,
    const QStringList & messageIDList,
    const QString & extra) 
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(RemoveMessage);
    j->setFolder(folder);
    j->setMessageIDList(messageIDList);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::mark(
    const EmpathURL & folder,
    const QStringList & messageIDList,
    RMM::MessageStatus status,
    const QString & extra) 
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(MarkMessage);
    j->setFolder(folder);
    j->setMessageIDList(messageIDList);
    j->setMessageStatus(status);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::mark(
    const EmpathURL url,
    RMM::MessageStatus status,
    const QString & extra)
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(MarkMessage);
    j->setFolder(url);
    j->setMessageIDList(url.messageID());
    j->setMessageStatus(status);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::createFolder(const EmpathURL folder, const QString & extra)
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(CreateFolder);
    j->setFolder(folder);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::removeFolder(const EmpathURL & url, const QString & extraInfo)
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(RemoveFolder);
    j->setFolder(folder);
    j->setExtraInfo(extra);

    _enqueue(j);
}

    EmpathJob *
EmpathJob::retrieve(const EmpathURL & url, const QString & extraInfo)
{
    EmpathJobPrivate * j = new EmpathJob;

    j->setType(RetrieveMessage);
    j->setMessageURL(url);
    j->setExtraInfo(extra);

    _enqueue(j);
}

EmpathJob::~EmpathJob()
{
}

    ActionType
EmpathJobPrivate::type()
{
    return type_;
}

    EmpathURL
EmpathJobPrivate::folder()
{
    return folder_;
}

    EmpathURL
EmpathJobPrivate::sourceFolder()
{
    return sourceFolder_;
}

    EmpathURL
EmpathJobPrivate::destinationFolder()
{
    return destinationFolder_;
}

    QString
EmpathJobPrivate::xinfo()
{
    return xinfo_;
}

    QStringList
EmpathJobPrivate::IDList()
{
    return current_.IDList;
}

    RMM::RMessage
EmpathJobPrivate::message()
{
    return message_;
}

    QString
EmpathJob::messageID()
{
    return current_.messageID;
}

    RMM::MessageStatus
EmpathJob::status()
{ 
    return current_.status;
}

    void
EmpathJob::setType(ActionType t)
{
    current_.type = t;
}

    void
EmpathJob::setMessageID(const QString & s)
{
    current_.messageID = s;

    if (current_.IDList.isEmpty())
        current_.IDList << s;
}

    void
EmpathJob::setMessage(RMM::RMessage & m)
{
    current_.message = m;
}

    bool
EmpathJob::success(const QString & id)
{
    return current_.successMap[id];
}

    void
EmpathJob::setSuccess(const QString & id, bool b)
{
    if (id.isEmpty()) {
      empathDebug("Can't set success for empty id !");
      return;
    }

    current_.successMap[id] = b;
}

    void
EmpathJob::setSuccessMap(QMap<QString, bool> map)
{
    current_.successMap = map;
}

    void
EmpathJob::done(bool ok)
{
    current_.generalSuccess = ok;
    empath->jobFinished(*this);
}

    EmpathJob
EmpathJob::original()
{
    EmpathJob copyMyOriginal(original_);
    return copyMyOriginal;
}

    void
EmpathJob::_queue(EmpathJobPrivate * j)
{
    subJobQueue_.enqueue(j);

    if (!subJobQueue_.count() == 1)
        subJobQueue_->head()->run();
}

#if 0
EmpathJob::nextActionRequired()
{
    ActionType t(NoAction);

    if ((current_.type == RetrieveMessage) &&
        ((original_.type == CopyMessage) || (original_.type == MoveMessage)))
        t = WriteMessage;

    else if ((current_.type == WriteMessage) && (original_.type == MoveMessage))
        t = RemoveMessage;

    return t;
}
#endif

// vim:ts=4:sw=4:tw=78
