/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#ifndef NOTEEDITTEST_H
#define NOTEEDITTEST_H

#include <QObject>


class NoteEditTest : public QObject
{
    Q_OBJECT
public:
    NoteEditTest();

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
    void shouldEmitNoteWhenPressEnter();
    void shouldEmitNotEmitNoteWhenTextIsEmpty();
    void shouldNoteHasCorrectSubject();
    void shouldClearAllWhenCloseWidget();
    void shouldEmitCollectionChangedWhenCurrentCollectionWasChanged();
    void shouldClearLineAfterEmitNewNote();
    void shouldEmitCorrectCollection();
    void shouldHideWidgetWhenClickOnCloseButton();
    void shouldHideWidgetWhenPressEscape();
    void shouldHideWidgetWhenSaveClicked();
    void shouldSaveCollectionSettings();
    void shouldSaveCollectionSettingsWhenCloseWidget();
    void shouldNotEmitNoteWhenMessageIsNull();
    void shouldHaveLineEditFocus();
    void shouldEmitNotEmitNoteWhenTextTrimmedIsEmpty();
    void shouldSaveCollectionSettingsWhenDeleteWidget();
    void shouldSetFocusWhenWeCallNoteEdit();
    void shouldShouldEnabledSaveEditorButton();
};

#endif // NOTEEDITTEST_H

