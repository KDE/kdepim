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

#ifndef EMPATH_JOB_H
#define EMPATH_JOB_H

// Qt includes
#include <qmap.h>
#include <qqueue.h>
#include <qstring.h>
#include <qstringlist.h>

// Local includes
#include <RMM_Message.h>
#include "EmpathEnum.h"
#include "EmpathURL.h"

class EmpathJobPrivate
{
    public:

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

        void run();
        
        void done(bool ok);
        
        const char * className() const { return "EmpathJobPrivate"; }

    protected:
        
        void setType(ActionType t);
        void setMessageURL(const EmpathURL &);
        void setFolder(const EmpathURL &);
        void setSourceFolder(const EmpathURL &);
        void setDestinationFolder(const EmpathURL &);
        void setMessageIDList(const QString & s);
        void setMessage(RMM::RMessage &);
        void setMessageStatus(RMM::RMessage &);
        bool success(const QString & id);
        void setSuccess(const QString & id, bool b);
        void setSuccessMap(QMap<QString, bool> map);

    private:

        ActionType type;
        EmpathURL messageURL;
        EmpathURL folder;
        EmpathURL source;
        EmpathURL destination;
        QString xinfo;
        QStringList IDList;
        QString messageID;
        RMM::RMessage message;
        QMap<QString, bool> successMap;
        RMM::MessageStatus status;
        bool generalSuccess;
};

class EmpathJob
{
    public:

            static EmpathJob *
        copy(
            const EmpathURL & from,
            const EmpathURL & to,
            const QString & extraInfo
        );

            static EmpathJob *
        move(
            const EmpathURL & from,
            const EmpathURL & to,
            const QString & extraInfo
        );

            static EmpathJob *
        write(
            RMM::RMessage & msg,
            const EmpathURL & folder,
            const QString & extraInfo
        );

            static EmpathJob *
        remove(
            const EmpathURL & url,
            const QString & extraInfo
        );
 
            static EmpathJob *
        remove(
            const EmpathURL & folder,
            const QStringList & messageIDList,
            const QString & extraInfo
        );

            static EmpathJob *
        mark(
            const EmpathURL & folder,
            const QStringList & messageIDList,
            RMM::MessageStatus status,
            const QString & extraInfo
        );

            static EmpathJob *
        mark(
            const EmpathURL & url,
            RMM::MessageStatus status,
            const QString & extraInfo
        );

            static EmpathJob *
        createFolder(
            const EmpathURL & url,
            const QString & extraInfo
        );

            static EmpathJob *
        removeFolder(
            const EmpathURL & url,
            const QString & extraInfo
        );

            static EmpathJob *
        retrieve(
            const EmpathURL & url,
            const QString & extraInfo
        );

       
    private:
        
        EmpathJob();
        virtual ~EmpathJob();

        void _runQueue();

        QQueue<EmpathJobPrivate> subJobQueue_;
};

#endif
