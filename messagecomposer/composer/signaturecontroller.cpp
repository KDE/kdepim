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

#include <KPIMIdentities/kpimidentities/identity.h>
#include <KPIMIdentities/kpimidentities/identitycombo.h>
#include <KPIMIdentities/kpimidentities/identitymanager.h>

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

void SignatureController::setIdentityCombo ( KPIMIdentities::IdentityCombo* combo )
{
    m_identityCombo = combo;
    m_currentIdentityId = combo->currentIdentity();
    resume();
}

void SignatureController::identityChanged ( uint id )
{
    Q_ASSERT( m_identityCombo );
    const KPIMIdentities::Identity &newIdentity = m_identityCombo->identityManager()->identityForUoid( id );
    if ( newIdentity.isNull() || !m_editor )
        return;

    const KPIMIdentities::Identity &oldIdentity = m_identityCombo->identityManager()->identityForUoidOrDefault( m_currentIdentityId );

    const KPIMIdentities::Signature oldSig = const_cast<KPIMIdentities::Identity&>( oldIdentity ).signature();
    const KPIMIdentities::Signature newSig = const_cast<KPIMIdentities::Identity&>( newIdentity ).signature();

    const bool replaced = m_editor->replaceSignature( oldSig, newSig );

    // Just append the signature if there was no old signature
    if ( !replaced && oldSig.rawText().isEmpty() )
        applySignature( newSig );

    m_currentIdentityId = id;
}

void SignatureController::suspend()
{
    if ( m_identityCombo )
        disconnect( m_identityCombo, SIGNAL(identityChanged(uint)), this, SLOT(identityChanged(uint)) );
}

void SignatureController::resume()
{
    if ( m_identityCombo )
        connect(m_identityCombo, &KPIMIdentities::IdentityCombo::identityChanged, this, &SignatureController::identityChanged);
}

void SignatureController::appendSignature()
{
    insertSignatureHelper( KPIMIdentities::Signature::End );
}

void SignatureController::prependSignature()
{
    insertSignatureHelper( KPIMIdentities::Signature::Start );
}

void SignatureController::insertSignatureAtCursor()
{
    insertSignatureHelper( KPIMIdentities::Signature::AtCursor );
}

void SignatureController::cleanSpace()
{
    if ( !m_editor || !m_identityCombo )
        return;
    const KPIMIdentities::Identity &ident = m_identityCombo->identityManager()->identityForUoidOrDefault( m_identityCombo->currentIdentity() );
    const KPIMIdentities::Signature signature = const_cast<KPIMIdentities::Identity&>( ident ).signature();
    m_editor->cleanWhitespace( signature );
}

void SignatureController::insertSignatureHelper ( KPIMIdentities::Signature::Placement placement )
{
    if ( !m_identityCombo || !m_editor )
        return;

    // Identity::signature() is not const, although it should be, therefore the
    // const_cast.
    KPIMIdentities::Identity &ident = const_cast<KPIMIdentities::Identity&>(
                m_identityCombo->identityManager()->identityForUoidOrDefault(
                    m_identityCombo->currentIdentity() ) );
    const KPIMIdentities::Signature signature = ident.signature();

    if ( signature.isInlinedHtml() &&
         signature.type() == KPIMIdentities::Signature::Inlined ) {
        emit enableHtml();
    }

    KPIMIdentities::Signature::AddedText addedText = KPIMIdentities::Signature::AddNewLines;
    if ( MessageComposer::MessageComposerSettings::self()->dashDashSignature() )
        addedText |= KPIMIdentities::Signature::AddSeparator;
    signature.insertIntoTextEdit( placement, addedText, m_editor );
    if ((placement == KPIMIdentities::Signature::Start) || (placement == KPIMIdentities::Signature::End)) {
        emit signatureAdded();
    }
}

void SignatureController::applySignature( const KPIMIdentities::Signature &signature )
{
    if ( !m_editor )
        return;

    if ( MessageComposer::MessageComposerSettings::self()->autoTextSignature() == QLatin1String("auto") ) {
        KPIMIdentities::Signature::AddedText addedText = KPIMIdentities::Signature::AddNewLines;
        if ( MessageComposer::MessageComposerSettings::self()->dashDashSignature() )
            addedText |= KPIMIdentities::Signature::AddSeparator;
        if ( MessageComposer::MessageComposerSettings::self()->prependSignature() )
            signature.insertIntoTextEdit( KPIMIdentities::Signature::Start,
                                          addedText, m_editor );
        else
            signature.insertIntoTextEdit( KPIMIdentities::Signature::End,
                                          addedText, m_editor );
    }
}

void SignatureController::applyCurrentSignature()
{
    if ( !m_identityCombo )
        return;
    KPIMIdentities::Identity &ident = const_cast<KPIMIdentities::Identity&>(
                m_identityCombo->identityManager()->identityForUoidOrDefault(
                    m_identityCombo->currentIdentity() ) );
    applySignature( ident.signature() );
}



