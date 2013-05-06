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
#include "pimcommon/simplestringlisteditor.h"

#include <KLineEdit>
#include <KLocale>
#include <KDesktopFile>
#include <KConfigGroup>
#include <KZip>
#include <KTemporaryFile>

#include <QGridLayout>
#include <QLabel>
#include <QDir>

DesktopFilePage::DesktopFilePage(QWidget *parent)
    : QWidget(parent),
      mChanged(false)
{
    QGridLayout *lay = new QGridLayout;
    QLabel *lab = new QLabel(i18n("Name:"));
    mName = new KLineEdit;
    mName->setReadOnly(true);
    lay->addWidget(lab,0,0);
    lay->addWidget(mName,0,1);

    lab = new QLabel(i18n("Description:"));
    mDescription = new KLineEdit;
    lay->addWidget(lab,1,0);
    lay->addWidget(mDescription,1,1);

    lab = new QLabel(i18n("Filename:"));
    mFilename = new KLineEdit;
    mFilename->setText(QLatin1String("header.html"));
    lay->addWidget(lab,2,0);
    lay->addWidget(mFilename,2,1);

    lab = new QLabel(i18n("Extract Headers:"));
    lay->addWidget(lab,3,0);

    mExtraDisplayHeaders = new PimCommon::SimpleStringListEditor;
    lay->addWidget(mExtraDisplayHeaders, 4, 0, 4, 2);
    setLayout(lay);
    connect(mExtraDisplayHeaders, SIGNAL(changed()), this, SLOT(slotChanged()));
    connect(mFilename, SIGNAL(textChanged(QString)), this, SLOT(slotChanged()));
    connect(mDescription, SIGNAL(textChanged(QString)), this, SLOT(slotChanged()));
}

DesktopFilePage::~DesktopFilePage()
{
}

void DesktopFilePage::slotChanged()
{
    mChanged = true;
}

void DesktopFilePage::createZip(const QString &themeName, KZip *zip)
{
    KTemporaryFile tmp;
    tmp.open();
    saveAsFilename(tmp.fileName());
    const bool fileAdded  = zip->addLocalFile(tmp.fileName(), themeName + QLatin1Char('/') + QLatin1String("header.desktop"));
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
    const QString filename = path + QDir::separator() + QLatin1String("header.desktop");
    KDesktopFile desktopFile(filename);
    mName->setText(desktopFile.desktopGroup().readEntry(QLatin1String("Name")));
    mDescription->setText(desktopFile.desktopGroup().readEntry(QLatin1String("Description")));
    mFilename->setText(desktopFile.desktopGroup().readEntry(QLatin1String("FileName")));
    const QStringList displayExtraHeaders = desktopFile.desktopGroup().readEntry(QLatin1String("DisplayExtraHeaders"),QStringList());
    mExtraDisplayHeaders->setStringList(displayExtraHeaders);
    mChanged = false;
}

void DesktopFilePage::saveTheme(const QString &path)
{
    const QString filename = path + QDir::separator() + QLatin1String("header.desktop");
    saveAsFilename(filename);
    mChanged = false;
}

void DesktopFilePage::saveAsFilename(const QString &filename)
{
    KDesktopFile desktopFile(filename);
    desktopFile.desktopGroup().writeEntry(QLatin1String("Name"), mName->text());
    desktopFile.desktopGroup().writeEntry(QLatin1String("Description"), mDescription->text());
    desktopFile.desktopGroup().writeEntry(QLatin1String("FileName"), mFilename->text());
    const QStringList displayExtraHeaders = mExtraDisplayHeaders->stringList();
    if (!displayExtraHeaders.isEmpty())
        desktopFile.desktopGroup().writeEntry(QLatin1String("DisplayExtraHeaders"), mExtraDisplayHeaders->stringList());
    desktopFile.desktopGroup().sync();
}

bool DesktopFilePage::wasChanged() const
{
    return mChanged;
}

void DesktopFilePage::installTheme(const QString &themePath)
{
    const QString filename = themePath + QDir::separator() + QLatin1String("header.desktop");
    saveAsFilename(filename);
}

#include "desktopfilepage.moc"
