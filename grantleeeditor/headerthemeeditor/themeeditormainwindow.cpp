/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "themeeditormainwindow.h"
#include "themeeditorpage.h"
#include "newthemedialog.h"
#include "themeconfiguredialog.h"
#include "managethemes.h"

#include <KStandardAction>
#include <KApplication>
#include <QAction>
#include <KToggleAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KFileDialog>
#include <QDebug>
#include <KStandardDirs>
#include <KRecentFilesAction>
#include <KNS3/KNewStuffAction>
#include <KUrl>
#include <KGlobal>

#include <QPointer>
#include <QCloseEvent>
#include <QActionGroup>
#include <KSharedConfig>

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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group( QLatin1String("ThemeEditorMainWindow") );
    group.writeEntry( "Size", size() );
    mRecentFileAction->saveEntries(group);
}

void ThemeEditorMainWindow::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = KConfigGroup( config, "ThemeEditorMainWindow" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void ThemeEditorMainWindow::updateActions()
{
    const bool projectDirectoryIsEmpty = (mThemeEditor!=0);
    mAddExtraPage->setEnabled(projectDirectoryIsEmpty);
    mCloseAction->setEnabled(projectDirectoryIsEmpty);
    mUploadTheme->setEnabled(projectDirectoryIsEmpty);
    mSaveAction->setEnabled(projectDirectoryIsEmpty);
    mInstallTheme->setEnabled(projectDirectoryIsEmpty);
    mInsertFile->setEnabled(projectDirectoryIsEmpty);
    mPrintingMode->setEnabled(projectDirectoryIsEmpty);
    mNormalMode->setEnabled(projectDirectoryIsEmpty);
    mUpdateView->setEnabled(projectDirectoryIsEmpty);
    mSaveAsAction->setEnabled(projectDirectoryIsEmpty);
}

void ThemeEditorMainWindow::setupActions()
{
    mRecentFileAction = new KRecentFilesAction(i18n("Load Recent Theme..."), this);
    connect(mRecentFileAction, SIGNAL(urlSelected(QUrl)), this, SLOT(slotThemeSelected(QUrl)));
    actionCollection()->addAction( QLatin1String( "load_recent_theme" ), mRecentFileAction );
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup groupConfig = config->group( QLatin1String("ThemeEditorMainWindow") );
    mRecentFileAction->loadEntries(groupConfig);

    mAddExtraPage = new QAction(i18n("Add Extra Page..."), this);
    connect(mAddExtraPage, SIGNAL(triggered(bool)),SLOT(slotAddExtraPage()));
    actionCollection()->addAction( QLatin1String( "add_extra_page" ), mAddExtraPage );

    mUploadTheme = KNS3::standardAction(i18n("Upload theme..."), this, SLOT(slotUploadTheme()), actionCollection(), "upload_theme");

    mNewThemeAction = KStandardAction::openNew(this, SLOT(slotNewTheme()), actionCollection());
    mNewThemeAction->setText(i18n("New theme..."));

    mOpenAction = KStandardAction::open(this, SLOT(slotOpenTheme()), actionCollection());
    mOpenAction->setText(i18n("Open theme..."));
    mSaveAction = KStandardAction::save(this, SLOT(slotSaveTheme()), actionCollection());
    mSaveAction->setText(i18n("Save theme..."));

    mSaveAsAction = KStandardAction::saveAs(this, SLOT(slotSaveAsTheme()), actionCollection());
    mSaveAsAction->setText(i18n("Save theme as..."));

    mCloseAction = KStandardAction::close( this, SLOT(slotCloseTheme()), actionCollection());
    KStandardAction::quit(this, SLOT(slotQuitApp()), actionCollection() );
    KStandardAction::preferences( this, SLOT(slotConfigure()), actionCollection() );

    mInstallTheme = new QAction(i18n("Install theme"), this);
    actionCollection()->addAction( QLatin1String( "install_theme" ), mInstallTheme );
    connect(mInstallTheme, SIGNAL(triggered(bool)), SLOT(slotInstallTheme()));

    mInsertFile = new QAction(i18n("Insert File..."), this);
    actionCollection()->addAction( QLatin1String( "insert_file" ), mInsertFile );
    connect(mInsertFile, SIGNAL(triggered(bool)), SLOT(slotInsertFile()));

    QActionGroup *group = new QActionGroup( this );

    mPrintingMode  = new KToggleAction(i18n("Printing mode"), this);
    actionCollection()->addAction(QLatin1String("printing_mode"), mPrintingMode );
    connect(mPrintingMode, SIGNAL(triggered(bool)), SLOT(slotPrintingMode()));
    group->addAction( mPrintingMode );

    mNormalMode  = new KToggleAction(i18n("Normal mode"), this);
    mNormalMode->setChecked(true);
    actionCollection()->addAction(QLatin1String("normal_mode"), mNormalMode );
    connect(mNormalMode, SIGNAL(triggered(bool)), SLOT(slotNormalMode()));
    group->addAction( mNormalMode );

    mManageTheme = new QAction(i18n("Manage themes..."), this);
    connect(mManageTheme, SIGNAL(triggered(bool)),SLOT(slotManageTheme()));
    actionCollection()->addAction( QLatin1String( "manage_themes" ), mManageTheme );

    mUpdateView = new QAction(i18n("Update view"), this);
    mUpdateView->setShortcut(QKeySequence( Qt::Key_F5 ));
    connect(mUpdateView, SIGNAL(triggered(bool)),SLOT(slotUpdateView()));
    actionCollection()->addAction( QLatin1String( "update_view" ), mUpdateView );
}

void ThemeEditorMainWindow::slotManageTheme()
{
    QPointer<GrantleeThemeEditor::ManageThemes> dialog = new GrantleeThemeEditor::ManageThemes(QLatin1String("messageviewer/themes/"), this);
    dialog->exec();
    delete dialog;
}

void ThemeEditorMainWindow::slotNormalMode()
{
    mThemeEditor->setPrinting(false);
}

void ThemeEditorMainWindow::slotPrintingMode()
{
    mThemeEditor->setPrinting(true);
}

void ThemeEditorMainWindow::slotInsertFile()
{
    mThemeEditor->insertFile();
}

void ThemeEditorMainWindow::slotConfigure()
{
    QPointer<ThemeConfigureDialog> dialog = new ThemeConfigureDialog(this);
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
        const QString localThemePath = KStandardDirs::locateLocal("data",QLatin1String("messageviewer/themes/"));
        mThemeEditor->installTheme(localThemePath);
    }
}

