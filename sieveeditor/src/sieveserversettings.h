/*
   Copyright (C) 2014-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SieveServerSettings_H
#define SieveServerSettings_H

#include <QObject>
namespace KWallet
{
class Wallet;
}
class SieveServerSettings : public QObject
{
    Q_OBJECT
public:
    ~SieveServerSettings();
    static SieveServerSettings *self();

    KWallet::Wallet *wallet();

    void closeWallet();
private Q_SLOTS:
    void slotWalletClosed();

private:
    explicit SieveServerSettings(QObject *parent = Q_NULLPTR);
    friend class SieveServerSettingsPrivate;
    KWallet::Wallet *mWallet;
};

#endif // SieveServerSettings_H
