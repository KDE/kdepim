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

#include <qstring.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qgroupbox.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>

/////////////////////////////////
// Look and feel page
CardViewLookAndFeelPage::CardViewLookAndFeelPage(QWidget *parent, 
                                                 const char *name)
  : QWidget(parent, name)
{
  initGUI();
}
    
void CardViewLookAndFeelPage::readConfig(KConfig *config)
{
  mBordersBox->setChecked(config->readBoolEntry("DrawBorder", true));
  mSeparatorsBox->setChecked(config->readBoolEntry("DrawSeparators", true));
  mLabelsBox->setChecked(config->readBoolEntry("DrawFieldLabels", true));
  mEmptyFieldsBox->setChecked(config->readBoolEntry("ShowEmptyFields", true));
}
    
void CardViewLookAndFeelPage::writeConfig(KConfig *config)
{
  config->writeEntry("DrawBorder", mBordersBox->isChecked());
  config->writeEntry("DrawSeparators", mSeparatorsBox->isChecked());
  config->writeEntry("DrawFieldLabels", mLabelsBox->isChecked());
  config->writeEntry("ShowEmptyFields", mEmptyFieldsBox->isChecked());
}
    
void CardViewLookAndFeelPage::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(KDialog::spacingHint());
  layout->setMargin(KDialog::marginHint());
  
  QGroupBox *group = 0;
    
  group = new QGroupBox(1, Qt::Horizontal, i18n("General"), this);
  layout->addWidget(group);
  
  mSeparatorsBox = new QCheckBox(i18n("Draw separators between columns"),
                                 group, "mSeparatorsBox");
      
  group = new QGroupBox(1, Qt::Horizontal, i18n("Cards"), this);
  layout->addWidget(group);
      
  mBordersBox = new QCheckBox(i18n("Draw borders"), group, "mBordersBox");
  mLabelsBox = new QCheckBox(i18n("Show field labels"), group, "mLabelsBox");
  mEmptyFieldsBox = new QCheckBox(i18n("Show fields with no value"), group,
                                  "mEmptyFieldsBox");
}

/////////////////////////////////
// ConfigureCardViewDialog

ConfigureCardViewDialog::ConfigureCardViewDialog(const QString &viewName, 
                                                   KABC::AddressBook *doc,
                                                   QWidget *parent, 
                                                   const char *name)
  : ConfigureViewDialog(viewName, doc, parent, name)
{
  initGUI();
}
                                           
ConfigureCardViewDialog::~ConfigureCardViewDialog()
{
}
    
void ConfigureCardViewDialog::readConfig(KConfig *config)
{
  ConfigureViewDialog::readConfig(config);
  
  mPage->readConfig(config);
}

void ConfigureCardViewDialog::writeConfig(KConfig *config)
{
  ConfigureViewDialog::writeConfig(config);
  
  mPage->writeConfig(config);
}
    
void ConfigureCardViewDialog::initGUI()
{
  QWidget *page = addVBoxPage(i18n("Look & Feel"), QString::null,
                              KGlobal::iconLoader()->loadIcon("looknfeel",
                                                            KIcon::Desktop));
  
  mPage = new CardViewLookAndFeelPage(page, "mPage");
}

