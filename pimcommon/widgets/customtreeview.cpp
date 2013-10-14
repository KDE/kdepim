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

#include "customtreeview.h"

#include <KGlobalSettings>

#include <QApplication>
#include <QPainter>

using namespace PimCommon;

CustomTreeView::CustomTreeView(QWidget *parent)
    : QTreeWidget(parent),
      mShowDefaultText(false)
{
    connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()),
             this, SLOT(slotGeneralFontChanged()));
    connect( KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
             this, SLOT(slotGeneralPaletteChanged()));
}

CustomTreeView::~CustomTreeView()
{
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
    mDefaultText = text;
}

void CustomTreeView::slotGeneralPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha( 128 );
    mTextColor = color;
}

void CustomTreeView::slotGeneralFontChanged()
{
    setFont( KGlobalSettings::generalFont() );
}

void CustomTreeView::paintEvent( QPaintEvent *event )
{
    if ( mShowDefaultText && !mDefaultText.isEmpty() ) {
        QPainter p( viewport() );

        QFont font = p.font();
        font.setItalic( true );
        p.setFont( font );

        if (!mTextColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        p.setPen( mTextColor );

        p.drawText( QRect( 0, 0, width(), height() ), Qt::AlignCenter, mDefaultText );
    } else {
        QTreeWidget::paintEvent(event);
    }
}

#include "customtreeview.moc"
