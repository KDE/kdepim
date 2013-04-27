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

#include <KTabWidget>
#include <KLocale>
#include <KInputDialog>

#include <QHBoxLayout>
#include <QDir>

ThemeEditorPage::ThemeEditorPage(const QString &themeName, QWidget *parent)
    : QWidget(parent),
      mThemeSession(0)
{
    mThemeSession = new ThemeSession;
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


#include "themeeditorpage.moc"
