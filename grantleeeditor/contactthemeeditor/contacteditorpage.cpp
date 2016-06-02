/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

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

#include "contacteditorpage.h"
#include "desktopfilepage.h"
#include "editorpage.h"
#include "contacteditorwidget.h"
#include "previewwidget.h"
#include "themesession.h"
#include "themeeditortabwidget.h"

#include <kns3/uploaddialog.h>

#include <KLocalizedString>
#include <QInputDialog>
#include <KZip>
#include <QTemporaryDir>
#include "contactthemeeditor_debug.h"
#include <KMessageBox>
#include <QUrl>

#include <QHBoxLayout>
#include <QDir>
#include <QPointer>
#include <QFileDialog>

ContactEditorPage::ContactEditorPage(const QString &projectDir, const QString &themeName, QWidget *parent)
    : QWidget(parent),
      mThemeSession(new GrantleeThemeEditor::ThemeSession(projectDir, QStringLiteral("contactthemeeditor"))),
      mChanged(false)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mTabWidget = new GrantleeThemeEditor::ThemeEditorTabWidget;
    connect(mTabWidget, &GrantleeThemeEditor::ThemeEditorTabWidget::currentChanged, this, &ContactEditorPage::slotCurrentWidgetChanged);
    lay->addWidget(mTabWidget);
    mEditorPage = new EditorPage(EditorPage::MainPage, projectDir);
    mEditorPage->setPageFileName(QStringLiteral("contact.html"));
    connect(mEditorPage, &EditorPage::needUpdateViewer, this, &ContactEditorPage::slotUpdateViewer);
    connect(mEditorPage, &EditorPage::changed, this, &ContactEditorPage::slotChanged);
    mTabWidget->addTab(mEditorPage, i18n("Editor") + QLatin1String(" (contact.html)"));

    mEditorEmbeddedPage = createCustomPage(QStringLiteral("contact_embedded.html"));

    mEditorGroupPage = createCustomPage(QStringLiteral("contactgroup.html"));

    mEditorGroupEmbeddedPage = createCustomPage(QStringLiteral("contactgroup_embedded.html"));

    GrantleeThemeEditor::DesktopFilePage::DesktopFileOptions opt;
    mDesktopPage = new GrantleeThemeEditor::DesktopFilePage(QStringLiteral("contact.html"), opt);
    mDesktopPage->setDefaultDesktopName(QStringLiteral("theme.desktop"));
    mDesktopPage->setThemeName(themeName);
    mTabWidget->addTab(mDesktopPage, i18n("Desktop File"));

    connect(mDesktopPage, &GrantleeThemeEditor::DesktopFilePage::changed, this, &ContactEditorPage::slotChanged);
    connect(mTabWidget, &GrantleeThemeEditor::ThemeEditorTabWidget::tabCloseRequested, this, &ContactEditorPage::slotCloseTab);
    setLayout(lay);
}

ContactEditorPage::~ContactEditorPage()
{
    qDeleteAll(mExtraPage);
    mExtraPage.clear();
    delete mThemeSession;
}

void ContactEditorPage::updatePreview()
{
    mEditorPage->preview()->updateViewer();
}

void ContactEditorPage::slotChanged()
{
    setChanged(true);
}

void ContactEditorPage::setChanged(bool b)
{
    if (mChanged != b) {
        mChanged = b;
        Q_EMIT changed(b);
    }
}

void ContactEditorPage::slotUpdateViewer()
{
    if (themeWasChanged()) {
        saveTheme(false);
    }
    mEditorPage->preview()->updateViewer();
}

void ContactEditorPage::slotCloseTab(int index)
{
    mTabWidget->removeTab(index);
    setChanged(true);
}

void ContactEditorPage::insertFile()
{
    QWidget *w = mTabWidget->currentWidget();
    if (!w) {
        return;
    }
    GrantleeThemeEditor::EditorPage *page = dynamic_cast<GrantleeThemeEditor::EditorPage *>(w);
    if (page) {
        const QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty()) {
            page->insertFile(fileName);
        }
    }
}

bool ContactEditorPage::themeWasChanged() const
{
    return mChanged;
}

