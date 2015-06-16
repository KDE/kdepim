/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "imagescalingtest.h"
#include "settings/messagecomposersettings.h"
#include "../imagescaling.h"
#include <qtest.h>
#include <QStandardPaths>

ImageScalingTest::ImageScalingTest(QObject *parent)
    : QObject(parent)
{

}

ImageScalingTest::~ImageScalingTest()
{

}

void ImageScalingTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}


void ImageScalingTest::shouldHaveDefaultValue()
{
    MessageComposer::ImageScaling scaling;
    //Image is empty
    QVERIFY(!scaling.resizeImage());
    QVERIFY(scaling.generateNewName().isEmpty());
}

void ImageScalingTest::shouldHaveRenameFile_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<QByteArray>("format");
    QTest::addColumn<QString>("saveasformat");
    QTest::newRow("no rename png when file is empty") <<  QString() << QString() << QByteArray("image/png") << QStringLiteral("PNG");
    QTest::newRow("no rename jpg when file is empty") <<  QString() << QString() << QByteArray("image/jpeg") << QStringLiteral("PNG");
    QTest::newRow("no rename to png") <<  QStringLiteral("foo.jpeg") << QStringLiteral("foo.jpeg") << QByteArray("image/jpeg") << QStringLiteral("PNG");
    QTest::newRow("no rename to jpeg") <<  QStringLiteral("foo.png") << QStringLiteral("foo.png") << QByteArray("image/png") << QStringLiteral("JPG");
    QTest::newRow("rename to jpeg") <<  QStringLiteral("foo.png") << QStringLiteral("foo.jpg") << QByteArray("image/mng") << QStringLiteral("JPG");
    QTest::newRow("rename to png") <<  QStringLiteral("foo.jpg") << QStringLiteral("foo.png") << QByteArray("image/mng") << QStringLiteral("PNG");
}

void ImageScalingTest::shouldHaveRenameFile()
{
    QFETCH(QString, input);
    QFETCH(QString, output);
    QFETCH(QByteArray, format);
    QFETCH(QString, saveasformat);

    MessageComposer::MessageComposerSettings::self()->setWriteFormat(saveasformat);
    MessageComposer::MessageComposerSettings::self()->save();
    MessageComposer::ImageScaling scaling;
    scaling.setName(input);
    scaling.setMimetype(format);
    QCOMPARE(scaling.generateNewName(), output);
}

QTEST_MAIN(ImageScalingTest)
