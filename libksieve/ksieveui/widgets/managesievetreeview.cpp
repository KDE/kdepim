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

#include "managesievetreeview.h"

#include <KGlobalSettings>
#include <KLocale>

#include <QApplication>
#include <QDebug>
#include <QPainter>

using namespace KSieveUi;

ManageSieveTreeView::ManageSieveTreeView(QWidget *parent)
    : QTreeWidget(parent),
      mImapFound(true)
{
    connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()),
             this, SLOT(slotGeneralFontChanged()));
    connect( KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
             this, SLOT(slotGeneralPaletteChanged()));
}

ManageSieveTreeView::~ManageSieveTreeView()
{
}

void ManageSieveTreeView::slotGeneralPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha( 128 );
    mTextColor = color;
}

void ManageSieveTreeView::slotGeneralFontChanged()
{
    setFont( KGlobalSettings::generalFont() );
}

void ManageSieveTreeView::setImapFound(bool found)
{
    if (mImapFound != found) {
        mImapFound = found;
        update();
    }
}

void ManageSieveTreeView::paintEvent( QPaintEvent *event )
{
    if ( mImapFound ) {
        QTreeWidget::paintEvent(event);
    } else {
        QPainter p( viewport() );

        QFont font = p.font();
        font.setItalic( true );
        p.setFont( font );

        if (!mTextColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        p.setPen( mTextColor );

        p.drawText( QRect( 0, 0, width(), height() ), Qt::AlignCenter, i18n( "No imap server configured..." ) );
    }
}

#include "managesievetreeview.moc"
