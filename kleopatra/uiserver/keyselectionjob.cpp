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

#include "keyselectionjob.h"
#include "keyselectiondialog.h"
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

class KeySelectionJob::Private
{
public:
    Private( KeySelectionJob* qq ) : q( qq ), m_secretKeysOnly( false ), m_silent( false ), m_keyListings( 0 ), m_started( false ), m_useCache( true ) {}
    ~Private();

    void startKeyListing();
    void showKeySelectionDialog();

    std::vector<GpgME::Key> m_keys;
    QStringList m_patterns;
    bool m_secretKeysOnly;
    bool m_silent;
    QPointer<KeySelectionDialog> m_keySelector;
    int m_keyListings;
    GpgME::KeyListResult m_listResult;
    bool m_started;
    bool m_useCache;

    void nextKey( const GpgME::Key& key );
    void keyListingDone();
    void keyListingDone( const GpgME::KeyListResult& result );
    void keySelectionDialogClosed();

private:
    void emitError( int error, 
                    const GpgME::KeyListResult& result );
    void emitResult( const std::vector<GpgME::Key>& keys );

private:
    KeySelectionJob* q;    
};

KeySelectionJob::Private::~Private()
{
    delete m_keySelector;
}

void KeySelectionJob::Private::emitError( int error, const GpgME::KeyListResult& result )
{
    q->deleteLater();
    emit q->error( GpgME::Error( AssuanCommand::makeError( error ) ), result );
}

void KeySelectionJob::Private::emitResult( const std::vector<GpgME::Key>& keys )
{
    q->deleteLater();
    emit q->result( keys );
}

void KeySelectionJob::Private::keyListingDone()
{
    keyListingDone( GpgME::KeyListResult() );
}

void KeySelectionJob::Private::keyListingDone( const GpgME::KeyListResult& result )
{
    m_listResult = result;
    if ( result.error() ) {
        emitError( result.error(), result );
        return;
    }
    --m_keyListings;
    assert( m_keyListings >= 0 );

    if ( m_keyListings == 0 ) {
        showKeySelectionDialog();
    }
}

void KeySelectionJob::Private::nextKey( const GpgME::Key& key )
{
    m_keys.push_back( key );
}

KeySelectionJob::KeySelectionJob( QObject* parent ) : QObject( parent ), d( new Private( this ) )
{
}

void KeySelectionJob::Private::keySelectionDialogClosed()
{
    if ( m_keySelector->result() == QDialog::Rejected ) {
        emitError( GPG_ERR_CANCELED, m_listResult );
        return;
    }
    emitResult( m_keySelector->selectedKeys() );
}

void KeySelectionJob::Private::startKeyListing()
{
    if ( m_useCache )
    {
        if ( m_secretKeysOnly )
        {
            Q_FOREACH ( const GpgME::Key& i, KeyCache::instance()->keys() )
            {
                if ( i.isSecret() )
                    m_keys.push_back( i );
            }
        }
        else
        {
            m_keys = KeyCache::instance()->keys();
        }
        m_keys = KeyCache::instance()->keys();
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

void KeySelectionJob::Private::showKeySelectionDialog()
{
    assert( !m_keySelector );
    if ( m_silent ) {
        emitResult( m_keys );
        return;
    }
    m_keySelector = new KeySelectionDialog();
    QObject::connect( m_keySelector, SIGNAL( accepted() ), q, SLOT( keySelectionDialogClosed() ) );
    QObject::connect( m_keySelector, SIGNAL( rejected() ), q, SLOT( keySelectionDialogClosed() ) );
    m_keySelector->addKeys( m_keys );
    m_keySelector->show();
}

void KeySelectionJob::start()
{
    assert( !d->m_started );
    if ( d->m_started )
        return;
    d->m_started = true;
    d->startKeyListing();
}

void KeySelectionJob::setPatterns( const QStringList& patterns )
{
    d->m_patterns = patterns;
}

QStringList KeySelectionJob::patterns() const
{
    return d->m_patterns;
}

void KeySelectionJob::setSecretKeysOnly( bool secretOnly )
{
    d->m_secretKeysOnly = secretOnly;
}

bool KeySelectionJob::secretKeysOnly() const
{
    return d->m_secretKeysOnly;
}

void KeySelectionJob::setSilent( bool silent )
{
    d->m_silent = silent;
}

bool KeySelectionJob::silent() const
{
    return d->m_silent;
}

void KeySelectionJob::setUseKeyCache( bool useCache )
{
    d->m_useCache = useCache;
}

bool KeySelectionJob::useKeyCache() const
{
    return d->m_useCache;
}


#include "keyselectionjob.moc"

