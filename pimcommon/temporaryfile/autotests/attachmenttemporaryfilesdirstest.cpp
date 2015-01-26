/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "attachmenttemporaryfilesdirstest.h"
#include "../attachmenttemporaryfilesdirs.h"
#include <KTempDir>
#include <qtest_kde.h>
#include <KDebug>

using namespace PimCommon;

AttachmentTemporaryFilesDirsTest::AttachmentTemporaryFilesDirsTest(QObject *parent)
    : QObject(parent)
{

}

AttachmentTemporaryFilesDirsTest::~AttachmentTemporaryFilesDirsTest()
{

}

void AttachmentTemporaryFilesDirsTest::shouldHaveDefaultValue()
{
    AttachmentTemporaryFilesDirs attachmentDir;
    QVERIFY(attachmentDir.temporaryFiles().isEmpty());
    QVERIFY(attachmentDir.temporaryDirs().isEmpty());
}

void AttachmentTemporaryFilesDirsTest::shouldAddTemporaryFiles()
{
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempFile(QLatin1String("foo"));
    QCOMPARE(attachmentDir.temporaryFiles().count(), 1);
    attachmentDir.addTempFile(QLatin1String("foo1"));
    QCOMPARE(attachmentDir.temporaryFiles().count(), 2);
}

void AttachmentTemporaryFilesDirsTest::shouldAddTemporaryDirs()
{
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempDir(QLatin1String("foo"));
    QCOMPARE(attachmentDir.temporaryDirs().count(), 1);
    attachmentDir.addTempDir(QLatin1String("foo1"));
    QCOMPARE(attachmentDir.temporaryDirs().count(), 2);
}

void AttachmentTemporaryFilesDirsTest::shouldNotAddSameFiles()
{
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempFile(QLatin1String("foo"));
    QCOMPARE(attachmentDir.temporaryFiles().count(), 1);
    attachmentDir.addTempFile(QLatin1String("foo"));
    QCOMPARE(attachmentDir.temporaryFiles().count(), 1);
}

void AttachmentTemporaryFilesDirsTest::shouldNotAddSameDirs()
{
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempDir(QLatin1String("foo"));
    QCOMPARE(attachmentDir.temporaryDirs().count(), 1);
    attachmentDir.addTempDir(QLatin1String("foo"));
    QCOMPARE(attachmentDir.temporaryDirs().count(), 1);
}

void AttachmentTemporaryFilesDirsTest::shouldForceRemoveTemporaryDirs()
{
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempDir(QLatin1String("foo"));
    attachmentDir.addTempDir(QLatin1String("foo1"));
    QCOMPARE(attachmentDir.temporaryDirs().count(), 2);
    attachmentDir.forceCleanTempFiles();
    QCOMPARE(attachmentDir.temporaryDirs().count(), 0);
    QCOMPARE(attachmentDir.temporaryFiles().count(), 0);
}

void AttachmentTemporaryFilesDirsTest::shouldForceRemoveTemporaryFiles()
{
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempFile(QLatin1String("foo"));
    attachmentDir.addTempFile(QLatin1String("foo2"));
    QCOMPARE(attachmentDir.temporaryFiles().count(), 2);
    attachmentDir.forceCleanTempFiles();
    QCOMPARE(attachmentDir.temporaryFiles().count(), 0);
    QCOMPARE(attachmentDir.temporaryDirs().count(), 0);
}

void AttachmentTemporaryFilesDirsTest::shouldCreateDeleteTemporaryFiles()
{
    KTempDir tmpDir;
    QVERIFY(tmpDir.exists());
    QFile file(tmpDir.name() + QLatin1String("/foo"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kDebug()<<"Can open file";
        return;
    }
    tmpDir.setAutoRemove(false);
    file.close();
    QVERIFY(file.exists());
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempDir(tmpDir.name());
    attachmentDir.addTempFile(file.fileName());
    QVERIFY(!attachmentDir.temporaryFiles().isEmpty());
    QCOMPARE(attachmentDir.temporaryFiles().first(), file.fileName());
    const QString path = tmpDir.name();
    attachmentDir.forceCleanTempFiles();
    QCOMPARE(attachmentDir.temporaryFiles().count(), 0);
    QCOMPARE(attachmentDir.temporaryDirs().count(), 0);
    QVERIFY(!QDir(path).exists());
}

void AttachmentTemporaryFilesDirsTest::shouldRemoveTemporaryFilesAfterTime()
{
    KTempDir tmpDir;
    QVERIFY(tmpDir.exists());
    QFile file(tmpDir.name() + QLatin1String("/foo"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kDebug()<<"Can open file";
        return;
    }
    tmpDir.setAutoRemove(false);
    file.close();
    QVERIFY(file.exists());
    AttachmentTemporaryFilesDirs attachmentDir;
    attachmentDir.addTempDir(tmpDir.name());
    attachmentDir.addTempFile(file.fileName());
    QVERIFY(!attachmentDir.temporaryFiles().isEmpty());
    QCOMPARE(attachmentDir.temporaryFiles().first(), file.fileName());
    attachmentDir.setDelayRemoveAllInMs(500);
    QTest::qSleep(1000);
    attachmentDir.removeTempFiles();
    const QString path = tmpDir.name();
    attachmentDir.forceCleanTempFiles();
    QCOMPARE(attachmentDir.temporaryFiles().count(), 0);
    QCOMPARE(attachmentDir.temporaryDirs().count(), 0);
    QVERIFY(!QDir(path).exists());
}

QTEST_KDEMAIN(AttachmentTemporaryFilesDirsTest, NoGUI)
