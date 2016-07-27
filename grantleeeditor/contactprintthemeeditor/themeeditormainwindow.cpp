/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "themeeditormainwindow.h"
#include "themeeditorpage.h"
#include "newthemedialog.h"
#include "contactprintthemeconfiguredialog.h"
#include "managethemes.h"

#include <KStandardAction>
#include <QAction>
#include <KToggleAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <QFileDialog>
#include <KRecentFilesAction>
#include <kns3/knewstuffaction.h>
#include <KConfigGroup>

#include <QApplication>
#include <QPointer>
#include <QCloseEvent>
#include <QActionGroup>
#include <KSharedConfig>
#include <QStandardPaths>

ThemeEditorMainWindow::ThemeEditorMainWindow()
    : KXmlGuiWindow(),
      mThemeEditor(0)
{
    setupActions();
    setupGUI();
    updateActions();
    readConfig();
}

ThemeEditorMainWindow::~ThemeEditorMainWindow()
{
}

void ThemeEditorMainWindow::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QStringLiteral("ThemeEditorMainWindow"));
    group.writeEntry("Size", size());
    mRecentFileAction->saveEntries(group);
}

void ThemeEditorMainWindow::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = KConfigGroup(config, "ThemeEditorMainWindow");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void ThemeEditorMainWindow::updateActions()
{
    const bool projectDirectoryIsEmpty = (mThemeEditor != 0);
    mAddExtraPage->setEnabled(projectDirectoryIsEmpty);
    mCloseAction->setEnabled(projectDirectoryIsEmpty);
    mUploadTheme->setEnabled(projectDirectoryIsEmpty);
    mSaveAction->setEnabled(projectDirectoryIsEmpty);
    mInstallTheme->setEnabled(projectDirectoryIsEmpty);
    mInsertFile->setEnabled(projectDirectoryIsEmpty);
    mUpdateView->setEnabled(projectDirectoryIsEmpty);
    mSaveAsAction->setEnabled(projectDirectoryIsEmpty);
}

void ThemeEditorMainWindow::setupActions()
{
    mRecentFileAction = new KRecentFilesAction(i18n("Load Recent Theme..."), this);
    connect(mRecentFileAction, &KRecentFilesAction::urlSelected, this, &ThemeEditorMainWindow::slotThemeSelected);
    actionCollection()->addAction(QStringLiteral("load_recent_theme"), mRecentFileAction);
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup groupConfig = config->group(QStringLiteral("ThemeEditorMainWindow"));
    mRecentFileAction->loadEntries(groupConfig);

    mAddExtraPage = new QAction(i18n("Add Extra Page..."), this);
    connect(mAddExtraPage, &QAction::triggered, this, &ThemeEditorMainWindow::slotAddExtraPage);
    actionCollection()->addAction(QStringLiteral("add_extra_page"), mAddExtraPage);

    mUploadTheme = KNS3::standardAction(i18n("Upload theme..."), this, SLOT(slotUploadTheme()), actionCollection(), "upload_theme");

    mNewThemeAction = KStandardAction::openNew(this, &ThemeEditorMainWindow::slotNewTheme, actionCollection());
    mNewThemeAction->setText(i18n("New theme..."));

    mOpenAction = KStandardAction::open(this, &ThemeEditorMainWindow::slotOpenTheme, actionCollection());
    mOpenAction->setText(i18n("Open theme..."));
    mSaveAction = KStandardAction::save(this, &ThemeEditorMainWindow::slotSaveTheme, actionCollection());
    mSaveAction->setText(i18n("Save theme..."));

    mSaveAsAction = KStandardAction::saveAs(this, &ThemeEditorMainWindow::slotSaveAsTheme, actionCollection());
    mSaveAsAction->setText(i18n("Save theme as..."));

    mCloseAction = KStandardAction::close(this, &ThemeEditorMainWindow::slotCloseTheme, actionCollection());
    KStandardAction::quit(this, &ThemeEditorMainWindow::slotQuitApp, actionCollection());
    KStandardAction::preferences(this, &ThemeEditorMainWindow::slotConfigure, actionCollection());

    mInstallTheme = new QAction(i18n("Install theme"), this);
    actionCollection()->addAction(QStringLiteral("install_theme"), mInstallTheme);
    connect(mInstallTheme, &QAction::triggered, this, &ThemeEditorMainWindow::slotInstallTheme);

    mInsertFile = new QAction(i18n("Insert File..."), this);
    actionCollection()->addAction(QStringLiteral("insert_file"), mInsertFile);
    connect(mInsertFile, &QAction::triggered, this, &ThemeEditorMainWindow::slotInsertFile);

    mManageTheme = new QAction(i18n("Manage themes..."), this);
    connect(mManageTheme, &QAction::triggered, this, &ThemeEditorMainWindow::slotManageTheme);
    actionCollection()->addAction(QStringLiteral("manage_themes"), mManageTheme);

    mUpdateView = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("Update view"), this);
    actionCollection()->setDefaultShortcut(mUpdateView, QKeySequence(Qt::Key_F5));
    connect(mUpdateView, &QAction::triggered, this, &ThemeEditorMainWindow::slotUpdateView);
    actionCollection()->addAction(QStringLiteral("update_view"), mUpdateView);

}

void ThemeEditorMainWindow::slotManageTheme()
{
    QPointer<GrantleeThemeEditor::ManageThemes> dialog = new GrantleeThemeEditor::ManageThemes(QStringLiteral("kaddressbook/printing/themes/"), this);
    dialog->exec();
    delete dialog;
}

void ThemeEditorMainWindow::slotInsertFile()
{
    mThemeEditor->insertFile();
}

