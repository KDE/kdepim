/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqstring.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqvbox.h>
#include <tqgroupbox.h>
#include <tqspinbox.h>
#include <tqtabwidget.h>
#include <tqwhatsthis.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kfontdialog.h>
#include <kpushbutton.h>

#include "colorlistbox.h"

#include "configurecardviewdialog.h"

/////////////////////////////////
// ConfigureCardViewDialog

ConfigureCardViewWidget::ConfigureCardViewWidget( KABC::AddressBook *ab, TQWidget *parent,
                                                  const char *name )
  : ViewConfigureWidget( ab, parent, name )
{
  TQWidget *page = addPage( i18n( "Look & Feel" ), TQString::null,
                           DesktopIcon( "looknfeel" ) );
  mAdvancedPage = new CardViewLookNFeelPage( page );
}

ConfigureCardViewWidget::~ConfigureCardViewWidget()
{
}

void ConfigureCardViewWidget::restoreSettings( KConfig *config )
{
  ViewConfigureWidget::restoreSettings( config );

  mAdvancedPage->restoreSettings( config );
}

void ConfigureCardViewWidget::saveSettings( KConfig *config )
{
  ViewConfigureWidget::saveSettings( config );

  mAdvancedPage->saveSettings( config );
}

////////////////////////
// CardViewLookNFeelPage
CardViewLookNFeelPage::CardViewLookNFeelPage( TQWidget *parent, const char *name )
  : TQVBox( parent, name )
{
  initGUI();
}

CardViewLookNFeelPage::~CardViewLookNFeelPage()
{
}

void CardViewLookNFeelPage::restoreSettings( KConfig *config )
{
  // colors
  cbEnableCustomColors->setChecked( config->readBoolEntry( "EnableCustomColors", false ) );
  TQColor c;
  c = KGlobalSettings::baseColor();
  lbColors->insertItem( new ColorListItem( i18n("Background Color"),
        config->readColorEntry( "BackgroundColor", &c ) ) );
  c = colorGroup().foreground();
  lbColors->insertItem( new ColorListItem( i18n("Text Color"),
        config->readColorEntry( "TextColor", &c ) ) );
  c = colorGroup().button();
  lbColors->insertItem( new ColorListItem( i18n("Header, Border & Separator Color"),
        config->readColorEntry( "HeaderColor", &c ) ) );
  c = colorGroup().buttonText();
  lbColors->insertItem( new ColorListItem( i18n("Header Text Color"),
        config->readColorEntry( "HeaderTextColor", &c ) ) );
  c = colorGroup().highlight();
  lbColors->insertItem( new ColorListItem( i18n("Highlight Color"),
        config->readColorEntry( "HighlightColor", &c ) ) );
  c = colorGroup().highlightedText();
  lbColors->insertItem( new ColorListItem( i18n("Highlighted Text Color"),
        config->readColorEntry( "HighlightedTextColor", &c ) ) );

  enableColors();

  // fonts
  TQFont fnt = font();
  updateFontLabel( config->readFontEntry( "TextFont", &fnt ), (TQLabel*)lTextFont );
  fnt.setBold( true );
  updateFontLabel( config->readFontEntry( "HeaderFont",  &fnt ), (TQLabel*)lHeaderFont );
  cbEnableCustomFonts->setChecked( config->readBoolEntry( "EnableCustomFonts", false ) );
  enableFonts();

  // layout
  sbMargin->setValue( config->readNumEntry( "ItemMargin", 0 ) );
  sbSpacing->setValue( config->readNumEntry( "ItemSpacing", 10 ) );
  sbSepWidth->setValue( config->readNumEntry( "SeparatorWidth", 2 ) );
  cbDrawSeps->setChecked( config->readBoolEntry( "DrawSeparators", true ) );
  cbDrawBorders->setChecked( config->readBoolEntry( "DrawBorder", true ) );

  // behaviour
  cbShowFieldLabels->setChecked( config->readBoolEntry( "DrawFieldLabels", false ) );
  cbShowEmptyFields->setChecked( config->readBoolEntry( "ShowEmptyFields", false ) );
}

