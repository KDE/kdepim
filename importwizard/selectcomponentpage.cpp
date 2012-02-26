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
#include "selectcomponentpage.h"
#include "ui_selectcomponentpage.h"

SelectComponentPage::SelectComponentPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SelectComponentPage)
{
  ui->setupUi(this);
  connect( ui->everything, SIGNAL( clicked ( bool ) ), this, SLOT( slotEverythingClicked( bool ) ) );
}

SelectComponentPage::~SelectComponentPage()
{
  delete ui;
}

void SelectComponentPage::slotEverythingClicked( bool clicked )
{
  ui->addressbooks->setEnabled( !clicked && (mOptions & PimImportAbstract::AddressBook));
  ui->filters->setEnabled( !clicked && (mOptions & PimImportAbstract::Filters));
  ui->mails->setEnabled( !clicked && (mOptions & PimImportAbstract::Mails));
  ui->settings->setEnabled( !clicked && (mOptions & PimImportAbstract::Settings));
}


void SelectComponentPage::setEnabledComponent(PimImportAbstract::TypeSupportedOptions options)
{
  mOptions = options;
  slotEverythingClicked(ui->everything->isChecked());
}

PimImportAbstract::TypeSupportedOptions SelectComponentPage::selectedComponents() const
{
    if( ui->everything->isChecked() )
        return mOptions;
    else {
        PimImportAbstract::TypeSupportedOptions newOptions;
        if(ui->addressbooks->isChecked()) {
            newOptions|=PimImportAbstract::AddressBook;
        }
        if(ui->filters->isChecked()) {
            newOptions|=PimImportAbstract::Filters;
        }
        if(ui->mails->isChecked()) {
            newOptions|=PimImportAbstract::Mails;
        }
        if(ui->settings->isChecked()) {
            newOptions|=PimImportAbstract::Settings;
        }
        return newOptions;
    }
}

#include "selectcomponentpage.moc"
