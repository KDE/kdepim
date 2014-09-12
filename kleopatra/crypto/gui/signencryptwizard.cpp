/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signencryptwizard.cpp

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

#include <config-kleopatra.h>

#include "signencryptwizard.h"

#include "objectspage.h"
#include "resolverecipientspage.h"
#include "signerresolvepage.h"
#include "resultpage.h"

#include <crypto/task.h>
#include <crypto/taskcollection.h>
#include <crypto/certificateresolver.h>

#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>

#include <gpgme++/key.h>

#include <KConfig>

#include <QFileInfo>
#include <QTimer>

#include <boost/bind.hpp>
#include <KSharedConfig>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class SignEncryptWizard::Private {
    friend class ::Kleo::Crypto::Gui::SignEncryptWizard;
    SignEncryptWizard * q;
public:
    explicit Private( SignEncryptWizard * qq );
    ~Private();

    void setCommitPage( Page page );

    Gui::ResolveRecipientsPage * recipientResolvePage; // clashes with enum of same name
    SignerResolvePage * signerResolvePage;
    Gui::ObjectsPage * objectsPage; // clashes with enum of same name
    Gui::ResultPage * resultPage;  // clashes with enum of same name
};


SignEncryptWizard::Private::Private( SignEncryptWizard * qq )
    : q( qq ),
      recipientResolvePage( new Gui::ResolveRecipientsPage ),
      signerResolvePage( new SignerResolvePage ),
      objectsPage( new Gui::ObjectsPage ),
      resultPage( new Gui::ResultPage )
{
    connect(resultPage, &Gui::ResultPage::linkActivated, q, &SignEncryptWizard::linkActivated);
    q->setPage( SignEncryptWizard::ResolveSignerPage, signerResolvePage );
    q->setPage( SignEncryptWizard::ObjectsPage, objectsPage );
    q->setPage( SignEncryptWizard::ResolveRecipientsPage, recipientResolvePage );
    q->setPage( SignEncryptWizard::ResultPage, resultPage );
    //TODO: move the RecipientPreferences creation out of here, don't create a new instance for each wizard
    recipientResolvePage->setRecipientPreferences( shared_ptr<RecipientPreferences>( new KConfigBasedRecipientPreferences( KSharedConfig::openConfig() ) ) );
    signerResolvePage->setSigningPreferences( shared_ptr<SigningPreferences>( new KConfigBasedSigningPreferences( KSharedConfig::openConfig() ) ) );
    q->resize( QSize( 640, 480 ).expandedTo( q->sizeHint() ) );
}

void SignEncryptWizard::onNext( int currentId )
{
    if ( currentId == ResolveRecipientsPage )
        QTimer::singleShot( 0, this, SIGNAL(recipientsResolved()) );
    if ( currentId == ResolveSignerPage ) {
        //FIXME: Sign&Encrypt is only supported by OpenPGP. Remove this when we support this for CMS, too
        if ( encryptionSelected() && signingSelected() )
            setPresetProtocol( OpenPGP );
        QTimer::singleShot( 0, this, SIGNAL(signersResolved()) );
    }
    if ( currentId == ObjectsPage )
        QTimer::singleShot( 0, this, SIGNAL(objectsResolved()) );
}

SignEncryptWizard::Private::~Private() {}

SignEncryptWizard::SignEncryptWizard( QWidget * p, Qt::WindowFlags f )
    : Wizard( p, f ), d( new Private( this ) )
{
}


SignEncryptWizard::~SignEncryptWizard() {}

void SignEncryptWizard::setCommitPage( Page page )
{
    d->setCommitPage( page );
}

void SignEncryptWizard::Private::setCommitPage( Page page )
{
    q->page( ResolveSignerPage )->setCommitPage( false );
    q->page( ResolveRecipientsPage )->setCommitPage( false );
    q->page( ObjectsPage )->setCommitPage( false );
    q->page( ResultPage )->setCommitPage( false );
    q->page( page )->setCommitPage( true );
}

