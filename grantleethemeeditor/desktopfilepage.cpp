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
#include "pimcommon/widgets/simplestringlisteditor.h"

#include <KLineEdit>
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

DesktopFilePage::DesktopFilePage(bool allowToAddExtraDisplayVariables, QWidget *parent)
    : QWidget(parent),
      mExtraDisplayHeaders(0)
{
    QGridLayout *lay = new QGridLayout;
    QLabel *lab = new QLabel(i18n("Name:"));
    mName = new KLineEdit;
    mName->setReadOnly(true);
    lay->addWidget(lab,0,0);
    lay->addWidget(mName,0,1);


    lab = new QLabel(i18n("Author:"));
    mAuthor = new KLineEdit;
    mAuthor->setClearButtonShown(true);
    lay->addWidget(lab,1,0);
    lay->addWidget(mAuthor,1,1);

    lab = new QLabel(i18n("Email:"));
    mEmail = new KLineEdit;
    mEmail->setClearButtonShown(true);
    lay->addWidget(lab,2,0);
    lay->addWidget(mEmail,2,1);

    lab = new QLabel(i18n("Description:"));
    mDescription = new KLineEdit;
    mDescription->setClearButtonShown(true);
    lay->addWidget(lab,3,0);
    lay->addWidget(mDescription,3,1);

    lab = new QLabel(i18n("Filename:"));
    mFilename = new KLineEdit;
    mFilename->setText(QLatin1String("header.html"));
    lay->addWidget(lab,4,0);
    lay->addWidget(mFilename,4,1);

    lab = new QLabel(i18n("Version:"));
    mVersion = new KLineEdit;
    mVersion->setText(QLatin1String("0.1"));
    lay->addWidget(lab,5,0);
    lay->addWidget(mVersion,5,1);

    if (allowToAddExtraDisplayVariables) {
        lab = new QLabel(i18n("Extract Headers:"));
        lay->addWidget(lab,6,0);

        lab = new QLabel(QLatin1String("<qt><b>") +i18n("Be careful, Grantlee does not support '-' in variable name. So when you want to add extra header as \"X-Original-To\" add \"X-Original-To\" in list, but use \"XOriginalTo\" as variable in Grantlee (remove '-' in name).")+QLatin1String("</b></qt>"));
        lab->setWordWrap(true);
        lay->addWidget(lab,7,0,1,2);

        mExtraDisplayHeaders = new PimCommon::SimpleStringListEditor;
        lay->addWidget(mExtraDisplayHeaders, 8, 0, 1, 2);
        connect(mExtraDisplayHeaders, SIGNAL(changed()), this, SLOT(slotExtraDisplayHeadersChanged()));
    }
    setLayout(lay);
    connect(mFilename, SIGNAL(textChanged(QString)), this, SLOT(slotFileNameChanged(QString)));
    connect(mDescription, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
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
        KMessageBox::error(this, i18n("We can not add file in zip file"), i18n("Failed to add file."));
    }
}

void DesktopFilePage::setThemeName(const QString &themeName)
{
    mName->setText(themeName);
}

QString DesktopFilePage::filename() const
{
    return mFilename->text();
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
    mDescription->setText(desktopFile.desktopGroup().readEntry(QLatin1String("Description")));
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
    desktopFile.desktopGroup().writeEntry(QLatin1String("Description"), mDescription->text());
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
    return mDescription->text();
}

#include "desktopfilepage.moc"
