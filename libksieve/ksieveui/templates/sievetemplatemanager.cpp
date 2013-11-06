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

#include <QDirIterator>

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


void SieveTemplateManager::loadTemplates()
{

}

void SieveTemplateManager::slotDirectoryChanged()
{

}

void SieveTemplateManager::initTemplatesDirectories(const QString &themesRelativePath)
{
    if (!themesRelativePath.isEmpty()) {
        mTemplatesDirectories = KGlobal::dirs()->findDirs("data", themesRelativePath);
        if (mTemplatesDirectories.count() < 2) {
            //Make sure to add local directory
            const QString localDirectory = KStandardDirs::locateLocal("data", themesRelativePath);
            if (!mTemplatesDirectories.contains(localDirectory)) {
                mTemplatesDirectories.append(localDirectory);
            }
        }
    }
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
        QStringList alreadyLoadedThemeName;
        while ( dirIt.hasNext() ) {
            dirIt.next();
            const QString dirName = dirIt.fileName();

            //TODO mSieveTemplateWidget->addDefaultTemplate(const QString &templateName, const QString &templateScript)

            /*
            GrantleeTheme::Theme theme = q->loadTheme( dirIt.filePath(), dirName, defaultDesktopFileName );
            if (theme.isValid()) {
                QString themeName = theme.name();
                if (alreadyLoadedThemeName.contains(themeName)) {
                    int i = 2;
                    const QString originalName(theme.name());
                    while (alreadyLoadedThemeName.contains(themeName)) {
                        themeName = originalName + QString::fromLatin1(" (%1)").arg(i);
                        ++i;
                    }
                    theme.setName(themeName);
                }
                alreadyLoadedThemeName << themeName;
                themes.insert( dirName, theme );
                //kDebug()<<" theme.name()"<<theme.name();
            }
            */
        }
        mDirWatch->addDir( directory );
    }

    mDirWatch->startScan();
}


#include "moc_sievetemplatemanager.cpp"
