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
#include <qobject.h>
#include <qmap.h>
#include <qqueue.h>
#include <qstring.h>
#include <qstringlist.h>

// Local includes
#include <RMM_Message.h>
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "EmpathEnum.h"
#include "EmpathURL.h"
#include "config.h"

#ifdef USE_QPTHREAD
#include <qpthr/qp.h>
#endif

class EmpathJobScheduler;

class EmpathJob : public QObject
#ifdef USE_QPTHREAD
    , public QpThread
#endif
{
    Q_OBJECT

    public:

        EmpathJob(ActionType t);

        virtual ~EmpathJob()
        {
            // Empty.
        }

        bool success() const { return success_; }

#ifdef USE_QPTHREAD
        virtual void Run() { run(); }
#endif
        virtual void run() = 0;

        EmpathJobID id() const { return id_; }

    protected:

        EmpathJob();
        EmpathJob(const EmpathJob &);

        ActionType type() const { return type_; }
        void setSuccess(bool ok) { success_ = ok; }
        void setSuccessMap(EmpathSuccessMap map) { successMap_ = map; }

    private:

        static EmpathJobID ID_;

        EmpathJobID id_;
        ActionType type_;
        bool success_;
        EmpathSuccessMap successMap_;
};

class EmpathSingleJob : public EmpathJob
{
    Q_OBJECT

    public:

        EmpathSingleJob(ActionType t);
        EmpathSingleJob(const EmpathSingleJob &);

        virtual ~EmpathSingleJob();

        virtual void run() = 0;
};

class EmpathMultiJob : public EmpathJob
{
    Q_OBJECT

    public:

        EmpathMultiJob(ActionType t);
        EmpathMultiJob(const EmpathMultiJob &);

        virtual ~EmpathMultiJob();

        virtual void run() = 0;
};

class EmpathWriteJob : public EmpathSingleJob
{
    Q_OBJECT

    public:

        EmpathWriteJob(
            RMM::RMessage & message,
            const EmpathURL & folder
        );

        EmpathWriteJob(const EmpathWriteJob &);

        virtual ~EmpathWriteJob();

        virtual void run();

        RMM::RMessage message() const { return message_; }
        EmpathURL folder() const { return folder_; }

        QString messageID() const { return messageID_; }
    
    signals:

        void done(EmpathWriteJob);

    private:

        RMM::RMessage message_;
        EmpathURL folder_;
        QString messageID_;
};

class EmpathCopyJob : public EmpathMultiJob
{
    Q_OBJECT

    public:

        EmpathCopyJob(
            const EmpathURL & source,
            const EmpathURL & destination
        );

        EmpathCopyJob(const EmpathCopyJob &);

        virtual ~EmpathCopyJob();

        virtual void run();
        
        EmpathURL source() const { return source_; }
        EmpathURL destination() const { return destination_; }
    
    signals:

        void done(EmpathCopyJob);

    private:

        EmpathURL source_;
        EmpathURL destination_;
};

class EmpathMoveJob : public EmpathMultiJob
{
    Q_OBJECT

    public:

        EmpathMoveJob(
            const EmpathURL & source,
            const EmpathURL & destination
        );
        
        EmpathMoveJob(const EmpathMoveJob &);

        virtual ~EmpathMoveJob();

        virtual void run();
        
        EmpathURL source() const { return source_; }
        EmpathURL destination() const { return destination_; }

    signals:

        void done(EmpathMoveJob);

    private:
        
        EmpathURL source_;
        EmpathURL destination_;
};

class EmpathRemoveJob : public EmpathSingleJob
{
    Q_OBJECT

    public:

        EmpathRemoveJob(
            const EmpathURL & folder,
            const QStringList & IDList
        );

        EmpathRemoveJob(const EmpathURL & url);

        EmpathRemoveJob(const EmpathRemoveJob &);
        
        virtual ~EmpathRemoveJob();

        virtual void run();

        QMap<QString, bool> successMap() const { return successMap_; }
        EmpathURL url() const { return url_; }
        EmpathURL folder() const { return folder_; }
        QStringList IDList() const { return IDList_; }
    
    signals:

        void done(EmpathRemoveJob);

    private:

        QMap<QString, bool> successMap_;
        EmpathURL url_;
        EmpathURL folder_;
        QStringList IDList_;
};

class EmpathRetrieveJob : public EmpathSingleJob
{
    Q_OBJECT

    public:

        EmpathRetrieveJob(const EmpathURL & url);

        EmpathRetrieveJob(const EmpathRetrieveJob &);
        
        virtual ~EmpathRetrieveJob();

        virtual void run();
        
        EmpathURL url() const { return url_; }

        RMM::RMessage message() const { return message_; }
    
    signals:

        void done(EmpathRetrieveJob);

    private:

        EmpathURL url_;
        RMM::RMessage message_;
};

class EmpathMarkJob : public EmpathSingleJob
{
    Q_OBJECT

    public:

        EmpathMarkJob(
            const EmpathURL & folder,
            const QStringList & IDList,
            EmpathIndexRecord::Status flags
        );
        
        EmpathMarkJob(
            const EmpathURL & url,
            EmpathIndexRecord::Status flags
        );

        EmpathMarkJob(const EmpathMarkJob &);

        virtual ~EmpathMarkJob();

        virtual void run();

        QMap<QString, bool> successMap() const { return successMap_; }
        EmpathURL folder() const { return folder_; }
        QStringList IDList() const { return IDList_; }
        EmpathURL url() const { return url_; }
        EmpathIndexRecord::Status flags() const { return flags_; }

    signals:

        void done(EmpathMarkJob);

    private:

        QMap<QString, bool> successMap_;
        EmpathURL url_;
        EmpathURL folder_;
        QStringList IDList_;
        EmpathIndexRecord::Status flags_;
};

class EmpathCreateFolderJob : public EmpathSingleJob
{
    Q_OBJECT

    public:

        EmpathCreateFolderJob(const EmpathURL & folder);

        EmpathCreateFolderJob(const EmpathCreateFolderJob &);

        virtual ~EmpathCreateFolderJob();

        virtual void run();

        EmpathURL folder() const { return folder_; }

    signals:

        void done(EmpathCreateFolderJob);

    private:

        EmpathURL folder_;
};

class EmpathRemoveFolderJob : public EmpathSingleJob
{
    Q_OBJECT

    public:

        EmpathRemoveFolderJob(const EmpathURL & folder);
        
        EmpathRemoveFolderJob(const EmpathRemoveFolderJob &);

        virtual ~EmpathRemoveFolderJob();

        virtual void run();

        EmpathURL folder() const { return folder_; }

    signals:

        void done(EmpathRemoveFolderJob);

    private:

        EmpathURL folder_;
};

#endif
// vim:ts=4:sw=4:tw=78
