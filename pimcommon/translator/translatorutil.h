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

#include <KLocalizedString>
#include <QMap>
#include <QPair>
#include <QString>

class KComboBox;

namespace PimCommon
{

namespace TranslatorUtil
{
enum translatorType {
    GoogleTranslator = 0
};

static const QPair<const char *, QString> automatic(I18N_NOOP("Detect language"), QStringLiteral("auto"));
static const QPair<const char *, QString> en(I18N_NOOP("English"), QStringLiteral("en"));
static const QPair<const char *, QString> zh(I18N_NOOP("Chinese (Simplified)"), QStringLiteral("zh"));
static const QPair<const char *, QString> zt(I18N_NOOP("Chinese (Traditional)"), QStringLiteral("zt"));
static const QPair<const char *, QString> nl(I18N_NOOP("Dutch"), QStringLiteral("nl"));
static const QPair<const char *, QString> fr(I18N_NOOP("French"), QStringLiteral("fr"));
static const QPair<const char *, QString> de(I18N_NOOP("German"), QStringLiteral("de"));
static const QPair<const char *, QString> el(I18N_NOOP("Greek"), QStringLiteral("el"));
static const QPair<const char *, QString> it(I18N_NOOP("Italian"), QStringLiteral("it"));
static const QPair<const char *, QString> ja(I18N_NOOP("Japanese"), QStringLiteral("ja"));
static const QPair<const char *, QString> ko(I18N_NOOP("Korean"), QStringLiteral("ko"));
static const QPair<const char *, QString> pt(I18N_NOOP("Portuguese"), QStringLiteral("pt"));
static const QPair<const char *, QString> ru(I18N_NOOP("Russian"), QStringLiteral("ru"));
static const QPair<const char *, QString> es(I18N_NOOP("Spanish"), QStringLiteral("es"));

static const QPair<const char *, QString> af(I18N_NOOP("Afrikaans"), QStringLiteral("af"));
static const QPair<const char *, QString> sq(I18N_NOOP("Albanian"), QStringLiteral("sq"));
static const QPair<const char *, QString> ar(I18N_NOOP("Arabic"), QStringLiteral("ar"));
static const QPair<const char *, QString> hy(I18N_NOOP("Armenian"), QStringLiteral("hy"));
static const QPair<const char *, QString> az(I18N_NOOP("Azerbaijani"), QStringLiteral("az"));
static const QPair<const char *, QString> eu(I18N_NOOP("Basque"), QStringLiteral("eu"));
static const QPair<const char *, QString> be(I18N_NOOP("Belarusian"), QStringLiteral("be"));
static const QPair<const char *, QString> bg(I18N_NOOP("Bulgarian"), QStringLiteral("bg"));
static const QPair<const char *, QString> ca(I18N_NOOP("Catalan"), QStringLiteral("ca"));
static const QPair<const char *, QString> zh_cn_google(I18N_NOOP("Chinese (Simplified)"), QStringLiteral("zh-CN")); // For google only
static const QPair<const char *, QString> zh_tw_google(I18N_NOOP("Chinese (Traditional)"), QStringLiteral("zh-TW")); // For google only
static const QPair<const char *, QString> hr(I18N_NOOP("Croatian"), QStringLiteral("hr"));
static const QPair<const char *, QString> cs(I18N_NOOP("Czech"), QStringLiteral("cs"));
static const QPair<const char *, QString> da(I18N_NOOP("Danish"), QStringLiteral("da"));
static const QPair<const char *, QString> et(I18N_NOOP("Estonian"), QStringLiteral("et"));
static const QPair<const char *, QString> tl(I18N_NOOP("Filipino"), QStringLiteral("tl"));
static const QPair<const char *, QString> fi(I18N_NOOP("Finnish"), QStringLiteral("fi"));
static const QPair<const char *, QString> gl(I18N_NOOP("Galician"), QStringLiteral("gl"));
static const QPair<const char *, QString> ka(I18N_NOOP("Georgian"), QStringLiteral("ka"));
static const QPair<const char *, QString> ht(I18N_NOOP("Haitian Creole"), QStringLiteral("ht"));
static const QPair<const char *, QString> iw(I18N_NOOP("Hebrew"), QStringLiteral("iw"));
static const QPair<const char *, QString> hi(I18N_NOOP("Hindi"), QStringLiteral("hi"));
static const QPair<const char *, QString> hu(I18N_NOOP("Hungarian"), QStringLiteral("hu"));
static const QPair<const char *, QString> is(I18N_NOOP("Icelandic"), QStringLiteral("is"));
static const QPair<const char *, QString> id(I18N_NOOP("Indonesian"), QStringLiteral("id"));
static const QPair<const char *, QString> ga(I18N_NOOP("Irish"), QStringLiteral("ga"));
static const QPair<const char *, QString> lv(I18N_NOOP("Latvian"), QStringLiteral("lv"));
static const QPair<const char *, QString> lt(I18N_NOOP("Lithuanian"), QStringLiteral("lt"));
static const QPair<const char *, QString> mk(I18N_NOOP("Macedonian"), QStringLiteral("mk"));
static const QPair<const char *, QString> ms(I18N_NOOP("Malay"), QStringLiteral("ms"));
static const QPair<const char *, QString> mt(I18N_NOOP("Maltese"), QStringLiteral("mt"));
static const QPair<const char *, QString> no(I18N_NOOP("Norwegian"), QStringLiteral("no"));
static const QPair<const char *, QString> fa(I18N_NOOP("Persian"), QStringLiteral("fa"));
static const QPair<const char *, QString> pl(I18N_NOOP("Polish"), QStringLiteral("pl"));
static const QPair<const char *, QString> ro(I18N_NOOP("Romanian"), QStringLiteral("ro"));
static const QPair<const char *, QString> sr(I18N_NOOP("Serbian"), QStringLiteral("sr"));
static const QPair<const char *, QString> sk(I18N_NOOP("Slovak"), QStringLiteral("sk"));
static const QPair<const char *, QString> sl(I18N_NOOP("Slovenian"), QStringLiteral("sl"));
static const QPair<const char *, QString> sw(I18N_NOOP("Swahili"), QStringLiteral("sw"));
static const QPair<const char *, QString> sv(I18N_NOOP("Swedish"), QStringLiteral("sv"));
static const QPair<const char *, QString> th(I18N_NOOP("Thai"), QStringLiteral("th"));
static const QPair<const char *, QString> tr(I18N_NOOP("Turkish"), QStringLiteral("tr"));
static const QPair<const char *, QString> uk(I18N_NOOP("Ukrainian"), QStringLiteral("uk"));
static const QPair<const char *, QString> ur(I18N_NOOP("Urdu"), QStringLiteral("ur"));
static const QPair<const char *, QString> vi(I18N_NOOP("Vietnamese"), QStringLiteral("vi"));
static const QPair<const char *, QString> cy(I18N_NOOP("Welsh"), QStringLiteral("cy"));
static const QPair<const char *, QString> yi(I18N_NOOP("Yiddish"), QStringLiteral("yi"));

void addPairToMap(QMap<QString, QString> &map, const QPair<const char *, QString> &pair);
void addItemToFromComboBox(KComboBox *combo, const QPair<const char *, QString> &pair);
}
}
#endif // TRANSLATORUTIL_H
