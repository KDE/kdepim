/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "customtreeview.h"

#include <QPainter>
#include <QFontDatabase>
#include <QEvent>

using namespace PimCommon;

CustomTreeView::CustomTreeView(QWidget *parent)
    : QTreeWidget(parent),
      mShowDefaultText(true)
{
}

CustomTreeView::~CustomTreeView()
{
}

void CustomTreeView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        generalPaletteChanged();
    } else if (event->type() == QEvent::FontChange) {
        generalFontChanged();
    }
    QTreeWidget::changeEvent(event);
}

void CustomTreeView::setShowDefaultText(bool b)
{
    if (mShowDefaultText != b) {
        mShowDefaultText = b;
        update();
    }
}

bool CustomTreeView::showDefaultText() const
{
    return mShowDefaultText;
}

void CustomTreeView::setDefaultText(const QString &text)
{
    if (mDefaultText != text) {
        mDefaultText = text;
        update();
    }
}

void CustomTreeView::generalPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha(128);
    mTextColor = color;
}

void CustomTreeView::generalFontChanged()
{
    setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
}

void CustomTreeView::paintEvent(QPaintEvent *event)
{
    if (mShowDefaultText && !mDefaultText.isEmpty()) {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        if (!mTextColor.isValid()) {
            generalPaletteChanged();
        }
        p.setPen(mTextColor);

        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, mDefaultText);
    } else {
        QTreeWidget::paintEvent(event);
    }
}

