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

#include "themeeditorpage.h"
#include "desktopfilepage.h"
#include "editorpage.h"
#include "themeeditorwidget.h"
#include "previewwidget.h"
#include "themesession.h"
#include "themeeditortabwidget.h"

#include <kns3/uploaddialog.h>

#include <KLocalizedString>
#include <QInputDialog>
#include <KZip>
#include <QTemporaryDir>
#include <QDebug>
#include <KMessageBox>
#include <QUrl>

#include <QHBoxLayout>
#include <QDir>
#include <QPointer>
#include <QDebug>
#include <QFileDialog>

ThemeEditorPage::ThemeEditorPage(const QString &projectDir, const QString &themeName, QWidget *parent)
    : QWidget(parent),
      mThemeSession(new GrantleeThemeEditor::ThemeSession(projectDir, QLatin1String("headerthemeeditor"))),
      mChanged(false)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mTabWidget = new GrantleeThemeEditor::ThemeEditorTabWidget;
    connect(mTabWidget, &GrantleeThemeEditor::ThemeEditorTabWidget::currentChanged, this, &ThemeEditorPage::slotCurrentWidgetChanged);
    lay->addWidget(mTabWidget);
    mEditorPage = new EditorPage(EditorPage::MainPage, projectDir);
    connect(mEditorPage, &EditorPage::needUpdateViewer, this, &ThemeEditorPage::slotUpdateViewer);
    connect(mEditorPage, &EditorPage::changed, this, &ThemeEditorPage::slotChanged);
    mTabWidget->addTab(mEditorPage, i18n("Editor") + QLatin1String(" (header.html)"));

    GrantleeThemeEditor::DesktopFilePage::DesktopFileOptions opt;
    opt |= GrantleeThemeEditor::DesktopFilePage::ExtraDisplayVariables;
    opt |= GrantleeThemeEditor::DesktopFilePage::SpecifyFileName;

    mDesktopPage = new GrantleeThemeEditor::DesktopFilePage(QLatin1String("header.html"), opt);
    mDesktopPage->setDefaultDesktopName(QLatin1String("header.desktop"));
    mDesktopPage->setThemeName(themeName);
    mTabWidget->addTab(mDesktopPage, i18n("Desktop File"));

    connect(mDesktopPage, SIGNAL(mainFileNameChanged(QString)), mEditorPage->preview(), SLOT(slotMainFileNameChanged(QString)));
    connect(mDesktopPage, &GrantleeThemeEditor::DesktopFilePage::mainFileNameChanged, mTabWidget, &GrantleeThemeEditor::ThemeEditorTabWidget::slotMainFileNameChanged);
    connect(mDesktopPage, &GrantleeThemeEditor::DesktopFilePage::extraDisplayHeaderChanged, this, &ThemeEditorPage::slotExtraHeaderDisplayChanged);
    connect(mDesktopPage, &GrantleeThemeEditor::DesktopFilePage::changed, this, &ThemeEditorPage::slotChanged);
    connect(mTabWidget, &GrantleeThemeEditor::ThemeEditorTabWidget::tabCloseRequested, this, &ThemeEditorPage::slotCloseTab);
    setLayout(lay);
}

ThemeEditorPage::~ThemeEditorPage()
{
    qDeleteAll(mExtraPage);
    mExtraPage.clear();
    delete mThemeSession;
}

void ThemeEditorPage::slotCurrentWidgetChanged(int index)
{
    if (index < 0) {
        return;
    }
    GrantleeThemeEditor::EditorPage *page = dynamic_cast<GrantleeThemeEditor::EditorPage *>(mTabWidget->widget(index));
    Q_EMIT canInsertFile(page);
}

void ThemeEditorPage::updatePreview()
{
    mEditorPage->preview()->updateViewer();
}

void ThemeEditorPage::setPrinting(bool print)
{
    mEditorPage->preview()->setPrinting(print);
}

void ThemeEditorPage::slotExtraHeaderDisplayChanged(const QStringList &extraHeaders)
{
    mEditorPage->preview()->slotExtraHeaderDisplayChanged(extraHeaders);

    QStringList result;
    Q_FOREACH(QString var, extraHeaders) {
        var = QLatin1String("header.") + var.remove(QLatin1Char('-'));
        result << var;
    }

    mEditorPage->editor()->createCompleterList(result);
    Q_FOREACH(EditorPage * page, mExtraPage) {
        page->editor()->createCompleterList(result);
    }
}

void ThemeEditorPage::slotChanged()
{
    setChanged(true);
}

void ThemeEditorPage::setChanged(bool b)
{
    if (mChanged != b) {
        mChanged = b;
        Q_EMIT changed(b);
    }
}

void ThemeEditorPage::slotUpdateViewer()
{
    if (themeWasChanged()) {
        saveTheme(false);
    }
    mEditorPage->preview()->updateViewer();
}

void ThemeEditorPage::slotCloseTab(int index)
{
    mTabWidget->removeTab(index);
    setChanged(true);
}

void ThemeEditorPage::insertFile()
{
    QWidget *w = mTabWidget->currentWidget();
    if (!w) {
        return;
    }
    GrantleeThemeEditor::EditorPage *page = dynamic_cast<GrantleeThemeEditor::EditorPage *>(w);
    if (page) {
        const QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(), QLatin1String("*"));
        if (!fileName.isEmpty()) {
            page->insertFile(fileName);
        }
    }
}