void ThemeEditorMainWindow::slotUploadTheme()
{
    //Save before upload :)
    if (slotSaveTheme())
        mThemeEditor->uploadTheme();
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
    if (!saveCurrentProject(SaveOnly))
        return;

    const QString directory = KFileDialog::getExistingDirectory(KUrl( "kfiledialog:///OpenTheme" ), this, i18n("Select theme directory"));
    if (directory.isEmpty()) {
        return;
    }
    closeThemeEditor();
    if (loadTheme(directory))
        mRecentFileAction->addUrl(KUrl(directory));
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
        connect(mThemeEditor, SIGNAL(changed(bool)), mSaveAction, SLOT(setEnabled(bool)));
        connect(mThemeEditor, SIGNAL(canInsertFile(bool)), this, SLOT(slotCanInsertFile(bool)));
        mThemeEditor->loadTheme(filename);
        setCentralWidget(mThemeEditor);
        updateActions();
    }
    return true;
}

void ThemeEditorMainWindow::slotAddExtraPage()
{
    if (mThemeEditor)
        mThemeEditor->addExtraPage();
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
    switch(act) {
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
            mRecentFileAction->addUrl(KUrl(projectDirectory));
            mThemeEditor = new ThemeEditorPage(projectDirectory, newTheme);
            connect(mThemeEditor, SIGNAL(changed(bool)), mSaveAction, SLOT(setEnabled(bool)));
            connect(mThemeEditor, SIGNAL(canInsertFile(bool)), this, SLOT(slotCanInsertFile(bool)));
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
    if (!saveCurrentProject(SaveAndCloseTheme))
        e->ignore();
    else
        e->accept();
}

void ThemeEditorMainWindow::slotQuitApp()
{
    if (saveCurrentProject(SaveAndCloseTheme))
        kapp->quit();
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
    if (!saveCurrentProject(SaveAndCloseTheme))
        return;
    loadTheme(url.path());
    mSaveAction->setEnabled(false);
}

void ThemeEditorMainWindow::slotSaveAsTheme()
{
    const QString directory = KFileDialog::getExistingDirectory(KUrl( "kfiledialog:///SaveTheme" ), this, i18n("Select theme directory"));
    if (!directory.isEmpty()) {
        if (mThemeEditor)
            mThemeEditor->saveThemeAs(directory);
    }
}

