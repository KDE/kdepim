/*
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

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

#include "setupenv.h"

#include <Libkleo/KeyListJob>
#include <gpgme++/keylistresult.h>
#include <Libkleo/CryptoBackendFactory>

#include <QFile>
#include <QDir>
#include <QTest>

void MessageComposer::Test::setupEnv()
{
    setenv("LC_ALL", "C", 1);
    setenv("KDEHOME", QFile::encodeName(QDir::homePath() + QString::fromLatin1("/.qttest")), 1);

    QStringList reasons;
    Kleo::CryptoBackendFactory::instance()->scanForBackends(&reasons);
    if (!reasons.isEmpty()) {
        Q_FOREACH (const QString &reason, reasons) {
            qWarning() << reason;
        }
    }
}

std::vector< GpgME::Key, std::allocator< GpgME::Key > > MessageComposer::Test::getKeys(bool smime)
{
    Kleo::KeyListJob *job = 0;

    if (smime) {
        const Kleo::CryptoBackend::Protocol *const backend = Kleo::CryptoBackendFactory::instance()->smime();
        Q_ASSERT(backend);
        job = backend->keyListJob(false);
    } else {
        const Kleo::CryptoBackend::Protocol *const backend = Kleo::CryptoBackendFactory::instance()->openpgp();
        Q_ASSERT(backend);
        job = backend->keyListJob(false);
    }
    Q_ASSERT(job);

    std::vector< GpgME::Key > keys;
    GpgME::KeyListResult res = job->exec(QStringList(), true, keys);

    if (!smime) {
        Q_ASSERT(keys.size() == 3);
    }

    Q_ASSERT(!res.error());
    qDebug() << "got private keys:" << keys.size();

    for (std::vector< GpgME::Key >::iterator i = keys.begin(); i != keys.end(); ++i) {
        qDebug() << "key isnull:" << i->isNull() << "isexpired:" << i->isExpired();
        qDebug() << "key numuserIds:" << i->numUserIDs();
        for (uint k = 0; k < i->numUserIDs(); ++k) {
            qDebug() << "userIDs:" << i->userID(k).email();
        }
    }

    return keys;
}
