/***************************************************************************
                          knappsettings.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Gebauer
    email                : gebauer@bigfoot.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qcheckbox.h>

#include <kcolordialog.h>
#include <kfontdialog.h>
#include <kseparator.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

#include "knglobals.h"
#include "knappmanager.h"
#include "knappsettings.h"


//===================================================================================
// code taken from KMail, Copyright (C) 2000 Espen Sand, espen@kde.org

KNAppSettings::ColorListItem::ColorListItem( const QString &text, const QColor &color )
  : QListBoxText(text), mColor( color )
{
}


KNAppSettings::ColorListItem::~ColorListItem()
{
}


void KNAppSettings::ColorListItem::paint( QPainter *p )
{
  QFontMetrics fm = p->fontMetrics();
  int h = fm.height();

  p->drawText( 30+3*2, fm.ascent() + fm.leading()/2, text() );

  p->setPen( Qt::black );
  p->drawRect( 3, 1, 30, h-1 );
  p->fillRect( 4, 2, 28, h-3, mColor );
}


int KNAppSettings::ColorListItem::height(const QListBox *lb ) const
{
  return( lb->fontMetrics().lineSpacing()+1 );
}


int KNAppSettings::ColorListItem::width(const QListBox *lb ) const
{
  return( 30 + lb->fontMetrics().width( text() ) + 6 );
}


//===================================================================================


KNAppSettings::FontListItem::FontListItem( const QString &name, const QFont &font )
  : QListBoxText(name), f_ont(font)
{
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


KNAppSettings::FontListItem::~FontListItem()
{
}


void KNAppSettings::FontListItem::setFont(const QFont &font)
{
  f_ont = font;
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


void KNAppSettings::FontListItem::paint( QPainter *p )
{
  QFont fnt = p->font();
  fnt.setWeight(QFont::Bold);
  p->setFont(fnt);
  int fontInfoWidth = p->fontMetrics().width(fontInfo);
  int h = p->fontMetrics().ascent() + p->fontMetrics().leading()/2;
  p->drawText(2, h, fontInfo );
  fnt.setWeight(QFont::Normal);
  p->setFont(fnt);
  p->drawText(5 + fontInfoWidth, h, text() );
}


int KNAppSettings::FontListItem::width(const QListBox *lb ) const
{
  return( lb->fontMetrics().width(fontInfo) + lb->fontMetrics().width(text()) + 20 );
}


//===================================================================================


KNAppSettings::KNAppSettings(QWidget *p) : KNSettingsWidget(p)
{
  QGridLayout *topL=new QGridLayout(this, 9,2, 5,5);

  longCB = new QCheckBox(i18n("Show long group list"),this);
  topL->addWidget(longCB,0,0);

  colorCB = new QCheckBox(i18n("Use custom colors"),this);
  connect(colorCB, SIGNAL(toggled(bool)), SLOT(slotColCBtoggled(bool)));
  topL->addWidget(colorCB,1,0);

  cList = new QListBox(this);
  connect(cList, SIGNAL(selectionChanged()),SLOT(slotColSelectionChanged()));
  connect(cList, SIGNAL(selected(QListBoxItem*)),SLOT(slotColItemSelected(QListBoxItem*)));
  topL->addMultiCellWidget(cList,2,4,0,0);

  changeColorB=new QPushButton(i18n("Cha&nge"), this);
  connect(changeColorB, SIGNAL(clicked()), this, SLOT(slotChangeColorBtnClicked()));
  topL->addWidget(changeColorB,2,1);

  defaultColorB=new QPushButton(i18n("&Defaults"), this);
  connect(defaultColorB, SIGNAL(clicked()), this, SLOT(slotDefaultColorBtnClicked()));
  topL->addWidget(defaultColorB,3,1);

  fontCB = new QCheckBox(i18n("Use custom fonts"),this);
  connect(fontCB, SIGNAL(toggled(bool)), SLOT(slotFontCBtoggled(bool)));
  topL->addWidget(fontCB,5,0);

  fList = new QListBox(this);
  connect(fList, SIGNAL(selectionChanged()),SLOT(slotFontSelectionChanged()));
  connect(fList, SIGNAL(selected(QListBoxItem*)),SLOT(slotFontItemSelected(QListBoxItem*)));
  topL->addMultiCellWidget(fList,6,8,0,0);

  changeFontB=new QPushButton(i18n("Chan&ge"), this);
  connect(changeFontB, SIGNAL(clicked()), this, SLOT(slotChangeFontBtnClicked()));
  topL->addWidget(changeFontB,6,1);

  defaultFontB=new QPushButton(i18n("D&efaults"), this);
  connect(defaultFontB, SIGNAL(clicked()), this, SLOT(slotDefaultFontBtnClicked()));
  topL->addWidget(defaultFontB,7,1);

  init();
}


KNAppSettings::~KNAppSettings()
{
}


void KNAppSettings::init()
{
  KNAppManager* man = knGlobals.appManager;

  longCB->setChecked(man->longGroupList());

  colorCB->setChecked(man->useColors());
  cList->setEnabled(man->useColors());
  changeColorB->setEnabled(false);
  defaultColorB->setEnabled(man->useColors());
  for (int i=0;i<man->colorCount();i++)
    cList->insertItem(new ColorListItem(man->colorName(i),man->color(i)));

  fontCB->setChecked(man->useFonts());
  fList->setEnabled(man->useFonts());
  changeFontB->setEnabled(false);
  defaultFontB->setEnabled(man->useFonts());
  for (int i=0;i<man->fontCount();i++)
    fList->insertItem(new FontListItem(man->fontName(i),man->font(i)));
}


void KNAppSettings::apply()
{
  KNAppManager* man = knGlobals.appManager;

  man->setLongGroupList(longCB->isChecked());

  man->setUseColors(colorCB->isChecked());
  for (int i=0;i<man->colorCount();i++)
    man->color(i) = static_cast<ColorListItem*>(cList->item(i))->color();

  man->setUseFonts(fontCB->isChecked());
  for (int i=0;i<man->fontCount();i++)
    man->font(i) = static_cast<FontListItem*>(fList->item(i))->font();
}


void KNAppSettings::slotColCBtoggled(bool b)
{
  cList->setEnabled(b);
  changeColorB->setEnabled(b && (cList->currentItem()!=-1));
  defaultColorB->setEnabled(b);
}


void KNAppSettings::slotColSelectionChanged()
{
  changeColorB->setEnabled(cList->currentItem()!=-1);
}


// show color dialog for the entry
void KNAppSettings::slotColItemSelected(QListBoxItem *it)
{
  if (it) {
    ColorListItem *colorItem = static_cast<ColorListItem*>(it);
    QColor col = colorItem->color();
    int result = KColorDialog::getColor(col,this);

    if (result == KColorDialog::Accepted) {
      colorItem->setColor(col);
      cList->triggerUpdate(false);
    }
  }
}


void KNAppSettings::slotChangeColorBtnClicked()
{
  if (cList->currentItem()!=-1)
    slotColItemSelected(cList->item(cList->currentItem()));
}


void KNAppSettings::slotDefaultColorBtnClicked()
{
  for (int i=0;i<knGlobals.appManager->colorCount();i++)
    static_cast<ColorListItem*>(cList->item(i))->setColor(knGlobals.appManager->defaultColor(i));
  cList->triggerUpdate(true);
}


void KNAppSettings::slotFontCBtoggled(bool b)
{
  fList->setEnabled(b);
  changeFontB->setEnabled(b && (fList->currentItem()!=-1));
  defaultFontB->setEnabled(b);
}


void KNAppSettings::slotFontSelectionChanged()
{
  changeFontB->setEnabled(fList->currentItem()!=-1);
}


// show font dialog for the entry
void KNAppSettings::slotFontItemSelected(QListBoxItem *it)
{
  if (it) {
    FontListItem *fontItem = static_cast<FontListItem*>(it);
    QFont font = fontItem->font();
    int result = KFontDialog::getFont(font,false,this);

    if (result == KFontDialog::Accepted) {
      fontItem->setFont(font);
      fList->triggerUpdate(false);
    }
  }
}


void KNAppSettings::slotChangeFontBtnClicked()
{
  if (fList->currentItem()!=-1)
    slotFontItemSelected(fList->item(fList->currentItem()));
}


void KNAppSettings::slotDefaultFontBtnClicked()
{
  for (int i=0;i<knGlobals.appManager->fontCount();i++)
    static_cast<FontListItem*>(fList->item(i))->setFont(knGlobals.appManager->defaultFont(i));
  fList->triggerUpdate(true);
}


//--------------------------------

#include "knappsettings.moc"

