/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  Copyright (c) 2003 Zack Rusin <zack@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef MAILCOMMON_FOLDERJOB_H
#define MAILCOMMON_FOLDERJOB_H

#include <Collection>
#include "mailcommon_export.h"

namespace MailCommon {

class MAILCOMMON_EXPORT FolderJob : public QObject
{
    Q_OBJECT

public:
    FolderJob();

    virtual ~FolderJob();

    /**
     * Start the job
     */
    void start();

    /**
     * Interrupt the job. Note that the finished() and result() signal
     * will be emitted, unless you called setPassiveDestructor(true) before.
     * This kills the job, don't use it afterwards.
     */
    virtual void kill();

    /**
     * @return the error code of the job. This must only be called from
     * the slot connected to the finished() signal.
     */
    int error() const
    {
        return mErrorCode;
    }

    /**
     * @return true if this job can be canceled, e.g. to exit the application
     */
    bool isCancellable() const
    {
        return mCancellable;
    }

    /**
     * Call this to change the "cancellable" property of this job.
     * By default, tListMessages, tGetMessage, tGetFolder and tCheckUidValidity
     * are cancellable, the others are not. But when copying, a non-cancellable
     * tGetMessage is needed.
     */
    void setCancellable( bool b )
    {
        mCancellable = b;
    }

signals:
    /**
     * Emitted when the job finishes all processing.
     */
    void finished();

    /**
     * Emitted when the job finishes all processing.
     * More convenient signal than finished(), since it provides a pointer to the job.
     * This signal is emitted by the FolderJob destructor => do NOT downcast
     * the job to a subclass!
     */
    void result( FolderJob *job );

protected:
    /**
     * Has to be reimplemented. It's called by the start() method. Should
     * start the processing of the specified job function.
     */
    virtual void execute() = 0;

    Akonadi::Collection mSrcFolder;
    int                 mErrorCode;

    bool                mStarted;
    bool                mCancellable;
};

}

#endif
