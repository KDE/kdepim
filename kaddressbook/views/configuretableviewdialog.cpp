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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include "configuretableviewdialog.h"

#include <qstring.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>

#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <kconfig.h>

ConfigureTableViewWidget::ConfigureTableViewWidget( ViewManager *vm,
                                                    QWidget *parent, 
                                                    const char *name )
  : ViewConfigureWidget( vm, parent, name )
{
  QWidget *page = addPage( i18n( "Look & Feel" ), QString::null,
                           KGlobal::iconLoader()->loadIcon( "looknfeel",
                           KIcon::Desktop ) );
  
  mPage = new LookAndFeelPage( page );
}
                                           
ConfigureTableViewWidget::~ConfigureTableViewWidget()
{
}

void ConfigureTableViewWidget::restoreSettings( KConfig *config )
{
  ViewConfigureWidget::restoreSettings( config );

  mPage->restoreSettings( config );
}

void ConfigureTableViewWidget::saveSettings( KConfig *config )
{
  ViewConfigureWidget::saveSettings( config );

  mPage->saveSettings( config );
}



LookAndFeelPage::LookAndFeelPage(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  initGUI();
  
  // Set initial state
  enableBackgroundToggled(mBackgroundBox->isChecked());
}
    
void LookAndFeelPage::restoreSettings( KConfig *config )
{
  mAlternateButton->setChecked(config->readBoolEntry("ABackground", true));
  mLineButton->setChecked(config->readBoolEntry("SingleLine", false));
  mToolTipBox->setChecked(config->readBoolEntry("ToolTips", true));
      
  if (!mAlternateButton->isChecked() & !mLineButton->isChecked())
    mNoneButton->setChecked(true);
  
  mBackgroundBox->setChecked(config->readBoolEntry("Background", false));
  mBackgroundName->lineEdit()->setText(config->readEntry("BackgroundName"));
}
    
void LookAndFeelPage::saveSettings( KConfig *config )
{
  config->writeEntry("ABackground", mAlternateButton->isChecked());
  config->writeEntry("SingleLine", mLineButton->isChecked());
  config->writeEntry("ToolTips", mToolTipBox->isChecked());
  config->writeEntry("Background", mBackgroundBox->isChecked());
  config->writeEntry("BackgroundName", mBackgroundName->lineEdit()->text());
}
    
void LookAndFeelPage::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(KDialog::spacingHint());
  layout->setMargin(KDialog::marginHint());
      
  QButtonGroup *group = new QButtonGroup(1, Qt::Horizontal, 
                                         i18n("Row Separator"), this);
  layout->addWidget(group);
      
  mAlternateButton = new QRadioButton(i18n("Alternating backgrounds"),
                                      group, "mAlternateButton");
  mLineButton = new QRadioButton(i18n("Single line"), group, "mLineButton");
  mNoneButton = new QRadioButton(i18n("None"), group, "mNoneButton");
      
  // Background Checkbox/Selector
  QHBoxLayout *backgroundLayout = new QHBoxLayout();
  layout->addLayout(backgroundLayout);
      
  mBackgroundBox = new QCheckBox(i18n("Enable background image:"), this,
                                 "mBackgroundBox");
  connect(mBackgroundBox, SIGNAL(toggled(bool)),
          SLOT(enableBackgroundToggled(bool)));
  backgroundLayout->addWidget(mBackgroundBox);
  
  mBackgroundName = new KURLRequester(this, "mBackgroundName");
  mBackgroundName->setMode(KFile::File | KFile::ExistingOnly |
                           KFile::LocalOnly);
  mBackgroundName->setFilter(i18n("*.png *.jpg *.xpm|Image Files\n*|All Files"));
  backgroundLayout->addWidget(mBackgroundName);
      
  // ToolTip Checkbox
  mToolTipBox = new QCheckBox(i18n("Enable contact tooltips"), this,
                              "mToolTipBox");
  layout->addWidget(mToolTipBox);
}

void LookAndFeelPage::enableBackgroundToggled(bool enabled)
{
  mBackgroundName->setEnabled(enabled);
}
    
#include "configuretableviewdialog.moc"
