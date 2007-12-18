/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/keyselectionjob.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "certificateselectionjob.h"
#include "certificateselectiondialog.h"
#include "kleo-assuan.h"
#include "assuancommand.h" // for AssuanCommand::makeError()
#include "models/keycache.h"

#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>

#include <QPointer>
#include <QStringList>
#include <QTimer>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

using namespace Kleo;

class CertificateSelectionJob::Private
{
public:
    Private( CertificateSelectionJob* qq ) : q( qq ), m_secretKeysOnly( false ), m_silent( false ), m_keyListings( 0 ), m_started( false ), m_useCache( true ) {}
    ~Private();

    void startKeyListing();
    void showCertificateSelectionDialog();

    std::vector<GpgME::Key> m_keys;
    QStringList m_patterns;
    bool m_secretKeysOnly;
    bool m_silent;
    QPointer<CertificateSelectionDialog> m_keySelector;
    int m_keyListings;
    GpgME::KeyListResult m_listResult;
    bool m_started;
    bool m_useCache;

    void nextKey( const GpgME::Key& key );
    void keyListingDone();
    void keyListingDone( const GpgME::KeyListResult& result );
    void certificateSelectionDialogClosed();

private:
    void emitError( int error, 
                    const GpgME::KeyListResult& result );
    void emitResult( const std::vector<GpgME::Key>& keys );

private:
    CertificateSelectionJob* q;    
};

CertificateSelectionJob::Private::~Private()
{
    delete m_keySelector;
}

void CertificateSelectionJob::Private::emitError( int error, const GpgME::KeyListResult& result )
{
    q->deleteLater();
    emit q->error( GpgME::Error( AssuanCommand::makeError( error ) ), result );
}

void CertificateSelectionJob::Private::emitResult( const std::vector<GpgME::Key>& keys )
{
    q->deleteLater();
    emit q->result( keys );
}

void CertificateSelectionJob::Private::keyListingDone()
{
    keyListingDone( GpgME::KeyListResult() );
}

void CertificateSelectionJob::Private::keyListingDone( const GpgME::KeyListResult& result )
{
    m_listResult = result;
    if ( result.error() ) {
        emitError( result.error(), result );
        return;
    }
    --m_keyListings;
    assert( m_keyListings >= 0 );

    if ( m_keyListings == 0 ) {
        showCertificateSelectionDialog();
    }
}

void CertificateSelectionJob::Private::nextKey( const GpgME::Key& key )
{
    m_keys.push_back( key );
}

CertificateSelectionJob::CertificateSelectionJob( QObject* parent ) : QObject( parent ), d( new Private( this ) )
{
}

void CertificateSelectionJob::Private::certificateSelectionDialogClosed()
{
    if ( m_keySelector->result() == QDialog::Rejected ) {
        emitError( GPG_ERR_CANCELED, m_listResult );
        return;
    }
    emitResult( m_keySelector->selectedKeys() );
}

void CertificateSelectionJob::Private::startKeyListing()
{
    if ( m_useCache ) {
        if ( m_secretKeysOnly ) {
            m_keys = SecretKeyCache::instance()->keys();
        } else {
            m_keys = PublicKeyCache::instance()->keys();
        }
        QTimer::singleShot( 0, q, SLOT( keyListingDone() ) );
        return;
    }
    m_keyListings = 2; // openpgp and cms
    KeyListJob *keylisting = CryptoBackendFactory::instance()->protocol( "openpgp" )->keyListJob();
    QObject::connect( keylisting, SIGNAL( result( GpgME::KeyListResult ) ),
             q, SLOT( keyListingDone( GpgME::KeyListResult ) ) );
    QObject::connect( keylisting, SIGNAL( nextKey( GpgME::Key ) ),
             q, SLOT( nextKey( GpgME::Key ) ) );
    if ( const GpgME::Error err = keylisting->start( m_patterns, m_secretKeysOnly ) ) {
        q->deleteLater();
        throw assuan_exception( err, "Unable to start keylisting" );
    }
    keylisting = Kleo::CryptoBackendFactory::instance()->protocol( "smime" )->keyListJob();
    QObject::connect( keylisting, SIGNAL( result( GpgME::KeyListResult ) ),
                      q, SLOT( keyListingDone( GpgME::KeyListResult ) ) );
    QObject::connect( keylisting, SIGNAL( nextKey( GpgME::Key ) ),
                      q, SLOT( nextKey( GpgME::Key ) ) );
    if ( const GpgME::Error err = keylisting->start( m_patterns, m_secretKeysOnly ) ) {
        q->deleteLater();
        throw assuan_exception( err, "Unable to start keylisting" );
    }
}

void CertificateSelectionJob::Private::showCertificateSelectionDialog()
{
    assert( !m_keySelector );
    if ( m_silent ) {
        emitResult( m_keys );
        return;
    }
    m_keySelector = new CertificateSelectionDialog();
    QObject::connect( m_keySelector, SIGNAL( accepted() ), q, SLOT( certificateSelectionDialogClosed() ) );
    QObject::connect( m_keySelector, SIGNAL( rejected() ), q, SLOT( certificateSelectionDialogClosed() ) );
    m_keySelector->addKeys( m_keys );
    m_keySelector->show();
}

void CertificateSelectionJob::start()
{
    assert( !d->m_started );
    if ( d->m_started )
        return;
    d->m_started = true;
    d->startKeyListing();
}

void CertificateSelectionJob::setPatterns( const QStringList& patterns )
{
    d->m_patterns = patterns;
}

QStringList CertificateSelectionJob::patterns() const
{
    return d->m_patterns;
}

void CertificateSelectionJob::setSecretKeysOnly( bool secretOnly )
{
    d->m_secretKeysOnly = secretOnly;
}

bool CertificateSelectionJob::secretKeysOnly() const
{
    return d->m_secretKeysOnly;
}

void CertificateSelectionJob::setSilent( bool silent )
{
    d->m_silent = silent;
}

bool CertificateSelectionJob::silent() const
{
    return d->m_silent;
}

void CertificateSelectionJob::setUseKeyCache( bool useCache )
{
    d->m_useCache = useCache;
}

bool CertificateSelectionJob::useKeyCache() const
{
    return d->m_useCache;
}


#include "certificateselectionjob.moc"

