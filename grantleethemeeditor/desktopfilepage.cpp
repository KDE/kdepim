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


#include "desktopfilepage.h"
#include "globalsettings_base.h"

#include "pimcommon/widgets/simplestringlisteditor.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"

#include <KLineEdit>
#include <KTextEdit>
#include <KLocale>
#include <KDesktopFile>
#include <KConfigGroup>
#include <KZip>
#include <KTemporaryFile>
#include <KMessageBox>

#include <QGridLayout>
#include <QLabel>
#include <QDir>

using namespace GrantleeThemeEditor;

DesktopFilePage::DesktopFilePage(const QString &defaultFileName, DesktopFilePage::DesktopFileOptions options, QWidget *parent)
    : QWidget(parent),
      mFilename(0),
      mExtraDisplayHeaders(0)
{
    QGridLayout *lay = new QGridLayout;
    QLabel *lab = new QLabel(i18n("Name:"));
    mName = new KLineEdit;
    mName->setReadOnly(true);
    int row = 0;
    lay->addWidget(lab, row,0);
    lay->addWidget(mName, row,1);

    ++row;
    lab = new QLabel(i18n("Author:"));
    mAuthor = new KLineEdit;
    mAuthor->setClearButtonShown(true);
    lay->addWidget(lab,row,0);
    lay->addWidget(mAuthor,row,1);

    ++row;
    lab = new QLabel(i18n("Email:"));
    mEmail = new KLineEdit;
    mEmail->setClearButtonShown(true);
    lay->addWidget(lab, row,0);
    lay->addWidget(mEmail,row,1);

    ++row;
    lab = new QLabel(i18n("Description:"));
    mDescription = new PimCommon::RichTextEditorWidget;
    mDescription->setAcceptRichText(false);
    lay->addWidget(lab, row ,0);
    lay->addWidget(mDescription, row,1);

    if (options & SpecifyFileName) {
        ++row;
        lab = new QLabel(i18n("Filename:"));
        mFilename = new KLineEdit;
        mFilename->setText(defaultFileName);
        connect(mFilename, SIGNAL(textChanged(QString)), this, SLOT(slotFileNameChanged(QString)));
        lay->addWidget(lab, row,0);
        lay->addWidget(mFilename, row,1);
    }

    ++row;
    lab = new QLabel(i18n("Version:"));
    mVersion = new KLineEdit;
    mVersion->setClearButtonShown(true);
    mVersion->setText(QLatin1String("0.1"));
    lay->addWidget(lab, row,0);
    lay->addWidget(mVersion, row,1);

    ++row;
    if (options & ExtraDisplayVariables) {
        lab = new QLabel(i18n("Extract Headers:"));
        lay->addWidget(lab, row,0);

        ++row;
        lab = new QLabel(QLatin1String("<qt><b>") +i18n("Be careful, Grantlee does not support '-' in variable name. So when you want to add extra header as \"X-Original-To\" add \"X-Original-To\" in list, but use \"XOriginalTo\" as variable in Grantlee (remove '-' in name).")+QLatin1String("</b></qt>"));
        lab->setWordWrap(true);
        lay->addWidget(lab, row ,0,1,2);

        ++row;
        mExtraDisplayHeaders = new PimCommon::SimpleStringListEditor;
        lay->addWidget(mExtraDisplayHeaders, row, 0, 1, 2);
        connect(mExtraDisplayHeaders, SIGNAL(changed()), this, SLOT(slotExtraDisplayHeadersChanged()));
    } else {
        lay->setRowStretch(row,1);
    }
    setLayout(lay);

    mEmail->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::authorEmail());
    mAuthor->setText(GrantleeThemeEditor::GrantleeThemeEditorSettings::author());

    connect(mDescription->editor(), SIGNAL(textChanged()), this, SIGNAL(changed()));
}

DesktopFilePage::~DesktopFilePage()
{
}

void DesktopFilePage::slotExtraDisplayHeadersChanged()
{
    Q_EMIT extraDisplayHeaderChanged(mExtraDisplayHeaders->stringList());
    Q_EMIT changed();
}

void DesktopFilePage::slotFileNameChanged(const QString &filename)
{
    Q_EMIT mainFileNameChanged(filename);
    Q_EMIT changed();
}

void DesktopFilePage::createZip(const QString &themeName, KZip *zip)
{
    KTemporaryFile tmp;
    tmp.open();
    saveAsFilename(tmp.fileName());
    const bool fileAdded  = zip->addLocalFile(tmp.fileName(), themeName + QLatin1Char('/') + mDefaultDesktopName);
    if (!fileAdded) {
        KMessageBox::error(this, i18n("We cannot add file in zip file"), i18n("Failed to add file."));
    }
}

void DesktopFilePage::setThemeName(const QString &themeName)
{
    mName->setText(themeName);
}

QString DesktopFilePage::filename() const
{
    if (mFilename)
       return mFilename->text();
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
    mName->setText(desktopFile.desktopGroup().readEntry(QLatin1String("Name")));
    mDescription->setPlainText(desktopFile.desktopGroup().readEntry(QLatin1String("Description")));
    if (mFilename)
        mFilename->setText(desktopFile.desktopGroup().readEntry(QLatin1String("FileName")));
    mAuthor->setText(desktopFile.desktopGroup().readEntry(QLatin1String("Author")));
    mEmail->setText(desktopFile.desktopGroup().readEntry(QLatin1String("AuthorEmail")));
    mVersion->setText(desktopFile.desktopGroup().readEntry(QLatin1String("ThemeVersion")));
    if (mExtraDisplayHeaders) {
        const QStringList displayExtraHeaders = desktopFile.desktopGroup().readEntry(QLatin1String("DisplayExtraVariables"),QStringList());
        mExtraDisplayHeaders->setStringList(displayExtraHeaders);
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
    desktopFile.desktopGroup().writeEntry(QLatin1String("Name"), mName->text());
    desktopFile.desktopGroup().writeEntry(QLatin1String("Description"), mDescription->toPlainText());
    if (mFilename)
        desktopFile.desktopGroup().writeEntry(QLatin1String("FileName"), mFilename->text());
    if (mExtraDisplayHeaders) {
        const QStringList displayExtraHeaders = mExtraDisplayHeaders->stringList();
        if (!displayExtraHeaders.isEmpty())
            desktopFile.desktopGroup().writeEntry(QLatin1String("DisplayExtraVariables"), mExtraDisplayHeaders->stringList());
    }

    desktopFile.desktopGroup().writeEntry(QLatin1String("Author"), mAuthor->text());
    desktopFile.desktopGroup().writeEntry(QLatin1String("AuthorEmail"), mEmail->text());
    desktopFile.desktopGroup().writeEntry(QLatin1String("ThemeVersion"), mVersion->text());
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

