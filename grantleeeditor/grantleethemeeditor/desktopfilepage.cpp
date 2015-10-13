/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "desktopfilepage.h"
#include "globalsettings_base.h"

#include <KEditListWidget>
#include <QLineEdit>
#include <KLocalizedString>
#include <KDesktopFile>
#include <KZip>
#include <QTemporaryFile>
#include <KMessageBox>

#include <QGridLayout>
#include <QLabel>
#include <QDir>

#include <pimcommon/plaintexteditorwidget.h>
#include <pimcommon/plaintexteditor.h>

using namespace GrantleeThemeEditor;

DesktopFilePage::DesktopFilePage(const QString &defaultFileName, DesktopFilePage::DesktopFileOptions options, QWidget *parent)
    : QWidget(parent),
      mFilename(0),
      mExtraDisplayHeaders(0)
{
    QGridLayout *lay = new QGridLayout;
    QLabel *lab = new QLabel(i18n("Name:"));
    mName = new QLineEdit;
    mName->setReadOnly(true);
    int row = 0;
    lay->addWidget(lab, row, 0);
    lay->addWidget(mName, row, 1);

    ++row;
    lab = new QLabel(i18n("Author:"));
    mAuthor = new QLineEdit;
    mAuthor->setClearButtonEnabled(true);
    lay->addWidget(lab, row, 0);
    lay->addWidget(mAuthor, row, 1);

    ++row;
    lab = new QLabel(i18n("Email:"));
    mEmail = new QLineEdit;
    mEmail->setClearButtonEnabled(true);
    lay->addWidget(lab, row, 0);
    lay->addWidget(mEmail, row, 1);

    ++row;
    lab = new QLabel(i18n("Description:"));
    mDescription = new PimCommon::PlainTextEditorWidget;
    mDescription->editor()->setSearchSupport(false);
    lay->addWidget(lab, row, 0);
    lay->addWidget(mDescription, row, 1);

    if (options & SpecifyFileName) {
        ++row;
        lab = new QLabel(i18n("Filename:"));
        mFilename = new QLineEdit;
        mFilename->setText(defaultFileName);
        connect(mFilename, &QLineEdit::textChanged, this, &DesktopFilePage::slotFileNameChanged);
        lay->addWidget(lab, row, 0);
        lay->addWidget(mFilename, row, 1);
    }

    ++row;
    lab = new QLabel(i18n("Version:"));
    mVersion = new QLineEdit;
    mVersion->setClearButtonEnabled(true);
    mVersion->setText(QStringLiteral("0.1"));
    lay->addWidget(lab, row, 0);
    lay->addWidget(mVersion, row, 1);

    ++row;
    if (options & ExtraDisplayVariables) {
        lab = new QLabel(i18n("Extract Headers:"));
        lay->addWidget(lab, row, 0);

        ++row;
        lab = new QLabel(QStringLiteral("<qt><b>") + i18n("Be careful, Grantlee does not support '-' in variable name. So when you want to add extra header as \"X-Original-To\" add \"X-Original-To\" in list, but use \"XOriginalTo\" as variable in Grantlee (remove '-' in name).") + QStringLiteral("</b></qt>"));
        lab->setWordWrap(true);
        lay->addWidget(lab, row, 0, 1, 2);

        ++row;
        mExtraDisplayHeaders = new KEditListWidget;
        lay->addWidget(mExtraDisplayHeaders, row, 0, 1, 2);
        connect(mExtraDisplayHeaders, &KEditListWidget::changed, this, &DesktopFilePage::slotExtraDisplayHeadersChanged);
    } else {
        lay->setRowStretch(row, 1);
    }
    setLayout(lay);

    mEmail->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::authorEmail());
    mAuthor->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::author());

    connect(mDescription->editor(), &QPlainTextEdit::textChanged, this, &DesktopFilePage::changed);
    connect(mEmail, &QLineEdit::textChanged, this, &DesktopFilePage::changed);
    connect(mAuthor, &QLineEdit::textChanged, this, &DesktopFilePage::changed);
    connect(mVersion, &QLineEdit::textChanged, this, &DesktopFilePage::changed);
}

