/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORMAINWIDGET_H
#define SIEVEEDITORMAINWIDGET_H

#include <QSplitter>
#include <QUrl>
#include "ksieveui/sieveeditorwidget.h"
class QTabWidget;
class QStackedWidget;
class SieveEditorTabWidget;
class SieveEditorScriptManagerWidget;
class SieveEditorPageWidget;
class SieveEditorEmptyTabWidgetLabel;
class SieveEditorMainWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit SieveEditorMainWidget(QWidget *parent = Q_NULLPTR);
    ~SieveEditorMainWidget();

    void createNewScript();
    void deleteScript();
    void updateServerList();
    void editScript();
    void desactivateScript();
    void refreshList();
    void uploadScript();
    bool needToSaveScript();

    QTabWidget *tabWidget() const;

    KSieveUi::SieveEditorWidget::EditorMode pageMode() const;
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    bool isWordWrap() const;

    bool hasSelection() const;
    QString currentHelpTitle() const;
    QUrl currentHelpUrl() const;
    void openBookmarkUrl(const QUrl &url);
public Q_SLOTS:
    void slotDebugSieveScript();
    void slotGoToLine();

    void slotReplace();
    void slotFind();
    void slotCopy();
    void slotPaste();
    void slotCut();
    void slotSaveAs();
    void slotCheckSpelling();
    void slotShare();
    void slotImport();
    void slotAutoGenerateScript();
    void slotCheckSyntax();
    void slotComment();
    void slotUncomment();
    void slotUpperCase();
    void slotSentenceCase();
    void slotLowerCase();
    void slotReverseCase();
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomReset();
    void slotWordWrap(bool);

Q_SIGNALS:
    void updateButtons(bool newScriptAction, bool editScriptAction, bool deleteScriptAction, bool desactivateScriptAction);
    void updateScriptList();
    void modeEditorChanged(KSieveUi::SieveEditorWidget::EditorMode);
    void serverSieveFound(bool);
    void undoAvailable(bool);
    void redoAvailable(bool);
    void copyAvailable(bool);

private Q_SLOTS:
    void slotCreateScriptPage(const QUrl &url, const QStringList &capabilities, bool isNewScript);
    void slotScriptDeleted(const QUrl &url);
    void slotScriptModified(bool modified, SieveEditorPageWidget *page);
    void slotGeneralPaletteChanged();
    void slotTabCloseRequested(int index);
    void slotTabRemoveAllExclude(int index);
    void slotUndo();
    void slotRedo();
    void slotSelectAll();
    void slotTabCloseAllRequested();
private:
    void updateStackedWidget();
    QWidget *hasExistingPage(const QUrl &url);
    QColor mModifiedScriptColor;
    QColor mScriptColor;
    SieveEditorTabWidget *mTabWidget;
    SieveEditorScriptManagerWidget *mScriptManagerWidget;
    QStackedWidget *mStackedWidget;
    SieveEditorEmptyTabWidgetLabel *mEditorEmptyLabel;
};

#endif // SIEVEEDITORMAINWIDGET_H
