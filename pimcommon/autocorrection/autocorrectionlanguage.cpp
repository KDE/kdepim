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

#include "autocorrectionlanguage.h"
#include <KLocale>
#include <KGlobal>

using namespace PimCommon;

AutoCorrectionLanguage::AutoCorrectionLanguage(QWidget *parent)
    : KComboBox(parent)
{
    KLocale *locale = KGlobal::locale();
    const QStringList lstLang = locale->allLanguagesList();
    Q_FOREACH (const QString& lang, lstLang) {
        if (lang != QLatin1String("x-test")) {
            addItem ( locale->languageCodeToName(lang) , lang );
        }
    }
    const QString defaultLang = locale->languageList().first();
    const int index = findData(defaultLang);
    setCurrentIndex(index);
    model()->sort(0);
}

AutoCorrectionLanguage::~AutoCorrectionLanguage()
{

}

QString AutoCorrectionLanguage::language() const
{
    return itemData(currentIndex()).toString();
}

void AutoCorrectionLanguage::setLanguage(const QString &language)
{
    const int index = findData(language);
    setCurrentIndex(index);
}
