/* -*- mode: c++; c-basic-offset:4 -*-
    filesystemwatcher.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "filesystemwatcher.h"

#include <QFileSystemWatcher>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <cassert>

using namespace Kleo;

class FileSystemWatcher::Private {
    FileSystemWatcher* const q;
public:
    explicit Private(  FileSystemWatcher* qq, const QStringList& paths=QStringList() );
    ~Private() {
        delete m_watcher;
    }

    void onFileChanged( const QString& path );
    void onDirectoryChanged( const QString& path );
    void handleTimer();
    void onTimeout();

    void connectWatcher();

    QFileSystemWatcher* m_watcher;
    QTimer m_timer;
    QSet<QString> m_cachedDirectories;
    QSet<QString> m_cachedFiles;
    QStringList m_paths;
};

FileSystemWatcher::Private::Private( FileSystemWatcher* qq, const QStringList& paths )
    : q( qq ),
      m_watcher( 0 ),
      m_paths( paths )
{
    m_timer.setSingleShot( true );
    connect( &m_timer, SIGNAL( timeout() ), q, SLOT( onTimeout() ) );
}

void FileSystemWatcher::Private::onFileChanged( const QString& path )
{
    m_cachedFiles.insert( path );
    handleTimer();
}

void FileSystemWatcher::Private::onDirectoryChanged( const QString& path )
{
    m_cachedDirectories.insert( path );
    handleTimer();
}

void FileSystemWatcher::Private::onTimeout()
{
    Q_FOREACH( const QString& i, m_cachedDirectories )
        emit q->directoryChanged( i );
    m_cachedDirectories.clear();
    Q_FOREACH( const QString& i, m_cachedFiles )
        emit q->fileChanged( i );
    m_cachedFiles.clear();
}

void FileSystemWatcher::Private::handleTimer()
{
    if ( m_timer.interval() == 0 ) {
        onTimeout();
        return;
    }
    m_timer.start();
}

void FileSystemWatcher::Private::connectWatcher() {
    if ( !m_watcher )
        return;
    connect( m_watcher, SIGNAL(directoryChanged(QString)),
             q, SLOT(onDirectoryChanged(QString)) );
    connect( m_watcher, SIGNAL(fileChanged(QString)),
             q, SLOT(onFileChanged(QString)) );
}

FileSystemWatcher::FileSystemWatcher( QObject* p )
    : QObject( p ), d( new Private( this ) )
{
    setEnabled( true );
}

FileSystemWatcher::FileSystemWatcher( const QStringList& paths, QObject* p )
    : QObject( p ), d( new Private( this, paths ) )
{
    setEnabled( true );
}

void FileSystemWatcher::setEnabled( bool enable )
{
    if ( isEnabled() == enable )
        return;
    if ( enable ) {
        assert( !d->m_watcher );
        d->m_watcher = new QFileSystemWatcher;
        d->m_watcher->addPaths( d->m_paths );
        d->connectWatcher();
    } else {
       assert( d->m_watcher );
       delete d->m_watcher;
       d->m_watcher = 0;
    }
}

bool FileSystemWatcher::isEnabled() const
{
    return d->m_watcher != 0;
}

FileSystemWatcher::~FileSystemWatcher()
{
}

void FileSystemWatcher::setDelay( int ms )
{
    assert( ms >= 0 );
    d->m_timer.setInterval( ms );
}

int FileSystemWatcher::delay() const
{
    return d->m_timer.interval();
}

void FileSystemWatcher::addPaths( const QStringList& paths )
{
    d->m_paths += paths;
    if ( d->m_watcher )
        d->m_watcher->addPaths( paths );
}

void FileSystemWatcher::addPath( const QString& path )
{
    addPaths( QStringList( path ) );
}

void FileSystemWatcher::removePaths( const QStringList& paths )
{
    Q_FOREACH ( const QString& i, paths )
        d->m_paths.removeAll( i );
    if ( d->m_watcher )
        d->m_watcher->removePaths( paths );
}

void FileSystemWatcher::removePath( const QString& path )
{
    removePaths( QStringList( path ) );
}

#include "filesystemwatcher.moc"
