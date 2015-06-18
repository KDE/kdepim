/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORPAGEWIDGET_H
#define SIEVEEDITORPAGEWIDGET_H
#include <QWidget>
#include <QUrl>
#include "ksieveui/editor/sieveeditorwidget.h"
namespace KManageSieve
{
class SieveJob;
}
namespace KSieveUi
{
class SieveEditorWidget;
}

class SieveEditorPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveEditorPageWidget(QWidget *parent = Q_NULLPTR);
    ~SieveEditorPageWidget();

    void loadScript(const QUrl &url, const QStringList &capabilities);
    QUrl currentUrl() const;
    void setIsNewScript(bool isNewScript);
    void uploadScript(bool showInformation = true, bool forceSave = false);
    bool needToSaveScript();

    bool isModified() const;
    void goToLine();
    KSieveUi::SieveEditorWidget::EditorMode pageMode() const;

    void find();
    void replace();
    void redo();
    void undo();
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    void paste();
    void cut();
    void copy();
    bool hasSelection() const;

    void selectAll();
    void saveAs();
    void checkSpelling();
    void share();
    void import();
    void autoGenerateScript();
    void checkSyntax();
    void comment();
    void uncomment();
    void lowerCase();
    void upperCase();
    void sentenceCase();
    void reverseCase();
    void zoomIn();
    void zoomOut();
    QString currentHelpTitle() const;
    QUrl currentHelpUrl() const;
Q_SIGNALS:
    void refreshList();
    void scriptModified(bool, SieveEditorPageWidget *);
    void modeEditorChanged(KSieveUi::SieveEditorWidget::EditorMode);
    void undoAvailable(bool);
    void redoAvailable(bool);
    void copyAvailable(bool);

private Q_SLOTS:
    void slotGetResult(KManageSieve::SieveJob *, bool success, const QString &script, bool isActive);
    void slotCheckSyntaxClicked();
    void slotPutResultDebug(KManageSieve::SieveJob *, bool success, const QString &errorMsg);
    void slotPutResult(KManageSieve::SieveJob *, bool success);
    void slotValueChanged(bool b);

private:
    void setModified(bool b);
    QUrl mCurrentURL;
    KSieveUi::SieveEditorWidget *mSieveEditorWidget;
    bool mWasActive;
    bool mIsNewScript;
};

#endif // SIEVEEDITORPAGEWIDGET_H
