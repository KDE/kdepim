/***************************************************************************
  knappmanager.h  -  stores & handles various appearance related settings
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


#ifndef KNAPPMANAGER_H
#define KNAPPMANAGER_H

#include <qcolor.h>
#include <qfont.h>
#include <qstringlist.h>

class KNUserEntry;


class KNAppManager {

  public:
    enum ColorCode { background=0, header=1, normalText=2, Quoted1=3, Quoted2=4,
                     Quoted3=5, url=6, readArticle=7 };

    enum FontCode  { article=0, composer=1, groupList=2, articleList=3 };

    KNAppManager();
    ~KNAppManager();
    
    void readOptions();
    void saveOptions();

    void updateVisualDefaults();

    bool longGroupList()          { return l_ongGroupList; }
    void setLongGroupList(bool b) { l_ongGroupList = b; }

    bool useColors()             { return u_seColors; }
    void setUseColors(bool b)    { u_seColors = b; }
    int colorCount()             { return colors.count(); }
    QColor& color(int code)      { return colors[code]; }
    QColor& defaultColor(int code) { return defaultColors[code]; }
    QString& colorName(int code) { return colorNames[code]; }

    bool useFonts()             { return u_seFonts; }
    void setUseFonts(bool b)    { u_seFonts = b; }
    int fontCount()             { return fonts.count(); }
    QFont& defaultFont(int code) { return defaultFonts[code]; }
    QFont& font(int code)       { return fonts[code]; }
    QString& fontName(int code) { return fontNames[code]; }

    // ok, this has nothing to do with appearance, but we need to
    // convert the whole configuration management anyway...
    KNUserEntry* defaultUser()    { return d_efaultUser; }

  protected:

    bool l_ongGroupList, u_seColors, u_seFonts;
    QValueList<QColor> colors, defaultColors;
    QValueList<QFont> fonts, defaultFonts;
    QStringList colorNames, fontNames;

    KNUserEntry *d_efaultUser;

};

#endif
