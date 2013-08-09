/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

namespace PimCommon {

namespace TranslatorUtil {
  enum translatorType {
    GoogleTranslator = 0
#if 0 //Babel fish is dead in may 2012
    ,BabelFishTranslator = 1
#endif
  };


  static const QPair<const char *, QString> automatic( I18N_NOOP("Detect language"),QLatin1String("auto"));
  static const QPair<const char *, QString> en( I18N_NOOP("English"), QLatin1String( "en" ) );
  static const QPair<const char *, QString> zh( I18N_NOOP("Chinese (Simplified)"), QLatin1String( "zh" ) );
  static const QPair<const char *, QString> zt( I18N_NOOP("Chinese (Traditional)"), QLatin1String( "zt" ) );
  static const QPair<const char *, QString> nl( I18N_NOOP("Dutch"), QLatin1String( "nl" ) );
  static const QPair<const char *, QString> fr( I18N_NOOP("French"), QLatin1String( "fr" ) );
  static const QPair<const char *, QString> de( I18N_NOOP("German"), QLatin1String( "de" ) );
  static const QPair<const char *, QString> el( I18N_NOOP("Greek"), QLatin1String( "el" ) );
  static const QPair<const char *, QString> it( I18N_NOOP("Italian"), QLatin1String( "it" ) );
  static const QPair<const char *, QString> ja( I18N_NOOP("Japanese"), QLatin1String( "ja" ) );
  static const QPair<const char *, QString> ko( I18N_NOOP("Korean"), QLatin1String( "ko" ) );
  static const QPair<const char *, QString> pt( I18N_NOOP("Portuguese"), QLatin1String( "pt" ) );
  static const QPair<const char *, QString> ru( I18N_NOOP("Russian"), QLatin1String( "ru" ) );
  static const QPair<const char *, QString> es( I18N_NOOP("Spanish"), QLatin1String( "es" ) );

  static const QPair<const char *, QString> af(I18N_NOOP("Afrikaans"), QLatin1String("af") );
  static const QPair<const char *, QString> sq(I18N_NOOP("Albanian"),QLatin1String("sq"));
  static const QPair<const char *, QString> ar(I18N_NOOP("Arabic"), QLatin1String("ar"));
  static const QPair<const char *, QString> hy(I18N_NOOP("Armenian"),QLatin1String("hy"));
  static const QPair<const char *, QString> az(I18N_NOOP("Azerbaijani"),QLatin1String("az"));
  static const QPair<const char *, QString> eu(I18N_NOOP("Basque"),QLatin1String("eu"));
  static const QPair<const char *, QString> be(I18N_NOOP("Belarusian"),QLatin1String("be"));
  static const QPair<const char *, QString> bg(I18N_NOOP("Bulgarian"),QLatin1String("bg"));
  static const QPair<const char *, QString> ca(I18N_NOOP("Catalan"),QLatin1String("ca"));
  static const QPair<const char *, QString> zh_cn_google(I18N_NOOP("Chinese (Simplified)"),QLatin1String("zh-CN")); // For google only
  static const QPair<const char *, QString> zh_tw_google(I18N_NOOP("Chinese (Traditional)"),QLatin1String("zh-TW")); // For google only
  static const QPair<const char *, QString> hr(I18N_NOOP("Croatian"),QLatin1String("hr"));
  static const QPair<const char *, QString> cs(I18N_NOOP("Czech"),QLatin1String("cs"));
  static const QPair<const char *, QString> da(I18N_NOOP("Danish"),QLatin1String("da"));
  static const QPair<const char *, QString> et(I18N_NOOP("Estonian"),QLatin1String("et"));
  static const QPair<const char *, QString> tl(I18N_NOOP("Filipino"),QLatin1String("tl"));
  static const QPair<const char *, QString> fi(I18N_NOOP("Finnish"),QLatin1String("fi"));
  static const QPair<const char *, QString> gl(I18N_NOOP("Galician"),QLatin1String("gl"));
  static const QPair<const char *, QString> ka(I18N_NOOP("Georgian"),QLatin1String("ka"));
  static const QPair<const char *, QString> ht(I18N_NOOP("Haitian Creole"),QLatin1String("ht"));
  static const QPair<const char *, QString> iw(I18N_NOOP("Hebrew"),QLatin1String("iw"));
  static const QPair<const char *, QString> hi(I18N_NOOP("Hindi"),QLatin1String("hi"));
  static const QPair<const char *, QString> hu(I18N_NOOP("Hungarian"),QLatin1String("hu"));
  static const QPair<const char *, QString> is(I18N_NOOP("Icelandic"),QLatin1String("is"));
  static const QPair<const char *, QString> id(I18N_NOOP("Indonesian"),QLatin1String("id"));
  static const QPair<const char *, QString> ga(I18N_NOOP("Irish"),QLatin1String("ga"));
  static const QPair<const char *, QString> lv(I18N_NOOP("Latvian"),QLatin1String("lv"));
  static const QPair<const char *, QString> lt(I18N_NOOP("Lithuanian"),QLatin1String("lt"));
  static const QPair<const char *, QString> mk(I18N_NOOP("Macedonian"),QLatin1String("mk"));
  static const QPair<const char *, QString> ms(I18N_NOOP("Malay"),QLatin1String("ms"));
  static const QPair<const char *, QString> mt(I18N_NOOP("Maltese"),QLatin1String("mt"));
  static const QPair<const char *, QString> no(I18N_NOOP("Norwegian"),QLatin1String("no"));
  static const QPair<const char *, QString> fa(I18N_NOOP("Persian"),QLatin1String("fa"));
  static const QPair<const char *, QString> pl(I18N_NOOP("Polish"),QLatin1String("pl"));
  static const QPair<const char *, QString> ro(I18N_NOOP("Romanian"),QLatin1String("ro"));
  static const QPair<const char *, QString> sr(I18N_NOOP("Serbian"),QLatin1String("sr"));
  static const QPair<const char *, QString> sk(I18N_NOOP("Slovak"),QLatin1String("sk"));
  static const QPair<const char *, QString> sl(I18N_NOOP("Slovenian"),QLatin1String("sl"));
  static const QPair<const char *, QString> sw(I18N_NOOP("Swahili"),QLatin1String("sw"));
  static const QPair<const char *, QString> sv(I18N_NOOP("Swedish"),QLatin1String("sv"));
  static const QPair<const char *, QString> th(I18N_NOOP("Thai"),QLatin1String("th"));
  static const QPair<const char *, QString> tr(I18N_NOOP("Turkish"),QLatin1String("tr"));
  static const QPair<const char *, QString> uk(I18N_NOOP("Ukrainian"),QLatin1String("uk"));
  static const QPair<const char *, QString> ur(I18N_NOOP("Urdu"),QLatin1String("ur"));
  static const QPair<const char *, QString> vi(I18N_NOOP("Vietnamese"),QLatin1String("vi"));
  static const QPair<const char *, QString> cy(I18N_NOOP("Welsh"),QLatin1String("cy"));
  static const QPair<const char *, QString> yi(I18N_NOOP("Yiddish"),QLatin1String("yi"));


  void addPairToMap( QMap<QString, QString> &map, const QPair<const char *, QString> &pair );
  void addItemToFromComboBox(KComboBox *combo, const QPair<const char *, QString> &pair );
}
}
#endif // TRANSLATORUTIL_H
