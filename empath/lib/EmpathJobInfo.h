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

struct EmpathJobInfoPrivate
{
    ActionType type;
    EmpathURL url1;
    EmpathURL url2;
    QString xinfo;
    QStringList IDList;
    QString messageID;
    RMM::RMessage message;
    QMap<QString, bool> successMap;
    RMM::MessageStatus status;
    bool generalSuccess;
};

/**
 * If anyone knows what the **** is going on here, please drop me a line.
 * Whoever wrote this must be on crack.
 *
 * @author Rikkus
 */
class EmpathJobInfo
{
    public:

        EmpathJobInfo();

        // Copy / Move message
        EmpathJobInfo(
            ActionType t,
            const EmpathURL & from,
            const EmpathURL & to,
            const QString & extraInfo = QString::null);

        // Write message
        EmpathJobInfo(
            ActionType t,
            const EmpathURL & url,
            RMM::RMessage & msg,
            const QString & extraInfo = QString::null);

        // Remove list of messages
        EmpathJobInfo(
            ActionType t,
            const EmpathURL & folder,
            const QStringList & messageIDList,
            const QString & extraInfo = QString::null);

        // Mark list of messages
        EmpathJobInfo(
            ActionType t,
            const EmpathURL & folder,
            const QStringList & messageIDList,
            RMM::MessageStatus status,
            const QString & extraInfo = QString::null);
    
        // Mark message
        EmpathJobInfo(
            ActionType t,
            const EmpathURL url,
            RMM::MessageStatus status,
            const QString & extraInfo = QString::null);

        // Create / Remove folder
        // Retrieve / Remove message
        EmpathJobInfo(
            ActionType t,
            const EmpathURL folder,
            const QString & extraInfo = QString::null);

        EmpathJobInfo(const EmpathJobInfo & j);

        EmpathJobInfo(EmpathJobInfoPrivate &);

        virtual ~EmpathJobInfo();

        bool success();
        ActionType type();
        EmpathURL folder();
        EmpathURL from();
        EmpathURL to();
        QString xinfo();
        QStringList IDList();
        RMM::RMessage message();
        QString messageID();
        RMM::MessageStatus status();
        
        void setType(ActionType t);
        void setMessageID(const QString & s);
        void setMessage(RMM::RMessage &);
        bool success(const QString & id);
        void setSuccess(const QString & id, bool b);
        void setSuccessMap(QMap<QString, bool> map);

        bool haveOriginal() const;

        void done(bool ok);

        void setOriginal(EmpathJobInfo &);
        EmpathJobInfo original();

        const char * className() const { return "EmpathJobInfo"; }

        bool haveOriginal_;

        ActionType nextActionRequired();
        
    private:

        EmpathJobInfoPrivate current_;
        EmpathJobInfoPrivate original_;
};

typedef QQueue<EmpathJobInfo> EmpathJobQueue;

#endif
