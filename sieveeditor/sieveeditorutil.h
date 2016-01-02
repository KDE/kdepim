/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORUTIL_H
#define SIEVEEDITORUTIL_H

#include <QString>
#include <QUrl>
#include <MailTransport/mailtransport/transport.h>

namespace KWallet
{
class Wallet;
}
namespace SieveEditorUtil
{
struct SieveServerConfig {
    SieveServerConfig()
        : authenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN),
          port(-1),
          enabled(true)
    {

    }

    QUrl url() const;

    QString userName;
    QString password;
    QString serverName;
    MailTransport::Transport::EnumAuthenticationType::type authenticationType;
    int port;
    bool enabled;
};

QVector<SieveServerConfig> readServerSieveConfig();
void writeServerSieveConfig(const QVector<SieveEditorUtil::SieveServerConfig> &lstConfig);
void addServerSieveConfig(const SieveEditorUtil::SieveServerConfig &conf);
void writeSieveSettings(KWallet::Wallet *wallet, KSharedConfigPtr cfg, const SieveEditorUtil::SieveServerConfig &conf, int index);
}
Q_DECLARE_TYPEINFO(SieveEditorUtil::SieveServerConfig, Q_MOVABLE_TYPE);

#endif // SIEVEEDITORUTIL_H