DesktopFilePage::~DesktopFilePage()
{
}

void DesktopFilePage::slotExtraDisplayHeadersChanged()
{
    Q_EMIT extraDisplayHeaderChanged(mExtraDisplayHeaders->items());
    Q_EMIT changed();
}

void DesktopFilePage::slotFileNameChanged(const QString &filename)
{
    Q_EMIT mainFileNameChanged(filename);
    Q_EMIT changed();
}

void DesktopFilePage::createZip(const QString &themeName, KZip *zip)
{
    QTemporaryFile tmp;
    tmp.open();
    saveAsFilename(tmp.fileName());
    const bool fileAdded  = zip->addLocalFile(tmp.fileName(), themeName + QLatin1Char('/') + mDefaultDesktopName);
    if (!fileAdded) {
        KMessageBox::error(this, i18n("Failed to add file into ZIP archive."),
                           i18nc("@title:window", "Failed to add file"));
    }
}

void DesktopFilePage::setThemeName(const QString &themeName)
{
    mName->setText(themeName);
}

QString DesktopFilePage::filename() const
{
    if (mFilename) {
        return mFilename->text();
    }
    return QString();
}

QString DesktopFilePage::themeName() const
{
    return mName->text();
}

void DesktopFilePage::loadTheme(const QString &path)
{
    const QString filename = path + QDir::separator() + mDefaultDesktopName;
    KDesktopFile desktopFile(filename);
    mName->setText(desktopFile.desktopGroup().readEntry(QStringLiteral("Name")));
    mDescription->setPlainText(desktopFile.desktopGroup().readEntry(QStringLiteral("Description")));
    if (mFilename) {
        mFilename->setText(desktopFile.desktopGroup().readEntry(QStringLiteral("FileName")));
    }
    mAuthor->setText(desktopFile.desktopGroup().readEntry(QStringLiteral("Author")));
    mEmail->setText(desktopFile.desktopGroup().readEntry(QStringLiteral("AuthorEmail")));
    mVersion->setText(desktopFile.desktopGroup().readEntry(QStringLiteral("ThemeVersion")));
    if (mExtraDisplayHeaders) {
        const QStringList displayExtraHeaders = desktopFile.desktopGroup().readEntry(QStringLiteral("DisplayExtraVariables"), QStringList());
        mExtraDisplayHeaders->setItems(displayExtraHeaders);
    }
}

void DesktopFilePage::saveTheme(const QString &path)
{
    const QString filename = path + QDir::separator() + mDefaultDesktopName;
    saveAsFilename(filename);
}

void DesktopFilePage::saveAsFilename(const QString &filename)
{
    KDesktopFile desktopFile(filename);
    desktopFile.desktopGroup().writeEntry(QStringLiteral("Name"), mName->text());
    desktopFile.desktopGroup().writeEntry(QStringLiteral("Description"), mDescription->toPlainText());
    if (mFilename) {
        desktopFile.desktopGroup().writeEntry(QStringLiteral("FileName"), mFilename->text());
    }
    if (mExtraDisplayHeaders) {
        const QStringList displayExtraHeaders = mExtraDisplayHeaders->items();
        if (!displayExtraHeaders.isEmpty()) {
            desktopFile.desktopGroup().writeEntry(QStringLiteral("DisplayExtraVariables"), displayExtraHeaders);
        }
    }

    desktopFile.desktopGroup().writeEntry(QStringLiteral("Author"), mAuthor->text());
    desktopFile.desktopGroup().writeEntry(QStringLiteral("AuthorEmail"), mEmail->text());
    desktopFile.desktopGroup().writeEntry(QStringLiteral("ThemeVersion"), mVersion->text());
    desktopFile.desktopGroup().sync();
}

void DesktopFilePage::installTheme(const QString &themePath)
{
    const QString filename = themePath + QDir::separator() + mDefaultDesktopName;
    saveAsFilename(filename);
}

void DesktopFilePage::setDefaultDesktopName(const QString &name)
{
    mDefaultDesktopName = name;
}

QString DesktopFilePage::description() const
{
    return mDescription->toPlainText();
}

