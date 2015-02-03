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

#ifndef TRANSLATORUTIL_H
#define TRANSLATORUTIL_H

#include <KLocalizedString>
#include <QMap>
#include <QPair>
#include <QString>

class KComboBox;

namespace PimCommon
{
class TranslatorUtil
{
public:
    TranslatorUtil();

    void addPairToMap(QMap<QString, QString> &map, const QPair<QString, QString> &pair);
    void addItemToFromComboBox(KComboBox *combo, const QPair<QString, QString> &pair);

    enum translatorType {
        GoogleTranslator = 0
    };

    enum languages {
        automatic,
        en,
        zh,
        zt,
        nl,
        fr,
        de,
        el,
        it,
        ja,
        ko,
        pt,
        ru,
        es,

        af,
        sq,
        ar,
        hy,
        az,
        eu,
        be,
        bg,
        ca,
        zh_cn_google, // For google only
        zh_tw_google, // For google only
        hr,
        cs,
        da,
        et,
        tl,
        fi,
        gl,
        ka,
        ht,
        iw,
        hi,
        hu,
        is,
        id,
        ga,
        lv,
        lt,
        mk,
        ms,
        mt,
        no,
        fa,
        pl,
        ro,
        sr,
        sk,
        sl,
        sw,
        sv,
        th,
        tr,
        uk,
        ur,
        vi,
        cy,
        yi
    };
    QPair<QString, QString> pair(TranslatorUtil::languages lang);
};
}

#endif // TRANSLATORUTIL_H
