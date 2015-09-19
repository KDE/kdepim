/*
    qgpgmebackend.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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

#ifndef __KLEO_QGPGMEBACKEND_H__
#define __KLEO_QGPGMEBACKEND_H__

#include "libkleo/cryptobackend.h"

namespace Kleo
{
class CryptoConfig;
}
class QString;

namespace Kleo
{

class QGpgMEBackend : public Kleo::CryptoBackend
{
public:
    QGpgMEBackend();
    ~QGpgMEBackend();

    QString name() const Q_DECL_OVERRIDE;
    QString displayName() const Q_DECL_OVERRIDE;

    CryptoConfig *config() const Q_DECL_OVERRIDE;

    Protocol *openpgp() const Q_DECL_OVERRIDE;
    Protocol *smime() const Q_DECL_OVERRIDE;
    Protocol *protocol(const char *name) const Q_DECL_OVERRIDE;

    bool checkForOpenPGP(QString *reason = Q_NULLPTR) const Q_DECL_OVERRIDE;
    bool checkForSMIME(QString *reason = Q_NULLPTR) const Q_DECL_OVERRIDE;
    bool checkForProtocol(const char *name, QString *reason) const Q_DECL_OVERRIDE;

    bool supportsOpenPGP() const Q_DECL_OVERRIDE
    {
        return true;
    }
    bool supportsSMIME() const Q_DECL_OVERRIDE
    {
        return true;
    }
    bool supportsProtocol(const char *name) const Q_DECL_OVERRIDE;

    const char *enumerateProtocols(int i) const Q_DECL_OVERRIDE;

private:
    mutable Kleo::CryptoConfig *mCryptoConfig;
    mutable Protocol *mOpenPGPProtocol;
    mutable Protocol *mSMIMEProtocol;
};

}

#endif // __KLEO_QGPGMEBACKEND_H__
