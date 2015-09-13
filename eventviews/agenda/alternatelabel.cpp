/*
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "alternatelabel.h"

using namespace EventViews;

AlternateLabel::AlternateLabel(const QString &shortlabel, const QString &longlabel,
                               const QString &extensivelabel, QWidget *parent)
    : QLabel(parent), mTextTypeFixed(false), mShortText(shortlabel),
      mLongText(longlabel), mExtensiveText(extensivelabel)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    if (mExtensiveText.isEmpty()) {
        mExtensiveText = mLongText;
    }
    squeezeTextToLabel();
}

AlternateLabel::~AlternateLabel()
{
}

void AlternateLabel::useShortText()
{
    mTextTypeFixed = true;
    QLabel::setText(mShortText);
    setToolTip(mExtensiveText);
}

void AlternateLabel::useLongText()
{
    mTextTypeFixed = true;
    QLabel::setText(mLongText);
    this->setToolTip(mExtensiveText);
}

void AlternateLabel::useExtensiveText()
{
    mTextTypeFixed = true;
    QLabel::setText(mExtensiveText);
    this->setToolTip(QString());
}

void AlternateLabel::useDefaultText()
{
    mTextTypeFixed = false;
    squeezeTextToLabel();
}

void AlternateLabel::squeezeTextToLabel()
{
    if (mTextTypeFixed) {
        return;
    }

    QFontMetrics fm(fontMetrics());
    int labelWidth = size().width();
    int textWidth = fm.width(mLongText);
    int longTextWidth = fm.width(mExtensiveText);
    if (longTextWidth <= labelWidth) {
        QLabel::setText(mExtensiveText);
        this->setToolTip(QString());
    } else if (textWidth <= labelWidth) {
        QLabel::setText(mLongText);
        this->setToolTip(mExtensiveText);
    } else {
        QLabel::setText(mShortText);
        this->setToolTip(mExtensiveText);
    }
}

void AlternateLabel::resizeEvent(QResizeEvent *)
{
    squeezeTextToLabel();
}

QSize AlternateLabel::minimumSizeHint() const
{
    QSize sh = QLabel::minimumSizeHint();
    sh.setWidth(-1);
    return sh;
}

AlternateLabel::TextType AlternateLabel::largestFittingTextType() const
{
    QFontMetrics fm(fontMetrics());
    const int labelWidth = size().width();
    const int longTextWidth = fm.width(mLongText);
    const int extensiveTextWidth = fm.width(mExtensiveText);
    if (extensiveTextWidth <= labelWidth) {
        return Extensive;
    } else if (longTextWidth <= labelWidth) {
        return Long;
    } else {
        return Short;
    }
}

void AlternateLabel::setFixedType(TextType type)
{
    switch (type) {
    case Extensive:
        useExtensiveText();
        break;
    case Long:
        useLongText();
        break;
    case Short:
        useShortText();
        break;
    }
}

