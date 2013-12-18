/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signencryptfileswizard.cpp

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

#include "signencryptfileswizard.h"
#include "signerresolvepage.h"

#include <KLocalizedString>

#include <gpgme++/key.h>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;
using namespace GpgME;

namespace {

    class SignerResolveValidator : public SignerResolvePage::Validator {
    public:
        explicit SignerResolveValidator( SignerResolvePage* page );
        bool isComplete() const;
        QString explanation() const;
        void update() const;
        QString customWindowTitle() const;

    private:
        SignerResolvePage* const m_page;
        mutable QString expl;
        mutable bool complete;
    };
}

SignerResolveValidator::SignerResolveValidator( SignerResolvePage* page ) : SignerResolvePage::Validator(), m_page( page ), complete( true )
{
    assert( m_page );
}

void SignerResolveValidator::update() const {
    expl.clear();
    const bool needPgpSC = m_page->operation() == SignerResolvePage::SignAndEncrypt;
    const bool isSignEncrypt = m_page->operation() == SignerResolvePage::SignAndEncrypt;
    const bool needAnySC = m_page->operation() != SignerResolvePage::EncryptOnly;
    const bool havePgpSC = !m_page->signingCertificates( GpgME::OpenPGP ).empty();
    const bool haveCmsSC = !m_page->signingCertificates( GpgME::CMS ).empty();
    const bool haveAnySC = havePgpSC || haveCmsSC;
    complete = ( !needPgpSC || havePgpSC ) && ( !needAnySC || haveAnySC );

#undef setAndReturn
#define setAndReturn(text) { expl = text; return; }

    if( needPgpSC && !havePgpSC )
        setAndReturn( i18n( "You need to select an OpenPGP signing certificate to perform this operation." ) );

    if ( needAnySC && !haveAnySC )
        setAndReturn( i18n( "You need to select at least one signing certificate to proceed." ) );

    if ( isSignEncrypt && needPgpSC && havePgpSC )
        setAndReturn( i18n( "Only OpenPGP certificates will be offered for selection because you specified a combined sign/encrypt operation that is only available for OpenPGP." ) );

    if ( isSignEncrypt && havePgpSC && !haveCmsSC )
        setAndReturn( i18n( "Only OpenPGP certificates will be offered for selection because you only specified an OpenPGP signing certificate." ) );

    if ( haveCmsSC && !havePgpSC )
        setAndReturn( i18n( "Only S/MIME certificates will be offered for selection because you only specified an S/MIME signing certificate." ) );

    switch ( m_page->operation() )
    {
    case SignerResolvePage::SignOnly:
        if ( havePgpSC && haveCmsSC )
            expl = i18n( "You have selected both OpenPGP and S/MIME signing certificate types, thus two signatures will be created." );
        break;
    case SignerResolvePage::SignAndEncrypt:
    case SignerResolvePage::EncryptOnly:
        expl = i18n( "If you select both OpenPGP and S/MIME receipient certificates, two encrypted files will be created: one for OpenPGP recipients, one for S/MIME recipients." );
        break;
    }


#undef setAndReturn
}

bool SignerResolveValidator::isComplete() const
{
    update();
    return complete;
}

QString SignerResolveValidator::explanation() const
{
    update();
    return expl;
}

QString SignerResolveValidator::customWindowTitle() const
{
    const bool sign = m_page->signingSelected();
    const bool encr = m_page->encryptionSelected();

    return sign && encr ? i18n( "Sign/Encrypt Files" ) :
           sign         ? i18n( "Sign Files" )         :
           encr         ? i18n( "Encrypt Files" ) : QString();
}

class SignEncryptFilesWizard::Private {
    friend class ::Kleo::Crypto::Gui::SignEncryptFilesWizard;
    SignEncryptFilesWizard * const q;
public:
    explicit Private( SignEncryptFilesWizard * qq );
    ~Private();

    void operationSelected();
private:

};


SignEncryptFilesWizard::Private::Private( SignEncryptFilesWizard * qq )
  : q( qq )
{
    q->connect( q, SIGNAL(signersResolved()), q, SLOT(operationSelected()) );
    std::vector<int> pageOrder;
    q->setSignerResolvePageValidator( boost::shared_ptr<SignerResolveValidator>( new SignerResolveValidator( q->signerResolvePage() ) ) );
    pageOrder.push_back( SignEncryptWizard::ResolveSignerPage );
    pageOrder.push_back( SignEncryptWizard::ObjectsPage );
    pageOrder.push_back( SignEncryptWizard::ResolveRecipientsPage );
    pageOrder.push_back( SignEncryptWizard::ResultPage );
    q->setPageOrder( pageOrder );
    q->setCommitPage( SignEncryptWizard::ResolveRecipientsPage );
    std::vector<Protocol> protocols;
    protocols.push_back( OpenPGP );
    protocols.push_back( CMS );
    q->setMultipleProtocolsAllowed( true );
    q->setRecipientsUserMutable( true );
}

SignEncryptFilesWizard::Private::~Private() {}

void SignEncryptFilesWizard::Private::operationSelected()
{
    const bool encrypt = q->encryptionSelected();
    q->setPageVisible( SignEncryptWizard::ResolveRecipientsPage, encrypt );
    q->setCommitPage( encrypt ? SignEncryptWizard::ResolveRecipientsPage : SignEncryptWizard::ObjectsPage );
}

SignEncryptFilesWizard::SignEncryptFilesWizard( QWidget * parent, Qt::WindowFlags f )
  : SignEncryptWizard( parent, f ), d( new Private( this ) )
{
}

SignEncryptFilesWizard::~SignEncryptFilesWizard() {}

void SignEncryptFilesWizard::onNext( int currentId )
{
    SignEncryptWizard::onNext( currentId );
    const bool encrypt = encryptionSelected();

    if ( ( encrypt && currentId == ResolveRecipientsPage ) || ( !encrypt && currentId == ObjectsPage ) )
        emit operationPrepared();
}

#include "moc_signencryptfileswizard.cpp"

