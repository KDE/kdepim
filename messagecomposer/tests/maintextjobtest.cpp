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

#include "maintextjobtest.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <messagecomposer/composer.h>
#include <messagecomposer/contentjob.h>
using namespace MessageComposer;

QTEST_KDEMAIN( ContentJobTest, GUI )

void MainTextJobTest::testPlainText()
{
  // LEFT TODO
}

void MainTextJobTest::testCustomCharset()
{
}

void MainTextJobTest::testBadCharset()
{
}

void MainTextJobTest::testFallbackCharset()
{
}

#include "maintextjobtest.moc"
