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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include "configurecardviewdialog.h"
#include "configurecardviewdialog.moc"
#include "colorlistbox.h"

#include <qstring.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qwhatsthis.h>

#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kfontdialog.h>
#include <kpushbutton.h>

#include <kdebug.h>

/////////////////////////////////
// ConfigureCardViewDialog

ConfigureCardViewWidget::ConfigureCardViewWidget( ViewManager *vm, QWidget *parent, 
                                                  const char *name )
  : ViewConfigureWidget( vm, parent, name )
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
  : QVBox( parent, name )
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
  lbColors->insertItem( new ColorListItem( i18n("Header, Border and Separator Color"),
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
  cbShowFieldLabels->setChecked( config->readBoolEntry( "DrawFieldLabels", true ) );
  cbShowEmptyFields->setChecked( config->readBoolEntry( "ShowEmptyFields", true ) );
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
  QVBox *loTab = new QVBox( this, "layouttab" );
  
  loTab->setSpacing( spacing );
  loTab->setMargin( margin );
  
  QGroupBox *gbGeneral = new QGroupBox( 1, Qt::Horizontal, i18n("General"), loTab );
  
  cbDrawSeps = new QCheckBox( i18n("Draw &Separators"), gbGeneral );

  QHBox *hbSW = new QHBox( gbGeneral );  
  QLabel *lSW = new QLabel( i18n("Separator &Width:"), hbSW );
  sbSepWidth = new QSpinBox( 1, 50, 1, hbSW );
  lSW->setBuddy( sbSepWidth);
  
  QHBox *hbPadding = new QHBox( gbGeneral );
  QLabel *lSpacing = new QLabel( i18n("&Padding:"), hbPadding );
  sbSpacing = new QSpinBox( 0, 100, 1, hbPadding );
  lSpacing->setBuddy( sbSpacing );
  
  QGroupBox *gbCards = new QGroupBox( 1, Qt::Horizontal, i18n("Cards"), loTab );
  
  QHBox *hbMargin = new QHBox( gbCards );
  QLabel *lMargin = new QLabel( i18n("&Margin:"), hbMargin );
  sbMargin = new QSpinBox( 0, 100, 1, hbMargin );
  lMargin->setBuddy( sbMargin );
  
  cbDrawBorders = new QCheckBox( i18n("Draw &Borders"), gbCards );
  
  loTab->setStretchFactor( new QWidget( loTab ), 1 );
  
  QWhatsThis::add( sbMargin, i18n(
        "The item margin is the distance (in pixels) between the item edge and the item data. Most notacibly, "
        "incrementing the item margin will add space between the focus rectangle and the item data."
        ) );
  QWhatsThis::add( lMargin, QWhatsThis::textFor( sbMargin ) );
  QWhatsThis::add( sbSpacing, i18n(
        "The Item Spacing decides the distance (in pixels) between the items and anything else: the view "
        "borders, other items or column separators."
        ) );
  QWhatsThis::add( lSpacing, QWhatsThis::textFor( sbSpacing ) );
  QWhatsThis::add( sbSepWidth, i18n("Sets the width of column separators") );
  QWhatsThis::add( lSW, QWhatsThis::textFor( sbSepWidth ) );
  
  tabs->addTab( loTab, i18n("&Layout") );
    
  // Colors
  QVBox *colorTab = new QVBox( this, "colortab" );
  colorTab->setSpacing( spacing );
  colorTab->setMargin( spacing );
  cbEnableCustomColors = new QCheckBox( i18n("&Enable custom Colors"), colorTab );
  connect( cbEnableCustomColors, SIGNAL(clicked()), this, SLOT(enableColors()) );
  lbColors = new ColorListBox( colorTab );
  tabs->addTab( colorTab, i18n("&Colors") );
  
  QWhatsThis::add( cbEnableCustomColors, i18n(
        "If custom colors are enabled, you may choose the colors for the view below. "
        "Otherwise colors from your current KDE color scheme are used."
        ) );
  QWhatsThis::add( lbColors, i18n(
        "Double click or press RETURN on a item to select a color for the related strings in the view."
        ) );
  
  // Fonts
  QVBox *fntTab = new QVBox( this, "fonttab" );
  
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

  QWhatsThis::add( cbEnableCustomFonts, i18n(
        "If custom fonts are enabled, you may choose which fonts to use for this view below. "
        "Otherwise the default KDE font will be used, in bold style for the header and "
        "normal style for the data."
        ) );
          
  tabs->addTab( fntTab, i18n("&Fonts") );
  
  // Behaviour
  QVBox *behaviourTab = new QVBox( this );
  behaviourTab->setMargin( margin );
  behaviourTab->setSpacing( spacing );
  
  cbShowEmptyFields = new QCheckBox( i18n("Show &Empty Fields"), behaviourTab );
  cbShowFieldLabels = new QCheckBox( i18n("Show Field &Labels"), behaviourTab );
  
  behaviourTab->setStretchFactor( new QWidget( behaviourTab ), 1 );
  
  tabs->addTab( behaviourTab, i18n("Be&haviour") );
    
}

void CardViewLookNFeelPage::updateFontLabel( QFont fnt, QLabel *l )
{
  l->setFont( fnt );
  l->setText(  QString( fnt.family() + " %1" ).arg( fnt.pointSize() ) );
}
