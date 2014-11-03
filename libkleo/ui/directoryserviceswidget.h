/*
    directoryserviceswidget.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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

#ifndef DIRECTORYSERVICESWIDGET_H
#define DIRECTORYSERVICESWIDGET_H

#include "kleo_export.h"
#include <kurl.h>
#include <QWidget>

namespace Kleo
{

class KLEO_EXPORT DirectoryServicesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DirectoryServicesWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DirectoryServicesWidget();

    enum Scheme {
        NoScheme = 0,
        HKP      = 1,
        HTTP     = 2,
        FTP      = 4,
        LDAP     = 8,

        AllSchemes = HKP | HTTP | FTP | LDAP
    };
    Q_DECLARE_FLAGS(Schemes, Scheme)

    enum Protocol {
        NoProtocol = 0,
        X509Protocol = 1,
        OpenPGPProtocol = 2,

        AllProtocols = X509Protocol | OpenPGPProtocol
    };
    Q_DECLARE_FLAGS(Protocols, Protocol)

    void setAllowedSchemes(Schemes schemes);
    Schemes allowedSchemes() const;

    void setAllowedProtocols(Protocols protocols);
    Protocols allowedProtocols() const;

    void setX509Allowed(bool allowed);
    void setOpenPGPAllowed(bool allowed);

    void setReadOnlyProtocols(Protocols protocols);
    Protocols readOnlyProtocols() const;

    void setOpenPGPReadOnly(bool ro);
    void setX509ReadOnly(bool ro);

    void addOpenPGPServices(const KUrl::List &urls);
    KUrl::List openPGPServices() const;

    void addX509Services(const KUrl::List &urls);
    KUrl::List x509Services() const;

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void changed();

private:
    class Private;
    Private *const d;
    Q_PRIVATE_SLOT(d, void slotNewClicked())
    Q_PRIVATE_SLOT(d, void slotNewX509Clicked())
    Q_PRIVATE_SLOT(d, void slotNewOpenPGPClicked())
    Q_PRIVATE_SLOT(d, void slotDeleteClicked())
    Q_PRIVATE_SLOT(d, void slotSelectionChanged())
    Q_PRIVATE_SLOT(d, void slotShowUserAndPasswordToggled(bool))
};

}

inline void Kleo::DirectoryServicesWidget::setOpenPGPAllowed(bool allowed)
{
    if (allowed) {
        setAllowedProtocols(allowedProtocols() | OpenPGPProtocol);
    } else {
        setAllowedProtocols(allowedProtocols() & ~OpenPGPProtocol);
    }
}

inline void Kleo::DirectoryServicesWidget::setX509Allowed(bool allowed)
{
    if (allowed) {
        setAllowedProtocols(allowedProtocols() | X509Protocol);
    } else {
        setAllowedProtocols(allowedProtocols() & ~X509Protocol);
    }
}

inline void Kleo::DirectoryServicesWidget::setOpenPGPReadOnly(bool ro)
{
    if (ro) {
        setReadOnlyProtocols(readOnlyProtocols() | OpenPGPProtocol);
    } else {
        setReadOnlyProtocols(readOnlyProtocols() & ~OpenPGPProtocol);
    }
}

inline void Kleo::DirectoryServicesWidget::setX509ReadOnly(bool ro)
{
    if (ro) {
        setReadOnlyProtocols(readOnlyProtocols() | X509Protocol);
    } else {
        setReadOnlyProtocols(readOnlyProtocols() & ~X509Protocol);
    }
}

#endif // DIRECTORYSERVICESWIDGET_H
