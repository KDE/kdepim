/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "attachmentparttest.h"
#include "qtest_libkdepim.h"

#include <QHash>

#include <KDebug>
#include <qtest_kde.h>

#include <libkdepim/attachmentpart.h>
#include <boost/shared_ptr.hpp>
using namespace KPIM;

QTEST_KDEMAIN( AttachmentPartTest, NoGUI )

void AttachmentPartTest::testApi()
{
  const QString str = QString::fromLatin1( "test" );
  AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );

  QHash<AttachmentPart::Ptr, QString> hash;
  hash[ part ] = str;
  QVERIFY( hash.contains( part ) );

#if 0
  FuckThis obj;
  QHash<FuckThis, QString> hash;
  hash[ obj ] = str;
  QVERIFY( hash.contains( obj ) );
#endif
}

#include "attachmentparttest.moc"
