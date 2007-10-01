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

#include <kleo/keylistjob.h>

#include <KLocale>

#include <QDebug>
#include <QList>
#include <QPointer>
#include <QStringList>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

using namespace Kleo;

class KeySelectionJob::Private
{
public:
    Private( KeySelectionJob* qq ) : q( qq ), m_secretKeysOnly( false ), m_keyListings( 0 ), m_started( false ) {}
    ~Private();

    void startKeyListing();
    bool startKeyListingForProtocol( const char* protocol );
    void showKeySelectionDialog();

    std::vector<GpgME::Key> m_keys;
    QStringList m_patterns;
    bool m_secretKeysOnly;
    QPointer<KeySelectionDialog> m_keySelector;
    int m_keyListings;
    GpgME::KeyListResult m_listResult;
    bool m_started;

    void nextKey( const GpgME::Key& key );
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
    emit q->error( GpgME::Error( AssuanCommand::makeError( error ) ), result );
    q->deleteLater();
}

void KeySelectionJob::Private::emitResult( const std::vector<GpgME::Key>& keys )
{
    emit q->result( keys );
    q->deleteLater();
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
    } else {
        emitResult( m_keySelector->selectedKeys() );
    }
}

bool KeySelectionJob::Private::startKeyListingForProtocol( const char* protocol )
{
    if ( !CryptoBackendFactory::instance()->protocol( protocol ) )
        return false;

    KeyListJob *keylisting = CryptoBackendFactory::instance()->protocol( protocol )->keyListJob();
    QObject::connect( keylisting, SIGNAL( result( GpgME::KeyListResult ) ),
             q, SLOT( keyListingDone( GpgME::KeyListResult ) ) );
    QObject::connect( keylisting, SIGNAL( nextKey( GpgME::Key ) ),
             q, SLOT( nextKey( GpgME::Key ) ) );
    if ( const GpgME::Error err = keylisting->start( m_patterns, m_secretKeysOnly ) ) {
        q->deleteLater();
        throw assuan_exception( err, i18n( "Unable to start keylisting for protocol %1", protocol ) );
    }

    return true;
}

void KeySelectionJob::Private::startKeyListing()
{
    m_keyListings = 0;

    QStringList protocols;
    protocols << "openpgp" << "smime";

    Q_FOREACH ( const QString i, protocols ) { 
        try {
            if ( startKeyListingForProtocol( i.toAscii().constData() ) )
                ++m_keyListings;
        } catch ( const assuan_exception& exp ) {
            qDebug() << QString::fromStdString( exp.message() );
        }
    }
 
    if ( m_keyListings == 0 ) {
        throw assuan_exception( AssuanCommand::makeError( GPG_ERR_GENERAL ), i18n( "Unable to start keylisting for any backend/no backends available" ) );
    }
}

void KeySelectionJob::Private::showKeySelectionDialog()
{
    assert( !m_keySelector );
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

#include "keyselectionjob.moc"