void ContactEditorPage::installTheme(const QString &themePath)
{
    QDir dir(themePath);
    QDir themeDir(themePath + QDir::separator() + mDesktopPage->themeName());
    if (themeDir.exists()) {
        if (KMessageBox::questionYesNo(this, i18n("Theme already exists. Do you want to overwrite it?"), i18n("Theme already exists")) == KMessageBox::No) {
            return;
        }
    } else {
        if (!dir.mkdir(mDesktopPage->themeName())) {
            KMessageBox::error(this, i18n("Cannot create theme folder."));
            return;
        }
    }
    const QString newPath = themePath + QDir::separator() + mDesktopPage->themeName();
    mEditorPage->installTheme(newPath);
    mEditorGroupPage->installTheme(newPath);
    mEditorGroupEmbeddedPage->installTheme(newPath);
    mEditorEmbeddedPage->installTheme(newPath);

    Q_FOREACH (EditorPage *page, mExtraPage) {
        page->installTheme(newPath);
    }
    mDesktopPage->installTheme(newPath);
    KMessageBox::information(this, i18n("Theme installed in \"%1\"", themeDir.absolutePath()));
}

void ContactEditorPage::uploadTheme()
{
    //force update for screenshot
    mEditorPage->preview()->updateViewer();
    QTemporaryDir tmp;
    const QString themename = mDesktopPage->themeName();
    const QString zipFileName = tmp.path() + QDir::separator() + themename + QLatin1String(".zip");
    KZip *zip = new KZip(zipFileName);
    if (zip->open(QIODevice::WriteOnly)) {

        //TODO reactivate it when we will be able to create a preview
        const QString previewContactFileName = tmp.path() + QDir::separator() + themename + QLatin1String("contact_preview.png");
        const QString previewContactGroupFileName = tmp.path() + QDir::separator() + themename + QLatin1String("contactgroup_preview.png");
        QStringList lst;
        lst << previewContactFileName << previewContactGroupFileName;

        mEditorPage->preview()->createScreenShot(lst);

        bool fileAdded  = zip->addLocalFile(previewContactFileName, themename + QLatin1Char('/') + QLatin1String("contact_preview.png"));
        if (!fileAdded) {
            KMessageBox::error(this, i18n("We cannot add preview file in zip file"), i18n("Failed to add file."));
            delete zip;
            return;
        }
        fileAdded  = zip->addLocalFile(previewContactGroupFileName, themename + QLatin1Char('/') + QLatin1String("contactgroup_preview.png"));
        if (!fileAdded) {
            KMessageBox::error(this, i18n("We cannot add preview file in zip file"), i18n("Failed to add file."));
            delete zip;
            return;
        }
        createZip(themename, zip);
        zip->close();
        //qCDebug(CONTACTTHEMEEDITOR_LOG)<< "zipFilename"<<zipFileName;

        QPointer<KNS3::UploadDialog> dialog = new KNS3::UploadDialog(QStringLiteral("kaddressbook_themes.knsrc"), this);
        dialog->setUploadFile(QUrl::fromLocalFile(zipFileName));
        dialog->setUploadName(themename);
        dialog->setPreviewImageFile(0, QUrl::fromLocalFile(previewContactFileName));
        const QString description = mDesktopPage->description();
        dialog->setDescription(description.isEmpty() ? i18n("My favorite Kaddressbook theme") : description);
        dialog->exec();
        delete dialog;
    } else {
        qCDebug(CONTACTTHEMEEDITOR_LOG) << " We can't open in zip write mode";
    }
    delete zip;
}

void ContactEditorPage::createZip(const QString &themeName, KZip *zip)
{
    mEditorPage->createZip(themeName, zip);
    mEditorGroupPage->createZip(themeName, zip);
    mEditorGroupEmbeddedPage->createZip(themeName, zip);
    mEditorEmbeddedPage->createZip(themeName, zip);

    Q_FOREACH (EditorPage *page, mExtraPage) {
        page->createZip(themeName, zip);
    }
    mDesktopPage->createZip(themeName, zip);
}

void ContactEditorPage::addExtraPage()
{
    QString filename = QInputDialog::getText(this, i18n("Filename of extra page"), i18n("Filename:"));
    if (!filename.trimmed().isEmpty()) {
        if (!filename.endsWith(QStringLiteral(".html")) && !filename.endsWith(QStringLiteral(".css")) && !filename.endsWith(QStringLiteral(".js"))) {
            filename += QLatin1String(".html");
        }
        createExtraPage(filename);
        mThemeSession->addExtraPage(filename);
        setChanged(true);
    }
}

