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

#include "acllistview.h"
#include <KLocalizedString>
#include <QPainter>

using namespace PimCommon;

AclListView::AclListView(QWidget *parent)
    : QListView(parent),
      mCanBeAdministrated(true)
{

}

AclListView::~AclListView()
{

}

void AclListView::slotCollectionCanBeAdministrated(bool b)
{
    if (mCanBeAdministrated != b) {
        mCanBeAdministrated = b;
        update();
    }
}

void AclListView::generalPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha(128);
    mTextColor = color;
}

void AclListView::paintEvent(QPaintEvent *event)
{
    if (!mCanBeAdministrated) {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        if (!mTextColor.isValid()) {
            generalPaletteChanged();
        }
        p.setPen(mTextColor);

        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, i18n("Folder cannot be administrated."));
    } else {
        QListView::paintEvent(event);
    }
}

