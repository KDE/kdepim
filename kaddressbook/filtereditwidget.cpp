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

#include <qgroupbox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>

#include <klistbox.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <klocale.h>

#include "filtereditwidget.h"
#include "filter.h"
#include "kabprefs.h"

FilterEditWidget::FilterEditWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  initGUI();
  
  clear();
}

FilterEditWidget::~FilterEditWidget()
{
}

void FilterEditWidget::clear()
{
  // Default filter
  mFilter = Filter();
  
  mAvailableBox->clear();
  mSelectedBox->clear();
  
  availableHighlighted(-1);
  selectedHighlighted(-1);
}
    
void FilterEditWidget::setFilter(const Filter &f)
{
  clear();
  
  mFilter = f;
  QStringList cats = KABPrefs::instance()->mCustomCategories;
  QStringList filterCats = mFilter.categories();
  
  QStringList::Iterator iter;
  for (iter = filterCats.begin(); iter != filterCats.end(); ++iter)
    cats.erase(cats.find(*iter));
    
  mAvailableBox->insertStringList(cats);
  mSelectedBox->insertStringList(filterCats);
  
  if (mFilter.matchRule() == Filter::Matching)
    mMatchRuleGroup->setButton(0);
  else
    mMatchRuleGroup->setButton(1);
}

const Filter &FilterEditWidget::filter()
{
  QStringList cats;
  for (unsigned int i = 0; i < mSelectedBox->count(); ++i)
    cats << mSelectedBox->text(i);
    
  mFilter.setCategories(cats);
  if (mMatchRuleGroup->find(0)->isOn())
    mFilter.setMatchRule(Filter::Matching);
  else
    mFilter.setMatchRule(Filter::NotMatching);
    
  return mFilter;
}

void FilterEditWidget::availableHighlighted(int index)
{
  mAddButton->setEnabled(index >= 0);
}

void FilterEditWidget::selectedHighlighted(int index)
{
  mRemoveButton->setEnabled(index >= 0);
}

void FilterEditWidget::add()
{
  QString text = mAvailableBox->currentText();
  mAvailableBox->removeItem(mAvailableBox->currentItem());
  
  mSelectedBox->insertItem(text);
  mSelectedBox->setCurrentItem(mSelectedBox->count()-1);
  mSelectedBox->ensureCurrentVisible();
}

void FilterEditWidget::remove()
{
  QString text = mSelectedBox->currentText();
  mSelectedBox->removeItem(mSelectedBox->currentItem());
  
  mAvailableBox->insertItem(text);
  mAvailableBox->setCurrentItem(mAvailableBox->count()-1);
  mAvailableBox->ensureCurrentVisible();
}
    
void FilterEditWidget::initGUI()
{
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setAutoAdd(true);
  
  // Category selection
  QGroupBox *gb = new QGroupBox(i18n("Categories"), this);
  QBoxLayout *gbLayout = new QVBoxLayout(gb);
  gbLayout->setSpacing( KDialog::spacingHint() );
  gbLayout->setMargin( 20 );
  
  QBoxLayout *topGBLayout = new QHBoxLayout();
  topGBLayout->setSpacing( KDialog::spacingHint() );
  gbLayout->addLayout(topGBLayout);
  
  QVBoxLayout *listLayout = new QVBoxLayout();
  listLayout->setSpacing( KDialog::spacingHint() );
  topGBLayout->addLayout( listLayout );
  
  QLabel *label = new QLabel(i18n("Available categories:"), gb);
  listLayout->addWidget(label);
  mAvailableBox = new KListBox(gb, "mAvailableBox");
  mAvailableBox->setMinimumHeight(120);  // try to force the dialog taller
  connect(mAvailableBox, SIGNAL(highlighted(int)), 
          SLOT(availableHighlighted(int)));
  listLayout->addWidget(mAvailableBox);
  
  listLayout = new QVBoxLayout();
  listLayout->setSpacing( KDialog::spacingHint() );
  topGBLayout->addLayout( listLayout );
  
  QWidget *spacer = new QWidget(gb);
  listLayout->addWidget(spacer);  // spacer
  listLayout->setStretchFactor(spacer, 1);
  
  mAddButton = new QToolButton(gb, "mAddButton");
  mAddButton->setIconSet(SmallIconSet("forward"));
  connect(mAddButton, SIGNAL(clicked()), SLOT(add()));
  listLayout->addWidget(mAddButton);
  
  mRemoveButton = new QToolButton(gb, "mRemoveButton");
  mRemoveButton->setIconSet(SmallIconSet("back"));
  connect(mRemoveButton, SIGNAL(clicked()), SLOT(remove()));
  listLayout->addWidget(mRemoveButton);
  
  spacer = new QWidget(gb);
  listLayout->addWidget(spacer);  // spacer
  listLayout->setStretchFactor(spacer, 1);
  
  listLayout = new QVBoxLayout();
  listLayout->setSpacing( KDialog::spacingHint() );
  topGBLayout->addLayout( listLayout );
  
  label = new QLabel(i18n("Selected categories:"), gb);
  listLayout->addWidget(label);
  mSelectedBox = new KListBox(gb, "mSelectedBox");
  connect(mSelectedBox, SIGNAL(highlighted(int)), 
          SLOT(selectedHighlighted(int)));
  listLayout->addWidget(mSelectedBox);
  
  mMatchRuleGroup = new QButtonGroup();
  
  QRadioButton *radio = new QRadioButton(i18n("Show only contacts matching the selected categories"), gb);
  mMatchRuleGroup->insert(radio);
  gbLayout->addWidget(radio);
  
  radio = new QRadioButton(i18n("Show all contacts except those matching the selected categories"), gb);
  mMatchRuleGroup->insert(radio);
  gbLayout->addWidget(radio);
}

#include "filtereditwidget.moc"