EditorPage *ContactEditorPage::createCustomPage(const QString &filename)
{
    EditorPage *customPage = new EditorPage(EditorPage::SecondPage, QString());
    connect(customPage, &EditorPage::changed, this, &ContactEditorPage::slotChanged);
    customPage->setPageFileName(filename);
    mTabWidget->addTab(customPage, filename);
    return customPage;
}

EditorPage *ContactEditorPage::createExtraPage(const QString &filename)
{
    EditorPage *extraPage = new EditorPage(EditorPage::ExtraPage, QString());
    connect(extraPage, &EditorPage::changed, this, &ContactEditorPage::slotChanged);
    extraPage->setPageFileName(filename);
    mTabWidget->addTab(extraPage, filename);
    mTabWidget->setCurrentWidget(extraPage);
    mExtraPage.append(extraPage);
    return extraPage;
}

void ContactEditorPage::storeTheme(const QString &directory)
{
    const QString themeDirectory = directory.isEmpty() ? projectDirectory() : directory;
    mEditorPage->saveTheme(themeDirectory);

    mEditorGroupPage->saveTheme(themeDirectory);
    mEditorGroupEmbeddedPage->saveTheme(themeDirectory);
    mEditorEmbeddedPage->saveTheme(themeDirectory);

    Q_FOREACH (EditorPage *page, mExtraPage) {
        page->saveTheme(themeDirectory);
    }
    mDesktopPage->saveTheme(themeDirectory);
    mThemeSession->setMainPageFileName(mEditorPage->pageFileName());
    mThemeSession->writeSession(themeDirectory);
    if (directory.isEmpty()) {
        setChanged(false);
    }
}

bool ContactEditorPage::saveTheme(bool withConfirmation)
{
    if (themeWasChanged()) {
        if (withConfirmation) {
            const int result = KMessageBox::questionYesNoCancel(this, i18n("Do you want to save current project?"), i18n("Save current project"));
            if (result == KMessageBox::Yes) {
                storeTheme();
            } else if (result == KMessageBox::Cancel) {
                return false;
            }
        } else {
            storeTheme();
        }
    }
    setChanged(false);
    return true;
}

void ContactEditorPage::loadTheme(const QString &filename)
{
    if (mThemeSession->loadSession(filename)) {
        const QString projectDirectory = mThemeSession->projectDirectory();
        mDesktopPage->loadTheme(projectDirectory);
        mEditorGroupPage->loadTheme(projectDirectory + QDir::separator() + QLatin1String("contactgroup.html"));
        mEditorGroupEmbeddedPage->loadTheme(projectDirectory + QDir::separator() + QLatin1String("contactgroup_embedded.html"));
        mEditorEmbeddedPage->loadTheme(projectDirectory + QDir::separator() + QLatin1String("contact_embedded.html"));

        mEditorPage->loadTheme(projectDirectory + QDir::separator() + mThemeSession->mainPageFileName());
        mEditorPage->preview()->setThemePath(projectDirectory, mThemeSession->mainPageFileName());

        const QStringList lstExtraPages = mThemeSession->extraPages();
        Q_FOREACH (const QString &page, lstExtraPages) {
            EditorPage *extraPage = createExtraPage(page);
            extraPage->loadTheme(projectDirectory + QDir::separator() + page);
        }
        mTabWidget->setCurrentIndex(0);
        setChanged(false);
    }
}

void ContactEditorPage::reloadConfig()
{
    mEditorPage->preview()->loadConfig();
}

QString ContactEditorPage::projectDirectory() const
{
    return mThemeSession->projectDirectory();
}

void ContactEditorPage::slotCurrentWidgetChanged(int index)
{
    if (index < 0) {
        return;
    }

    GrantleeThemeEditor::EditorPage *page = dynamic_cast<GrantleeThemeEditor::EditorPage *>(mTabWidget->widget(index));
    Q_EMIT canInsertFile(page);
}

void ContactEditorPage::saveThemeAs(const QString &directory)
{
    storeTheme(directory);
}

