/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "adblocklistwidget.h"

#include <KLocalizedString>
#include <KGlobalSettings>

#include <QPainter>
#include <QPaintEvent>

using namespace MessageViewer;
AdBlockListWidget::AdBlockListWidget(QWidget *parent)
    : QListWidget(parent)
{
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
            this, SLOT(slotGeneralPaletteChanged()));
}

AdBlockListWidget::~AdBlockListWidget()
{

}

void AdBlockListWidget::slotGeneralPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha(128);
    mTextColor = color;
}

void AdBlockListWidget::paintEvent(QPaintEvent *event)
{
    if (!model() || model()->rowCount() == 0) {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        if (!mTextColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        p.setPen(mTextColor);

        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, i18n("Not subscription added..."));
    } else {
        QListWidget::paintEvent(event);
    }
}

