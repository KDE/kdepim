/*
  Copyright (C) 2009 Constantin Berzan <exit3219@gmail.com>

  Based on Akonadi code by:
  Copyright (C) 2009 Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef QTEST_MESSAGECOMPOSER_H
#define QTEST_MESSAGECOMPOSER_H

#include <gpgme++/key.h>

/**
 * Runs a MessageComposer::JobBase synchronously and aborts if the job failed.
 * Similar to QVERIFY( job->exec() ) but includes the job error message
 * in the output in case of a failure.
 */
#define VERIFYEXEC( job ) \
    QVERIFY2( job->exec(), job->errorString().toUtf8().constData() )

#endif
