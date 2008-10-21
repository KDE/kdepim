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

#include <KDebug>

#include <QFileSystemWatcher>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QDir>

#include "stl_util.h"

#include <boost/bind.hpp>

#include <set>
#include <cassert>

using namespace Kleo;
using namespace boost;

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
    std::set<QString> m_seenPaths;
    std::set<QString> m_cachedDirectories;
    std::set<QString> m_cachedFiles;
    QStringList m_paths, m_blacklist;
};

FileSystemWatcher::Private::Private( FileSystemWatcher* qq, const QStringList& paths )
    : q( qq ),
      m_watcher( 0 ),
      m_paths( paths )
{
    m_timer.setSingleShot( true );
    connect( &m_timer, SIGNAL( timeout() ), q, SLOT( onTimeout() ) );
}

static bool is_blacklisted( const QString & file, const QStringList & blacklist ) {
    Q_FOREACH( const QString & entry, blacklist )
        if ( QRegExp( entry, Qt::CaseInsensitive, QRegExp::Wildcard ).exactMatch( file ) ) {
            kDebug() << file << "matches blacklist entry" << entry;
            return true;
        }
    return false;
}

void FileSystemWatcher::Private::onFileChanged( const QString& path )
{
    kDebug() << path;
    const QFileInfo fi( path );
    if ( is_blacklisted( fi.fileName(), m_blacklist ) )
        return;
    m_seenPaths.insert( path );
    m_cachedFiles.insert( path );
    handleTimer();
}

static QStringList list_dir_absolute( const QString & path, const QStringList & blacklist ) {
    QDir dir( path );
    QStringList entries = dir.entryList( QDir::AllEntries|QDir::NoDotAndDotDot );
    entries.erase( std::remove_if( entries.begin(), entries.end(),
                                   bind( is_blacklisted, _1, cref( blacklist ) ) ), entries.end() );
    kdtools::sort( entries );

    std::transform( entries.begin(), entries.end(), entries.begin(),
                    bind( &QDir::absoluteFilePath, &dir, _1 ) );

    return entries;
}

static QStringList find_new_files( const QStringList & current, const std::set<QString> & seen ) {
    QStringList result;
    std::set_difference( current.begin(), current.end(),
                         seen.begin(), seen.end(),
                         std::back_inserter( result ) );
    return result;
}

void FileSystemWatcher::Private::onDirectoryChanged( const QString& path )
{
    kDebug() << path;
    const QFileInfo fi( path );
    if ( is_blacklisted( fi.fileName(), m_blacklist ) )
        return;

    const QStringList newFiles = find_new_files( list_dir_absolute( path, m_blacklist ), m_seenPaths );
    kDebug() << "newFiles" << newFiles;

    m_cachedFiles.insert( newFiles.begin(), newFiles.end() );
    q->addPaths( newFiles );

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
        if ( !d->m_paths.empty() )
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

void FileSystemWatcher::blacklistFiles( const QStringList& paths )
{
    d->m_blacklist += paths;
    QStringList blacklisted;
    d->m_paths.erase( kdtools::separate_if( d->m_paths.begin(), d->m_paths.end(),
                                            std::back_inserter( blacklisted ), d->m_paths.begin(),
                                            bind( is_blacklisted, _1, cref( d->m_blacklist ) ) ).second, d->m_paths.end() );
    if ( d->m_watcher && !blacklisted.empty() )
        d->m_watcher->removePaths( blacklisted );
}

static QStringList resolve( const QStringList & paths, const QStringList & blacklist ) {
    if ( paths.empty() )
        return QStringList();
    QStringList result;
    Q_FOREACH( const QString & path, paths )
        if ( QDir( path ).exists() )
            result += list_dir_absolute( path, blacklist );
    return result + resolve( result, blacklist );
}

void FileSystemWatcher::addPaths( const QStringList& paths )
{
    if ( paths.empty() )
        return;
    const QStringList newPaths = paths + resolve( paths, d->m_blacklist );
    kDebug( !newPaths.empty() ) << "adding\n " << newPaths.join( "\n " ) << "\n/end";
    d->m_paths += newPaths;
    d->m_seenPaths.insert( newPaths.begin(), newPaths.end() );
    if ( d->m_watcher && !newPaths.empty() )
        d->m_watcher->addPaths( newPaths );
}

void FileSystemWatcher::addPath( const QString& path )
{
    addPaths( QStringList( path ) );
}

void FileSystemWatcher::removePaths( const QStringList& paths )
{
    if ( paths.empty() )
        return;
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
