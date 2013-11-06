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

#include "sievetemplatemanager.h"
#include "sievetemplatewidget.h"

#include <KDirWatch>
#include <KStandardDirs>
#include <KConfigGroup>
#include <KConfig>

#include <QDirIterator>
#include <QDebug>

using namespace KSieveUi;
SieveTemplateManager::SieveTemplateManager(SieveTemplateWidget *sieveTemplateWidget)
    : QObject(sieveTemplateWidget),
      mSieveTemplateWidget(sieveTemplateWidget)
{
    mDirWatch = new KDirWatch( this );
    initTemplatesDirectories(QLatin1String("sieve/scripts"));

    connect( mDirWatch, SIGNAL(dirty(QString)), SLOT(slotDirectoryChanged()) );
    setTemplatePath();
}

SieveTemplateManager::~SieveTemplateManager()
{

}

void SieveTemplateManager::slotDirectoryChanged()
{
    setTemplatePath();
}

void SieveTemplateManager::initTemplatesDirectories(const QString &templatesRelativePath)
{
    if (!templatesRelativePath.isEmpty()) {
        mTemplatesDirectories = KGlobal::dirs()->findDirs("data", templatesRelativePath);
        if (mTemplatesDirectories.count() < 2) {
            //Make sure to add local directory
            const QString localDirectory = KStandardDirs::locateLocal("data", templatesRelativePath);
            if (!mTemplatesDirectories.contains(localDirectory)) {
                mTemplatesDirectories.append(localDirectory);
            }
        }
    }
    qDebug()<<" void SieveTemplateManager::initTemplatesDirectories(const QString &templatesRelativePath)"<<mTemplatesDirectories;
}

void SieveTemplateManager::setTemplatePath()
{
    if ( !mTemplatesDirectories.isEmpty() ) {
        mDirWatch->stopScan();
        Q_FOREACH (const QString &directory, mTemplatesDirectories) {
            mDirWatch->removeDir( directory );
        }
    } else {
        return;
    }

    Q_FOREACH (const QString &directory, mTemplatesDirectories) {
        QDirIterator dirIt( directory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
        while ( dirIt.hasNext() ) {
            dirIt.next();
            TemplateInfo info = loadTemplate(dirIt.filePath(), QLatin1String("template.desktop"));
            if (info.isValid()) {
                mSieveTemplateWidget->addDefaultTemplate(info.name, info.script);
            }
        }
        mDirWatch->addDir( directory );
    }
    mDirWatch->startScan();
}

TemplateInfo SieveTemplateManager::loadTemplate(const QString &themePath, const QString &defaultDesktopFileName)
{
    TemplateInfo info;
    const QString themeInfoFile = themePath + QDir::separator() + defaultDesktopFileName;
    KConfig config( themeInfoFile );
    KConfigGroup group( &config, QLatin1String( "Desktop Entry" ) );

    info.name = group.readEntry( "Name", QString() );
    const QString filename = group.readEntry( "FileName" , QString() );
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

#include "moc_sievetemplatemanager.cpp"
