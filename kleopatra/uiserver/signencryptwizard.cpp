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

#include "certificateresolver.h"
#include "objectspage.h"
#include "recipientresolvepage.h"
#include "signerresolvepage.h"
#include "resultdisplaywidget.h"
#include "wizardresultpage.h"
#include "task.h"
#include "kleo-assuan.h"

#include <utils/stl_util.h>

#include <kmime/kmime_header_parsing.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KPushButton>

#include <QFileInfo>
#include <QTimer>
#include <QWizard>

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

    void selectedProtocolChanged();

private:
    std::vector<Mailbox> recipients;
    Mode mode;

    RecipientResolvePage * recipientResolvePage;
    SignerResolvePage * signerResolvePage;
    Kleo::ObjectsPage * objectsPage;
    WizardResultPage * resultPage;
};


SignEncryptWizard::Private::Private( SignEncryptWizard * qq )
    : q( qq ),
      mode( EncryptOrSignFiles ),
      recipientResolvePage( new RecipientResolvePage ),
      signerResolvePage( new SignerResolvePage ),
      objectsPage( new Kleo::ObjectsPage ),
      resultPage( new WizardResultPage )
{
    q->connect( recipientResolvePage, SIGNAL( selectedProtocolChanged() ),
             q, SLOT( selectedProtocolChanged() ) );
    q->setPage( SignEncryptWizard::ResolveSignerPage, signerResolvePage );
    q->setPage( SignEncryptWizard::ObjectsPage, objectsPage );
    q->setPage( SignEncryptWizard::ResolveRecipientsPage, recipientResolvePage );
    q->setPage( SignEncryptWizard::ResultPage, resultPage );
    q->resize( QSize( 640, 480 ).expandedTo( q->sizeHint() ) );
}

void SignEncryptWizard::Private::selectedProtocolChanged()
{
    //TODO: this slot is a temporary workaround to keep the 
    // recipient resolving in the wizard for now. should be 
    // changed when reworking the recipientresolvepage
    const std::vector< std::vector<Key> > keys = CertificateResolver::resolveRecipients( recipients, q->selectedProtocol() );
    assuan_assert( !keys.empty() );
    assuan_assert( keys.size() == static_cast<size_t>( recipients.size() ) );

    for ( unsigned int i = 0, end = keys.size() ; i < end ; ++i ) {
        RecipientResolveWidget * const rr = recipientResolvePage->recipientResolveWidget( i );
        assuan_assert( rr );
        rr->setIdentifier( recipients[i].prettyAddress() );
        rr->setCertificates( keys[i] );
    }

}

void SignEncryptWizard::onNext( int currentId )
{
    assert( currentId != NoPage );
    if ( currentId == ResolveRecipientsPage )
        QTimer::singleShot( 0, this, SIGNAL( recipientsResolved() ) );
    if ( currentId == ResolveSignerPage )
        QTimer::singleShot( 0, this, SIGNAL( signersResolved() ) );
    if ( currentId == ObjectsPage )
        QTimer::singleShot( 0, this, SIGNAL( objectsResolved() ) );
}

SignEncryptWizard::Private::~Private() {}

SignEncryptWizard::SignEncryptWizard( QWidget * p, Qt::WindowFlags f )
    : Wizard( p, f ), d( new Private( this ) )
{
}


SignEncryptWizard::~SignEncryptWizard() {}

void SignEncryptWizard::setMode( Mode mode ) {
    std::vector<int> pageOrder;
    switch ( mode )
    {
    case EncryptEMail:
        pageOrder.push_back( ResolveRecipientsPage );
        pageOrder.push_back( ResultPage );
        break;
    case SignEMail:
        pageOrder.push_back( ResolveSignerPage );
        pageOrder.push_back( ResultPage );
        break;
    case SignOrEncryptFiles:
        pageOrder.push_back( ResolveSignerPage );
        pageOrder.push_back( ObjectsPage );
        pageOrder.push_back( ResolveRecipientsPage );
        pageOrder.push_back( ResultPage );
        break;
    default:
        assuan_assert( !"Case not yet implemented" );
        break;
    }
    setPageOrder( pageOrder );
    setCurrentPage( pageOrder.front() );
    d->mode = mode;
}

