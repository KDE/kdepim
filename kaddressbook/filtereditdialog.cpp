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

#include <qwidget.h>
#include <qtoolbutton.h>
#include <qstring.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qtooltip.h>
#include <qhbox.h>
#include <qlabel.h>

#include <kiconloader.h>
#include <klocale.h>
#include <klistbox.h>
#include <klineedit.h>
#include <kdebug.h>

#include "filtereditdialog.h"
#include "filtereditwidget.h"

class NameDialog : public KDialogBase
{
  public:
    NameDialog(QWidget *parent) 
      : KDialogBase(Plain, i18n("Filter Name"), Ok | Cancel, Ok, parent)
      {
        QWidget *page = plainPage();
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setMargin(marginHint());
        layout->setSpacing(spacingHint());
        layout->setAutoAdd(true);
        
        (void) new QLabel(i18n("Please enter a name for the filter:"), page);
        mNameEdit = new KLineEdit(page, "mNameEdit");
        mNameEdit->setFocus();
      }
      
    QString name() const { return mNameEdit->text(); }
    void setName(const QString &name)
    {
      mNameEdit->setText(name);
      mNameEdit->home(false);
      mNameEdit->end(true); // select all
    }
  
  private:
   KLineEdit *mNameEdit;
};

FilterEditDialog::FilterEditDialog(QWidget *parent, const char *name)
  : KDialogBase(Plain, i18n("Edit Address Book Filters"),
                Ok | Apply | Cancel, Ok, parent, name, false)
{
  initGUI();
  
  mCurrentIndex = -1;
  filterHighlighted(-1);
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::slotOk()
{
  slotApply();
  KDialogBase::slotOk();
}

void FilterEditDialog::slotApply()
{
  // save the current one
  filterHighlighted(mCurrentIndex);
  
  emit filtersChanged(mFilterList);
  
  KDialogBase::slotApply();
}

void FilterEditDialog::setFilters(const Filter::List &list)
{
  mFilterList.clear();
  mFilterListBox->clear();
  mEditWidget->clear();
  
  mFilterList = list;
  Filter::List::Iterator iter;
  for (iter = mFilterList.begin(); iter != mFilterList.end(); ++iter)
    mFilterListBox->insertItem((*iter).name());
}
    
void FilterEditDialog::add()
{
  NameDialog dialog(this);
  if (dialog.exec())
  {
    QString name = dialog.name();
    if (!name.isEmpty())
    {
      Filter f;
      f.setName(name);
      mFilterList.append(f);
      mFilterListBox->insertItem(name);
      mFilterListBox->setCurrentItem(mFilterListBox->count()-1);
      mFilterListBox->ensureCurrentVisible();
    }
  }
}

void FilterEditDialog::remove()
{
  mFilterList.erase(mFilterList.at(mCurrentIndex));
  
  mFilterListBox->removeItem(mCurrentIndex);
}

void FilterEditDialog::rename()
{
  NameDialog dialog(this);
  dialog.setName(mFilterListBox->currentText());
  if (dialog.exec())
  {
    QString name = dialog.name();
    if (!name.isEmpty())
    {
      Filter f = mEditWidget->filter();
      f.setName(name);
      mFilterList[mCurrentIndex] = f;
      mEditWidget->setFilter(f);
      mFilterListBox->changeItem(name, mCurrentIndex);
    }
  }
}
    
void FilterEditDialog::filterHighlighted(int index)
{
  // Save the previous one if there was one
  if (mCurrentIndex >= 0)
    mFilterList[mCurrentIndex] = mEditWidget->filter();
    
  mCurrentIndex = index;
  
  if (mCurrentIndex >= 0)
  {
    mEditWidget->setFilter(mFilterList[mCurrentIndex]);
    mEditWidget->setEnabled(true);
    mRemoveButton->setEnabled(true);
    mRenameButton->setEnabled(true);
  }
  else
  {
    mEditWidget->clear();
    mEditWidget->setEnabled(false);
    mRemoveButton->setEnabled(false);
    mRenameButton->setEnabled(false);
  }
}

void FilterEditDialog::initGUI()
{
  QWidget *page = plainPage();
  
  QHBoxLayout *topLayout = new QHBoxLayout(page);
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );
  topLayout->setAutoAdd( true );
  
  QGroupBox *gb = new QGroupBox(i18n("Available Filters"), page);
  QVBoxLayout *gbLayout = new QVBoxLayout(gb);
  gbLayout->setSpacing( spacingHint() );
  gbLayout->setMargin( 20 );  // Prevent drawing on the title
  
  mFilterListBox = new KListBox(gb, "mFilterListBox");
  connect(mFilterListBox, SIGNAL(highlighted(int)),
          SLOT(filterHighlighted(int)));
  gbLayout->addWidget(mFilterListBox);
  
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing( spacingHint() );
  gbLayout->addLayout(buttonLayout);
  
  QToolButton *addButton = new QToolButton(gb);
  addButton->setIconSet(SmallIconSet("filenew"));
  QToolTip::add(addButton, i18n("Add a new filter"));
  connect(addButton, SIGNAL(clicked()), SLOT(add()));
  buttonLayout->addWidget(addButton);
  
  mRemoveButton = new QToolButton(gb);
  mRemoveButton->setIconSet(SmallIconSet("remove"));
  QToolTip::add(mRemoveButton, i18n("Remove selected filter"));
  connect(mRemoveButton, SIGNAL(clicked()), SLOT(remove()));
  buttonLayout->addWidget(mRemoveButton);
  
  mRenameButton = new QToolButton(gb);
  mRenameButton->setIconSet(SmallIconSet("edit"));
  QToolTip::add(mRenameButton, i18n("Rename selected filter"));
  connect(mRenameButton, SIGNAL(clicked()), SLOT(rename()));
  buttonLayout->addWidget(mRenameButton);
  
  mEditWidget = new FilterEditWidget(page);
  
  topLayout->setStretchFactor(mEditWidget, 1);
}

#include "filtereditdialog.moc"
