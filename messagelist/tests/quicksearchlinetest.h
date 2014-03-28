/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#ifndef QUICKSEARCHLINETEST_H
#define QUICKSEARCHLINETEST_H

#include <QObject>

class QuickSearchLineTest : public QObject
{
    Q_OBJECT
public:
    QuickSearchLineTest();
private Q_SLOTS:
    void shouldHaveDefaultValueOnCreation();
    void shouldEmitTextChanged();
    void shouldShowExtraOptionWidget();
    void shouldHideExtraOptionWidgetWhenClearLineEdit();
    void shouldHideExtraOptionWidgetWhenResetFilter();
    void shouldEmitSearchOptionChanged();
    void shouldResetAllWhenResetFilter();
    void shouldShowTagComboBox();
    void shouldResetComboboxWhenResetFilter();
    void shouldNotEmitTextChangedWhenTextTrimmedIsEmpty();
    void shouldShowExtraOptionWidgetWhenTextTrimmedIsNotEmpty();
    void shouldShowMoreOptionWhenClickOnMoreButton();
    void shouldChangeFromButtonLabelWhenChangeOutboundMessagesValue();
    void shouldSearchToOrFrom();
    void shouldEmitSearchOptionChangedWhenUseTabPress();
};

#endif // QUICKSEARCHLINETEST_H
