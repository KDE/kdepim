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

#include "knuserentry.h"
#include "knappmanager.h"



KNAppManager::KNAppManager()
{
  d_efaultUser=new KNUserEntry();
  readOptions();
}


KNAppManager::~KNAppManager()
{
  delete d_efaultUser;
}


void KNAppManager::readOptions()
{
  KConfig *c=KGlobal::config();
  c->setGroup("VISUAL_APPEARANCE");

  l_ongGroupList = c->readBoolEntry("longGroupList", true);

  u_seColors = c->readBoolEntry("customColors", false);

  QColor defCol = kapp->palette().active().base();
  colors.append(c->readColorEntry("backgroundColor",&defCol));
  colorNames.append(i18n("Background"));

  defCol = kapp->palette().active().background();
  colors.append(c->readColorEntry("headerColor",&defCol));
  colorNames.append(i18n("Header Decoration"));

  defCol = kapp->palette().active().text();
  colors.append(c->readColorEntry("textColor",&defCol));
  colorNames.append(i18n("Normal Text"));

  defCol = kapp->palette().active().text();
  colors.append(c->readColorEntry("quote1Color",&defCol));
  colorNames.append(i18n("Quoted Text - First level"));

  defCol = kapp->palette().active().text();
  colors.append(c->readColorEntry("quote2Color",&defCol));
  colorNames.append(i18n("Quoted Text - Second level"));

  defCol = kapp->palette().active().text();
  colors.append(c->readColorEntry("quote3Color",&defCol));
  colorNames.append(i18n("Quoted Text - Third level"));

  defCol = KGlobalSettings::linkColor();
  colors.append(c->readColorEntry("URLColor",&defCol));
  colorNames.append(i18n("Link"));

  defCol = kapp->palette().disabled().text();
  colors.append(c->readColorEntry("readArticleColor",&defCol));
  colorNames.append(i18n("Read Article"));

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

  c->setGroup("IDENTITY");
  d_efaultUser->load(c);
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
  c->writeEntry("readArticleColor", colors[readArticle]);

  c->writeEntry("customFonts", u_seFonts);
  c->writeEntry("articleFont", fonts[article]);
  c->writeEntry("composerFont", fonts[composer]);
  c->writeEntry("groupListFont", fonts[groupList]);
  c->writeEntry("articleListFont", fonts[articleList]);

  c->setGroup("IDENTITY");
  d_efaultUser->save(c);
}



