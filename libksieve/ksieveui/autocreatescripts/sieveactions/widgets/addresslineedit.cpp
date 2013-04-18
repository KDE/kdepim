/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "addresslineedit.h"

#include <KColorScheme>

using namespace KSieveUi;

AddressLineEdit::AddressLineEdit(QWidget *parent)
    : KLineEdit(parent)
{
}

AddressLineEdit::~AddressLineEdit()
{

}

void AddressLineEdit::verifyAddress()
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;
    if (!text().isEmpty()) {
        const bool isAnEmailAddress = text().contains(QLatin1Char('@'));
        //TODO improve check
        if (mNegativeBackground.isEmpty()) {
            KStatefulBrush bgBrush(KColorScheme::View, KColorScheme::PositiveBackground);
            mPositiveBackground = QString::fromLatin1("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(this).color().name());
            bgBrush = KStatefulBrush(KColorScheme::View, KColorScheme::NegativeBackground);
            mNegativeBackground = QString::fromLatin1("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(this).color().name());
        }
        if (isAnEmailAddress)
            styleSheet = mPositiveBackground;
        else
            styleSheet = mNegativeBackground;
    }
    setStyleSheet(styleSheet);
#endif
}

void AddressLineEdit::focusOutEvent(QFocusEvent *ev)
{
    verifyAddress();
    KLineEdit::focusOutEvent(ev);
}

#include "addresslineedit.moc"
