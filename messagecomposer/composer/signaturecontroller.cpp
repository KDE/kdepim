/*
 * Copyright (c) 2010 Volker Krause <vkrause@kde.org>
 *
 * Based on kmail/kmcomposewin.cpp
 * Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * Based on KMail code by:
 * Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "signaturecontroller.h"
#include "composer/kmeditor.h"
#include "settings/messagecomposersettings.h"

#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <KIdentityManagement/kidentitymanagement/identitycombo.h>
#include <KIdentityManagement/kidentitymanagement/identitymanager.h>

using namespace MessageComposer;

SignatureController::SignatureController ( QObject* parent ) :
    QObject ( parent ),
    m_editor( 0 ),
    m_identityCombo( 0 ),
    m_currentIdentityId( 0 )
{
}

void SignatureController::setEditor ( MessageComposer::KMeditor* editor )
{
    m_editor = editor;
}

void SignatureController::setIdentityCombo ( KIdentityManagement::IdentityCombo* combo )
{
    m_identityCombo = combo;
    m_currentIdentityId = combo->currentIdentity();
    resume();
}

void SignatureController::identityChanged ( uint id )
{
    Q_ASSERT( m_identityCombo );
    const KIdentityManagement::Identity &newIdentity = m_identityCombo->identityManager()->identityForUoid( id );
    if ( newIdentity.isNull() || !m_editor )
        return;

    const KIdentityManagement::Identity &oldIdentity = m_identityCombo->identityManager()->identityForUoidOrDefault( m_currentIdentityId );

    const KIdentityManagement::Signature oldSig = const_cast<KIdentityManagement::Identity&>( oldIdentity ).signature();
    const KIdentityManagement::Signature newSig = const_cast<KIdentityManagement::Identity&>( newIdentity ).signature();

    const bool replaced = m_editor->replaceSignature( oldSig, newSig );

    // Just append the signature if there was no old signature
    if ( !replaced && oldSig.rawText().isEmpty() )
        applySignature( newSig );

    m_currentIdentityId = id;
}

void SignatureController::suspend()
{
    if ( m_identityCombo )
        disconnect(m_identityCombo, &KIdentityManagement::IdentityCombo::identityChanged, this, &SignatureController::identityChanged);
}

void SignatureController::resume()
{
    if ( m_identityCombo )
        connect(m_identityCombo, &KIdentityManagement::IdentityCombo::identityChanged, this, &SignatureController::identityChanged);
}

void SignatureController::appendSignature()
{
    insertSignatureHelper( KIdentityManagement::Signature::End );
}

void SignatureController::prependSignature()
{
    insertSignatureHelper( KIdentityManagement::Signature::Start );
}

void SignatureController::insertSignatureAtCursor()
{
    insertSignatureHelper( KIdentityManagement::Signature::AtCursor );
}

void SignatureController::cleanSpace()
{
    if ( !m_editor || !m_identityCombo )
        return;
    const KIdentityManagement::Identity &ident = m_identityCombo->identityManager()->identityForUoidOrDefault( m_identityCombo->currentIdentity() );
    const KIdentityManagement::Signature signature = const_cast<KIdentityManagement::Identity&>( ident ).signature();
    m_editor->cleanWhitespace( signature );
}

void SignatureController::insertSignatureHelper ( KIdentityManagement::Signature::Placement placement )
{
    if ( !m_identityCombo || !m_editor )
        return;

    // Identity::signature() is not const, although it should be, therefore the
    // const_cast.
    KIdentityManagement::Identity &ident = const_cast<KIdentityManagement::Identity&>(
                m_identityCombo->identityManager()->identityForUoidOrDefault(
                    m_identityCombo->currentIdentity() ) );
    const KIdentityManagement::Signature signature = ident.signature();

    if ( signature.isInlinedHtml() &&
         signature.type() == KIdentityManagement::Signature::Inlined ) {
        emit enableHtml();
    }

    KIdentityManagement::Signature::AddedText addedText = KIdentityManagement::Signature::AddNewLines;
    if ( MessageComposer::MessageComposerSettings::self()->dashDashSignature() )
        addedText |= KIdentityManagement::Signature::AddSeparator;
    signature.insertIntoTextEdit( placement, addedText, m_editor );
    if ((placement == KIdentityManagement::Signature::Start) || (placement == KIdentityManagement::Signature::End)) {
        emit signatureAdded();
    }
}

void SignatureController::applySignature( const KIdentityManagement::Signature &signature )
{
    if ( !m_editor )
        return;

    if ( MessageComposer::MessageComposerSettings::self()->autoTextSignature() == QLatin1String("auto") ) {
        KIdentityManagement::Signature::AddedText addedText = KIdentityManagement::Signature::AddNewLines;
        if ( MessageComposer::MessageComposerSettings::self()->dashDashSignature() )
            addedText |= KIdentityManagement::Signature::AddSeparator;
        if ( MessageComposer::MessageComposerSettings::self()->prependSignature() )
            signature.insertIntoTextEdit( KIdentityManagement::Signature::Start,
                                          addedText, m_editor );
        else
            signature.insertIntoTextEdit( KIdentityManagement::Signature::End,
                                          addedText, m_editor );
    }
}

void SignatureController::applyCurrentSignature()
{
    if ( !m_identityCombo )
        return;
    KIdentityManagement::Identity &ident = const_cast<KIdentityManagement::Identity&>(
                m_identityCombo->identityManager()->identityForUoidOrDefault(
                    m_identityCombo->currentIdentity() ) );
    applySignature( ident.signature() );
}



