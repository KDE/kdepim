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

#include "invalidfilterdialogtest.h"
#include "../filter/invalidfilters/invalidfilterdialog.h"
#include <qtest.h>
#include "../filter/invalidfilters/invalidfilterwidget.h"
#include "../filter/invalidfilters/invalidfilterinfowidget.h"

InvalidFilterDialogTest::InvalidFilterDialogTest(QObject *parent)
    : QObject(parent)
{

}

InvalidFilterDialogTest::~InvalidFilterDialogTest()
{

}

void InvalidFilterDialogTest::shouldHaveDefaultValue()
{
    MailCommon::InvalidFilterDialog dlg;
    MailCommon::InvalidFilterWidget *widget = dlg.findChild<MailCommon::InvalidFilterWidget *>(QStringLiteral("invalid_filter_widget"));
    QVERIFY(widget);

    MailCommon::InvalidFilterInfoWidget *infoWidget = dlg.findChild<MailCommon::InvalidFilterInfoWidget *>(QStringLiteral("invalid_filter_infowidget"));
    QVERIFY(infoWidget);
    QVERIFY(!infoWidget->isVisible());
}

QTEST_MAIN(InvalidFilterDialogTest)
