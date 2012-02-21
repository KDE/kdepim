/*
    This file is part of Akregator2.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR2_COMMAND_H
#define AKREGATOR2_COMMAND_H

#include "akregator2_export.h"

#include <KJob>

#include <QtCore/QObject>

class QWidget;

namespace Akregator2 {

class EmitResultGuard;

class AKREGATOR2INTERFACES_EXPORT Command : public KJob
{
    Q_OBJECT

    friend class ::Akregator2::EmitResultGuard;

public:

    enum Error {
        UserCanceled = KJob::UserDefinedError,
        SomeError,
        UserDefinedCommandError
    };
    explicit Command( QObject* parent = 0 );
    virtual ~Command();

    QWidget* parentWidget() const;
    void setParentWidget( QWidget* parentWidget );

    /* reimp */ void start();

    /**
     * whether the UI should display the job e.g. via progress items
     * defaults to @p true
     */
    bool isUserVisible() const;
    void setUserVisible( bool visible );

Q_SIGNALS:
    void started();

protected:
    void setShowErrorDialog( bool );
    virtual void doStart() = 0;
    void setErrorAndEmitResult( const QString& errorText, int error=SomeError );
    void emitCanceled();

private Q_SLOTS:
    void jobFinished();
    void delayedStart();

private:
    class Private;
    Private* const d;
};

}

#endif // AKREGATOR2_COMMAND_H
