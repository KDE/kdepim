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
#include "EmpathJobInfo.h"
#include "EmpathDefines.h"
#include "Empath.h"

EmpathJobInfo::EmpathJobInfo()
{
    haveOriginal_ = false;
}

EmpathJobInfo::EmpathJobInfo(const EmpathJobInfo & ji)
{
    original_.type           = ji.original_.type;
    original_.url1           = ji.original_.url1;
    original_.url2           = ji.original_.url2;
    original_.xinfo          = ji.original_.xinfo;
    original_.IDList         = ji.original_.IDList;
    original_.messageID      = ji.original_.messageID;
    original_.message        = ji.original_.message;
    original_.successMap     = ji.original_.successMap;
    original_.status         = ji.original_.status;
    original_.generalSuccess = ji.original_.generalSuccess;

    current_.type           = ji.current_.type;
    current_.url1           = ji.current_.url1;
    current_.url2           = ji.current_.url2;
    current_.xinfo          = ji.current_.xinfo;
    current_.IDList         = ji.current_.IDList;
    current_.messageID      = ji.current_.messageID;
    current_.message        = ji.current_.message;
    current_.successMap     = ji.current_.successMap;
    current_.status         = ji.current_.status;
    current_.generalSuccess = ji.current_.generalSuccess;

    haveOriginal_ = ji.haveOriginal_; 
}

EmpathJobInfo::EmpathJobInfo(EmpathJobInfoPrivate & jip)
{
    current_.type           = jip.type;
    current_.url1           = jip.url1;
    current_.url2           = jip.url2;
    current_.xinfo          = jip.xinfo;
    current_.IDList         = jip.IDList;
    current_.messageID      = jip.messageID;
    current_.message        = jip.message;
    current_.successMap     = jip.successMap;
    current_.status         = jip.status;
    current_.generalSuccess = jip.generalSuccess;
}

// Copy / Move message
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL & from,
    const EmpathURL & to,
    const QString & extraInfo)
{
    current_.type   = t;
    current_.url1   = from;
    current_.url2   = to;
    current_.xinfo  = extraInfo;

    haveOriginal_ = false;
}

// Write message
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL & url,
    RMM::Message & msg,
    const QString & extraInfo)
{
    current_.type       = t;
    current_.url1       = url;
    current_.xinfo      = extraInfo;
    current_.message    = msg;

    haveOriginal_ = false;
}

// Remove list of messages
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL & folder,
    const QStringList & messageIDList,
    const QString & extraInfo) 
{
    current_.type       = t;
    current_.url1       = folder;
    current_.xinfo      = extraInfo;
    current_.IDList     = messageIDList;

    haveOriginal_ = false;
}

// Mark list of messages
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL & folder,
    const QStringList & messageIDList,
    RMM::MessageStatus status,
    const QString & extraInfo) 
{
    current_.type   = t;
    current_.url1   = folder;
    current_.xinfo  = extraInfo;
    current_.IDList = messageIDList;
    current_.status = status;

    if (current_.IDList.isEmpty())
        current_.IDList << folder.messageID();

    haveOriginal_ = false;
}

// Mark message
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL url,
    RMM::MessageStatus status,
    const QString & extraInfo = QString::null)
{
    current_.type   = t;
    current_.url1   = url;
    current_.xinfo  = extraInfo;
    current_.status = status;

    current_.IDList << url.messageID();

    haveOriginal_ = false;
}

// Create / Remove folder
// Retrieve / Remove message
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL folder,
    const QString & extraInfo)
{
    current_.type   = t;
    current_.url1   = folder;
    current_.xinfo  = extraInfo;

    if (current_.IDList.isEmpty())
        current_.IDList << folder.messageID();

    haveOriginal_ = false;
}

EmpathJobInfo::~EmpathJobInfo()
{
}

    bool
EmpathJobInfo::success()
{
    return current_.generalSuccess;
}

    ActionType
EmpathJobInfo::type()
{
    return current_.type;
}

    EmpathURL
EmpathJobInfo::folder()
{
    return current_.url1;
}

    EmpathURL
EmpathJobInfo::from()
{
    return current_.url1;
}

    EmpathURL
EmpathJobInfo::to()
{
    return current_.url2;
}

    QString
EmpathJobInfo::xinfo()
{
    return current_.xinfo;
}

    QStringList
EmpathJobInfo::IDList()
{
    return current_.IDList;
}

    RMM::Message
EmpathJobInfo::message()
{
    return current_.message;
}

    QString
EmpathJobInfo::messageID()
{
    return current_.messageID;
}

    RMM::MessageStatus
EmpathJobInfo::status()
{ 
    return current_.status;
}

    void
EmpathJobInfo::setType(ActionType t)
{
    current_.type = t;
}

    void
EmpathJobInfo::setMessageID(const QString & s)
{
    current_.messageID = s;

    if (current_.IDList.isEmpty())
        current_.IDList << s;
}

    void
EmpathJobInfo::setMessage(RMM::Message & m)
{
    current_.message = m;
}

    bool
EmpathJobInfo::success(const QString & id)
{
    return current_.successMap[id];
}

    void
EmpathJobInfo::setSuccess(const QString & id, bool b)
{
    if (id.isEmpty()) {
      empathDebug("Can't set success for empty id !");
      return;
    }

    current_.successMap[id] = b;
}

    void
EmpathJobInfo::setSuccessMap(QMap<QString, bool> map)
{
    current_.successMap = map;
}

    void
EmpathJobInfo::done(bool ok)
{
    current_.generalSuccess = ok;
    empath->jobFinished(*this);
}

    void
EmpathJobInfo::setOriginal(EmpathJobInfo & ji)
{
    haveOriginal_ = true;
    original_.type           = ji.current_.type;
    original_.url1           = ji.current_.url1;
    original_.url2           = ji.current_.url2;
    original_.xinfo          = ji.current_.xinfo;
    original_.IDList         = ji.current_.IDList;
    original_.messageID      = ji.current_.messageID;
    original_.message        = ji.current_.message;
    original_.successMap     = ji.current_.successMap;
    original_.status         = ji.current_.status;
    original_.generalSuccess = ji.current_.generalSuccess;
}

    EmpathJobInfo
EmpathJobInfo::original()
{
    EmpathJobInfo copyMyOriginal(original_);
    return copyMyOriginal;
}

    bool
EmpathJobInfo::haveOriginal() const
{
    return haveOriginal_;
}

    ActionType
EmpathJobInfo::nextActionRequired()
{
    ActionType t(NoAction);

    if ((current_.type == RetrieveMessage) &&
        ((original_.type == CopyMessage) || (original_.type == MoveMessage)))
        t = WriteMessage;

    else if ((current_.type == WriteMessage) && (original_.type == MoveMessage))
        t = RemoveMessage;

    return t;
}

// vim:ts=4:sw=4:tw=78
