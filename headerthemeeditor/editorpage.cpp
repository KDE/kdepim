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

#include "editorpage.h"
#include "editorwidget.h"
#include "previewwidget.h"
#include "themeeditorwidget.h"
#include "themetemplatewidget.h"

#include <KTextEdit>
#include <KTemporaryFile>
#include <KLocale>
#include <KZip>
#include <KConfigGroup>
#include <KMessageBox>

#include <QSplitter>
#include <QVBoxLayout>
#include <QTextStream>
#include <QDir>

EditorPage::EditorPage(GrantleeThemeEditor::EditorPage::PageType type, const QString &projectDirectory, QWidget *parent)
    : GrantleeThemeEditor::EditorPage(type, parent),
      mPreview(0),
      mWidgetSplitter(0)
{
    QVBoxLayout *lay = new QVBoxLayout;

    mMainSplitter = new QSplitter;
    if (mType == MainPage) {
        mWidgetSplitter = new QSplitter;
        mWidgetSplitter->setOrientation(Qt::Vertical);
        mWidgetSplitter->setChildrenCollapsible(false);
        lay->addWidget(mWidgetSplitter);

        mWidgetSplitter->addWidget(mMainSplitter);

        mPreview = new PreviewWidget(projectDirectory);
        mWidgetSplitter->addWidget(mPreview);
        connect(mPreview, SIGNAL(needUpdateViewer()), this, SIGNAL(needUpdateViewer()));
    } else {
        lay->addWidget(mMainSplitter);
    }

    mEditor = new ThemeEditorWidget;

    if (mType == MainPage) {
        KConfigGroup group( KGlobal::config(), QLatin1String("Global"));
        mEditor->setPlainText(group.readEntry("defaultTemplate",QString()));
    }
    mMainSplitter->addWidget(mEditor);
    mMainSplitter->setChildrenCollapsible(false);
    mThemeTemplate = new ThemeTemplateWidget(i18n("Theme Templates:"));
    connect(mThemeTemplate, SIGNAL(insertTemplate(QString)), mEditor, SLOT(insertPlainText(QString)));
    mMainSplitter->addWidget(mThemeTemplate);

    connect(mEditor, SIGNAL(textChanged()), this, SIGNAL(changed()));


    if (mType == MainPage) {
        KConfigGroup group( KGlobal::config(), "EditorPage" );
        QList<int> size;
        size << 400 << 100;
        mMainSplitter->setSizes(group.readEntry( "mainSplitter", size));
        mWidgetSplitter->setSizes(group.readEntry( "widgetSplitter", size));
    }
    setLayout(lay);
}

EditorPage::~EditorPage()
{
    if (mType == MainPage) {
        KConfigGroup group( KGlobal::config(), "EditorPage" );
        group.writeEntry( "mainSplitter", mMainSplitter->sizes());
        group.writeEntry("widgetSplitter", mWidgetSplitter->sizes());
    }
}

void EditorPage::insertFile(const QString &filename)
{
    mEditor->insertFile(filename);
}

void EditorPage::createZip(const QString &themeName, KZip *zip)
{
    KTemporaryFile tmp;
    tmp.open();
    saveAsFilename(tmp.fileName());
    const bool fileAdded  = zip->addLocalFile(tmp.fileName(), themeName + QLatin1Char('/') + mPageFileName);
    if (!fileAdded) {
        KMessageBox::error(this, i18n("We can not add file in zip file"), i18n("Failed to add file."));
    }
}

void EditorPage::loadTheme(const QString &path)
{
    mEditor->clear();
    QFile file(path);
    if (file.open(QIODevice::Text|QIODevice::ReadOnly)) {
        const QByteArray data = file.readAll();
        const QString str = QString::fromUtf8(data);
        file.close();
        mEditor->setPlainText(str);
    }
}

void EditorPage::saveTheme(const QString &path)
{
    const QString filename = path + QDir::separator() + mPageFileName;
    saveAsFilename(filename);
}

void EditorPage::saveAsFilename(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << mEditor->toPlainText();
        file.close();
    }
}

void EditorPage::setPageFileName(const QString &filename)
{
    mPageFileName = filename;
}

QString EditorPage::pageFileName() const
{
    return mPageFileName;
}

void EditorPage::installTheme(const QString &themePath)
{
    const QString filename = themePath + QDir::separator() + mPageFileName;
    saveAsFilename(filename);
}

PreviewWidget *EditorPage::preview() const
{
    return mPreview;
}

EditorPage::PageType EditorPage::pageType() const
{
    return mType;
}

GrantleeThemeEditor::EditorWidget *EditorPage::editor() const
{
    return mEditor;
}

#include "editorpage.moc"
