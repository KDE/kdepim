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
#include <kvbox.h>
#include <q3groupbox.h>
#include <qspinbox.h>
#include <qtabwidget.h>

//Added by qt3to4:
#include <QFrame>
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
  QWidget *page = addPage( i18n( "Look & Feel" ), QString(),
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
  : KVBox( parent )
{
  initGUI();
}

CardViewLookNFeelPage::~CardViewLookNFeelPage()
{
}

void CardViewLookNFeelPage::restoreSettings( KConfig *config )
{
  // colors
  cbEnableCustomColors->setChecked( config->readEntry( "EnableCustomColors", false ) );
  QColor c;
  c = KGlobalSettings::baseColor();
  lbColors->insertItem( new ColorListItem( i18n("Background Color"),
        config->readEntry( "BackgroundColor", c ) ) );
  c = colorGroup().foreground();
  lbColors->insertItem( new ColorListItem( i18n("Text Color"),
        config->readEntry( "TextColor", c ) ) );
  c = colorGroup().button();
  lbColors->insertItem( new ColorListItem( i18n("Header, Border & Separator Color"),
        config->readEntry( "HeaderColor", c ) ) );
  c = colorGroup().buttonText();
  lbColors->insertItem( new ColorListItem( i18n("Header Text Color"),
        config->readEntry( "HeaderTextColor", c ) ) );
  c = colorGroup().highlight();
  lbColors->insertItem( new ColorListItem( i18n("Highlight Color"),
        config->readEntry( "HighlightColor", c ) ) );
  c = colorGroup().highlightedText();
  lbColors->insertItem( new ColorListItem( i18n("Highlighted Text Color"),
        config->readEntry( "HighlightedTextColor", c ) ) );

  enableColors();

  // fonts
  QFont fnt = font();
  updateFontLabel( config->readEntry( "TextFont", fnt ), (QLabel*)lTextFont );
  fnt.setBold( true );
  updateFontLabel( config->readEntry( "HeaderFont",  fnt ), (QLabel*)lHeaderFont );
  cbEnableCustomFonts->setChecked( config->readEntry( "EnableCustomFonts", false ) );
  enableFonts();

  // layout
  sbMargin->setValue( config->readEntry( "ItemMargin", 0 ) );
  sbSpacing->setValue( config->readEntry( "ItemSpacing", 10 ) );
  sbSepWidth->setValue( config->readEntry( "SeparatorWidth", 2 ) );
  cbDrawSeps->setChecked( config->readEntry( "DrawSeparators", true ) );
  cbDrawBorders->setChecked( config->readEntry( "DrawBorder", true ) );

  // behaviour
  cbShowFieldLabels->setChecked( config->readEntry( "DrawFieldLabels", false ) );
  cbShowEmptyFields->setChecked( config->readEntry( "ShowEmptyFields", false ) );
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
  KVBox *loTab = new KVBox( this );

  loTab->setSpacing( spacing );
  loTab->setMargin( margin );

  Q3GroupBox *gbGeneral = new Q3GroupBox( 1, Qt::Horizontal, i18n("General"), loTab );

  cbDrawSeps = new QCheckBox( i18n("Draw &separators"), gbGeneral );

  KHBox *hbSW = new KHBox( gbGeneral );
  QLabel *lSW = new QLabel( i18n("Separator &width:"), hbSW );
  sbSepWidth = new QSpinBox( hbSW );
  sbSepWidth->setRange( 1, 50 );
  lSW->setBuddy( sbSepWidth);

  KHBox *hbPadding = new KHBox( gbGeneral );
  QLabel *lSpacing = new QLabel( i18n("&Padding:"), hbPadding );
  sbSpacing = new QSpinBox( hbPadding );
  sbSpacing->setRange( 0, 100 );
  lSpacing->setBuddy( sbSpacing );

  Q3GroupBox *gbCards = new Q3GroupBox( 1, Qt::Horizontal, i18n("Cards"), loTab );

  KHBox *hbMargin = new KHBox( gbCards );
  QLabel *lMargin = new QLabel( i18n("&Margin:"), hbMargin );
  sbMargin = new QSpinBox( hbMargin );
  sbMargin->setRange( 0, 100 );
  lMargin->setBuddy( sbMargin );

  cbDrawBorders = new QCheckBox( i18n("Draw &borders"), gbCards );

  loTab->setStretchFactor( new QWidget( loTab ), 1 );
  QString text = i18n(
        "The item margin is the distance (in pixels) between the item edge and the item data. Most noticeably, "
        "incrementing the item margin will add space between the focus rectangle and the item data."
        );
  sbMargin->setWhatsThis( text  );
  lMargin->setWhatsThis( text );
  text = i18n(
        "The item spacing decides the distance (in pixels) between the items and anything else: the view "
        "borders, other items or column separators."
        );
  sbSpacing->setWhatsThis( text );
  lSpacing->setWhatsThis( text );
  text = i18n("Sets the width of column separators");
  sbSepWidth->setWhatsThis( text );
  lSW->setWhatsThis( text );

  tabs->addTab( loTab, i18n("&Layout") );

  // Colors
  KVBox *colorTab = new KVBox( this );
  colorTab->setSpacing( spacing );
  colorTab->setMargin( spacing );
  cbEnableCustomColors = new QCheckBox( i18n("&Enable custom colors"), colorTab );
  connect( cbEnableCustomColors, SIGNAL(clicked()), this, SLOT(enableColors()) );
  lbColors = new ColorListBox( colorTab );
  tabs->addTab( colorTab, i18n("&Colors") );

  cbEnableCustomColors->setWhatsThis( i18n(
        "If custom colors is enabled, you may choose the colors for the view below. "
        "Otherwise colors from your current KDE color scheme are used."
        ) );
  lbColors->setWhatsThis( i18n(
        "Double click or press RETURN on a item to select a color for the related strings in the view."
        ) );

  // Fonts
  KVBox *fntTab = new KVBox( this );

  fntTab->setSpacing( spacing );
  fntTab->setMargin( spacing );

  cbEnableCustomFonts = new QCheckBox( i18n("&Enable custom fonts"), fntTab );
  connect( cbEnableCustomFonts, SIGNAL(clicked()), this, SLOT(enableFonts()) );

  vbFonts = new QWidget( fntTab );
  QGridLayout *gFnts = new QGridLayout( vbFonts, 2, 3 );
  gFnts->setSpacing( spacing );
  gFnts->setAutoAdd( true );
  gFnts->setColumnStretch( 1, 1 );
  QLabel *lTFnt = new QLabel( i18n("&Text font:"), vbFonts );
  lTextFont = new QLabel( vbFonts );
  lTextFont->setFrameStyle( QFrame::Panel|QFrame::Sunken );
  btnFont = new KPushButton( i18n("Choose..."), vbFonts );
  lTFnt->setBuddy( btnFont );
  connect( btnFont, SIGNAL(clicked()), this, SLOT(setTextFont()) );

  QLabel *lHFnt = new QLabel( i18n("&Header font:"), vbFonts );
  lHeaderFont = new QLabel( vbFonts );
  lHeaderFont->setFrameStyle( QFrame::Panel|QFrame::Sunken );
  btnHeaderFont = new KPushButton( i18n("Choose..."), vbFonts );
  lHFnt->setBuddy( btnHeaderFont );
  connect( btnHeaderFont, SIGNAL(clicked()), this, SLOT(setHeaderFont()) );

  fntTab->setStretchFactor( new QWidget( fntTab ), 1 );

  cbEnableCustomFonts->setWhatsThis( i18n(
        "If custom fonts are enabled, you may choose which fonts to use for this view below. "
        "Otherwise the default KDE font will be used, in bold style for the header and "
        "normal style for the data."
        ) );

  tabs->addTab( fntTab, i18n("&Fonts") );

  // Behaviour
  KVBox *behaviourTab = new KVBox( this );
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
