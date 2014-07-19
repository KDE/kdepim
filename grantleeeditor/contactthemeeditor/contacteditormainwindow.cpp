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

#include "contacteditormainwindow.h"
#include "contacteditorpage.h"
#include "newthemedialog.h"
#include "managethemes.h"
#include "contactconfigurationdialog.h"


#include <KStandardAction>
#include <KApplication>
#include <QAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KFileDialog>
#include <KStandardDirs>
#include <KRecentFilesAction>
#include <KLocalizedString>
#include <KLocale>
#include <KUrl>
#include <KConfigGroup>

#include <KNS3/KNewStuffAction>

#include <QPointer>
#include <QCloseEvent>
#include <KSharedConfig>
#include <QStandardPaths>

ContactEditorMainWindow::ContactEditorMainWindow()
    : KXmlGuiWindow(),
      mContactEditor(0)
{
    setupActions();
    setupGUI();
    updateActions();
    readConfig();
}

ContactEditorMainWindow::~ContactEditorMainWindow()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group( QLatin1String("ContactEditorMainWindow") );
    group.writeEntry( "Size", size() );
    mRecentFileAction->saveEntries(group);
}

void ContactEditorMainWindow::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = KConfigGroup( config, "ContactEditorMainWindow" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void ContactEditorMainWindow::updateActions()
{
    const bool projectDirectoryIsEmpty = (mContactEditor!=0);
    mAddExtraPage->setEnabled(projectDirectoryIsEmpty);
    mCloseAction->setEnabled(projectDirectoryIsEmpty);
    mUploadTheme->setEnabled(projectDirectoryIsEmpty);
    mSaveAction->setEnabled(projectDirectoryIsEmpty);
    mInstallTheme->setEnabled(projectDirectoryIsEmpty);
    mInsertFile->setEnabled(projectDirectoryIsEmpty);
    mUpdateView->setEnabled(projectDirectoryIsEmpty);
    mSaveAsAction->setEnabled(projectDirectoryIsEmpty);
}

void ContactEditorMainWindow::setupActions()
{
    mRecentFileAction = new KRecentFilesAction(i18n("Load Recent Theme..."), this);
    connect(mRecentFileAction, SIGNAL(urlSelected(QUrl)), this, SLOT(slotThemeSelected(QUrl)));
    actionCollection()->addAction( QLatin1String( "load_recent_theme" ), mRecentFileAction );
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup groupConfig = config->group( QLatin1String("ContactEditorMainWindow") );
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

    mManageTheme = new QAction(i18n("Manage themes..."), this);
    connect(mManageTheme, SIGNAL(triggered(bool)),SLOT(slotManageTheme()));
    actionCollection()->addAction( QLatin1String( "manage_themes" ), mManageTheme );

    mUpdateView = new QAction(i18n("Update view"), this);
    mUpdateView->setShortcut(QKeySequence( Qt::Key_F5 ));
    connect(mUpdateView, SIGNAL(triggered(bool)),SLOT(slotUpdateView()));
    actionCollection()->addAction( QLatin1String( "update_view" ), mUpdateView );
}

void ContactEditorMainWindow::slotConfigure()
{
    QPointer<ContactConfigureDialog> dialog = new ContactConfigureDialog(this);
    if (dialog->exec()) {
        if (mContactEditor) {
            mContactEditor->reloadConfig();
        }
    }
    delete dialog;
}

void ContactEditorMainWindow::slotManageTheme()
{
    QPointer<GrantleeThemeEditor::ManageThemes> dialog = new GrantleeThemeEditor::ManageThemes(QLatin1String("kaddressbook/viewertemplates/"), this);
    dialog->exec();
    delete dialog;
}

void ContactEditorMainWindow::slotInsertFile()
{
    mContactEditor->insertFile();
}

void ContactEditorMainWindow::slotInstallTheme()
{
    //Save before installing :)
    if (slotSaveTheme()) {
        const QString localThemePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kaddressbook/viewertemplates/");
        mContactEditor->installTheme(localThemePath);
    }
}

void ContactEditorMainWindow::slotUploadTheme()
{
    //Save before upload :)
    if (slotSaveTheme())
        mContactEditor->uploadTheme();
}

bool ContactEditorMainWindow::slotSaveTheme()
{
    bool result = false;
    if (mContactEditor) {
        result = mContactEditor->saveTheme(false);
        mSaveAction->setEnabled(result);
    }
    return result;
}

void ContactEditorMainWindow::slotCloseTheme()
{
    saveCurrentProject(SaveAndCloseTheme);
}

void ContactEditorMainWindow::slotOpenTheme()
{
    if (!saveCurrentProject(SaveOnly))
        return;

    const QString directory = KFileDialog::getExistingDirectory(KUrl( "kfiledialog:///OpenTheme" ), this, i18n("Select theme directory"));
    if (directory.isEmpty())
        return;
    closeThemeEditor();
    if (loadTheme(directory))
        mRecentFileAction->addUrl(KUrl(directory));
    mSaveAction->setEnabled(false);
}

bool ContactEditorMainWindow::loadTheme(const QString &directory)
{
    if (!directory.isEmpty()) {
        const QString filename = directory + QDir::separator() + QLatin1String("theme.themerc");
        QFile file(filename);
        if (!file.exists()) {
            KMessageBox::error(this, i18n("Directory does not contain a theme file. We cannot load theme."));
            return false;
        }

        mContactEditor = new ContactEditorPage(QString(), QString());
        connect(mContactEditor, SIGNAL(changed(bool)), mSaveAction, SLOT(setEnabled(bool)));
        connect(mContactEditor, SIGNAL(canInsertFile(bool)), this, SLOT(slotCanInsertFile(bool)));
        mContactEditor->loadTheme(filename);
        setCentralWidget(mContactEditor);
        updateActions();
    }
    return true;
}


void ContactEditorMainWindow::slotAddExtraPage()
{
    if (mContactEditor)
        mContactEditor->addExtraPage();
}

void ContactEditorMainWindow::closeThemeEditor()
{
    delete mContactEditor;
    mContactEditor = 0;
    setCentralWidget(0);
    updateActions();
}

bool ContactEditorMainWindow::saveCurrentProject(ActionSaveTheme act)
{
    if (mContactEditor) {
        if (!mContactEditor->saveTheme())
            return false;
    }
    switch(act) {
    case SaveOnly:
        break;
    case SaveAndCloseTheme: {
        closeThemeEditor();
        break;
    }
    case SaveAndCreateNewTheme: {
        delete mContactEditor;
        mContactEditor = 0;
        QPointer<GrantleeThemeEditor::NewThemeDialog> dialog = new GrantleeThemeEditor::NewThemeDialog(this);
        QString newTheme;
        QString projectDirectory;
        if (dialog->exec()) {
            newTheme = dialog->themeName();
            projectDirectory = dialog->directory();
        }
        if (!projectDirectory.isEmpty()) {
            mRecentFileAction->addUrl(KUrl(projectDirectory));
            mContactEditor = new ContactEditorPage(projectDirectory, newTheme);
            connect(mContactEditor, SIGNAL(changed(bool)), mSaveAction, SLOT(setEnabled(bool)));
            connect(mContactEditor, SIGNAL(canInsertFile(bool)), this, SLOT(slotCanInsertFile(bool)));
            setCentralWidget(mContactEditor);
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

void ContactEditorMainWindow::slotNewTheme()
{
    saveCurrentProject(SaveAndCreateNewTheme);
}

void ContactEditorMainWindow::closeEvent(QCloseEvent *e)
{
    if (!saveCurrentProject(SaveAndCloseTheme))
        e->ignore();
    else
        e->accept();
}

void ContactEditorMainWindow::slotQuitApp()
{
    if (saveCurrentProject(SaveAndCloseTheme))
        kapp->quit();
}

void ContactEditorMainWindow::slotUpdateView()
{
    if (mContactEditor) {
        mContactEditor->saveTheme(false);
        mContactEditor->updatePreview();
    }
}

void ContactEditorMainWindow::slotCanInsertFile(bool b)
{
    mInsertFile->setEnabled(b);
}

void ContactEditorMainWindow::slotThemeSelected(const QUrl &url)
{
    if (!saveCurrentProject(SaveAndCloseTheme))
        return;
    loadTheme(url.path());
    mSaveAction->setEnabled(false);
}

void ContactEditorMainWindow::slotSaveAsTheme()
{
    const QString directory = KFileDialog::getExistingDirectory(KUrl( "kfiledialog:///SaveTheme" ), this, i18n("Select theme directory"));
    if (!directory.isEmpty()) {
        if (mContactEditor)
            mContactEditor->saveThemeAs(directory);
    }
}

