/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signencryptemailconflictdialog.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_CRYPTO_GUI_SIGNENCRYPTEMAILCONFLICTDIALOG_H__
#define __KLEOPATRA_CRYPTO_GUI_SIGNENCRYPTEMAILCONFLICTDIALOG_H__

#include <QDialog>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <vector>

namespace boost
{
template <typename T> class shared_ptr;
}

namespace GpgME
{
class Key;
}

namespace Kleo
{
namespace Crypto
{
class Sender;
class Recipient;
}
}

namespace Kleo
{
namespace Crypto
{
namespace Gui
{

class SignEncryptEMailConflictDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SignEncryptEMailConflictDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~SignEncryptEMailConflictDialog();

    // Inputs

    void setPresetProtocol(GpgME::Protocol proto);
    void setSubject(const QString &subject);

    void setSenders(const std::vector<Sender> &senders);
    void setRecipients(const std::vector<Recipient> &recipients);

    void setSign(bool on);
    void setEncrypt(bool on);

    void setQuickMode(bool on);

    // To wrap up inputs:
    void pickProtocol();
    void setConflict(bool conflict);

    // Intermediate

    bool isComplete() const;

    // Outputs

    GpgME::Protocol selectedProtocol() const;
    std::vector<GpgME::Key> resolvedSigningKeys() const;
    std::vector<GpgME::Key> resolvedEncryptionKeys() const;

    bool isQuickMode() const;

private:
    Q_PRIVATE_SLOT(d, void slotCompleteChanged())
    Q_PRIVATE_SLOT(d, void slotShowAllRecipientsToggled(bool))
    Q_PRIVATE_SLOT(d, void slotProtocolChanged())
    Q_PRIVATE_SLOT(d, void slotCertificateSelectionDialogRequested())
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

}
}
}

#endif /* __KLEOPATRA_CRYPTO_GUI_SIGNENCRYPTEMAILCONFLICTDIALOG_H__ */
