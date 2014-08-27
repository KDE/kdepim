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

#ifndef MESSAGECOMPSER_SIGNATURECONTROLLER_H
#define MESSAGECOMPSER_SIGNATURECONTROLLER_H

#include "messagecomposer_export.h"
#include <QtCore/QObject>
#include <KIdentityManagement/kidentitymanagement/signature.h>

namespace KIdentityManagement
{
class IdentityCombo;
}

namespace MessageComposer {
class KMeditor;

/** Controls signature (the footer thing, not the crypto thing) operations
 *  happening on a KMEditor triggerd by identity selection or menu actions.
 *  @since 4.5
 */
class MESSAGECOMPOSER_EXPORT SignatureController : public QObject
{
    Q_OBJECT
public:
    explicit SignatureController( QObject* parent = 0 );

    void setEditor( MessageComposer::KMeditor* editor );
    void setIdentityCombo( KIdentityManagement::IdentityCombo* combo );

    /** Temporarily disable identity tracking, useful for initial loading for example. */
    void suspend();
    /** Resume identity change tracking after a previous call to suspend(). */
    void resume();

    /** Adds the given signature to the editor, taking user preferences into account.
    */
    void applySignature( const KIdentityManagement::Signature& signature );

    /** Applys the currently selected signature according to user preferences. */
    void applyCurrentSignature();

public slots:
    /**
     * Append signature to the end of the text in the editor.
     */
    void appendSignature();

    /**
     * Prepend signature at the beginning of the text in the editor.
     */
    void prependSignature();

    /**
     * Insert signature at the cursor position of the text in the editor.
     */
    void insertSignatureAtCursor();

    void cleanSpace();

signals:
    /**
     * A HTML signature is about to be inserted, so enable HTML support in the editor.
     */
    void enableHtml();
    void signatureAdded();

private:
    /**
     * Helper to insert the signature of the current identity arbitrarily
     * in the editor, connecting slot functions to KMeditor::insertSignature().
     * @param placement the position of the signature
     */
    void insertSignatureHelper( KIdentityManagement::Signature::Placement placement );

private slots:
    void identityChanged ( uint id );

private:
    MessageComposer::KMeditor* m_editor;
    KIdentityManagement::IdentityCombo* m_identityCombo;
    uint m_currentIdentityId;
};

}

#endif
