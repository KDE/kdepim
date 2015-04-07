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

#ifndef FILTERACTIONADDHEADERTEST_H
#define FILTERACTIONADDHEADERTEST_H

#include <QObject>

class FilterActionAddHeaderTest : public QObject
{
    Q_OBJECT
public:
    explicit FilterActionAddHeaderTest(QObject *parent = Q_NULLPTR);
    ~FilterActionAddHeaderTest();
private Q_SLOTS:
    void shouldCreateWidget();
    void shouldAddValue();
    void shouldAddValue_data();
    void shouldClearWidget();
    void shouldReturnSieveCode();
    void shouldBeEmpty();

    void shouldNotExecuteActionWhenParameterIsEmpty();
    void shouldNotExecuteActionWhenValueIsEmpty();
    void shouldAddNewHeaderWhenNotExistingHeader();
    void shouldReplaceHeaderWhenExistingHeader();
    void shouldRequiresSieve();
};

#endif // FILTERACTIONADDHEADERTEST_H