void ThemeEditorMainWindow::slotConfigure()
{
    QPointer<ContactPrintThemeConfigureDialog> dialog = new ContactPrintThemeConfigureDialog(this);
    if (dialog->exec()) {
        if (mThemeEditor) {
            mThemeEditor->reloadConfig();
        }
    }
    delete dialog;
}

void ThemeEditorMainWindow::slotInstallTheme()
{
    //Save before installing :)
    if (slotSaveTheme()) {
        const QString localThemePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kaddressbook/printing/themes");
        if (QDir().mkpath(localThemePath)) {
            mThemeEditor->installTheme(localThemePath);
        }
    }
}

void ThemeEditorMainWindow::slotUploadTheme()
{
    //Save before upload :)
    if (slotSaveTheme()) {
        mThemeEditor->uploadTheme();
    }
}

bool ThemeEditorMainWindow::slotSaveTheme()
{
    bool result = false;
    if (mThemeEditor) {
        result = mThemeEditor->saveTheme(false);
        mSaveAction->setEnabled(!result);
    }
    return result;
}

void ThemeEditorMainWindow::slotCloseTheme()
{
    saveCurrentProject(SaveAndCloseTheme);
}

void ThemeEditorMainWindow::slotOpenTheme()
{
    if (!saveCurrentProject(SaveOnly)) {
        return;
    }

    const QString directory = QFileDialog::getExistingDirectory(this, i18n("Select theme directory"));
    if (directory.isEmpty()) {
        return;
    }
    closeThemeEditor();
    if (loadTheme(directory)) {
        mRecentFileAction->addUrl(QUrl::fromLocalFile(directory));
    }
    mSaveAction->setEnabled(false);
}

bool ThemeEditorMainWindow::loadTheme(const QString &directory)
{
    if (!directory.isEmpty()) {
        const QString filename = directory + QDir::separator() + QLatin1String("theme.themerc");
        QFile file(filename);
        if (!file.exists()) {
            KMessageBox::error(this, i18n("Directory does not contain a theme file. We cannot load theme."));
            return false;
        }

        mThemeEditor = new ThemeEditorPage(QString(), QString());
        connect(mThemeEditor, &ThemeEditorPage::changed, mSaveAction, &QAction::setEnabled);
        connect(mThemeEditor, &ThemeEditorPage::canInsertFile, this, &ThemeEditorMainWindow::slotCanInsertFile);
        mThemeEditor->loadTheme(filename);
        setCentralWidget(mThemeEditor);
        updateActions();
    }
    return true;
}

void ThemeEditorMainWindow::slotAddExtraPage()
{
    if (mThemeEditor) {
        mThemeEditor->addExtraPage();
    }
}

void ThemeEditorMainWindow::closeThemeEditor()
{
    delete mThemeEditor;
    mThemeEditor = 0;
    setCentralWidget(0);
    updateActions();
}

bool ThemeEditorMainWindow::saveCurrentProject(ActionSaveTheme act)
{
    if (mThemeEditor) {
        if (!mThemeEditor->saveTheme()) {
            return false;
        }
    }
    switch (act) {
    case SaveOnly:
        break;
    case SaveAndCloseTheme: {
        closeThemeEditor();
        break;
    }
    case SaveAndCreateNewTheme: {
        delete mThemeEditor;
        mThemeEditor = 0;
        QPointer<GrantleeThemeEditor::NewThemeDialog> dialog = new GrantleeThemeEditor::NewThemeDialog(this);
        QString newTheme;
        QString projectDirectory;
        if (dialog->exec()) {
            newTheme = dialog->themeName();
            projectDirectory = dialog->directory();
        }
        if (!projectDirectory.isEmpty()) {
            mRecentFileAction->addUrl(QUrl::fromLocalFile(projectDirectory));
            mThemeEditor = new ThemeEditorPage(projectDirectory, newTheme);
            connect(mThemeEditor, &ThemeEditorPage::changed, mSaveAction, &QAction::setEnabled);
            connect(mThemeEditor, &ThemeEditorPage::canInsertFile, this, &ThemeEditorMainWindow::slotCanInsertFile);
            setCentralWidget(mThemeEditor);
        } else {
            setCentralWidget(0);
        }
        delete dialog;
        updateActions();
        break;
    }
    }

    return true;
}

void ThemeEditorMainWindow::slotNewTheme()
{
    saveCurrentProject(SaveAndCreateNewTheme);
}

void ThemeEditorMainWindow::closeEvent(QCloseEvent *e)
{
    if (!saveCurrentProject(SaveAndCloseTheme)) {
        e->ignore();
    } else {
        writeConfig();
        e->accept();
    }
}

void ThemeEditorMainWindow::slotQuitApp()
{
    if (saveCurrentProject(SaveAndCloseTheme)) {
        writeConfig();
        qApp->quit();
    }
}

void ThemeEditorMainWindow::slotUpdateView()
{
    if (mThemeEditor) {
        mThemeEditor->saveTheme(false);
        mThemeEditor->updatePreview();
    }
}

void ThemeEditorMainWindow::slotCanInsertFile(bool b)
{
    mInsertFile->setEnabled(b);
}

void ThemeEditorMainWindow::slotThemeSelected(const QUrl &url)
{
    if (!saveCurrentProject(SaveAndCloseTheme)) {
        return;
    }
    loadTheme(url.path());
    mSaveAction->setEnabled(false);
}

void ThemeEditorMainWindow::slotSaveAsTheme()
{
    const QString directory = QFileDialog::getExistingDirectory(this, i18n("Select theme directory"));
    if (!directory.isEmpty()) {
        if (mThemeEditor) {
            mThemeEditor->saveThemeAs(directory);
        }
    }
}

