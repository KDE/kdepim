/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resolverecipientspage.h

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

#ifndef __KLEOPATRA_CRYPTO_GUI_RESOLVERECIPIENTSPAGE_H__
#define __KLEOPATRA_CRYPTO_GUI_RESOLVERECIPIENTSPAGE_H__

#include <crypto/gui/wizardpage.h>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace GpgME
{
class Key;
}

namespace KMime
{
namespace Types
{
class Mailbox;
}
}

namespace Kleo
{
namespace Crypto
{

class RecipientPreferences;

namespace Gui
{

class ResolveRecipientsPage : public WizardPage
{
    Q_OBJECT
public:
    explicit ResolveRecipientsPage(QWidget *parent = 0);
    ~ResolveRecipientsPage();

    bool isComplete() const;

    /**
     * The protocol selected by the user (which is chosen by
     * the user in case none was preset)
     */
    GpgME::Protocol selectedProtocol() const;

    /**
     * the protocol set before the dialog is shown. Defaults to
     * GpgME::UnknownProtocol */
    GpgME::Protocol presetProtocol() const;
    void setPresetProtocol(GpgME::Protocol protocol);

    bool multipleProtocolsAllowed() const;
    void setMultipleProtocolsAllowed(bool allowed);

    bool symmetricEncryptionSelected() const;
    void setSymmetricEncryptionSelected(bool enabled);

    bool symmetricEncryptionSelectable() const;
    void setSymmetricEncryptionSelectable(bool selectable);

    /** if true, the user is allowed to remove/add recipients via the UI.
     * Defaults to @p false.
     */
    bool recipientsUserMutable() const;
    void setRecipientsUserMutable(bool isMutable);

    void setAdditionalRecipientsInfo(const std::vector<GpgME::Key> &recipients);

    void setRecipients(const std::vector<KMime::Types::Mailbox> &recipients, const std::vector<KMime::Types::Mailbox> &encryptToSelfRecipients);
    std::vector<GpgME::Key> resolvedCertificates() const;

    boost::shared_ptr<RecipientPreferences> recipientPreferences() const;
    void setRecipientPreferences(const boost::shared_ptr<RecipientPreferences> &prefs);

Q_SIGNALS:
    void selectedProtocolChanged();

private:
    /*reimpl*/ void onNext();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void selectionChanged())
    Q_PRIVATE_SLOT(d, void protocolSelected(int))
    Q_PRIVATE_SLOT(d, void addRecipient())
    Q_PRIVATE_SLOT(d, void removeSelectedEntries())
    Q_PRIVATE_SLOT(d, void completeChangedInternal())
    class ListWidget;
    class ItemWidget;
};

}
}
}

#endif // __KLEOPATRA_CRYPTO_GUI_RESOLVERECIPIENTSPAGE_H__
