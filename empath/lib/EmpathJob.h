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

class EmpathJob
{
    public:

        EmpathJob(ActionType t);

        virtual ~EmpathJob()
        {
            // Empty.
        }

        bool success() const { return success_; }

        virtual void run() = 0;

        void done(bool ok);

        EmpathJobID id() const { return id_; }

        const char * className() const { return "EmpathJob"; }

    protected:

        ActionType type() const { return type_; }
        void setSuccess(bool ok) { success_ = ok; }

    private:

        static EmpathJobID ID_;

        EmpathJob();
        EmpathJobID id_;
        ActionType type_;
        bool success_;
};



class EmpathSingleJob : public EmpathJob
{
    public:

        EmpathSingleJob(ActionType t)
            : EmpathJob(t)
        {
            // Empty.
        }

        virtual ~EmpathSingleJob()
        {
            // Empty.
        }

        virtual void run() = 0;
        
        const char * className() const { return "EmpathSingleJob"; }
};

class EmpathMultiJob : public EmpathJob
{
    public:

        EmpathMultiJob(ActionType t)
            : EmpathJob(t)
        {
            // Empty.
        }

        virtual ~EmpathMultiJob()
        {
            // Empty.
        }

        const char * className() const { return "EmpathMultiJob"; }
};

class EmpathWriteJob : public EmpathSingleJob
{
    public:

        EmpathWriteJob(RMM::RMessage & message, const EmpathURL & folder)
          : EmpathSingleJob(WriteMessage),
            message_(message),
            folder_(folder)
        {
            // Empty.
        }

        virtual ~EmpathWriteJob()
        {
            // Empty
        }

        virtual void run();

        RMM::RMessage message() const { return message_; }
        EmpathURL folder() const { return folder_; }

    private:

        RMM::RMessage message_;
        EmpathURL folder_;
};

class EmpathCopyJob : public EmpathMultiJob
{
    public:

        EmpathCopyJob(const EmpathURL & source, const EmpathURL & destination)
          : EmpathMultiJob(CopyMessage),
            source_(source),
            destination_(destination)
        {
            // Empty.
        }

        virtual ~EmpathCopyJob()
        {
            // Empty
        }

        virtual void run();
        
        EmpathURL source() const { return source_; }
        EmpathURL destination() const { return destination_; }

    private:

        EmpathURL source_;
        EmpathURL destination_;
};

class EmpathMoveJob : public EmpathMultiJob
{
    public:

        EmpathMoveJob(const EmpathURL & source, const EmpathURL & destination)
          : EmpathMultiJob(MoveMessage),
            source_(source),
            destination_(destination)
        {
            // Empty.
        }
        
        virtual ~EmpathMoveJob()
        {
            // Empty
        }

        virtual void run();
        
        EmpathURL source() const { return source_; }
        EmpathURL destination() const { return destination_; }

    private:
        
        EmpathURL source_;
        EmpathURL destination_;
};

class EmpathRemoveJob : public EmpathSingleJob
{
    public:

        EmpathRemoveJob(const EmpathURL & folder, const QStringList & IDList)
          : EmpathSingleJob(RemoveMessage),
            folder_(folder),
            IDList_(IDList)
        {
            // Empty.
        }

        EmpathRemoveJob(const EmpathURL & url)
          : EmpathSingleJob(RemoveMessage),
            url_(url)
        {
            // Empty.
        }
        
        virtual ~EmpathRemoveJob()
        {
            // Empty
        }
        virtual void run();

        QMap<QString, bool> successMap() const { return successMap_; }
        EmpathURL url() const { return url_; }
        EmpathURL folder() const { return folder_; }
        QStringList IDList() const { return IDList_; }

    private:

        QMap<QString, bool> successMap_;
        EmpathURL url_;
        EmpathURL folder_;
        QStringList IDList_;
};

class EmpathRetrieveJob : public EmpathSingleJob
{
    public:

        EmpathRetrieveJob(const EmpathURL & url)
          : EmpathSingleJob(RetrieveMessage),
            url_(url)
        {
            // Empty.
        }
        
        virtual ~EmpathRetrieveJob()
        {
            // Empty
        }

        virtual void run();
        
        EmpathURL url() const { return url_; }

    private:

        EmpathURL url_;
};

class EmpathMarkJob : public EmpathSingleJob
{
    public:

        EmpathMarkJob(
            const EmpathURL & folder,
            const QStringList & IDList,
            RMM::MessageStatus flags
        )
          : EmpathSingleJob(MarkMessage),
            folder_(folder),
            IDList_(IDList),
            flags_(flags)
        {
            // Empty.
        }
        
        EmpathMarkJob(
            const EmpathURL & url,
            RMM::MessageStatus flags
        )
          : EmpathSingleJob(MarkMessage),
            url_(url),
            flags_(flags)
        {
            // Empty.
        }

        virtual ~EmpathMarkJob()
        {
            // Empty
        }

        virtual void run();

        QMap<QString, bool> successMap() const { return successMap_; }
        EmpathURL url() const { return url_; }
        RMM::MessageStatus flags() const { return flags_; }

    private:

        QMap<QString, bool> successMap_;
        EmpathURL url_;
        EmpathURL folder_;
        QStringList IDList_;
        RMM::MessageStatus flags_;
};

class EmpathCreateFolderJob : public EmpathSingleJob
{
    public:

        EmpathCreateFolderJob(const EmpathURL & folder)
          : EmpathSingleJob(CreateFolder),
            folder_(folder)
        {
            // Empty.
        }

        virtual ~EmpathCreateFolderJob()
        {
            // Empty
        }

        virtual void run();

        EmpathURL folder() const { return folder_; }

    private:

        EmpathURL folder_;
};

class EmpathRemoveFolderJob : public EmpathSingleJob
{
    public:

        EmpathRemoveFolderJob(const EmpathURL & folder)
          : EmpathSingleJob(CreateFolder),
            folder_(folder)
        {
            // Empty.
        }

        virtual ~EmpathRemoveFolderJob()
        {
            // Empty
        }

        virtual void run();

        EmpathURL folder() const { return folder_; }

    private:

        EmpathURL folder_;
};

#endif