void SignEncryptWizard::setPresetProtocol( Protocol proto ) {
    d->signerResolvePage->setPresetProtocol( proto );
    d->signerResolvePage->setProtocolSelectionUserMutable( proto == UnknownProtocol );
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
    return d->recipientResolvePage->multipleProtocolsAllowed();
}

void SignEncryptWizard::setMultipleProtocolsAllowed( bool allowed )
{
    d->signerResolvePage->setMultipleProtocolsAllowed( allowed );
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
    QFileInfoList fileInfos;
    Q_FOREACH( const QString& i, files )
        fileInfos.push_back( QFileInfo( i ) );
    return fileInfos;
}

bool SignEncryptWizard::signingSelected() const {
    return d->signerResolvePage->signingSelected();
}

bool SignEncryptWizard::encryptionSelected() const {
    return d->signerResolvePage->encryptionSelected();
}

void SignEncryptWizard::setRecipients( const std::vector<Mailbox> & recipients, const std::vector<Mailbox> & encryptToSelfRecipients ) {
    d->recipientResolvePage->setRecipients( recipients, encryptToSelfRecipients );
}

void SignEncryptWizard::setSignersAndCandidates( const std::vector<Mailbox> & signers, const std::vector< std::vector<Key> > & keys ) {
    d->signerResolvePage->setSignersAndCandidates( signers, keys );
}


void SignEncryptWizard::setTaskCollection( const shared_ptr<TaskCollection> & coll )
{
    kleo_assert( coll );
    d->resultPage->setTaskCollection( coll );
}

std::vector<Key> SignEncryptWizard::resolvedCertificates() const {
    return d->recipientResolvePage->resolvedCertificates();
}

std::vector<Key> SignEncryptWizard::resolvedSigners() const {
    return d->signerResolvePage->resolvedSigners();
}

bool SignEncryptWizard::isAsciiArmorEnabled() const
{
    return d->signerResolvePage->isAsciiArmorEnabled();
}

void SignEncryptWizard::setAsciiArmorEnabled( bool enabled )
{
    d->signerResolvePage->setAsciiArmorEnabled( enabled );
}

bool SignEncryptWizard::removeUnencryptedFile() const
{
   return d->signerResolvePage->removeUnencryptedFile();
}

void SignEncryptWizard::setRemoveUnencryptedFile( bool remove )
{
    d->signerResolvePage->setRemoveUnencryptedFile( remove );
}

bool SignEncryptWizard::recipientsUserMutable() const
{
    return d->recipientResolvePage->recipientsUserMutable();
}

void SignEncryptWizard::setRecipientsUserMutable( bool isMutable )
{
    d->recipientResolvePage->setRecipientsUserMutable( isMutable );
}

void SignEncryptWizard::setSignerResolvePageValidator( const boost::shared_ptr<SignerResolvePage::Validator>& validator )
{
    d->signerResolvePage->setValidator( validator );
}

Gui::SignerResolvePage* SignEncryptWizard::signerResolvePage()
{
    return d->signerResolvePage;
}

const Gui::SignerResolvePage* SignEncryptWizard::signerResolvePage() const
{
    return d->signerResolvePage;
}

Gui::ResolveRecipientsPage* SignEncryptWizard::resolveRecipientsPage()
{
    return d->recipientResolvePage;
}

Gui::ObjectsPage* SignEncryptWizard::objectsPage()
{
    return d->objectsPage;
}

Gui::ResultPage* SignEncryptWizard::resultPage()
{
    return d->resultPage;
}

bool SignEncryptWizard::keepResultPageOpenWhenDone() const
{
    return d->resultPage->keepOpenWhenDone();
}

void SignEncryptWizard::setKeepResultPageOpenWhenDone( bool keep )
{
    d->resultPage->setKeepOpenWhenDone( keep );
}

