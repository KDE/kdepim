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

#include "templatemanager.h"
#include "templatewidgets/templatelistwidget.h"

#include <KDirWatch>

#include <KConfigGroup>
#include <KConfig>

#include <QDirIterator>
#include <QStandardPaths>

using namespace PimCommon;

class PimCommon::TemplateManagerPrivate
{
public:
    TemplateManagerPrivate()
        : mTemplateListWidget(Q_NULLPTR),
          mDirWatch(Q_NULLPTR)
    {

    }

    QStringList mTemplatesDirectories;
    PimCommon::TemplateListWidget *mTemplateListWidget;
    KDirWatch *mDirWatch;
};

TemplateManager::TemplateManager(const QString &relativeTemplateDir, PimCommon::TemplateListWidget *templateListWidget)
    : QObject(templateListWidget),
      d(new PimCommon::TemplateManagerPrivate)

{
    d->mTemplateListWidget = templateListWidget;
    d->mDirWatch = new KDirWatch(this);
    initTemplatesDirectories(relativeTemplateDir);

    connect(d->mDirWatch, &KDirWatch::dirty, this, &TemplateManager::slotDirectoryChanged);
    loadTemplates(true);
}

TemplateManager::~TemplateManager()
{
    delete d;
}

void TemplateManager::slotDirectoryChanged()
{
    d->mTemplateListWidget->loadTemplates();
    loadTemplates();
}

void TemplateManager::initTemplatesDirectories(const QString &templatesRelativePath)
{
    if (!templatesRelativePath.isEmpty()) {
        d->mTemplatesDirectories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, templatesRelativePath, QStandardPaths::LocateDirectory);
        if (d->mTemplatesDirectories.count() < 2) {
            //Make sure to add local directory
            const QString localDirectory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + templatesRelativePath;
            if (!d->mTemplatesDirectories.contains(localDirectory)) {
                d->mTemplatesDirectories.append(localDirectory);
            }
        }
    }
}

void TemplateManager::loadTemplates(bool init)
{
    if (!init) {
        if (!d->mTemplatesDirectories.isEmpty()) {
            Q_FOREACH (const QString &directory, d->mTemplatesDirectories) {
                d->mDirWatch->removeDir(directory);
            }
        } else {
            return;
        }
    }

    Q_FOREACH (const QString &directory, d->mTemplatesDirectories) {
        QDirIterator dirIt(directory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
        while (dirIt.hasNext()) {
            dirIt.next();
            TemplateInfo info = loadTemplate(dirIt.filePath(), QStringLiteral("template.desktop"));
            if (info.isValid()) {
                d->mTemplateListWidget->addDefaultTemplate(info.name, info.script);
            }
        }
        d->mDirWatch->addDir(directory);
    }
    d->mDirWatch->startScan();
}

TemplateInfo TemplateManager::loadTemplate(const QString &themePath, const QString &defaultDesktopFileName)
{
    TemplateInfo info;
    const QString themeInfoFile = themePath + QDir::separator() + defaultDesktopFileName;
    KConfig config(themeInfoFile);
    KConfigGroup group(&config, QStringLiteral("Desktop Entry"));

    info.name = group.readEntry("Name", QString());
    const QString filename = group.readEntry("FileName", QString());
    if (!filename.isEmpty()) {
        QFile file(themePath + QDir::separator() + filename);
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly)) {
                info.script = QString::fromUtf8(file.readAll());
            }
        }
    }
    return info;
}

#include "moc_templatemanager.cpp"