void SignEncryptWizard::setPresetProtocol( Protocol proto ) {
    assuan_assert( d->mode == EncryptEMail || d->mode == SignEMail || d->mode == SignOrEncryptFiles );
    d->signerResolvePage->setProtocol( proto );
    d->recipientResolvePage->setPresetProtocol( proto );
}

GpgME::Protocol SignEncryptWizard::selectedProtocol() const
{
    return d->recipientResolvePage->selectedProtocol();
}

GpgME::Protocol SignEncryptWizard::presetProtocol() const
{
    return d->recipientResolvePage->presetProtocol();
}
 
void SignEncryptWizard::setEncryptionSelected( bool selected )
{
    d->signerResolvePage->setEncryptionSelected( selected );
}

void SignEncryptWizard::setSigningSelected( bool selected )
{
    d->signerResolvePage->setSigningSelected( selected );
}

bool SignEncryptWizard::isSigningUserMutable() const
{
    return d->signerResolvePage->isSigningUserMutable();
}

void SignEncryptWizard::setSigningUserMutable( bool isMutable )
{
    d->signerResolvePage->setSigningUserMutable( isMutable );
}

bool SignEncryptWizard::isEncryptionUserMutable() const
{
    return d->signerResolvePage->isEncryptionUserMutable();
}


bool SignEncryptWizard::isMultipleProtocolsAllowed() const
{
    return d->recipientResolvePage->isMultipleProtocolsAllowed();
}

void SignEncryptWizard::setMultipleProtocolsAllowed( bool allowed )
{
    d->recipientResolvePage->setMultipleProtocolsAllowed( allowed );
}

void SignEncryptWizard::setEncryptionUserMutable( bool isMutable )
{
    d->signerResolvePage->setEncryptionUserMutable( isMutable );
}

void SignEncryptWizard::setFiles( const QStringList & files ) {
    d->objectsPage->setFiles( files );
}

QFileInfoList SignEncryptWizard::resolvedFiles() const {
    const QStringList files = d->objectsPage->files();
    QFileInfoList infos;
    foreach ( const QString& i, files )
        infos.push_back( QFileInfo( i ) );
    return infos;          
}

bool SignEncryptWizard::signingSelected() const {
    return d->signerResolvePage->signingSelected();
}

bool SignEncryptWizard::encryptionSelected() const {
    return d->signerResolvePage->encryptionSelected();
}

void SignEncryptWizard::setRecipients( const std::vector<Mailbox> & recipients ) {
    assuan_assert( d->mode == EncryptEMail );
    d->recipients = recipients;
    d->recipientResolvePage->ensureIndexAvailable( recipients.size() - 1 );
    d->selectedProtocolChanged();
}

void SignEncryptWizard::setSignersAndCandidates( const std::vector<Mailbox> & signers, const std::vector< std::vector<Key> > & keys ) {;
    assuan_assert( d->mode == SignEMail );
    d->signerResolvePage->setSignersAndCandidates( signers, keys );
}



void SignEncryptWizard::connectTask( const shared_ptr<Task> & task, unsigned int idx ) {
    assuan_assert( task );
    ResultDisplayWidget* const item = new ResultDisplayWidget;
    item->setLabel( task->label() );
    connect( task.get(), SIGNAL( progress( QString, int, int ) ),
             item, SLOT( setProgress( QString, int, int ) ) );
    connect( task.get(), SIGNAL( error( int, QString ) ),
             item, SLOT( setError( int, QString ) ) );
    connect( task.get(), SIGNAL(result( boost::shared_ptr<const Kleo::Task::Result> ) ),
             item, SLOT( setResult( boost::shared_ptr<const Kleo::Task::Result> ) ) );
    d->resultPage->addResultItem( item );
}

std::vector<Key> SignEncryptWizard::resolvedCertificates() const {
    std::vector<Key> result;
    for ( unsigned int i = 0, end = d->recipientResolvePage->numRecipientResolveWidgets() ; i < end ; ++i )
        result.push_back( d->recipientResolvePage->recipientResolveWidget( i )->chosenCertificate() );
    return result;
}

std::vector<Key> SignEncryptWizard::resolvedSigners() const {
    return d->signerResolvePage->resolvedSigners();
}


#include "moc_signencryptwizard.cpp"
