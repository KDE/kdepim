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

#include <KTabWidget>
#include <KLocale>
#include <KInputDialog>

#include <QHBoxLayout>

ThemeEditorPage::ThemeEditorPage(const QString &themeName, QWidget *parent)
    : QWidget(parent)
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
}

void ThemeEditorPage::addExtraPage()
{
    const QString filename = KInputDialog::getText(i18n("Filename of extra page"), i18n("Filename:"));
    if (!filename.isEmpty()) {
        EditorPage *extraPage = new EditorPage;
        mTabWidget->addTab(extraPage, filename);
        mExtraPage.append(extraPage);
    }
}

void ThemeEditorPage::saveTheme(const QString &path)
{
    mEditorPage->saveTheme(path);
    Q_FOREACH (EditorPage *page, mExtraPage) {
        page->saveTheme(path);
    }
    mDesktopPage->saveTheme(path);

}

#include "themeeditorpage.moc"