void CardViewLookNFeelPage::saveSettings( KConfig *config )
{
  // colors
  config->writeEntry( "EnableCustomColors", cbEnableCustomColors->isChecked() );
  if ( cbEnableCustomColors->isChecked() ) // ?? - Hmmm.
  {
    config->writeEntry( "BackgroundColor", lbColors->color( 0 ) );
    config->writeEntry( "TextColor", lbColors->color( 1 ) );
    config->writeEntry( "HeaderColor", lbColors->color( 2 ) );
    config->writeEntry( "HeaderTextColor", lbColors->color( 3 ) );
    config->writeEntry( "HighlightColor", lbColors->color( 4 ) );
    config->writeEntry( "HighlightedTextColor", lbColors->color( 5 ) );
  }
  // fonts
  config->writeEntry( "EnableCustomFonts", cbEnableCustomFonts->isChecked() );
  if ( cbEnableCustomFonts->isChecked() )
  {
    config->writeEntry( "TextFont", lTextFont->font() );
    config->writeEntry( "HeaderFont", lHeaderFont->font() );
  }
  // layout
  config->writeEntry( "ItemMargin", sbMargin->value() );
  config->writeEntry( "ItemSpacing", sbSpacing->value() );
  config->writeEntry( "SeparatorWidth", sbSepWidth->value() );
  config->writeEntry("DrawBorder", cbDrawBorders->isChecked());
  config->writeEntry("DrawSeparators", cbDrawSeps->isChecked());

  // behaviour
  config->writeEntry("DrawFieldLabels", cbShowFieldLabels->isChecked());
  config->writeEntry("ShowEmptyFields", cbShowEmptyFields->isChecked());
}

void CardViewLookNFeelPage::setTextFont()
{
  TQFont f( lTextFont->font() );
  if ( KFontDialog::getFont( f, false, this ) == TQDialog::Accepted )
    updateFontLabel( f, lTextFont );
}

void CardViewLookNFeelPage::setHeaderFont()
{
  TQFont f( lHeaderFont->font() );
  if ( KFontDialog::getFont( f,false, this ) == TQDialog::Accepted )
    updateFontLabel( f, lHeaderFont );
}

void CardViewLookNFeelPage::enableFonts()
{
  vbFonts->setEnabled( cbEnableCustomFonts->isChecked() );
}

void CardViewLookNFeelPage::enableColors()
{
  lbColors->setEnabled( cbEnableCustomColors->isChecked() );
}

