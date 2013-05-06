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

#include "themeeditorpage.h"
#include "desktopfilepage.h"
#include "editorpage.h"
#include "previewpage.h"
#include "themesession.h"

#include <knewstuff3/uploaddialog.h>

#include <KTabWidget>
#include <KLocale>
#include <KInputDialog>
#include <KZip>
#include <KTempDir>
#include <KDebug>
#include <KMessageBox>

#include <QHBoxLayout>
#include <QDir>
#include <QPointer>
#include <QDebug>

ThemeEditorPage::ThemeEditorPage(const QString &themeName, QWidget *parent)
    : QWidget(parent),
      mThemeSession(new ThemeSession)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mTabWidget = new KTabWidget;
    lay->addWidget(mTabWidget);
    mEditorPage = new EditorPage;
    mTabWidget->addTab(mEditorPage, i18n("Editor"));

    mDesktopPage = new DesktopFilePage;
    mDesktopPage->setThemeName(themeName);
    mTabWidget->addTab(mDesktopPage, i18n("Desktop File"));

    mPreviewPage = new PreviewPage;
    mTabWidget->addTab(mPreviewPage, i18n("Preview"));

    setLayout(lay);
}

ThemeEditorPage::~ThemeEditorPage()
{
    qDeleteAll(mExtraPage);
    mExtraPage.clear();
    delete mThemeSession;
}

bool ThemeEditorPage::themeWasChanged() const
{
    bool wasChanged = mEditorPage->wasChanged();
    if (wasChanged)
        return true;

    Q_FOREACH (EditorPage *page, mExtraPage) {
        wasChanged = page->wasChanged();
        if (wasChanged)
            return true;
    }
    wasChanged = mDesktopPage->wasChanged();
    return wasChanged;
}

void ThemeEditorPage::installTheme(const QString &themePath)
{
    QDir dir(themePath);
    QDir themeDir(themePath + QDir::separator() + mDesktopPage->themeName());
    if (themeDir.exists()) {
        if (KMessageBox::questionYesNo(this, i18n("Theme already exists. Do you want to overwrite it?"), i18n("Theme already exists")) == KMessageBox::No) {
            return;
        }
    } else {
        if (!dir.mkdir(mDesktopPage->themeName())) {
            KMessageBox::error(this, i18n("Can not create theme folder."));
            return;
        }
    }
    const QString newPath = themePath + QDir::separator() + mDesktopPage->themeName();
    mEditorPage->installTheme(newPath);

    Q_FOREACH (EditorPage *page, mExtraPage) {
        page->installTheme(newPath);
    }
    mDesktopPage->installTheme(newPath);
}

void ThemeEditorPage::uploadTheme()
{
    //force update for screenshot
    mPreviewPage->slotUpdateViewer();
    KTempDir tmp;
    const QString themename = mDesktopPage->themeName();
    const QString zipFileName = tmp.name() + QDir::separator() + themename + QLatin1String(".zip");
    KZip *zip = new KZip(zipFileName);
    if (zip->open(QIODevice::WriteOnly)) {
        createZip(themename, zip);
        zip->close();
        qDebug()<< "zipFilename"<<zipFileName;

        const QString previewFileName = tmp.name() + QDir::separator() + themename + QLatin1String("_preview.png");
        qDebug()<<" previewFileName"<<previewFileName;
        mPreviewPage->createScreenShot(previewFileName);

        QPointer<KNS3::UploadDialog> dialog = new KNS3::UploadDialog(QLatin1String("messageviewer_header_themes.knsrc"), this);
        dialog->setUploadFile(zipFileName);
        dialog->setUploadName(themename);
        dialog->setPreviewImageFile(0, KUrl(previewFileName));
        dialog->setDescription(i18n("My favorite KMail header"));
        dialog->exec();
        delete dialog;
    } else {
        kDebug()<<" We can't open in zip write mode";
    }
    delete zip;

}

void ThemeEditorPage::createZip(const QString &themeName, KZip *zip)
{
    mEditorPage->createZip(themeName, zip);

    Q_FOREACH (EditorPage *page, mExtraPage) {
        page->createZip(themeName, zip);
    }
    mDesktopPage->createZip(themeName, zip);
}

void ThemeEditorPage::addExtraPage()
{
    QString filename = KInputDialog::getText(i18n("Filename of extra page"), i18n("Filename:"));
    if (!filename.isEmpty()) {
        if (!filename.endsWith(QLatin1String(".html"))) {
            filename += QLatin1String(".html");
        }
        EditorPage *extraPage = new EditorPage;
        extraPage->setPageFileName(filename);
        mTabWidget->addTab(extraPage, filename);
        mThemeSession->addExtraPage(filename);
        mExtraPage.append(extraPage);
    }
}

void ThemeEditorPage::saveTheme()
{
    if (themeWasChanged()) {
        if (KMessageBox::questionYesNo(this, i18n("Do you want to save current project?"), i18n("Save current project")) == KMessageBox::Yes) {
            //set default page filename before saving
            mEditorPage->setPageFileName(mDesktopPage->filename());
            mEditorPage->saveTheme(projectDirectory());

            Q_FOREACH (EditorPage *page, mExtraPage) {
                page->saveTheme(projectDirectory());
            }
            mDesktopPage->saveTheme(projectDirectory());
            mThemeSession->setMainPageFileName(mDesktopPage->filename());
            mThemeSession->writeSession();
        }
    }
}

void ThemeEditorPage::loadTheme(const QString &filename)
{
    mThemeSession->loadSession(filename);
    mDesktopPage->loadTheme(mThemeSession->projectDirectory());
    mEditorPage->loadTheme(mThemeSession->projectDirectory() + QDir::separator() + mThemeSession->mainPageFileName());
    const QStringList lstExtraPages = mThemeSession->extraPages();
    Q_FOREACH(const QString &page, lstExtraPages) {
        EditorPage *extraPage = new EditorPage;
        extraPage->setPageFileName(page);
        mTabWidget->addTab(extraPage, page);
        mExtraPage.append(extraPage);
        extraPage->loadTheme(mThemeSession->projectDirectory() + QDir::separator() + page);
    }
}

QString ThemeEditorPage::projectDirectory() const
{
    return mThemeSession->projectDirectory();
}

void ThemeEditorPage::setProjectDirectory(const QString &dir)
{
    mThemeSession->setProjectDirectory(dir);
}

void ThemeEditorPage::reloadConfig()
{
    mPreviewPage->loadConfig();
}

#include "themeeditorpage.moc"
