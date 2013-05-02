/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <knewstuff3/uploaddialog.h>

#include <KTemporaryFile>
#include <KStandardAction>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KFileDialog>
#include <KZip>
#include <KDebug>

#include <QPointer>
#include <QCloseEvent>

ThemeEditorMainWindow::ThemeEditorMainWindow()
    : KXmlGuiWindow(),
      mThemeEditor(0)
{
    setupActions();
    setupGUI();
    updateActions();
    updateActions();
}

ThemeEditorMainWindow::~ThemeEditorMainWindow()
{
}

void ThemeEditorMainWindow::updateActions()
{
    const bool projectDirectoryIsEmpty = (mThemeEditor!=0);
    mAddExtraPage->setEnabled(projectDirectoryIsEmpty);
    mCloseAction->setEnabled(projectDirectoryIsEmpty);
    mUploadTheme->setEnabled(projectDirectoryIsEmpty);
    mSaveAction->setEnabled(projectDirectoryIsEmpty);
}

void ThemeEditorMainWindow::setupActions()
{
    mAddExtraPage = new KAction(i18n("Add Extra Page"), this);
    connect(mAddExtraPage, SIGNAL(triggered(bool)),SLOT(slotAddExtraPage()));
    actionCollection()->addAction( QLatin1String( "add_extra_page" ), mAddExtraPage );

    mUploadTheme = new KAction(i18n("Upload theme..."), this);
    actionCollection()->addAction( QLatin1String( "upload_theme" ), mUploadTheme );
    connect(mUploadTheme, SIGNAL(triggered(bool)), SLOT(slotUploadTheme()));

    mNewThemeAction = KStandardAction::openNew(this, SLOT(slotNewTheme()), actionCollection());
    mNewThemeAction->setText(i18n("New theme..."));

    mOpenAction = KStandardAction::open(this, SLOT(slotOpenTheme()), actionCollection());
    mSaveAction = KStandardAction::save(this, SLOT(slotSaveTheme()), actionCollection());
    mCloseAction = KStandardAction::close( this, SLOT(slotCloseTheme()), actionCollection());
    KStandardAction::quit(this, SLOT(slotQuitApp()), actionCollection() );
    KStandardAction::preferences( this, SLOT(slotConfigure()), actionCollection() );
}

void ThemeEditorMainWindow::slotConfigure()
{

}

void ThemeEditorMainWindow::slotUploadTheme()
{
    //Save before upload :)
    slotSaveTheme();
    //force update for screenshot
    mThemeEditor->forceUpdateViewer();
    KTemporaryFile tmp;
    tmp.open();
    KZip *zip = new KZip(tmp.fileName());
    if (zip->open(QIODevice::WriteOnly)) {
        mThemeEditor->createZip(zip);
        zip->close();
        qDebug()<< "tmp.fileName()"<<tmp.fileName();
        QPointer<KNS3::UploadDialog> dialog = new KNS3::UploadDialog(QLatin1String("messageviewer_header_themes.knsrc"), this);
        //TODO
        dialog->exec();
        delete dialog;
    } else {
        kDebug()<<" We can't open in zip write mode";
    }
    delete zip;
}

void ThemeEditorMainWindow::slotSaveTheme()
{
    mThemeEditor->saveTheme();
}

void ThemeEditorMainWindow::slotCloseTheme()
{
    saveCurrentProject(false);
}

void ThemeEditorMainWindow::slotOpenTheme()
{
    saveCurrentProject(false);
    const QString directory = KFileDialog::getExistingDirectory(KUrl(), this, i18n("Select theme"));
    if (!directory.isEmpty()) {
        const QString filename = directory + QDir::separator() + QLatin1String("theme.themerc");
        QFile file(filename);
        if (!file.exists()) {
            KMessageBox::error(this, i18n("Directory doesn't contains a theme file. We can not load theme."));
            return;
        }

        mThemeEditor = new ThemeEditorPage(QString());
        mThemeEditor->loadTheme(filename);
        setCentralWidget(mThemeEditor);
        updateActions();
    }
}

void ThemeEditorMainWindow::slotAddExtraPage()
{
    if (mThemeEditor)
        mThemeEditor->addExtraPage();
}

void ThemeEditorMainWindow::saveCurrentProject(bool createNewTheme)
{
    if (mThemeEditor) {
        if (KMessageBox::questionYesNo(this, i18n("Do you want to save current project?"), i18n("Save current project")) == KMessageBox::Yes) {
            mThemeEditor->saveTheme();
        }
    }
    if (createNewTheme) {
        delete mThemeEditor;
        mThemeEditor = 0;
        QPointer<NewThemeDialog> dialog = new NewThemeDialog(this);
        QString newTheme;
        QString projectDirectory;
        if (dialog->exec()) {
            newTheme = dialog->themeName();
            projectDirectory = dialog->directory();
        }
        if (!projectDirectory.isEmpty()) {
            mThemeEditor = new ThemeEditorPage(newTheme);
            mThemeEditor->setProjectDirectory(projectDirectory);
            setCentralWidget(mThemeEditor);
        } else {
            setCentralWidget(0);
        }
        delete dialog;
        updateActions();
    } else {
        delete mThemeEditor;
        mThemeEditor = 0;
        setCentralWidget(0);
        updateActions();
    }
}

void ThemeEditorMainWindow::slotNewTheme()
{
    saveCurrentProject(true);
}

void ThemeEditorMainWindow::closeEvent(QCloseEvent *e)
{
    saveCurrentProject(false);
    e->accept();
}

void ThemeEditorMainWindow::slotQuitApp()
{
    saveCurrentProject(false);
    kapp->quit();
}

#include "themeeditormainwindow.moc"
