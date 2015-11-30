/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "composerviewbasetest.h"
#include <qtest.h>
#include "../src/composer/composerviewbase.h"

ComposerViewBaseTest::ComposerViewBaseTest(QObject *parent)
    : QObject(parent)
{

}

ComposerViewBaseTest::~ComposerViewBaseTest()
{

}

void ComposerViewBaseTest::shouldHaveDefaultValue()
{
    MessageComposer::ComposerViewBase composerViewBase;
    QVERIFY(!composerViewBase.attachmentModel());
    QVERIFY(!composerViewBase.attachmentController());
    QVERIFY(!composerViewBase.recipientsEditor());
    QVERIFY(!composerViewBase.signatureController());
    QVERIFY(!composerViewBase.identityCombo());
    QVERIFY(!composerViewBase.identityManager());
    QVERIFY(!composerViewBase.editor());
    QVERIFY(!composerViewBase.transportComboBox());
    QVERIFY(!composerViewBase.fccCombo());
    QVERIFY(!composerViewBase.dictionary());
#if 0 //mrecipient is null
    QVERIFY(composerViewBase.to().isEmpty());
    QVERIFY(composerViewBase.cc().isEmpty());
    QVERIFY(composerViewBase.bcc().isEmpty());
#endif
    QVERIFY(composerViewBase.from().isEmpty());
    QVERIFY(composerViewBase.replyTo().isEmpty());
    QVERIFY(composerViewBase.subject().isEmpty());
}

QTEST_MAIN(ComposerViewBaseTest)
