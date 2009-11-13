/*
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

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

#include "qtest_messagecomposer.h"


#include <kleo/enum.h>
#include <kleo/keylistjob.h>
#include <kleo/cryptobackendfactory.h>

#include <KDebug>
#include <QDir>
#include <QFile>

#include <stdlib.h>
#include <gpgme++/keylistresult.h>

std::vector<GpgME::Key> getKeys()
{

  setenv("GNUPGHOME", QDir::currentPath().toLocal8Bit() +  "/gnupg_home" , 1 );
  setenv("LC_ALL", "C", 1); \
  setenv("KDEHOME", QFile::encodeName( QDir::homePath() + QString::fromAscii( "/.kde-unit-test" ) ), 1);

  const Kleo::CryptoBackend::Protocol * const backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );
  Kleo::KeyListJob * job = backend->keyListJob( false );
  Q_ASSERT( job );

  std::vector< GpgME::Key > keys;
  GpgME::KeyListResult res = job->exec( QStringList(), true, keys );

  Q_ASSERT( keys.size() == 1 );
  Q_ASSERT( !res.error() );
  kDebug() << "got private keys:" << keys.size();

  for(std::vector< GpgME::Key >::iterator i = keys.begin(); i != keys.end(); ++i ) {
    kDebug() << "key isnull:" << i->isNull() << "isexpired:" << i->isExpired();
    kDebug() << "key numuserIds:" << i->numUserIDs();
    for(uint k = 0; k < i->numUserIDs(); ++k ) {
      kDebug() << "userIDs:" << i->userID( k ).email();
    }
  }

  return keys;
}

