/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/
#ifndef SieveServerSettings_H
#define SieveServerSettings_H

#include <QObject>
namespace KWallet {
class Wallet;
}
class SieveServerSettings : public QObject
{
    Q_OBJECT
public:
    ~SieveServerSettings();
    static SieveServerSettings *self();

    KWallet::Wallet *wallet();

private:
    explicit SieveServerSettings(QObject *parent=0);
    friend class SieveServerSettingsPrivate;
    KWallet::Wallet *mWallet;
};

#endif // SieveServerSettings_H
