/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "texttospeechlanguagecombobox.h"
using namespace PimCommon;

TextToSpeechLanguageComboBox::TextToSpeechLanguageComboBox(QWidget *parent)
    : QComboBox(parent)
{

}

TextToSpeechLanguageComboBox::~TextToSpeechLanguageComboBox()
{

}

void TextToSpeechLanguageComboBox::selectLocaleName(const QString &localeName)
{
    const int countItem(count());
    for (int i = 0; i < countItem; ++i) {
        if (itemData(i).value<QLocale>().name() == localeName) {
            setCurrentIndex(i);
            break;
        }
    }
}

void TextToSpeechLanguageComboBox::updateAvailableLocales(const QVector<QLocale> &locales, const QLocale &current)
{
    clear();
    Q_FOREACH (const QLocale &locale, locales) {
        QVariant localeVariant(locale);
        addItem(QLocale::languageToString(locale.language()), localeVariant);
        if (locale.name() == current.name()) {
            setCurrentIndex(count() - 1);
        }
    }
}

QSize TextToSpeechLanguageComboBox::minimumSizeHint() const
{
    return QSize(90, QComboBox::minimumSizeHint().height());
}
