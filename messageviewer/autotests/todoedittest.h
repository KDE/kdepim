/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef TODOEDITTEST_H
#define TODOEDITTEST_H

#include <QObject>


class TodoEditTest : public QObject
{
    Q_OBJECT
public:
    TodoEditTest();

private slots:
    void shouldNotEmitWhenMessageIsNotChanged();
    void shouldHaveDefaultValuesOnCreation();
    void shouldEmitCollectionChanged();
    void shouldEmitMessageChanged();
    void shouldNotEmitWhenCollectionIsNotChanged();
    void shouldHaveSameValueAfterSet();
    void shouldHaveASubject();
    void shouldEmptySubjectWhenMessageIsNull();
    void shouldEmptySubjectWhenMessageHasNotSubject();
    void shouldSelectLineWhenPutMessage();
    void shouldEmitCollectionChangedWhenChangeComboboxItem();
    void shouldEmitTodoWhenPressEnter();
    void shouldEmitNotEmitTodoWhenTextIsEmpty();
    void shouldTodoHasCorrectSubject();
    void shouldClearAllWhenCloseWidget();
    void shouldEmitCollectionChangedWhenCurrentCollectionWasChanged();
    void shouldClearLineAfterEmitNewNote();
    void shouldEmitCorrectCollection();
    void shouldHideWidgetWhenClickOnCloseButton();
    void shouldHideWidgetWhenPressEscape();
    void shouldHideWidgetWhenSaveClicked();
    void shouldSaveCollectionSettings();
    void shouldSaveCollectionSettingsWhenCloseWidget();
    void shouldNotEmitTodoWhenMessageIsNull();
    void shouldHaveLineEditFocus();
    void shouldEmitNotEmitTodoWhenTextTrimmedIsEmpty();
    void shouldSaveCollectionSettingsWhenDeleteWidget();
    void shouldSetFocusWhenWeCallTodoEdit();
    void shouldShowMessageWidget();
    void shouldHideMessageWidget();
    void shouldHideMessageWidgetWhenAddNewMessage();
    void shouldHideMessageWidgetWhenCloseWidget();
    void shouldShouldEnabledSaveOpenEditorButton();
};

#endif // TODOEDITTEST_H

