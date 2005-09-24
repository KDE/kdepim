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

#include <qstring.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <q3vbox.h>
#include <q3groupbox.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QGridLayout>

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

ConfigureCardViewWidget::ConfigureCardViewWidget( KABC::AddressBook *ab, QWidget *parent,
                                                  const char *name )
  : ViewConfigureWidget( ab, parent, name )
{
  QWidget *page = addPage( i18n( "Look & Feel" ), QString::null,
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
CardViewLookNFeelPage::CardViewLookNFeelPage( QWidget *parent, const char *name )
  : Q3VBox( parent, name )
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
  QColor c;
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
  QFont fnt = font();
  updateFontLabel( config->readFontEntry( "TextFont", &fnt ), (QLabel*)lTextFont );
  fnt.setBold( true );
  updateFontLabel( config->readFontEntry( "HeaderFont",  &fnt ), (QLabel*)lHeaderFont );
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
  QFont f( lTextFont->font() );
  if ( KFontDialog::getFont( f, false, this ) == QDialog::Accepted )
    updateFontLabel( f, lTextFont );
}

void CardViewLookNFeelPage::setHeaderFont()
{
  QFont f( lHeaderFont->font() );
  if ( KFontDialog::getFont( f,false, this ) == QDialog::Accepted )
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

  QTabWidget *tabs = new QTabWidget( this );

  // Layout
  Q3VBox *loTab = new Q3VBox( this, "layouttab" );

  loTab->setSpacing( spacing );
  loTab->setMargin( margin );

  Q3GroupBox *gbGeneral = new Q3GroupBox( 1, Qt::Horizontal, i18n("General"), loTab );

  cbDrawSeps = new QCheckBox( i18n("Draw &separators"), gbGeneral );

  Q3HBox *hbSW = new Q3HBox( gbGeneral );
  QLabel *lSW = new QLabel( i18n("Separator &width:"), hbSW );
  sbSepWidth = new QSpinBox( 1, 50, 1, hbSW );
  lSW->setBuddy( sbSepWidth);

  Q3HBox *hbPadding = new Q3HBox( gbGeneral );
  QLabel *lSpacing = new QLabel( i18n("&Padding:"), hbPadding );
  sbSpacing = new QSpinBox( 0, 100, 1, hbPadding );
  lSpacing->setBuddy( sbSpacing );

  Q3GroupBox *gbCards = new Q3GroupBox( 1, Qt::Horizontal, i18n("Cards"), loTab );

  Q3HBox *hbMargin = new Q3HBox( gbCards );
  QLabel *lMargin = new QLabel( i18n("&Margin:"), hbMargin );
  sbMargin = new QSpinBox( 0, 100, 1, hbMargin );
  lMargin->setBuddy( sbMargin );

  cbDrawBorders = new QCheckBox( i18n("Draw &borders"), gbCards );

  loTab->setStretchFactor( new QWidget( loTab ), 1 );
  QString text = i18n(
        "The item margin is the distance (in pixels) between the item edge and the item data. Most noticeably, "
        "incrementing the item margin will add space between the focus rectangle and the item data."
        );
  Q3WhatsThis::add( sbMargin, text  );
  Q3WhatsThis::add( lMargin, text );
  text = i18n(
        "The item spacing decides the distance (in pixels) between the items and anything else: the view "
        "borders, other items or column separators."
        );
  Q3WhatsThis::add( sbSpacing, text );
  Q3WhatsThis::add( lSpacing, text );
  text = i18n("Sets the width of column separators");
  Q3WhatsThis::add( sbSepWidth, text );
  Q3WhatsThis::add( lSW, text );

  tabs->addTab( loTab, i18n("&Layout") );

  // Colors
  Q3VBox *colorTab = new Q3VBox( this, "colortab" );
  colorTab->setSpacing( spacing );
  colorTab->setMargin( spacing );
  cbEnableCustomColors = new QCheckBox( i18n("&Enable custom colors"), colorTab );
  connect( cbEnableCustomColors, SIGNAL(clicked()), this, SLOT(enableColors()) );
  lbColors = new ColorListBox( colorTab );
  tabs->addTab( colorTab, i18n("&Colors") );

  Q3WhatsThis::add( cbEnableCustomColors, i18n(
        "If custom colors is enabled, you may choose the colors for the view below. "
        "Otherwise colors from your current KDE color scheme are used."
        ) );
  Q3WhatsThis::add( lbColors, i18n(
        "Double click or press RETURN on a item to select a color for the related strings in the view."
        ) );

  // Fonts
  Q3VBox *fntTab = new Q3VBox( this, "fonttab" );

  fntTab->setSpacing( spacing );
  fntTab->setMargin( spacing );

  cbEnableCustomFonts = new QCheckBox( i18n("&Enable custom fonts"), fntTab );
  connect( cbEnableCustomFonts, SIGNAL(clicked()), this, SLOT(enableFonts()) );

  vbFonts = new QWidget( fntTab );
  QGridLayout *gFnts = new QGridLayout( vbFonts, 2, 3 );
  gFnts->setSpacing( spacing );
  gFnts->setAutoAdd( true );
  gFnts->setColStretch( 1, 1 );
  QLabel *lTFnt = new QLabel( i18n("&Text font:"), vbFonts );
  lTextFont = new QLabel( vbFonts );
  lTextFont->setFrameStyle( Q3Frame::Panel|Q3Frame::Sunken );
  btnFont = new KPushButton( i18n("Choose..."), vbFonts );
  lTFnt->setBuddy( btnFont );
  connect( btnFont, SIGNAL(clicked()), this, SLOT(setTextFont()) );

  QLabel *lHFnt = new QLabel( i18n("&Header font:"), vbFonts );
  lHeaderFont = new QLabel( vbFonts );
  lHeaderFont->setFrameStyle( Q3Frame::Panel|Q3Frame::Sunken );
  btnHeaderFont = new KPushButton( i18n("Choose..."), vbFonts );
  lHFnt->setBuddy( btnHeaderFont );
  connect( btnHeaderFont, SIGNAL(clicked()), this, SLOT(setHeaderFont()) );

  fntTab->setStretchFactor( new QWidget( fntTab ), 1 );

  Q3WhatsThis::add( cbEnableCustomFonts, i18n(
        "If custom fonts are enabled, you may choose which fonts to use for this view below. "
        "Otherwise the default KDE font will be used, in bold style for the header and "
        "normal style for the data."
        ) );

  tabs->addTab( fntTab, i18n("&Fonts") );

  // Behaviour
  Q3VBox *behaviourTab = new Q3VBox( this );
  behaviourTab->setMargin( margin );
  behaviourTab->setSpacing( spacing );

  cbShowEmptyFields = new QCheckBox( i18n("Show &empty fields"), behaviourTab );
  cbShowFieldLabels = new QCheckBox( i18n("Show field &labels"), behaviourTab );

  behaviourTab->setStretchFactor( new QWidget( behaviourTab ), 1 );

  tabs->addTab( behaviourTab, i18n("Be&havior") );

}

void CardViewLookNFeelPage::updateFontLabel( QFont fnt, QLabel *l )
{
  l->setFont( fnt );
  l->setText(  QString( fnt.family() + " %1" ).arg( fnt.pointSize() ) );
}

#include "configurecardviewdialog.moc"
