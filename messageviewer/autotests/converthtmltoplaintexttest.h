/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef CONVERTHTMLTOPLAINTEXTTEST_H
#define CONVERTHTMLTOPLAINTEXTTEST_H

#include <QObject>

class ConvertHtmlToPlainTextTest : public QObject
{
    Q_OBJECT
public:
    explicit ConvertHtmlToPlainTextTest(QObject *parent = Q_NULLPTR);
    ~ConvertHtmlToPlainTextTest();

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldReturnEmptyStringIfInputTextIsEmpty();
    void shouldReturnNotEmptyStringIfInputTextIsNotEmpty();
    void shouldConvertToPlainText_data();
    void shouldConvertToPlainText();
};

#endif // CONVERTHTMLTOPLAINTEXTTEST_H
