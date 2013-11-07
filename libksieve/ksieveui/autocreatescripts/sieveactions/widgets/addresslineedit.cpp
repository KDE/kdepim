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
    : KLineEdit(parent),
      mIncorrectEmail(false)
{
    setClearButtonShown(true);
    connect(this, SIGNAL(textChanged(QString)),SLOT(slotTextChanged()));
}

AddressLineEdit::~AddressLineEdit()
{

}

void AddressLineEdit::slotTextChanged()
{
    if (mIncorrectEmail) {
        verifyAddress();
    }
}

void AddressLineEdit::verifyAddress()
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;
    if (!text().isEmpty()) {
        mIncorrectEmail = !text().contains(QLatin1Char('@'));
        //TODO improve check
        if (mNegativeBackground.isEmpty()) {
            KStatefulBrush bgBrush = KStatefulBrush(KColorScheme::View, KColorScheme::NegativeText);
            mNegativeBackground = QString::fromLatin1("QLineEdit{ color:%1 }").arg(bgBrush.brush(this).color().name());
        }
        if (mIncorrectEmail)
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

