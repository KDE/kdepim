/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptwizard.cpp

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

#include "signencryptwizard.h"

#include "recipientresolvepage.h"
#include "kleo-assuan.h"

#include <utils/stl_util.h>

#include <kmime/kmime_header_parsing.h>

#include <gpgme++/key.h>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class SignEncryptWizard::Private {
    friend class ::Kleo::SignEncryptWizard;
    SignEncryptWizard * q;
public:
    explicit Private( SignEncryptWizard * qq );
    ~Private();

    void onCurrentIdChange( int id );

    enum Pages {
        Prepare=0,
        Objects,
        ResolveRecipients,
        Results
    };
private:
    Mode mode;

    struct Ui {
        RecipientResolvePage recipientResolvePage;
        RecipientResolvePage testPage;
    } ui;
};


SignEncryptWizard::Private::Private( SignEncryptWizard * qq )
    : q( qq ),
      mode( EncryptOrSignFiles )
{
    q->setOptions( q->options() | NoBackButtonOnStartPage );
    q->setPage( ResolveRecipients, &ui.recipientResolvePage );
    q->setPage( Results, &ui.testPage );
    q->setStartId( ResolveRecipients );
    q->connect( q, SIGNAL( currentIdChanged( int ) ), q, SLOT( onCurrentIdChange( int ) ) );
}

SignEncryptWizard::Private::~Private() {}

SignEncryptWizard::SignEncryptWizard( QWidget * p, Qt::WindowFlags f )
    : QWizard( p, f ), d( new Private( this ) )
{
}

void SignEncryptWizard::Private::onCurrentIdChange( int id )
{
    //ugly, but there are no better hooks provided
    if ( id == ui.recipientResolvePage.nextId() )
        emit q->recipientsResolved();
} 

SignEncryptWizard::~SignEncryptWizard() {}

void SignEncryptWizard::setMode( Mode mode ) {
    // EncryptEMail:
    //   1. RecipientResolvePage
    //   2. ResultPage
    // SignEMail:
    //   1. ResultPage
    // EncryptOrSignFiles:
    //   1. OperationsPage
    //   2. ObjectsPage
    //   3. RecipientsPage
    //   4. ResultPage
    assuan_assert( mode == EncryptEMail || mode == SignEMail || !"Other cases are not yet implemented" );

    d->mode = EncryptEMail;
}

void SignEncryptWizard::setProtocol( Protocol proto ) {
    assuan_assert( d->mode == EncryptEMail || d->mode == SignEMail );
    // ### distribute to where needed....
    d->ui.recipientResolvePage.setProtocol( proto );
}

void SignEncryptWizard::setRecipientsAndCandidates( const std::vector<Mailbox> & recipients, const std::vector< std::vector<Key> > & keys ) {
    assuan_assert( !keys.empty() );
    assuan_assert( (size_t)recipients.size() == keys.size() );
    assuan_assert( d->mode == EncryptEMail );

    d->ui.recipientResolvePage.ensureIndexAvailable( keys.size() - 1 );

    for ( unsigned int i = 0, end = keys.size() ; i < end ; ++i ) {
        RecipientResolveWidget * const rr = d->ui.recipientResolvePage.recipientResolveWidget( i );
        assuan_assert( rr );
        rr->setIdentifier( recipients[i].prettyAddress() );
        rr->setCertificates( keys[i] );
    }
}

void SignEncryptWizard::setSignersAndCandidates( const std::vector<Mailbox> & signers, const std::vector< std::vector<Key> > & keys ) {
    assuan_assert( !keys.empty() );
    assuan_assert( d->mode == SignEMail );
    notImplemented();
}

bool SignEncryptWizard::canGoToNextPage() const {
    assuan_assert( currentPage() );
    return currentPage()->isComplete();
}

void SignEncryptWizard::connectTask( const shared_ptr<Task> & task, unsigned int idx ) {
    notImplemented();
}

std::vector<Key> SignEncryptWizard::resolvedCertificates() const {
    std::vector<Key> result;
    for ( unsigned int i = 0, end = d->ui.recipientResolvePage.numRecipientResolveWidgets() ; i < end ; ++i )
        result.push_back( d->ui.recipientResolvePage.recipientResolveWidget( i )->chosenCertificate() );
    return result;
}

#include "moc_signencryptwizard.cpp"
