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
#include <qthread.h>

// Local includes
#include <rmm/Message.h>
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "EmpathEnum.h"
#include "EmpathURL.h"
#include "config.h"

class EmpathIndex;

class EmpathJob : public QThread
{
    public:

        EmpathJob(QObject * parent, int tag, ActionType t);

        ActionType type() const { return type_; }

        virtual void run() = 0;

        bool success() const { return success_; }

        int id() const { return id_; }

        int tag() const { return tag_; }

    protected:

        QObject * parent() { return parent_; }

        void setSuccess(bool b) { success_ = b; }

    private:

        static int          ID;

        QObject             * parent_;
        int                 tag_;
        ActionType          type_;
        bool                success_;
        int                 id_;
};

class EmpathReadIndexJob : public EmpathJob
{
    public:

        EmpathReadIndexJob(
            QObject * parent,
            int i,
            const EmpathURL & folder
        );

        virtual ~EmpathReadIndexJob();

        virtual void run();

        EmpathIndex * index() const { return index_; }
        EmpathURL folder() const { return folder_; }

    private:

        EmpathReadIndexJob(const EmpathReadIndexJob &);
        EmpathReadIndexJob & operator = (const EmpathReadIndexJob &);

        EmpathURL folder_;
        EmpathIndex * index_;
};

class EmpathWriteJob : public EmpathJob
{
    public:

        EmpathWriteJob(
            QObject * parent,
            int i,
            RMM::Message & message,
            const EmpathURL & folder
        );

        virtual ~EmpathWriteJob();

        virtual void run();

        RMM::Message message() const { return message_; }
        QString messageID() const { return messageID_; }

    private:

        EmpathWriteJob(const EmpathWriteJob &);
        EmpathWriteJob & operator = (const EmpathWriteJob &);

        RMM::Message message_;
        EmpathURL folder_;
        QString messageID_;
};

class EmpathCopyJob : public EmpathJob
{
    public:

        EmpathCopyJob(
            QObject * parent,
            int i,
            const EmpathURL & source,
            const EmpathURL & destination
        );

        virtual ~EmpathCopyJob();

        virtual void run();

    private:

        EmpathCopyJob(const EmpathCopyJob &);
        EmpathCopyJob & operator = (const EmpathCopyJob &);

        EmpathURL source_;
        EmpathURL destination_;
};

class EmpathMoveJob : public EmpathJob
{
    public:

        EmpathMoveJob(
            QObject * parent,
            int i,
            const EmpathURL & source,
            const EmpathURL & destination
        );

        virtual ~EmpathMoveJob();

        virtual void run();

    private:

        EmpathMoveJob(const EmpathMoveJob &);
        EmpathMoveJob & operator =(const EmpathMoveJob &);

        EmpathURL source_;
        EmpathURL destination_;
};

class EmpathRemoveJob : public EmpathJob
{
    public:

        EmpathRemoveJob(
            QObject * parent,
            int i,
            const EmpathURL & folder,
            const QStringList & IDList
        );

        EmpathRemoveJob(
            QObject * parent,
            int i,
            const EmpathURL & url
        );

        virtual ~EmpathRemoveJob();

        virtual void run();

    private:

        EmpathRemoveJob(const EmpathRemoveJob &);
        EmpathRemoveJob & operator = (const EmpathRemoveJob &);

        QMap<QString, bool> successMap_;
        EmpathURL url_;
        EmpathURL folder_;
        QStringList IDList_;
};

class EmpathRetrieveJob : public EmpathJob
{
    public:

        EmpathRetrieveJob(
            QObject * parent,
            int i,
            const EmpathURL & url
        );

        virtual ~EmpathRetrieveJob();

        virtual void run();

        RMM::Message message() const { return message_; }

    private:

        EmpathRetrieveJob(const EmpathRetrieveJob &);
        EmpathRetrieveJob & operator = (const EmpathRetrieveJob &);

        EmpathURL url_;
        RMM::Message message_;
};

class EmpathMarkJob : public EmpathJob
{
    public:

        EmpathMarkJob(
            QObject * parent,
            int i,
            const EmpathURL & folder,
            const QStringList & IDList,
            EmpathIndexRecord::Status flags
        );

        EmpathMarkJob(
            QObject * parent,
            int i,
            const EmpathURL & url,
            EmpathIndexRecord::Status flags
        );

        virtual ~EmpathMarkJob();

        virtual void run();

    private:

        EmpathMarkJob(const EmpathMarkJob &);
        EmpathMarkJob & operator = (const EmpathMarkJob &);

        QMap<QString, bool> successMap_;
        EmpathURL url_;
        EmpathURL folder_;
        QStringList IDList_;
        EmpathIndexRecord::Status flags_;
};

class EmpathCreateFolderJob : public EmpathJob
{
    public:

        EmpathCreateFolderJob(
            QObject * parent,
            int i,
            const EmpathURL & folder
        );

        virtual ~EmpathCreateFolderJob();

        virtual void run();

    private:

        EmpathCreateFolderJob(const EmpathCreateFolderJob &);
        EmpathCreateFolderJob & operator = (const EmpathCreateFolderJob &);

        EmpathURL folder_;
};

class EmpathRemoveFolderJob : public EmpathJob
{
    public:

        EmpathRemoveFolderJob(
            QObject * parent,
            int i,
            const EmpathURL & folder
        );

        virtual ~EmpathRemoveFolderJob();

        virtual void run();

    private:

        EmpathRemoveFolderJob(const EmpathRemoveFolderJob &);
        EmpathRemoveFolderJob & operator = (const EmpathRemoveFolderJob &);

        EmpathURL folder_;
};

#endif
// vim:ts=4:sw=4:tw=78
