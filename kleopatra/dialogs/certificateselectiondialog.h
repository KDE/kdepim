/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/certificateselectiondialog.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_DIALOGS_CERTIFICATESELECTIONDIALOG_H__
#define __KLEOPATRA_DIALOGS_CERTIFICATESELECTIONDIALOG_H__

#include <QDialog>

#include <utils/pimpl_ptr.h>

#include <vector>

namespace GpgME
{
class Key;
}

namespace boost
{
template <typename T> class shared_ptr;
}

namespace Kleo
{

class KeyFilter;

namespace Dialogs
{

class CertificateSelectionDialog : public QDialog
{
    Q_OBJECT
    Q_FLAGS(Options)
public:
    enum Option {
        SingleSelection = 0x00,
        MultiSelection  = 0x01,

        SignOnly        = 0x02,
        EncryptOnly     = 0x04,
        AnyCertificate  = 0x06,

        OpenPGPFormat   = 0x08,
        CMSFormat       = 0x10,
        AnyFormat       = 0x18,

        Certificates    = 0x00,
        SecretKeys      = 0x20,

        OptionMask
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit CertificateSelectionDialog(QWidget *parent = 0);
    ~CertificateSelectionDialog();

    void setCustomLabelText(const QString &text);
    QString customLabelText() const;

    void setOptions(Options options);
    Options options() const;

    void selectCertificates(const std::vector<GpgME::Key> &certs);
    void selectCertificate(const GpgME::Key &key);

    std::vector<GpgME::Key> selectedCertificates() const;
    GpgME::Key selectedCertificate() const;

public Q_SLOTS:
    void setStringFilter(const QString &text);
    void setKeyFilter(const boost::shared_ptr<Kleo::KeyFilter> &filter);
    /* reimp */ void accept();

protected:
    void hideEvent(QHideEvent *);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void reload())
    Q_PRIVATE_SLOT(d, void create())
    Q_PRIVATE_SLOT(d, void lookup())
    Q_PRIVATE_SLOT(d, void slotKeysMayHaveChanged())
    Q_PRIVATE_SLOT(d, void slotSelectionChanged())
    Q_PRIVATE_SLOT(d, void slotDoubleClicked(QModelIndex))
    Q_PRIVATE_SLOT(d, void slotCurrentViewChanged(QAbstractItemView *))
};

}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Kleo::Dialogs::CertificateSelectionDialog::Options)

#endif /* __KLEOPATRA_DIALOGS_CERTIFICATESELECTIONDIALOG_H__ */
