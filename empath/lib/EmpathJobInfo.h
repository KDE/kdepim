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

#ifndef EMPATH_JOB_INFO_H
#define EMPATH_JOB_INFO_H

// Qt includes
#include <qmap.h>
#include <qqueue.h>
#include <qstring.h>
#include <qstringlist.h>

// Local includes
#include <RMM_Message.h>
#include "EmpathEnum.h"
#include "EmpathURL.h"

class EmpathJobInfo
{
    public:

        EmpathJobInfo();

        // Copy / Move / Retrieve message
        EmpathJobInfo(
            ActionType t,
            const EmpathURL & from,
            const EmpathURL & to,
            const QString & internalExtraInfo,
            const QString & extraInfo);

        // Write message
        EmpathJobInfo(
            const EmpathURL & from,
            const EmpathURL & to,
            RMM::RMessage & msg,
            const QString & internalExtraInfo,
            const QString & extraInfo);

        // Remove messages
        EmpathJobInfo(
            ActionType t,
            const EmpathURL & folder,
            const QStringList & messageIDList,
            const QString & internalExtraInfo,
            const QString & extraInfo);
      
        // Mark messages
        EmpathJobInfo(
            ActionType t,
            const EmpathURL & folder,
            const QStringList & messageIDList,
            RMM::MessageStatus status,
            const QString & internalExtraInfo,
            const QString & extraInfo);

        // Create / Remove folder
        // Retrieve message
        EmpathJobInfo(
            ActionType t,
            const EmpathURL folder,
            const QString & internalExtraInfo,
            const QString & extraInfo);

        EmpathJobInfo(const EmpathJobInfo & j);

        bool success();
        ActionType type();
        ActionType subType();
        EmpathURL folder();
        EmpathURL from();
        EmpathURL to();
        QString ixinfo();
        QString xinfo();
        QStringList IDList();
        RMM::RMessage message();
        QString messageID();
        RMM::MessageStatus status();
        
        void setType(ActionType t);
        void setSubType(ActionType t);
        void setMessageID(const QString & s);
        bool success(const QString & id);
        void setSuccess(const QString & id, bool b);
        void setSuccessMap(QMap<QString, bool> map);

        void done(bool ok);

    private:

        ActionType type_;
        ActionType subType_;
        EmpathURL url1_;
        EmpathURL url2_;
        QString ixinfo_;
        QString xinfo_;
        QStringList IDList_;
        QString messageID_;
        RMM::RMessage message_;
        QMap<QString, bool> successMap_;
        RMM::MessageStatus status_;
        bool generalSuccess_;
};

typedef QQueue<EmpathJobInfo> EmpathJobQueue;

#endif