bool ThemeEditorPage::themeWasChanged() const
{
    return mChanged;
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
            KMessageBox::error(this, i18n("Cannot create theme folder."));
            return;
        }
    }
    const QString newPath = themePath + QDir::separator() + mDesktopPage->themeName();
    mEditorPage->setPageFileName(mDesktopPage->filename());
    mEditorPage->installTheme(newPath);
    Q_FOREACH(EditorPage * page, mExtraPage) {
        page->installTheme(newPath);
    }
    mDesktopPage->installTheme(newPath);
    KMessageBox::information(this, i18n("Theme installed in \"%1\"", themeDir.absolutePath()));
}

void ThemeEditorPage::uploadTheme()
{
    //force update for screenshot
    mEditorPage->preview()->updateViewer();
    QTemporaryDir tmp;
    const QString themename = mDesktopPage->themeName();
    const QString zipFileName = tmp.path() + QDir::separator() + themename + QLatin1String(".zip");
    KZip *zip = new KZip(zipFileName);
    if (zip->open(QIODevice::WriteOnly)) {
        const QString previewFileName = tmp.path() + QDir::separator() + themename + QLatin1String("_preview.png");
        //qDebug()<<" previewFileName"<<previewFileName;
        QStringList lst;
        lst << previewFileName;
        mEditorPage->preview()->createScreenShot(lst);

        const bool fileAdded  = zip->addLocalFile(previewFileName, themename + QLatin1Char('/') + QLatin1String("theme_preview.png"));
        if (!fileAdded) {
            KMessageBox::error(this, i18n("We cannot add preview file in zip file"), i18n("Failed to add file."));
            delete zip;
            return;
        }

        createZip(themename, zip);
        zip->close();
        //qDebug()<< "zipFilename"<<zipFileName;

        QPointer<KNS3::UploadDialog> dialog = new KNS3::UploadDialog(QLatin1String("messageviewer_header_themes.knsrc"), this);
        dialog->setUploadFile(QUrl::fromLocalFile(zipFileName));
        dialog->setUploadName(themename);
        dialog->setPreviewImageFile(0, QUrl::fromLocalFile(previewFileName));
        const QString description = mDesktopPage->description();
        dialog->setDescription(description.isEmpty() ? i18n("My favorite KMail header") : description);
        dialog->exec();
        delete dialog;
    } else {
        qDebug() << " We can't open in zip write mode";
    }
    delete zip;
}

void ThemeEditorPage::createZip(const QString &themeName, KZip *zip)
{
    mEditorPage->createZip(themeName, zip);

    Q_FOREACH(EditorPage * page, mExtraPage) {
        page->createZip(themeName, zip);
    }
    mDesktopPage->createZip(themeName, zip);
}

void ThemeEditorPage::addExtraPage()
{
    QString filename = QInputDialog::getText(this, i18n("Filename of extra page"), i18n("Filename:"));
    if (!filename.isEmpty()) {
        if (!filename.endsWith(QLatin1String(".html")) && !filename.endsWith(QLatin1String(".css")) && !filename.endsWith(QLatin1String(".js"))) {
            filename += QLatin1String(".html");
        }
        createExtraPage(filename);
        mThemeSession->addExtraPage(filename);
        setChanged(true);
    }
}

EditorPage *ThemeEditorPage::createExtraPage(const QString &filename)
{
    EditorPage *extraPage = new EditorPage(EditorPage::ExtraPage, QString());
    connect(extraPage, &EditorPage::changed, this, &ThemeEditorPage::slotChanged);
    extraPage->setPageFileName(filename);
    mTabWidget->addTab(extraPage, filename);
    mTabWidget->setCurrentWidget(extraPage);
    mExtraPage.append(extraPage);
    return extraPage;
}

void ThemeEditorPage::storeTheme(const QString &directory)
{
    const QString themeDirectory = directory.isEmpty() ? projectDirectory() : directory;
    //set default page filename before saving
    mEditorPage->setPageFileName(mDesktopPage->filename());
    mEditorPage->saveTheme(themeDirectory);

    Q_FOREACH(EditorPage * page, mExtraPage) {
        page->saveTheme(themeDirectory);
    }
    mDesktopPage->saveTheme(themeDirectory);
    mThemeSession->setMainPageFileName(mDesktopPage->filename());
    mThemeSession->writeSession(directory);
    if (directory.isEmpty()) {
        setChanged(false);
    }
}

bool ThemeEditorPage::saveTheme(bool withConfirmation)
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

void ThemeEditorPage::loadTheme(const QString &filename)
{
    if (mThemeSession->loadSession(filename)) {
        mDesktopPage->loadTheme(mThemeSession->projectDirectory());
        mEditorPage->loadTheme(mThemeSession->projectDirectory() + QDir::separator() + mThemeSession->mainPageFileName());
        mEditorPage->preview()->setThemePath(mThemeSession->projectDirectory(), mThemeSession->mainPageFileName());

        const QStringList lstExtraPages = mThemeSession->extraPages();
        Q_FOREACH(const QString & page, lstExtraPages) {
            EditorPage *extraPage = createExtraPage(page);
            extraPage->loadTheme(mThemeSession->projectDirectory() + QDir::separator() + page);
        }
        mTabWidget->setCurrentIndex(0);
        setChanged(false);
    }
}

void ThemeEditorPage::reloadConfig()
{
    mEditorPage->preview()->loadConfig();
}

QString ThemeEditorPage::projectDirectory() const
{
    return mThemeSession->projectDirectory();
}

void ThemeEditorPage::saveThemeAs(const QString &directory)
{
    storeTheme(directory);
}

