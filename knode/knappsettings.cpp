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
  QVBoxLayout *topL=new QVBoxLayout(this, 5);
  topL->setAutoAdd(true);

  longCB = new QCheckBox(i18n("Show long group list"),this);
  colorCB = new QCheckBox(i18n("Use custom colors"),this);
  cList = new QListBox(this);
  connect(cList, SIGNAL(selected(QListBoxItem*)),SLOT(slotColItemSelected(QListBoxItem*)));
  connect(colorCB, SIGNAL(toggled(bool)),cList, SLOT(setEnabled(bool)));
  fontCB = new QCheckBox(i18n("Use custom fonts"),this);
  fList = new QListBox(this);
  connect(fList, SIGNAL(selected(QListBoxItem*)),SLOT(slotFontItemSelected(QListBoxItem*)));
  connect(fontCB, SIGNAL(toggled(bool)),fList, SLOT(setEnabled(bool)));

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
  for (int i=0;i<man->colorCount();i++)
    cList->insertItem(new ColorListItem(man->colorName(i),man->color(i)));

  fontCB->setChecked(man->useFonts());
  fList->setEnabled(man->useFonts());
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


//--------------------------------

#include "knappsettings.moc"




