/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>
  
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

#include "translatorutil.h"
#include <KComboBox>
using namespace PimCommon;

TranslatorUtil::TranslatorUtil()
{

}

QPair<QString, QString> TranslatorUtil::pair(TranslatorUtil::languages lang)
{
    QPair<QString, QString> ret;
    switch(lang) {
    case automatic:
        ret = QPair<QString, QString>( i18n("Detect language"),QLatin1String("auto"));
        break;
    case en:
        ret = QPair<QString, QString>( i18n("English"), QLatin1String( "en" ) );
        break;
    case zh:
        ret = QPair<QString, QString>( i18n("Chinese (Simplified)"), QLatin1String( "zh" ) );
        break;
    case zt:
        ret = QPair<QString, QString>( i18n("Chinese (Traditional)"), QLatin1String( "zt" ) );
        break;
    case nl:
        ret = QPair<QString, QString>( i18n("Dutch"), QLatin1String( "nl" ) );
        break;
    case fr:
        ret = QPair<QString, QString>( i18n("French"), QLatin1String( "fr" ) );
        break;
    case de:
        ret = QPair<QString, QString>( i18n("German"), QLatin1String( "de" ) );
        break;
    case el:
        ret = QPair<QString, QString>( i18n("Greek"), QLatin1String( "el" ) );
        break;
    case it:
        ret = QPair<QString, QString>( i18n("Italian"), QLatin1String( "it" ) );
        break;
    case ja:
        ret = QPair<QString, QString>( i18n("Japanese"), QLatin1String( "ja" ) );
        break;
    case ko:
        ret = QPair<QString, QString>( i18n("Korean"), QLatin1String( "ko" ) );
        break;
    case pt:
        ret = QPair<QString, QString>( i18n("Portuguese"), QLatin1String( "pt" ) );
        break;
    case ru:
        ret = QPair<QString, QString>( i18n("Russian"), QLatin1String( "ru" ) );
        break;
    case es:
        ret = QPair<QString, QString>( i18n("Spanish"), QLatin1String( "es" ) );
        break;
    case af:
        ret = QPair<QString, QString>( i18n("Afrikaans"), QLatin1String("af") );
        break;
    case sq:
        ret = QPair<QString, QString>( i18n("Albanian"),QLatin1String("sq"));
        break;
    case ar:
        ret = QPair<QString, QString>( i18n("Arabic"), QLatin1String("ar"));
        break;
    case hy:
        ret = QPair<QString, QString>( i18n("Armenian"),QLatin1String("hy"));
        break;
    case az:
        ret = QPair<QString, QString>( i18n("Azerbaijani"),QLatin1String("az"));
        break;
    case eu:
        ret = QPair<QString, QString>( i18n("Basque"),QLatin1String("eu"));
        break;
    case be:
        ret = QPair<QString, QString>( i18n("Belarusian"),QLatin1String("be"));
        break;
    case bg:
        ret = QPair<QString, QString>( i18n("Bulgarian"), QLatin1String("bg") );
        break;
    case ca:
        ret = QPair<QString, QString>( i18n("Catalan"),QLatin1String("ca"));
        break;
    case zh_cn_google: // For google only
        ret = QPair<QString, QString>( i18n("Chinese (Simplified)"),QLatin1String("zh-CN")); // For google only
        break;
    case zh_tw_google: // For google only
        ret = QPair<QString, QString>( i18n("Chinese (Traditional)"),QLatin1String("zh-TW")); // For google only
        break;
    case hr:
        ret = QPair<QString, QString>( i18n("Croatian"),QLatin1String("hr"));
        break;
    case cs:
        ret = QPair<QString, QString>( i18n("Czech"),QLatin1String("cs"));
        break;
    case da:
        ret = QPair<QString, QString>( i18n("Danish"),QLatin1String("da"));
        break;
    case et:
        ret = QPair<QString, QString>( i18n("Estonian"),QLatin1String("et"));
        break;
    case tl:
        ret = QPair<QString, QString>( i18n("Filipino"),QLatin1String("tl"));
        break;
    case fi:
        ret = QPair<QString, QString>( i18n("Finnish"),QLatin1String("fi"));
        break;
    case gl:
        ret = QPair<QString, QString>( i18n("Galician"),QLatin1String("gl"));
        break;
    case ka:
        ret = QPair<QString, QString>( i18n("Georgian"),QLatin1String("ka"));
        break;
    case ht:
        ret = QPair<QString, QString>( i18n("Haitian Creole"),QLatin1String("ht"));
        break;
    case iw:
        ret = QPair<QString, QString>( i18n("Hebrew"),QLatin1String("iw"));
        break;
    case hi:
        ret = QPair<QString, QString>( i18n("Hindi"),QLatin1String("hi"));
        break;
    case hu:
        ret = QPair<QString, QString>( i18n("Hungarian"),QLatin1String("hu"));
        break;
    case is:
        ret = QPair<QString, QString>( i18n("Icelandic"),QLatin1String("is"));
        break;
    case id:
        ret = QPair<QString, QString>( i18n("Indonesian"),QLatin1String("id"));
        break;
    case ga:
        ret = QPair<QString, QString>( i18n("Irish"),QLatin1String("ga"));
        break;
    case lv:
        ret = QPair<QString, QString>( i18n("Latvian"),QLatin1String("lv"));
        break;
    case lt:
        ret = QPair<QString, QString>( i18n("Lithuanian"),QLatin1String("lt"));
        break;
    case mk:
        ret = QPair<QString, QString>( i18n("Macedonian"),QLatin1String("mk"));
        break;
    case ms:
        ret = QPair<QString, QString>( i18n("Malay"),QLatin1String("ms"));
        break;
    case mt:
        ret = QPair<QString, QString>( i18n("Maltese"),QLatin1String("mt"));
        break;
    case no:
        ret = QPair<QString, QString>( i18n("Norwegian"),QLatin1String("no"));
        break;
    case fa:
        ret = QPair<QString, QString>( i18n("Persian"),QLatin1String("fa"));
        break;
    case pl:
        ret = QPair<QString, QString>( i18n("Polish"),QLatin1String("pl"));
        break;
    case ro:
        ret = QPair<QString, QString>( i18n("Romanian"),QLatin1String("ro"));
        break;
    case sr:
        ret = QPair<QString, QString>( i18n("Serbian"),QLatin1String("sr"));
        break;
    case sk:
        ret = QPair<QString, QString>( i18n("Slovak"),QLatin1String("sk"));
        break;
    case sl:
        ret = QPair<QString, QString>( i18n("Slovenian"),QLatin1String("sl"));
        break;
    case sw:
        ret = QPair<QString, QString>( i18n("Swahili"),QLatin1String("sw"));
        break;
    case sv:
        ret = QPair<QString, QString>( i18n("Swedish"),QLatin1String("sv"));
        break;
    case th:
        ret = QPair<QString, QString>( i18n("Thai"),QLatin1String("th"));
        break;
    case tr:
        ret = QPair<QString, QString>( i18n("Turkish"),QLatin1String("tr"));
        break;
    case uk:
        ret = QPair<QString, QString>( i18n("Ukrainian"),QLatin1String("uk"));
        break;
    case ur:
        ret = QPair<QString, QString>( i18n("Urdu"),QLatin1String("ur"));
        break;
    case vi:
        ret = QPair<QString, QString>( i18n("Vietnamese"),QLatin1String("vi"));
        break;
    case cy:
        ret = QPair<QString, QString>( i18n("Welsh"),QLatin1String("cy"));
        break;
    case yi:
        ret = QPair<QString, QString>( i18n("Yiddish"),QLatin1String("yi"));
        break;
    }
    return ret;
}

void PimCommon::TranslatorUtil::addPairToMap( QMap<QString, QString> &map, const QPair<QString, QString> &pair )
{
    map.insert( pair.first, pair.second );
}

void PimCommon::TranslatorUtil::addItemToFromComboBox( KComboBox *combo, const QPair<QString, QString> &pair )
{
    combo->addItem( pair.first, pair.second );
}

