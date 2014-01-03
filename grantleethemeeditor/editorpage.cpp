/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include <KZip>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTemporaryFile>

#include <QFile>
#include <QTextStream>
#include <QDir>


using namespace GrantleeThemeEditor;

EditorPage::EditorPage(PageType type, QWidget *parent)
    : QWidget(parent),
      mType(type),
      mPreview(0),
      mEditor(0)
{
}

EditorPage::~EditorPage()
{

}

EditorPage::PageType EditorPage::pageType() const
{
    return mType;
}

void EditorPage::setPageFileName(const QString &filename)
{
    mPageFileName = filename;
}

QString EditorPage::pageFileName() const
{
    return mPageFileName;
}

GrantleeThemeEditor::EditorWidget *EditorPage::editor() const
{
    return mEditor;
}

void EditorPage::insertFile(const QString &filename)
{
    if (mEditor)
        mEditor->insertFile(filename);
}

void EditorPage::loadTheme(const QString &path)
{
    if (!mEditor)
        return;

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
    if (!mEditor)
        return;

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
    } else {
        KMessageBox::error(this, i18n("Impossible to open file \"%1\"", filename));
    }
}

void EditorPage::createZip(const QString &themeName, KZip *zip)
{
    KTemporaryFile tmp;
    tmp.open();
    saveAsFilename(tmp.fileName());
    const bool fileAdded  = zip->addLocalFile(tmp.fileName(), themeName + QLatin1Char('/') + mPageFileName);
    if (!fileAdded) {
        KMessageBox::error(this, i18n("We cannot add file in zip file"), i18n("Failed to add file."));
    }
}

void EditorPage::installTheme(const QString &themePath)
{
    const QString filename = themePath + QDir::separator() + mPageFileName;
    saveAsFilename(filename);
}




