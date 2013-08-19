/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "favoritecollectionwidget.h"
#include "kernel/mailkernel.h"

#include <messagecore/settings/globalsettings.h>

#include <KDE/KGlobalSettings>
#include <KDE/KLocale>
#include <KDE/KXMLGUIClient>

#include <QPainter>

using namespace MailCommon;

class FavoriteCollectionWidget::Private
{
public:
  Private() {
  }
  QColor textColor;
};

FavoriteCollectionWidget::FavoriteCollectionWidget( KXMLGUIClient *xmlGuiClient, QWidget *parent )
  : Akonadi::EntityListView( xmlGuiClient, parent ), d( new Private )
{
  setFocusPolicy( Qt::NoFocus );

  connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()),
           this, SLOT(slotGeneralFontChanged()));
  connect( KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
           this, SLOT(slotGeneralPaletteChanged()));
  readConfig();
}

FavoriteCollectionWidget::~FavoriteCollectionWidget()
{
  delete d;
}

void FavoriteCollectionWidget::slotGeneralPaletteChanged()
{
  const QPalette palette = viewport()->palette();
  QColor color = palette.text().color();
  color.setAlpha( 128 );
  d->textColor = color;
}

void FavoriteCollectionWidget::slotGeneralFontChanged()
{
  // Custom/System font support
  if ( MessageCore::GlobalSettings::self()->useDefaultFonts() ) {
    setFont( KGlobalSettings::generalFont() );
  }
}

void FavoriteCollectionWidget::readConfig()
{
  // Custom/System font support
  if (!MessageCore::GlobalSettings::self()->useDefaultFonts() ) {
    KConfigGroup fontConfig( KernelIf->config(), "Fonts" );
    setFont( fontConfig.readEntry( "folder-font", KGlobalSettings::generalFont() ) );
  } else {
    setFont( KGlobalSettings::generalFont() );
  }
}

void FavoriteCollectionWidget::paintEvent( QPaintEvent *event )
{
  if ( !model() || model()->rowCount() == 0 ) {
    QPainter p( viewport() );

    QFont font = p.font();
    font.setItalic( true );
    p.setFont( font );

    if (!d->textColor.isValid()) {
        slotGeneralPaletteChanged();
    }
    p.setPen( d->textColor );

    p.drawText( QRect( 0, 0, width(), height() ), Qt::AlignCenter, i18n( "Drop your favorite folders here..." ) );
  } else {
    Akonadi::EntityListView::paintEvent( event );
  }
}

#include "favoritecollectionwidget.moc"
