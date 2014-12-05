/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
*/

#include <config-messageviewer.h>
#include "editorwatcher.h"
#include "utils/autoqpointer.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kopenwithdialog.h>
#include <kprocess.h>
#include <kmimetypetrader.h>
#include <krun.h>

#include <qsocketnotifier.h>

#include <cassert>

// inotify stuff taken from kdelibs/kio/kio/kdirwatch.cpp
#ifdef HAVE_SYS_INOTIFY_H
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#endif

namespace MessageViewer {
EditorWatcher::EditorWatcher( const KUrl & url, const QString &mimeType, OpenWithOption option,
                              QObject * parent, QWidget *parentWidget ) :
    QObject( parent ),
    mUrl( url ),
    mMimeType( mimeType ),
    mInotifyFd(-1),
    mInotifyWatch(-1),
    mEditor( 0 ),
    mParentWidget( parentWidget ),
    mOpenWithOption( option ),
    mHaveInotify( false ),
    mFileOpen( false ),
    mEditorRunning( false ),
    mFileModified( true ), // assume the worst unless we know better
    mDone( false )
{
    assert( mUrl.isLocalFile() );
    mTimer.setSingleShot( true );
    connect( &mTimer, SIGNAL(timeout()), SLOT(checkEditDone()) );
}

EditorWatcher::~EditorWatcher()
{
#ifdef HAVE_SYS_INOTIFY_H
    ::close( mInotifyFd );
#endif
}

bool EditorWatcher::start()
{
    // find an editor
    KUrl::List list;
    list.append( mUrl );
    KService::Ptr offer = KMimeTypeTrader::self()->preferredService( mMimeType, QLatin1String("Application") );
    if ( (mOpenWithOption == OpenWithDialog) || !offer ) {
        AutoQPointer<KOpenWithDialog> dlg( new KOpenWithDialog( list, i18n("Edit with:"),
                                                                QString(), mParentWidget ) );
        const int dlgrc = dlg->exec();
        if ( dlgrc && dlg ) {
            offer = dlg->service();
        }
        if ( !dlgrc || !offer )
            return false;
    }

#ifdef HAVE_SYS_INOTIFY_H
    // monitor file
    mInotifyFd = inotify_init();
    if ( mInotifyFd > 0 ) {
        (void)fcntl(mInotifyFd, F_SETFD, FD_CLOEXEC);
        qDebug()<<" mUrl.path().toLatin1()"<<mUrl.path().toLatin1();
        mInotifyWatch = inotify_add_watch( mInotifyFd, mUrl.path().toLatin1(), IN_CLOSE | IN_OPEN | IN_MODIFY | IN_ATTRIB );
        if ( mInotifyWatch >= 0 ) {
            QSocketNotifier *sn = new QSocketNotifier( mInotifyFd, QSocketNotifier::Read, this );
            connect( sn, SIGNAL(activated(int)), SLOT(inotifyEvent()) );
            mHaveInotify = true;
            mFileModified = false;
        }
    } else {
        kWarning() << "Failed to activate INOTIFY!";
    }
#endif

    // start the editor
    const QStringList params = KRun::processDesktopExec( *offer, list, false );
    mEditor = new KProcess( this );
    mEditor->setProgram( params );
    connect( mEditor, SIGNAL(finished(int,QProcess::ExitStatus)),
             SLOT(editorExited()) );
    mEditor->start();
    if ( !mEditor->waitForStarted() )
        return false;
    mEditorRunning = true;

    mEditTime.start();
    return true;
}

bool EditorWatcher::fileChanged() const
{
    return mFileModified;
}

KUrl EditorWatcher::url() const
{
    return mUrl;
}

void EditorWatcher::inotifyEvent()
{
    assert( mHaveInotify );

#ifdef HAVE_SYS_INOTIFY_H
    int pending = -1;
    int offsetStartRead = 0; // where we read into buffer
    char buf[8192];
    assert( mInotifyFd > -1 );
    ioctl( mInotifyFd, FIONREAD, &pending );

    while ( pending > 0 ) {

        const int bytesToRead = qMin( pending, (int)sizeof( buf ) - offsetStartRead );

        int bytesAvailable = read( mInotifyFd, &buf[offsetStartRead], bytesToRead );
        pending -= bytesAvailable;
        bytesAvailable += offsetStartRead;
        offsetStartRead = 0;

        int offsetCurrent = 0;
        while ( bytesAvailable >= (int)sizeof( struct inotify_event ) ) {
            const struct inotify_event * const event = (struct inotify_event *) &buf[offsetCurrent];
            const int eventSize = sizeof( struct inotify_event ) + event->len;
            if ( bytesAvailable < eventSize ) {
                break;
            }

            bytesAvailable -= eventSize;
            offsetCurrent += eventSize;
            if ( event->mask & IN_OPEN ) {
                mFileOpen = true;
            }
            if ( event->mask & IN_CLOSE ) {
                mFileOpen = false;
            }
            if ( event->mask & (IN_MODIFY|IN_ATTRIB) ) {
                mFileModified = true;
            }

        }
        if (bytesAvailable > 0) {
            // copy partial event to beginning of buffer
            memmove(buf, &buf[offsetCurrent], bytesAvailable);
            offsetStartRead = bytesAvailable;
        }
    }
#endif

    mTimer.start( 500 );

}

void EditorWatcher::editorExited()
{
    mEditorRunning = false;
    mTimer.start( 500 );
}

void EditorWatcher::checkEditDone()
{
    if ( mEditorRunning || (mFileOpen && mHaveInotify) || mDone )
        return;

    static QStringList readOnlyMimeTypes;
    if ( readOnlyMimeTypes.isEmpty() ) {
        readOnlyMimeTypes << QLatin1String("message/rfc822")
                          << QLatin1String("application/pdf");
    }

    // protect us against double-deletion by calling this method again while
    // the subeventloop of the message box is running
    mDone = true;

    // check if it's a mime type that's mostly handled read-only
    const bool isReadOnlyMimeType = ( readOnlyMimeTypes.contains( mMimeType ) ||
                                      mMimeType.startsWith( QLatin1String("image/") ) );

    // nobody can edit that fast, we seem to be unable to detect
    // when the editor will be closed
    if ( mEditTime.elapsed() <= 3000 && !isReadOnlyMimeType ) {
        KMessageBox::information( mParentWidget,
                                  i18n( "KMail is unable to detect when the chosen editor is closed. "
                                        "To avoid data loss, editing the attachment will be aborted." ),
                                  i18n( "Unable to edit attachment" ),
                                  QLatin1String("UnableToEditAttachment") );
    }

    emit editDone( this );
    deleteLater();
}
}
