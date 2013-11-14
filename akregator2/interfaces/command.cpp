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

#include "command.h"
#include "command_p.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QEventLoop>
#include <QPointer>
#include <QWidget>

using namespace Akregator2;

ShowErrorJob::ShowErrorJob( const QString& errorText, QWidget* parent )
    : KJob( parent )
    , m_parentWidget( parent )
    , m_errorText( errorText )
{
}

void ShowErrorJob::start() {
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ShowErrorJob::doStart() {
    QPointer<QObject> that( this );
    KMessageBox::error( m_parentWidget, m_errorText, i18nc("msg box caption", "Error") );
    if ( that )
        emitResult();
}

class Command::Private
{
public:
    Private();
    QPointer<QWidget> parentWidget;
    bool userVisible;
    bool showErrorDialog;
};

Command::Private::Private()
    : parentWidget()
    , userVisible( true )
    , showErrorDialog( false )
{
}

Command::Command( QObject* parent ) : KJob( parent ), d( new Private )
{
    connect( this, SIGNAL(finished(KJob*)),
             this, SLOT(jobFinished()) );
}

Command::~Command()
{
    delete d;
}

QWidget* Command::parentWidget() const
{
    return d->parentWidget;
}

void Command::setParentWidget( QWidget* parentWidget )
{
    d->parentWidget = parentWidget;
}

void Command::start()
{
    QMetaObject::invokeMethod( this, "delayedStart", Qt::QueuedConnection );
}

void Command::delayedStart()
{
    doStart();
    emit started();
}

bool Command::isUserVisible() const {
    return d->userVisible;
}

void Command::setUserVisible( bool visible ) {
    d->userVisible = visible;
}

void Command::setShowErrorDialog( bool s ) {
    d->showErrorDialog = s;
}

void Command::jobFinished() {
    if ( error() && error() != UserCanceled && d->showErrorDialog )
        //don't show error dialog synchronously, to not disturb the
        //finished signals with a local event loop
        (new ShowErrorJob( errorText(), d->parentWidget ))->start();
}

void Command::setErrorAndEmitResult( const QString& errorText, int error ) {
    setErrorText( errorText );
    setError( error );
    emitResult();
}

void Command::emitCanceled() {
    setErrorAndEmitResult( i18n("User canceled"), Command::UserCanceled );
}

