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
#include <KActionMenu>
#include <KActionCollection>

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

  createMenu(xmlGuiClient->actionCollection());
}

FavoriteCollectionWidget::~FavoriteCollectionWidget()
{
  delete d;
}

void FavoriteCollectionWidget::createMenu(KActionCollection *ac)
{
    KActionMenu *iconSizeMenu  = new KActionMenu(i18n("Icon size"), this);
    ac->addAction(QLatin1String("favorite_icon_size"), iconSizeMenu);

    static int icon_sizes[] = { 16, 22, 32 /*, 48, 64, 128 */ };

    QActionGroup *grp = new QActionGroup( iconSizeMenu );
    const int nbElement( (int)( sizeof( icon_sizes ) / sizeof( int ) ) );
    QAction *act = 0;
    for ( int i = 0; i < nbElement; ++i ) {
      act = new QAction(QString::fromLatin1( "%1x%2" ).arg( icon_sizes[ i ] ).arg( icon_sizes[ i ] ), iconSizeMenu);
      iconSizeMenu->addAction( act );
      act->setCheckable( true );
      grp->addAction( act );
      if ( iconSize().width() == icon_sizes[ i ] ) {
        act->setChecked( true );
      }
      act->setData( QVariant( icon_sizes[ i ] ) );
      connect( act, SIGNAL(triggered(bool)),
               SLOT(slotHeaderContextMenuChangeIconSize(bool)) );
    }
}

void FavoriteCollectionWidget::writeConfig()
{
    KConfigGroup myGroup( KernelIf->config(), "FavoriteCollectionWidget" );
    myGroup.writeEntry( "IconSize", iconSize().width() );
}

void FavoriteCollectionWidget::slotHeaderContextMenuChangeIconSize(bool )
{
    QAction *act = dynamic_cast< QAction * >( sender() );
    if ( !act ) {
      return;
    }

    QVariant data = act->data();

    bool ok;
    const int size = data.toInt( &ok );
    if ( !ok ) {
      return;
    }

    const QSize newIconSize( QSize( size, size ) );
    if ( newIconSize == iconSize() ) {
      return;
    }
    setIconSize( newIconSize );

    writeConfig();
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

  KConfigGroup myGroup( KernelIf->config(), "FavoriteCollectionWidget" );
  int iIconSize = myGroup.readEntry( "IconSize", iconSize().width() );
  if ( iIconSize < 16 || iIconSize > 32 ) {
    iIconSize = 22;
  }
  setIconSize( QSize( iIconSize, iIconSize ) );
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
