/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
    // Empty.
}

// Copy / Move / Retrieve message
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL & from,
    const EmpathURL & to,
    const QString & internalExtraInfo,
    const QString & extraInfo)
    :   type_(t),
        url1_(from),
        url2_(to),
        ixinfo_(internalExtraInfo),
        xinfo_(extraInfo)
{
    // Empty.
}

// Write message
EmpathJobInfo::EmpathJobInfo(
    const EmpathURL & from,
    const EmpathURL & to,
    RMM::RMessage & msg,
    const QString & internalExtraInfo,
    const QString & extraInfo)
    :   type_(WriteMessage),
        url1_(from),
        url2_(to),
        ixinfo_(internalExtraInfo),
        xinfo_(extraInfo),
        message_(msg)
{
    // Empty.
}

// Remove messages
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL & folder,
    const QStringList & messageIDList,
    const QString & internalExtraInfo,
    const QString & extraInfo) 
    :   type_(t),
        url1_(folder),
        ixinfo_(internalExtraInfo),
        xinfo_(extraInfo),
        IDList_(messageIDList)
{
    // Empty.
}

// Mark messages
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL & folder,
    const QStringList & messageIDList,
    RMM::MessageStatus status,
    const QString & internalExtraInfo,
    const QString & extraInfo) 
    :   type_(t),
        url1_(folder),
        ixinfo_(internalExtraInfo),
        xinfo_(extraInfo),
        IDList_(messageIDList),
        status_(status)
{
    if (IDList_.isEmpty())
        IDList_ << folder.messageID();
}

// Create / Remove folder
// Retrieve message
EmpathJobInfo::EmpathJobInfo(
    ActionType t,
    const EmpathURL folder,
    const QString & internalExtraInfo,
    const QString & extraInfo)
    :   type_(t),
        url1_(folder),
        ixinfo_(internalExtraInfo),
        xinfo_(extraInfo)
{
    // Empty.
}

EmpathJobInfo::EmpathJobInfo(const EmpathJobInfo & j)
    :   type_       (j.type_),
        subType_    (j.subType_),
        url1_       (j.url1_),
        url2_       (j.url2_),
        ixinfo_     (j.ixinfo_),
        xinfo_      (j.xinfo_),
        IDList_     (j.IDList_),
        messageID_  (j.messageID_),
        message_    (j.message_),
        successMap_ (j.successMap_)
{
    // Empty.
}

    bool
EmpathJobInfo::success()
{ return generalSuccess_;   }

    ActionType
EmpathJobInfo::type()
{
    return type_;
}

    ActionType
EmpathJobInfo::subType()
{
    return subType_;
}

    EmpathURL
EmpathJobInfo::folder()
{
    return url1_;
}

    EmpathURL
EmpathJobInfo::from()
{
    return url1_;
}

    EmpathURL
EmpathJobInfo::to()
{
    return url2_;
}

    QString
    EmpathJobInfo::ixinfo()
{
    return ixinfo_;
}

    QString
EmpathJobInfo::xinfo()
{
    return xinfo_;
}

    QStringList
EmpathJobInfo::IDList()
{
    return IDList_;
}

    RMM::RMessage
EmpathJobInfo::message()
{
    return message_;
}

    QString
EmpathJobInfo::messageID()
{
    return messageID_;
}

    RMM::MessageStatus
EmpathJobInfo::status()
{ 
    return status_;
}

    void
EmpathJobInfo::setType(ActionType t)
{
    type_ = t;
}

    void
EmpathJobInfo::setSubType(ActionType t)
{
    subType_ = t; 
}

    void
EmpathJobInfo::setMessageID(const QString & s)
{
    messageID_ = s;
}

    bool
EmpathJobInfo::success(const QString & id)
{
    return successMap_[id];
}

    void
EmpathJobInfo::setSuccess(const QString & id, bool b)
{
    if (id.isEmpty()) {
      empathDebug("Can't set success for empty id !");
      return;
    }
    successMap_[id] = b;
}

    void
EmpathJobInfo::setSuccessMap(QMap<QString, bool> map)
{
    successMap_ = map;
}

    void
EmpathJobInfo::done(bool ok)
{
    generalSuccess_ = ok;
    empath->jobFinished(*this);
}

// vim:ts=4:sw=4:tw=78
