/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#ifndef TRANSLATORUTIL_H
#define TRANSLATORUTIL_H

#include <KLocale>
#include <QMap>
#include <QPair>
#include <QString>

class KComboBox;

namespace MessageViewer {

namespace TranslatorUtil {
  enum translatorType {
    GoogleTranslator = 0
#if 0 //Babel fish is dead in may 2012
    ,BabelFishTranslator = 1
#endif
  };


  static const QPair<QString, QString> automatic( i18n("Detect language"),QLatin1String("auto"));
  static const QPair<QString, QString> en( i18n("English"), QLatin1String( "en" ) );
  static const QPair<QString, QString> zh( i18n("Chinese (Simplified)"), QLatin1String( "zh" ) );
  static const QPair<QString, QString> zt( i18n("Chinese (Traditional)"), QLatin1String( "zt" ) );
  static const QPair<QString, QString> nl( i18n("Dutch"), QLatin1String( "nl" ) );
  static const QPair<QString, QString> fr( i18n("French"), QLatin1String( "fr" ) );
  static const QPair<QString, QString> de( i18n("German"), QLatin1String( "de" ) );
  static const QPair<QString, QString> el( i18n("Greek"), QLatin1String( "el" ) );
  static const QPair<QString, QString> it( i18n("Italian"), QLatin1String( "it" ) );
  static const QPair<QString, QString> ja( i18n("Japanese"), QLatin1String( "ja" ) );
  static const QPair<QString, QString> ko( i18n("Korean"), QLatin1String( "ko" ) );
  static const QPair<QString, QString> pt( i18n("Portuguese"), QLatin1String( "pt" ) );
  static const QPair<QString, QString> ru( i18n("Russian"), QLatin1String( "ru" ) );
  static const QPair<QString, QString> es( i18n("Spanish"), QLatin1String( "es" ) );

  static const QPair<QString, QString> af(i18n("Afrikaans"), QLatin1String("af") );
  static const QPair<QString, QString> sq(i18n("Albanian"),QLatin1String("sq"));
  static const QPair<QString, QString> ar(i18n("Arabic"), QLatin1String("ar"));
  static const QPair<QString, QString> hy(i18n("Armenian"),QLatin1String("hy"));
  static const QPair<QString, QString> az(i18n("Azerbaijani"),QLatin1String("az"));
  static const QPair<QString, QString> eu(i18n("Basque"),QLatin1String("eu"));
  static const QPair<QString, QString> be(i18n("Belarusian"),QLatin1String("be"));
  static const QPair<QString, QString> bg(i18n("Bulgarian"),QLatin1String("bg"));
  static const QPair<QString, QString> ca(i18n("Catalan"),QLatin1String("ca"));
  static const QPair<QString, QString> zh_cn_google(i18n("Chinese (Simplified)"),QLatin1String("zh-CN")); // For google only
  static const QPair<QString, QString> zh_tw_google(i18n("Chinese (Traditional)"),QLatin1String("zh-TW")); // For google only
  static const QPair<QString, QString> hr(i18n("Croatian"),QLatin1String("hr"));
  static const QPair<QString, QString> cs(i18n("Czech"),QLatin1String("cs"));
  static const QPair<QString, QString> da(i18n("Danish"),QLatin1String("da"));
  static const QPair<QString, QString> et(i18n("Estonian"),QLatin1String("et"));
  static const QPair<QString, QString> tl(i18n("Filipino"),QLatin1String("tl"));
  static const QPair<QString, QString> fi(i18n("Finnish"),QLatin1String("fi"));
  static const QPair<QString, QString> gl(i18n("Galician"),QLatin1String("gl"));
  static const QPair<QString, QString> ka(i18n("Georgian"),QLatin1String("ka"));
  static const QPair<QString, QString> ht(i18n("Haitian Creole"),QLatin1String("ht"));
  static const QPair<QString, QString> iw(i18n("Hebrew"),QLatin1String("iw"));
  static const QPair<QString, QString> hi(i18n("Hindi"),QLatin1String("hi"));
  static const QPair<QString, QString> hu(i18n("Hungarian"),QLatin1String("hu"));
  static const QPair<QString, QString> is(i18n("Icelandic"),QLatin1String("is"));
  static const QPair<QString, QString> id(i18n("Indonesian"),QLatin1String("id"));
  static const QPair<QString, QString> ga(i18n("Irish"),QLatin1String("ga"));
  static const QPair<QString, QString> lv(i18n("Latvian"),QLatin1String("lv"));
  static const QPair<QString, QString> lt(i18n("Lithuanian"),QLatin1String("lt"));
  static const QPair<QString, QString> mk(i18n("Macedonian"),QLatin1String("mk"));
  static const QPair<QString, QString> ms(i18n("Malay"),QLatin1String("ms"));
  static const QPair<QString, QString> mt(i18n("Maltese"),QLatin1String("mt"));
  static const QPair<QString, QString> no(i18n("Norwegian"),QLatin1String("no"));
  static const QPair<QString, QString> fa(i18n("Persian"),QLatin1String("fa"));
  static const QPair<QString, QString> pl(i18n("Polish"),QLatin1String("pl"));
  static const QPair<QString, QString> ro(i18n("Romanian"),QLatin1String("ro"));
  static const QPair<QString, QString> sr(i18n("Serbian"),QLatin1String("sr"));
  static const QPair<QString, QString> sk(i18n("Slovak"),QLatin1String("sk"));
  static const QPair<QString, QString> sl(i18n("Slovenian"),QLatin1String("sl"));
  static const QPair<QString, QString> sw(i18n("Swahili"),QLatin1String("sw"));
  static const QPair<QString, QString> sv(i18n("Swedish"),QLatin1String("sv"));
  static const QPair<QString, QString> th(i18n("Thai"),QLatin1String("th"));
  static const QPair<QString, QString> tr(i18n("Turkish"),QLatin1String("tr"));
  static const QPair<QString, QString> uk(i18n("Ukrainian"),QLatin1String("uk"));
  static const QPair<QString, QString> ur(i18n("Urdu"),QLatin1String("ur"));
  static const QPair<QString, QString> vi(i18n("Vietnamese"),QLatin1String("vi"));
  static const QPair<QString, QString> cy(i18n("Welsh"),QLatin1String("cy"));
  static const QPair<QString, QString> yi(i18n("Yiddish"),QLatin1String("yi"));


  void addPairToMap( QMap<QString, QString>& map, const QPair<QString, QString>& pair );
  void addItemToFromComboBox( KComboBox *combo, const QPair<QString, QString>& pair );
}
}
#endif // TRANSLATORUTIL_H
