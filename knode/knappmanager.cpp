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
  updateVisualDefaults();

  KConfig *c=KGlobal::config();
  c->setGroup("VISUAL_APPEARANCE");

  l_ongGroupList = c->readBoolEntry("longGroupList", true);

  u_seColors = c->readBoolEntry("customColors", false);

  colors.append(c->readColorEntry("backgroundColor",&defaultColors[0]));
  colorNames.append(i18n("Background"));

  colors.append(c->readColorEntry("headerColor",&defaultColors[1]));
  colorNames.append(i18n("Header Decoration"));

  colors.append(c->readColorEntry("textColor",&defaultColors[2]));
  colorNames.append(i18n("Normal Text"));

  colors.append(c->readColorEntry("quote1Color",&defaultColors[3]));
  colorNames.append(i18n("Quoted Text - First level"));

  colors.append(c->readColorEntry("quote2Color",&defaultColors[4]));
  colorNames.append(i18n("Quoted Text - Second level"));

  colors.append(c->readColorEntry("quote3Color",&defaultColors[5]));
  colorNames.append(i18n("Quoted Text - Third level"));

  colors.append(c->readColorEntry("URLColor",&defaultColors[6]));
  colorNames.append(i18n("Link"));

  colors.append(c->readColorEntry("readArticleColor",&defaultColors[7]));
  colorNames.append(i18n("Read Article"));

  u_seFonts = c->readBoolEntry("customFonts", false);

  fonts.append(c->readFontEntry("articleFont",&defaultFonts[0]));
  fontNames.append(i18n("Article Body"));

  fonts.append(c->readFontEntry("composerFont",&defaultFonts[1]));
  fontNames.append(i18n("Composer"));

  fonts.append(c->readFontEntry("groupListFont",&defaultFonts[2]));
  fontNames.append(i18n("Group List"));

  fonts.append(c->readFontEntry("articleListFont",&defaultFonts[3]));
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


void KNAppManager::updateVisualDefaults()
{
  defaultColors.clear();
  QColor defCol = kapp->palette().active().base();  // backgroundColor
  defaultColors.append(defCol);
  defCol = kapp->palette().active().background();   // headerColor
  defaultColors.append(defCol);
  defCol = kapp->palette().active().text();         // textColor
  defaultColors.append(defCol);
  defCol = kapp->palette().active().text();         // quote1Color
  defaultColors.append(defCol);
  defCol = kapp->palette().active().text();         // quote2Color
  defaultColors.append(defCol);
  defCol = kapp->palette().active().text();         // quote3Color
  defaultColors.append(defCol);
  defCol = KGlobalSettings::linkColor();            // URLColor
  defaultColors.append(defCol);
  defCol = kapp->palette().disabled().text();       // readArticleColor
  defaultColors.append(defCol);

  QFont defFont = KGlobalSettings::generalFont();
  defaultFonts.clear();
  for (int i=0;i<4;i++)
    defaultFonts.append(defFont);
}

