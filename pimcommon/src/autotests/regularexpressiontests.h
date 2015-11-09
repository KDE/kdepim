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

#ifndef REGULAREXPRESSIONTESTS_H
#define REGULAREXPRESSIONTESTS_H

#include <QObject>

class RegularExpressionTests : public QObject
{
    Q_OBJECT
public:
    explicit RegularExpressionTests(QObject *parent = Q_NULLPTR);
    ~RegularExpressionTests();
private Q_SLOTS:
    void shouldVerifyQStringListFilterConversion_data();
    void shouldVerifyQStringListFilterConversion();

    void shouldVerifyQStringListFilterTwoConversion_data();
    void shouldVerifyQStringListFilterTwoConversion();

    void shouldVerifyQStringListFilterSpaceConversion();
    void shouldVerifyQStringListFilterSpaceConversion_data();

    void shouldVerifyQStringListFilterDoublePointConversion();
    void shouldVerifyQStringListFilterDoublePointConversion_data();

    void shouldVerifyQStringListFilterWithSharpConversion();
    void shouldVerifyQStringListFilterWithSharpConversion_data();

    void shouldReplaceString_data();
    void shouldReplaceString();

    void shouldRemoveString_data();
    void shouldRemoveString();

    void shouldVerifyQStringListFilterWithStartCharAndEndConversion_data();
    void shouldVerifyQStringListFilterWithStartCharAndEndConversion();

    void shouldVerifyQStringListFilterWithPmailSettingsConversion();
    void shouldVerifyQStringListFilterWithPmailSettingsConversion_data();

    void shouldContainsString_data();
    void shouldContainsString();

    void shouldCaptureValue_data();
    void shouldCaptureValue();

};

#endif // REGULAREXPRESSIONTESTS_H
