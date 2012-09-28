/*
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

#include "selectspecialchar.h"
#include <KCharSelect>
#include <KLocale>
#include <QHBoxLayout>

SelectSpecialChar::SelectSpecialChar(QWidget *parent)
  :KDialog(parent)
{
  setCaption( i18n("Select Special Characters") );
  setButtons( Ok|Cancel|User1 );
  setButtonText(KDialog::User1,i18n("Select"));
  QWidget *page = new QWidget( this );
  setMainWidget( page );
  QHBoxLayout *lay = new QHBoxLayout(page);
  mCharSelect = new KCharSelect(this,KCharSelect::CharacterTable|KCharSelect::BlockCombos);
  connect(mCharSelect,SIGNAL(charSelected(QChar)),this,SIGNAL(charSelected(QChar)));
  lay->addWidget(mCharSelect);
  connect(this,SIGNAL(user1Clicked()),SLOT(slotInsertChar()));
}

SelectSpecialChar::~SelectSpecialChar()
{
}

void SelectSpecialChar::showSelectButton(bool show)
{
  if(show)
    setButtons( Ok|Cancel|User1 );
  else
    setButtons( Ok|Cancel );
}

void SelectSpecialChar::slotInsertChar()
{
  Q_EMIT charSelected(mCharSelect->currentChar());
}

void SelectSpecialChar::setCurrentChar(const QChar &c)
{
  mCharSelect->setCurrentChar(c);
}

QChar SelectSpecialChar::currentChar() const
{
  return mCharSelect->currentChar();
}


void SelectSpecialChar::setOkButtonText(const QString& text)
{
  setButtonText(KDialog::User1,text);
}

#include "selectspecialchar.moc"
