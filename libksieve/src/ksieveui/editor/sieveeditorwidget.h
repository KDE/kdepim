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

#ifndef SIEVEEDITORWIDGET_H
#define SIEVEEDITORWIDGET_H
#include "ksieveui_export.h"

#include <QWidget>

class QStackedWidget;
class QLineEdit;
class QAction;
namespace KSieveUi
{
class SieveEditorTextModeWidget;
class SieveEditorGraphicalModeWidget;
class KSIEVEUI_EXPORT SieveEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveEditorWidget(bool useMenuBar, QWidget *parent = Q_NULLPTR);
    ~SieveEditorWidget();
    enum EditorMode {
        Unknown = -1,
        TextMode = 0,
        GraphicMode = 1
    };

    void setReadOnly(bool b);
    QString script() const;
    QString originalScript() const;
    void setScript(const QString &script);
    void setDebugScript(const QString &debug);
    void setScriptName(const QString &name);

    void resultDone();

    void setSieveCapabilities(const QStringList &capabilities);

    void addFailedMessage(const QString &err);
    void addOkMessage(const QString &msg);

    void setModified(bool b);
    bool isModified() const;

    EditorMode mode() const;

    bool isRedoAvailable() const;
    bool isUndoAvailable() const;
    bool hasSelection() const;

    void checkSpelling();
    void share();
    void import();
    void lowerCase();
    void upperCase();
    void sentenceCase();
    void reverseCase();
    void zoomOut();
    void zoomIn();
    QString currentHelpTitle() const;
    QUrl currentHelpUrl() const;
    void openBookmarkUrl(const QUrl &url);
    void debugSieveScript();
    void zoomReset();
    bool isWordWrap() const;
public Q_SLOTS:
    void find();
    void replace();
    void undo();
    void redo();
    void paste();
    void copy();
    void cut();
    void selectAll();
    void goToLine();
    void slotSaveAs();
    void slotImport();
    void slotUploadScripts();
    void slotAutoGenerateScripts();
    void slotCheckSyntax();
    void comment();
    void uncomment();
    void wordWrap(bool state);

private Q_SLOTS:
    void slotEnableButtonOk(bool b);
    void slotGenerateXml();
    void slotSwitchMode();
    void slotSwitchTextMode(const QString &script);
    void slotSwitchToGraphicalMode();
    void slotModified();
Q_SIGNALS:
    void checkSyntax();
    void enableButtonOk(bool b);
    void valueChanged(bool b);
    void modeEditorChanged(KSieveUi::SieveEditorWidget::EditorMode);
    void undoAvailable(bool);
    void redoAvailable(bool);
    void copyAvailable(bool);
    void changeModeEditor(bool);
private:
    void changeSwitchButtonText();
    void changeMode(EditorMode mode);
    void addMessageEntry(const QString &errorMsg, const QColor &color);
    QString mOriginalScript;
    SieveEditorTextModeWidget *mTextModeWidget;
    SieveEditorGraphicalModeWidget *mGraphicalModeWidget;
    QStackedWidget *mStackedWidget;
    QLineEdit *mScriptName;
    QAction *mCheckSyntax;
    QAction *mSwitchMode;
    QAction *mAutoGenerateScript;
    QAction *mSaveAs;
    QAction *mUpload;
#if !defined(NDEBUG)
    QAction *mGenerateXml;
#endif
    EditorMode mMode;
    bool mModified;
};

}

#endif // SIEVEEDITORWIDGET_H
