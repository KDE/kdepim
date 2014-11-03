/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signerresolvepage_p.h

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

#ifndef __KLEOPATRA_CRYPTO_GUI_SIGNERRESOLVEPAGE_P_H__
#define __KLEOPATRA_CRYPTO_GUI_SIGNERRESOLVEPAGE_P_H__

#include <gpgme++/global.h>

class QButtonGroup;
class QCheckBox;
class QLabel;

#include <vector>
#include <map>

namespace Kleo
{
namespace Crypto
{
namespace Gui
{

class AbstractSigningProtocolSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractSigningProtocolSelectionWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual void setProtocolChecked(GpgME::Protocol protocol, bool checked) = 0;
    virtual bool isProtocolChecked(GpgME::Protocol protocol) const = 0;
    virtual std::vector<GpgME::Protocol> checkedProtocols() const = 0;
    virtual void setCertificate(GpgME::Protocol protocol, const GpgME::Key &key) = 0;

Q_SIGNALS:
    void userSelectionChanged();
};

class SigningProtocolSelectionWidget : public AbstractSigningProtocolSelectionWidget
{
    Q_OBJECT
public:
    explicit SigningProtocolSelectionWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    void setProtocolChecked(GpgME::Protocol protocol, bool checked);
    bool isProtocolChecked(GpgME::Protocol protocol) const;
    std::vector<GpgME::Protocol> checkedProtocols() const;
    void setCertificate(GpgME::Protocol protocol, const GpgME::Key &key);

    void setExclusive(bool exclusive);
    bool isExclusive() const;

private:
    QCheckBox *button(GpgME::Protocol p) const;
    std::map<GpgME::Protocol, QCheckBox *> m_buttons;
    QButtonGroup *m_buttonGroup;
};

class ReadOnlyProtocolSelectionWidget : public AbstractSigningProtocolSelectionWidget
{
    Q_OBJECT
public:
    explicit ReadOnlyProtocolSelectionWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    void setProtocolChecked(GpgME::Protocol protocol, bool checked);
    bool isProtocolChecked(GpgME::Protocol protocol) const;
    std::vector<GpgME::Protocol> checkedProtocols() const;
    void setCertificate(GpgME::Protocol protocol, const GpgME::Key &key);

private:
    QLabel *label(GpgME::Protocol p) const;
    std::map<GpgME::Protocol, QLabel *> m_labels;
};

}
}
}

#endif // __KLEOPATRA_CRYPTO_GUI_SIGNERRESOLVEPAGE_P_H__