void CardViewLookNFeelPage::initGUI()
{
  int spacing = KDialog::spacingHint();
  int margin = KDialog::marginHint();

  TQTabWidget *tabs = new TQTabWidget( this );

  // Layout
  TQVBox *loTab = new TQVBox( this, "layouttab" );

  loTab->setSpacing( spacing );
  loTab->setMargin( margin );

  TQGroupBox *gbGeneral = new TQGroupBox( 1, Qt::Horizontal, i18n("General"), loTab );

  cbDrawSeps = new TQCheckBox( i18n("Draw &separators"), gbGeneral );

  TQHBox *hbSW = new TQHBox( gbGeneral );
  TQLabel *lSW = new TQLabel( i18n("Separator &width:"), hbSW );
  sbSepWidth = new TQSpinBox( 1, 50, 1, hbSW );
  lSW->setBuddy( sbSepWidth);

  TQHBox *hbPadding = new TQHBox( gbGeneral );
  TQLabel *lSpacing = new TQLabel( i18n("&Padding:"), hbPadding );
  sbSpacing = new TQSpinBox( 0, 100, 1, hbPadding );
  lSpacing->setBuddy( sbSpacing );

  TQGroupBox *gbCards = new TQGroupBox( 1, Qt::Horizontal, i18n("Cards"), loTab );

  TQHBox *hbMargin = new TQHBox( gbCards );
  TQLabel *lMargin = new TQLabel( i18n("&Margin:"), hbMargin );
  sbMargin = new TQSpinBox( 0, 100, 1, hbMargin );
  lMargin->setBuddy( sbMargin );

  cbDrawBorders = new TQCheckBox( i18n("Draw &borders"), gbCards );

  loTab->setStretchFactor( new TQWidget( loTab ), 1 );

  TQWhatsThis::add( sbMargin, i18n(
        "The item margin is the distance (in pixels) between the item edge and the item data. Most noticeably, "
        "incrementing the item margin will add space between the focus rectangle and the item data."
        ) );
  TQWhatsThis::add( lMargin, TQWhatsThis::textFor( sbMargin ) );
  TQWhatsThis::add( sbSpacing, i18n(
        "The item spacing decides the distance (in pixels) between the items and anything else: the view "
        "borders, other items or column separators."
        ) );
  TQWhatsThis::add( lSpacing, TQWhatsThis::textFor( sbSpacing ) );
  TQWhatsThis::add( sbSepWidth, i18n("Sets the width of column separators") );
  TQWhatsThis::add( lSW, TQWhatsThis::textFor( sbSepWidth ) );

  tabs->addTab( loTab, i18n("&Layout") );

  // Colors
  TQVBox *colorTab = new TQVBox( this, "colortab" );
  colorTab->setSpacing( spacing );
  colorTab->setMargin( spacing );
  cbEnableCustomColors = new TQCheckBox( i18n("&Enable custom colors"), colorTab );
  connect( cbEnableCustomColors, TQT_SIGNAL(clicked()), this, TQT_SLOT(enableColors()) );
  lbColors = new ColorListBox( colorTab );
  tabs->addTab( colorTab, i18n("&Colors") );

  TQWhatsThis::add( cbEnableCustomColors, i18n(
        "If custom colors is enabled, you may choose the colors for the view below. "
        "Otherwise colors from your current KDE color scheme are used."
        ) );
  TQWhatsThis::add( lbColors, i18n(
        "Double click or press RETURN on a item to select a color for the related strings in the view."
        ) );

  // Fonts
  TQVBox *fntTab = new TQVBox( this, "fonttab" );

  fntTab->setSpacing( spacing );
  fntTab->setMargin( spacing );

  cbEnableCustomFonts = new TQCheckBox( i18n("&Enable custom fonts"), fntTab );
  connect( cbEnableCustomFonts, TQT_SIGNAL(clicked()), this, TQT_SLOT(enableFonts()) );

  vbFonts = new TQWidget( fntTab );
  TQGridLayout *gFnts = new TQGridLayout( vbFonts, 2, 3 );
  gFnts->setSpacing( spacing );
  gFnts->setAutoAdd( true );
  gFnts->setColStretch( 1, 1 );
  TQLabel *lTFnt = new TQLabel( i18n("&Text font:"), vbFonts );
  lTextFont = new TQLabel( vbFonts );
  lTextFont->setFrameStyle( TQFrame::Panel|TQFrame::Sunken );
  btnFont = new KPushButton( i18n("Choose..."), vbFonts );
  lTFnt->setBuddy( btnFont );
  connect( btnFont, TQT_SIGNAL(clicked()), this, TQT_SLOT(setTextFont()) );

  TQLabel *lHFnt = new TQLabel( i18n("&Header font:"), vbFonts );
  lHeaderFont = new TQLabel( vbFonts );
  lHeaderFont->setFrameStyle( TQFrame::Panel|TQFrame::Sunken );
  btnHeaderFont = new KPushButton( i18n("Choose..."), vbFonts );
  lHFnt->setBuddy( btnHeaderFont );
  connect( btnHeaderFont, TQT_SIGNAL(clicked()), this, TQT_SLOT(setHeaderFont()) );

  fntTab->setStretchFactor( new TQWidget( fntTab ), 1 );

  TQWhatsThis::add( cbEnableCustomFonts, i18n(
        "If custom fonts are enabled, you may choose which fonts to use for this view below. "
        "Otherwise the default KDE font will be used, in bold style for the header and "
        "normal style for the data."
        ) );

  tabs->addTab( fntTab, i18n("&Fonts") );

  // Behaviour
  TQVBox *behaviourTab = new TQVBox( this );
  behaviourTab->setMargin( margin );
  behaviourTab->setSpacing( spacing );

  cbShowEmptyFields = new TQCheckBox( i18n("Show &empty fields"), behaviourTab );
  cbShowFieldLabels = new TQCheckBox( i18n("Show field &labels"), behaviourTab );

  behaviourTab->setStretchFactor( new TQWidget( behaviourTab ), 1 );

  tabs->addTab( behaviourTab, i18n("Be&havior") );

}

void CardViewLookNFeelPage::updateFontLabel( TQFont fnt, TQLabel *l )
{
  l->setFont( fnt );
  l->setText(  TQString( fnt.family() + " %1" ).arg( fnt.pointSize() ) );
}

#include "configurecardviewdialog.moc"
