/* -*- mode: c++; c-basic-offset:4 -*-
    smartcard/readerstatus.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include "readerstatus.h"

#include <utils/gnupg-helper.h>
#include <utils/stl_util.h>

#include <QFileSystemWatcher>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

#include <boost/static_assert.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <utility>

using namespace Kleo;
using namespace Kleo::SmartCard;
//using namespace GpgME;
using namespace boost;

static const char * flags[] = {
    "NOCARD",
    "PRESENT",
    "USABLE",
    "ACTIVE",
};
BOOST_STATIC_ASSERT(( sizeof flags/sizeof *flags == ReaderStatus::NumStatus ));

static ReaderStatus::Status readStatus( const QString & fileName ) {
    QFile file( fileName );
    if ( !file.exists() )
        return ReaderStatus::NoCard;
    if ( !file.open( QIODevice::ReadOnly ) ) {
        qDebug() << "ReaderStatus: failed to open" << fileName << ':' << file.errorString();
        return ReaderStatus::NoCard;
    }
    const QByteArray contents = file.readAll().trimmed();
    const char ** it = std::find( begin( flags ), end( flags ), contents );
    if ( it == end( flags ) )
        return ReaderStatus::NoCard;
    else
        return static_cast<ReaderStatus::Status>( it - begin( flags ) );
}

static unsigned int parseFileName( const QString & fileName, bool * ok ) {
    QRegExp rx( QLatin1String( "reader_(\\d+)\\.status" ) );
    if ( ok )
        *ok = false;
    if ( rx.exactMatch( QFileInfo( fileName ).fileName() ) )
        return rx.cap(1).toUInt( ok, 10 );
    return 0;
}





class ReaderStatus::Private {
    friend class Kleo::SmartCard::ReaderStatus;
    ReaderStatus * const q;
public:
    explicit Private( ReaderStatus * qq )
        : q( qq ),
          gnupgHomeDirectory( Kleo::gnupgHomeDirectory() ),
          fileSystemWatcher(),
          seenFiles(),
          cardInfos()
    {
        KDAB_SET_OBJECT_NAME( fileSystemWatcher );

        fileSystemWatcher.addPath( gnupgHomeDirectory );


        connect( &fileSystemWatcher, SIGNAL(directoryChanged(QString)),
                 q, SLOT(slotDirectoryChanged(QString)) );
        connect( &fileSystemWatcher, SIGNAL(fileChanged(QString)),
                 q, SLOT(slotFileChanged(QString)) );

        slotDirectoryChanged( gnupgHomeDirectory );
    }

    void slotDirectoryChanged( const QString & dirName ) {

        if ( dirName != gnupgHomeDirectory ) {
            qDebug() << "ReaderStatus: got change notify for directory"
                     << dirName << "(expected" << gnupgHomeDirectory << ')';
            return;
        }

        const QDir gpgHome( gnupgHomeDirectory );
        QStringList readerFiles = gpgHome.entryList( QStringList( QLatin1String( "reader_[0-9].status" ) ), QDir::Files );
        qSort( readerFiles );
        std::transform( readerFiles.begin(), readerFiles.end(),
                        readerFiles.begin(),
                        bind( &QDir::absoluteFilePath, cref( gpgHome ), _1 ) );

        QStringList newFiles;
        std::set_difference( readerFiles.begin(), readerFiles.end(),
                             seenFiles.begin(), seenFiles.end(),
                             std::back_inserter( newFiles ) );
        seenFiles.clear();
        std::copy( readerFiles.begin(), readerFiles.end(),
                   std::inserter( seenFiles, seenFiles.begin() ) );

        Q_FOREACH( const QString & readerFile, newFiles )
            slotFileChanged( readerFile );

        if ( !newFiles.empty() )
            fileSystemWatcher.addPaths( newFiles );
    }

    void slotFileChanged( const QString & readerFile ) {
        qDebug() << "ReaderStatus::slotFileChanged(" << readerFile << ")";
        bool ok = false;
        const unsigned int idx = parseFileName( readerFile, &ok );
        if ( ok ) {
            const CardInfo ci( readerFile, readStatus( readerFile ) );
            cardInfos.resize( std::max<size_t>( idx+1, cardInfos.size() ) );
            if ( !cardInfos[idx].fileName.isEmpty() && cardInfos[idx].fileName != readerFile )
                qDebug() << "ReaderStatus: filename of slot" << idx
                         << "reader seems to have changed from" << cardInfos[idx].fileName
                         << "to" << readerFile;
            const bool changed = cardInfos[idx].status != ci.status ;
            qDebug() << "ReaderStatus: slot" << idx << "changed from" << flags[cardInfos[idx].status] << "to" << flags[ci.status];
            cardInfos[idx] = ci;
            if ( changed ) {
                emit q->cardStatusChanged( idx, ci.status );
                emit q->anyCardPresentChanged( isAnyCardPresent() );
            }
        } else {
            qDebug() << "ReaderStatus: failed to extract slot index from filename" << readerFile;
        }
    }

    bool isAnyCardPresent() const {
        return kdtools::any( cardInfos, bind( &CardInfo::status, _1 ) == CardUsable );
    }

private:
    struct CardInfo {
        CardInfo() : fileName(), status( NoCard ) {}
        CardInfo( const QString & fn, Status s ) : fileName( fn ), status( s ) {}
        QString fileName;
        Status status;
    };

    const QString gnupgHomeDirectory;

    QFileSystemWatcher fileSystemWatcher;
    std::set<QString> seenFiles;

    std::vector<CardInfo> cardInfos;
};


ReaderStatus::ReaderStatus( QObject * parent )
    : QObject( parent ), d( new Private( this ) )
{

}

ReaderStatus::~ReaderStatus() {}

ReaderStatus::Status ReaderStatus::cardStatus( unsigned int slot ) const {
    if ( slot < d->cardInfos.size() )
        return d->cardInfos[slot].status;
    else
        return NoCard;
}

bool ReaderStatus::isAnyCardPresent() const {
    return d->isAnyCardPresent();
}

#include "moc_readerstatus.cpp"
