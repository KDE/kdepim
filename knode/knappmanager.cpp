/***************************************************************************
  knappmanager.cpp -  stores & handles various appearance related settings
                             -------------------

    copyright            : (C) 2000 by Christian Gebauer
    email                : gebauer@bigfoot.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <klocale.h>
#include <kapp.h>

#include "knappmanager.h"



KNAppManager::KNAppManager()
{
  readOptions();
}


KNAppManager::~KNAppManager()
{
}


void KNAppManager::readOptions()
{
  KConfig *c=KGlobal::config();
  c->setGroup("VISUAL_APPEARANCE");

  l_ongGroupList = c->readBoolEntry("longGroupList", true);

  u_seColors = c->readBoolEntry("customColors", false);

  QColor defCol = QColor(kapp->palette().normal().base());
  colors.append(c->readColorEntry("backgroundColor",&defCol));
  colorNames.append(i18n("Background"));

  defCol = QColor(kapp->palette().normal().midlight());
  colors.append(c->readColorEntry("headerColor",&defCol));
  colorNames.append(i18n("Header Decoration"));

  defCol = QColor(kapp->palette().normal().text());
  colors.append(c->readColorEntry("textColor",&defCol));
  colorNames.append(i18n("Normal Text"));

  defCol = QColor(kapp->palette().normal().text());
  colors.append(c->readColorEntry("quote1Color",&defCol));
  colorNames.append(i18n("Quoted Text - First level"));

  defCol = QColor(kapp->palette().normal().text());
  colors.append(c->readColorEntry("quote2Color",&defCol));
  colorNames.append(i18n("Quoted Text - Second level"));

  defCol = QColor(kapp->palette().normal().text());
  colors.append(c->readColorEntry("quote3Color",&defCol));
  colorNames.append(i18n("Quoted Text - Third level"));

  defCol = KGlobalSettings::linkColor();
  colors.append(c->readColorEntry("URLColor",&defCol));
  colorNames.append(i18n("Link"));

  defCol = KGlobalSettings::visitedLinkColor();
  colors.append(c->readColorEntry("followedURLColor",&defCol));
  colorNames.append(i18n("Followed Link"));

  defCol = QColor("red");
  colors.append(c->readColorEntry("newArticleColor",&defCol));
  colorNames.append(i18n("New Article"));

  defCol = QColor("blue");
  colors.append(c->readColorEntry("unreadArticleColor",&defCol));
  colorNames.append(i18n("Unread Article"));

  u_seFonts = c->readBoolEntry("customFonts", false);

  QFont defFont = KGlobalSettings::generalFont();
  fonts.append(c->readFontEntry("articleFont",&defFont));
  fontNames.append(i18n("Article Body"));

  fonts.append(c->readFontEntry("composerFont",&defFont));
  fontNames.append(i18n("Composer"));

  fonts.append(c->readFontEntry("groupListFont",&defFont));
  fontNames.append(i18n("Group List"));

  fonts.append(c->readFontEntry("articleListFont",&defFont));
  fontNames.append(i18n("Article List"));
}


void KNAppManager::saveOptions()
{
  KConfig *c=KGlobal::config();
  c->setGroup("VISUAL_APPEARANCE");

  c->writeEntry("longGroupList", l_ongGroupList);

  c->writeEntry("customColors", u_seColors);
  c->writeEntry("backgroundColor", colors[background]);
  c->writeEntry("headerColor", colors[header]);
  c->writeEntry("textColor", colors[normalText]);
  c->writeEntry("quote1Color", colors[Quoted1]);
  c->writeEntry("quote2Color", colors[Quoted2]);
  c->writeEntry("quote3Color", colors[Quoted3]);
  c->writeEntry("URLColor", colors[url]);
  c->writeEntry("followedURLColor", colors[followedUrl]);
  c->writeEntry("newArticleColor", colors[newArticle]);
  c->writeEntry("unreadArticleColor", colors[unreadArticle]);

  c->writeEntry("customFonts", u_seFonts);
  c->writeEntry("articleFont", fonts[article]);
  c->writeEntry("composerFont", fonts[composer]);
  c->writeEntry("groupListFont", fonts[groupList]);
  c->writeEntry("articleListFont", fonts[articleList]);
}



