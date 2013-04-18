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

#include <KStandardAction>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KFileDialog>

#include <QPointer>

ThemeEditorMainWindow::ThemeEditorMainWindow()
    : KXmlGuiWindow(),
      mThemeEditor(0)
{
    setupActions();
    setupGUI();
    updateActions();
}

ThemeEditorMainWindow::~ThemeEditorMainWindow()
{
}

void ThemeEditorMainWindow::updateActions()
{
    const bool projectDirectoryIsEmpty = (mThemeEditor!=0);
    mAddExtraPage->setEnabled(!projectDirectoryIsEmpty);
    mCloseAction->setEnabled(!projectDirectoryIsEmpty);
    mUploadTheme->setEnabled(!projectDirectoryIsEmpty);
}

void ThemeEditorMainWindow::setupActions()
{
    mNewThemeAction = new KAction(i18n("New theme..."),this);
    connect(mNewThemeAction, SIGNAL(triggered(bool)),SLOT(slotNewTheme()));
    actionCollection()->addAction( QLatin1String( "new_theme" ), mNewThemeAction );

    mAddExtraPage = new KAction(i18n("Add Extra Page"), this);
    connect(mAddExtraPage, SIGNAL(triggered(bool)),SLOT(slotAddExtraPage()));
    actionCollection()->addAction( QLatin1String( "add_extra_page" ), mAddExtraPage );

    mUploadTheme = new KAction(i18n("Upload theme..."), this);
    actionCollection()->addAction( QLatin1String( "upload_theme" ), mUploadTheme );
    connect(mUploadTheme, SIGNAL(triggered(bool)), SLOT(slotUploadTheme()));

    mOpenAction = KStandardAction::open(this, SLOT(slotOpenTheme()), actionCollection());
    mCloseAction = KStandardAction::close( this, SLOT(slotCloseTheme()), actionCollection());
    KStandardAction::quit( kapp, SLOT(quit()), actionCollection() );
}

void ThemeEditorMainWindow::slotUploadTheme()
{
    QPointer<KNS3::UploadDialog> dialog = new KNS3::UploadDialog(this);
    //TODO
    dialog->exec();
    delete dialog;
}

void ThemeEditorMainWindow::slotCloseTheme()
{
    saveCurrentProject();
}

void ThemeEditorMainWindow::slotOpenTheme()
{
    saveCurrentProject(true);
    const QString fileName = KFileDialog::getOpenFileName(KUrl(), QString::fromLatin1("*.themerc"), this, i18n("Select theme"));
    if (!fileName.isEmpty()) {
        //TODO load it.
    }
}

void ThemeEditorMainWindow::slotAddExtraPage()
{
    if (mThemeEditor)
        mThemeEditor->addExtraPage();
}

void ThemeEditorMainWindow::saveCurrentProject(bool close)
{
    if (mThemeEditor) {
        if (KMessageBox::questionYesNo(this, i18n("Do you want to save current project?"), i18n("Save current project")) == KMessageBox::Yes) {
            mThemeEditor->saveTheme();
        }
    }
    if (!close) {
        delete mThemeEditor;
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
    }
}

void ThemeEditorMainWindow::slotNewTheme()
{
    saveCurrentProject();
}

void ThemeEditorMainWindow::closeEvent(QCloseEvent *e)
{
    saveCurrentProject(true);
    KXmlGuiWindow::closeEvent(e);
}

#include "themeeditormainwindow.moc"
