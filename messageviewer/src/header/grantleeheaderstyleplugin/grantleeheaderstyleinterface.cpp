/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "grantleeheaderstyleinterface.h"
#include "header/headerstyleplugin.h"
#include "header/headerstyle.h"
#include "globalsettings_grantleetheme.h"
#include <grantleetheme/grantleethememanager.h>
#include <KToggleAction>

using namespace MessageViewer;
GrantleeHeaderStyleInterface::GrantleeHeaderStyleInterface(MessageViewer::HeaderStylePlugin *plugin, QObject *parent)
    : MessageViewer::HeaderStyleInterface(plugin, parent),
      mThemeManager(Q_NULLPTR)
{

}

GrantleeHeaderStyleInterface::~GrantleeHeaderStyleInterface()
{

}

void GrantleeHeaderStyleInterface::createAction(KActionMenu *menu, QActionGroup *actionGroup, KActionCollection *ac)
{
    mThemeManager = new GrantleeTheme::GrantleeThemeManager(GrantleeTheme::GrantleeThemeManager::Mail, QStringLiteral("header.desktop"), ac, QStringLiteral("messageviewer/themes/"));
    mThemeManager->setDownloadNewStuffConfigFile(QStringLiteral("messageviewer_header_themes.knsrc"));
    connect(mThemeManager, &GrantleeTheme::GrantleeThemeManager::grantleeThemeSelected, this, &GrantleeHeaderStyleInterface::slotGrantleeHeaders);
    connect(mThemeManager, &GrantleeTheme::GrantleeThemeManager::updateThemes, this, &HeaderStyleInterface::styleUpdated);

    mThemeManager->setActionGroup(actionGroup);
    mThemeManager->setThemeMenu(menu);
    const QString themeName = GrantleeTheme::GrantleeSettings::self()->grantleeMailThemeName();
    mHeaderStylePlugin->headerStyle()->setTheme(mThemeManager->theme(themeName));
    addActionToMenu(menu, actionGroup);
}

void GrantleeHeaderStyleInterface::activateAction()
{
    KToggleAction *act = mThemeManager->actionForTheme();
    if (act) {
        act->setChecked(true);
    }
}

void GrantleeHeaderStyleInterface::slotGrantleeHeaders()
{
    const QString themeName = GrantleeTheme::GrantleeSettings::self()->grantleeMailThemeName();
    mHeaderStylePlugin->headerStyle()->setTheme(mThemeManager->theme(themeName));
    slotStyleChanged();
}
